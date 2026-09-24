//$Id: ParseMSOffice.cpp,v 1.30 2008/06/11 18:41:25 markus Rel $

//PROJECT     : Extract
//SUBSYSTEM   : ParseMSOffice
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision: 1.30 $
//AUTHOR      : Markus Schwab
//CREATED     : 8.10.2002
//COPYRIGHT   : Copyright (C) 2002 - 2008, 2024

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

#include <array>
#include <fstream>
#include <cstdint>
#include <memory>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "ParseOOXML.h"
#include "Properties.h"

static constexpr unsigned int ID_PKZIP_LOCALHDR (0x04034b50);


//-----------------------------------------------------------------------------
/// Tries to parse an Office Open XML document
/// \param stream: Stream to read from
/// \param result: Structure to hold the found information
//-----------------------------------------------------------------------------
void ParseOOXML::parse (std::istream& stream, Properties& result) {
   std::uint32_t posFile;
   stream.read (reinterpret_cast<char*> (&posFile), sizeof (posFile));
   posFile = YGP::getFileOffsetInArchive (dynamic_cast<std::ifstream&> (stream),
                                          reinterpret_cast<char*> (&posFile), "docProps/core.xml", 17);
   if (!posFile)
      throw YGP::ParseError (_("Not an OOXML document!"));

   TRACE1 ("ParseOOXML::parse (std::istream&, Properties&) - Skipping to " << posFile);
   stream.seekg (posFile, std::ios::beg);

   std::array<char, 1024> output;
   stream.read (output.data (), 30);
   TRACE9 ("Hdr: " << YGP::get4BytesLSB (output.data ()) << "; Compr: " << YGP::get2BytesLSB (output.data () + 8)
           << "; Len: " << YGP::get2BytesLSB (output.data () + 26));
   if ((YGP::get4BytesLSB (output.data ()) != ID_PKZIP_LOCALHDR)
       || (YGP::get2BytesLSB (output.data () + 8) != 8)
       || (YGP::get2BytesLSB (output.data () + 26) != 17))
      throw YGP::ParseError (_("Archive contains an invalid file entry!"));

   // Skip over filename and extra field
   TRACE9 ("ParseOOXML::parse (std::istream&, Properties&) - Skipping "
           << YGP::get2BytesLSB (output.data () + 28));
   stream.seekg (YGP::get2BytesLSB (output.data () + 28) + 17, std::ios::cur);

   // Read the number of bytes stored in the header
   std::string props;
   unsigned int size (YGP::get4BytesLSB (output.data () + 18));
   TRACE9 ("ParseOOXML::parse (std::istream&, Properties&) - Size: " << size);

   z_stream zStream {};                              // Allocate inflate state
   int rc (inflateInit2 (&zStream, -MAX_WBITS));
   if (rc != Z_OK)
      throw YGP::ParseError (_("Can't initialise zlib!"));
   const std::unique_ptr<z_stream, decltype (&inflateEnd)> cleanup (&zStream, &inflateEnd);

   std::array<char, 1024> input;
   while (stream && size) {
      stream.read (input.data (), (size > input.size ()) ? input.size () : size);
      TRACE6 ("ParseOOXML::parse (std::istream&, Properties&) - Bytes read: " << stream.gcount ());

      zStream.avail_in = stream.gcount ();
      if (!zStream.avail_in)
         break;
      zStream.next_in = reinterpret_cast<Bytef*> (input.data ());
      size -= zStream.avail_in;

      // Run inflate() on input until output buffer is full
      do {
         zStream.avail_out = output.size ();
         zStream.next_out = reinterpret_cast<Bytef*> (output.data ());

         switch (rc = inflate (&zStream, Z_NO_FLUSH)) {
         case Z_NEED_DICT:
         case Z_DATA_ERROR:
            throw YGP::ParseError (_("Error inflating stream: Not a deflated file!"));

         case Z_MEM_ERROR:
            throw YGP::ParseError (_("Error inflating stream: Out of memory!"));
         }

         props.append (output.data (), output.size () - zStream.avail_out);
      } while (!zStream.avail_out);

      if (rc == Z_STREAM_END) {
         static const std::pair<std::string, std::string Properties::*> entries[] =
            { { "dc:title>", &Properties::strTitle }, { "dc:creator>", &Properties::strAuthor },
              { "dc:description>", &Properties::strComment } };
         for (const auto& [tag, value] : entries) {
            std::size_t start (props.find ('<' + tag));
            if (start != std::string::npos) {
               start += 1 + tag.length ();
               std::size_t end (props.find ("</" + tag, start));
               if (end != std::string::npos)
                  result.*value = props.substr (start, end - start);
            }
         }
         return;
      }
   }
   throw YGP::ParseError (_("Unexpected end of file!"));
}

/* File docProps/core.xml
   Entries:
     - dc:title
     - dc:creator/cp:lastModifiedBy
     - dc:description
*/

#endif
