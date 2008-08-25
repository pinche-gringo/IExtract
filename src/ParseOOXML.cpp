//$Id: ParseMSOffice.cpp,v 1.30 2008/06/11 18:41:25 markus Rel $

//PROJECT     : Extract
//SUBSYSTEM   : ParseMSOffice
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.30 $
//AUTHOR      : Markus Schwab
//CREATED     : 8.10.2002
//COPYRIGHT   : Copyright (C) 2002 - 2008

// This file is part of IExtract.
//
// IExtract is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IExtract is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libYGP.  If not, see <http://www.gnu.org/licenses/>.


#include <IExtract-cfg.h>

#if defined (SUPPORT_OOXML) && defined (HAVE_LIBZ)

#include <zlib.h>

#include <cstring>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "ParseOOXML.h"
#include "Properties.h"

static const unsigned int ID_PKZIP_LOCALHDR       (0x04034b50);
static const unsigned int ID_PKZIP_END_CDR        (0x06054B50);
static const unsigned int ID_PKZIP_CENTRALFILEHDR (0x02014b50);


//-----------------------------------------------------------------------------
/// Checks if the passed stream is a ZIP-archive having the passed file inside
/// \param stream Stream from where to read more characters
/// \param buffer Buffer to inspect
/// \param file Name of file which must be inside the ZIP-file
/// \param length Lenght of file-name
/// \returns bool True, if the file is included
//-----------------------------------------------------------------------------
static UINT32 getFileInArchive (std::ifstream& stream, char* buffer,
				const char* file, unsigned int lenFile) {
   if (YGP::get4BytesLSB (buffer) == ID_PKZIP_LOCALHDR) {
      char buffer[80];
      memset (buffer, 0, sizeof (buffer));
      stream.seekg (-22, std::ios::end);
      stream.read (buffer, 22);

      if (YGP::get4BytesLSB (buffer) == ID_PKZIP_END_CDR) {
	 // Skip to central directory record
	 unsigned int cEntries (YGP::get4BytesLSB (buffer + 10));
	 stream.seekg (YGP::get4BytesLSB (buffer + 16), std::ios::beg);
	 TRACE6 ("getFileInArchive (2x const char*, unsigned int, std::ifstream&) - Start CDR: " << YGP::get4BytesLSB (buffer + 16) << " (" << cEntries << ')');

	 // Inspect all entries
	 while (cEntries--) {
	    stream.read (buffer, 46);
	    if (YGP::get4BytesLSB (buffer) == ID_PKZIP_CENTRALFILEHDR) {
	       unsigned int lenName (YGP::get2BytesLSB (buffer + 28));
	       unsigned int lenSkip (YGP::get2BytesLSB (buffer + 30) + YGP::get2BytesLSB (buffer + 32));
	       TRACE6 ("getFileInArchive (2x const char*, unsigned int, std::ifstream&) - Len of filename: " << lenName
		       << "; Skipping: " << lenSkip);

	       // Check if "meta.xml" entry has been found
	       if (lenName == lenFile) {
		  UINT32 posFile (YGP::get4BytesLSB (buffer + 42));
		  stream.read (buffer, lenFile);
		  if (!memcmp (file, buffer, lenFile))
		     return posFile;
		  lenName -= lenFile;
	       }
	       stream.seekg (lenName + lenSkip, std::ios::cur);
	    }
	    else
	       break;
	 } // end-while
      }
   }
   return NULL;
}

//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
ParseOOXML::ParseOOXML () {
}

//-----------------------------------------------------------------------------
/// Tries to parse an Office Open XML document
/// \param stream: Stream to read from
/// \param result: Structure to hold the found information
/// \throw YGP::ParseError: In case of an error an describing text
//-----------------------------------------------------------------------------
void ParseOOXML::parse (YGP::Xistream& stream, Properties& result) throw (YGP::ParseError) {
   UINT32 posFile;
   stream.read ((char*)&posFile, sizeof (posFile));
   posFile = getFileInArchive ((std::ifstream&)stream, (char*)&posFile, "docProps/core.xml", 17);
   if (posFile) {
      TRACE1 ("ParseOOXML::parse (YGP::Xistream&, Properties&) - Skipping to " << posFile);

      stream.seekg (posFile, std::ios::beg);
      unsigned char output[1024];
      stream.read ((char*)output, 30);
      TRACE9 ("Hdr: " << YGP::get4BytesLSB ((char*)output) << "; Compr: " << YGP::get2BytesLSB ((char*)output + 8)
	      << "; Len: " << YGP::get2BytesLSB ((char*)output + 26));
      if ((YGP::get4BytesLSB ((char*)output) != ID_PKZIP_LOCALHDR)
	  || (YGP::get2BytesLSB ((char*)output + 8) != 8)
	  || (YGP::get2BytesLSB ((char*)output + 26) != 17))
	 throw (YGP::ParseError (_("Archive contains an invalid file entry!")));

      // Skip over filename and extra field
      TRACE9 ("ParseOOXML::parse (YGP::Xistream&, Properties&) - Skipping "
	      << YGP::get2BytesLSB ((char*)output + 28));
      stream.seekg (YGP::get2BytesLSB ((char*)output + 28) + 17, std::ios::cur);

      // Read the number of bytes stored in the header
      std::string props;
      unsigned int size (YGP::get4BytesLSB ((char*)output + 18));
      TRACE9 ("ParseOOXML::parse (YGP::Xistream&, Properties&) - Size: " << size);

      unsigned char input[1024];
      z_stream zStream;

      zStream.zalloc = Z_NULL;                        // Allocate inflate state
      zStream.zfree = Z_NULL;
      zStream.opaque = Z_NULL;
      zStream.avail_in = 0;
      zStream.next_in = Z_NULL;
      int rc (inflateInit2 (&zStream, -MAX_WBITS));
      if (rc != Z_OK)
	 throw (YGP::ParseError (_("Can't initialise zlib!")));

      while (stream && size) {
	 stream.read ((char*)input, (size > sizeof (input)) ? sizeof (input) : size);
	 TRACE6 ("ParseOOXML::parse (YGP::Xistream&, Properties&) - Bytes read: " << stream.gcount ());

	 TRACE1 (std::hex);
	 for (unsigned int x (0); x < 8; ++x)
	    TRACE1 ((unsigned int)input[x] << ' ');
	 TRACE1 (std::hex);

	 zStream.avail_in = stream.gcount ();
	 if (!zStream.avail_in)
	    break;
	 zStream.next_in = input;
	 size -= zStream.avail_in;

	 // Run inflate() on input until output buffer is full
	 do {
	    zStream.avail_out = sizeof (output);
            zStream.next_out = output;

	    switch (rc = inflate (&zStream, Z_NO_FLUSH)) {
	    case Z_NEED_DICT:
	    case Z_DATA_ERROR:
	       inflateEnd (&zStream);
	       throw (YGP::ParseError (_("Error inflating stream: Not a deflated file!")));

            case Z_MEM_ERROR:
	       inflateEnd (&zStream);
	       throw (YGP::ParseError (_("Error inflating stream: Out of memory!")));
	    }

	    props.append ((char*)output, sizeof (output) - zStream.avail_out);
	 } while (!zStream.avail_out);

	 if (rc == Z_STREAM_END) {
	    inflateEnd (&zStream);
	    std::string values[] = { "dc:title>", "dc:creator>", "dc:description>" };
	    std::string Properties::* dest[] =
	       { &Properties::strTitle, &Properties::strAuthor, &Properties::strComment };
	    for (unsigned int i (0); i < (sizeof (values) / sizeof (values[0])); ++i) {
	       size_t start (props.find (std::string (1, '<') + values[i]));
	       if (start != std::string::npos) {
		  size_t end (props.find (std::string ("</") + values[i],
					  start + 1 + values[i].length ()));
		  if (end != std::string::npos) {
		     start += 1 + values[i].length ();
		     result.*(dest[i]) = props.substr (start, end - start);
		  }
	       }
	    }
	    return;
	 }
      }
      inflateEnd (&zStream);
      throw (YGP::ParseError (_("Unexpected end of file!")));
   }
   else
      throw (YGP::ParseError (_("Not an OOXML document!")));
}

/* File docProps/core.xml
   Entries:
     - dc:title
     - dc:creator/cp:lastModifiedBy
     - dc:description
*/

#endif
