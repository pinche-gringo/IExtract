//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : FileTypeChecker
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 13.07.2006
//COPYRIGHT   : Copyright (C) 2006

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <cctype>
#include <fstream>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Utility.h"

#include "FileTypeChk.h"


std::map<const char*, FileTypeChecker::FileType, FileTypeCheckerByName::lessDereferenced> FileTypeCheckerByName::types;
std::map<std::string, FileTypeChecker::FileType> FileTypeCheckerByContent::types;


static const char ID_PDF[]        = "%PDF";
static const char ID_RTF[]        = "{\\rtf";
static const char ID_ABIWORD[]    = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE abiword PUBLIC \"-//ABISOURCE//DTD AWML";
static const char ID_ID3[]        = "ID3";
static const char ID_GIF87[]      = "GIF87a";
static const char ID_GIF89[]      = "GIF89a";
static const char ID_OGG[]        = "OggS";
static const char ID_JPEG[]       = "\xFF\xD8\xFF\xE0\x00\x10JFIF";
static const char ID_PNG[]        = "\x89PNG\x0D\x0A\x1A\x0A";
static const char ID_HTML[]       = "<!DOCTYPE HTML";
static const char ID_DOCOFFICE[]  = "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1";
static const char ID_STAROFFICE[] = "SfxDocumentInfo";

static const unsigned int ID_MP3                  (0xE0FF);
static const unsigned int ID_PKZIP_LOCALHDR       (0x04034b50);
static const unsigned int ID_PKZIP_END_CDR        (0x06054B50);
static const unsigned int ID_PKZIP_CENTRALFILEHDR (0x02014b50);


//-----------------------------------------------------------------------------
/// Returns the type of the file (e.g. MS Word document) according to
/// passed file-name
/// \param file: File to inspect
/// \returns FileType: Type of file
//-----------------------------------------------------------------------------
FileTypeChecker::FileType FileTypeCheckerByName::getType (const char* file) {
   Check1 (file);
   TRACE1 ("FileTypeCheckerByName::getType (const char*) - Checking " << file);

   // Create table of file-types, if not already done
   if (types.empty ()) {
      types.insert (types.end (), std::pair<const char*, FileType> ("abw", ABIWORD));
      types.insert (types.end (), std::pair<const char*, FileType> ("doc", MSOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("gif", GIF));
      types.insert (types.end (), std::pair<const char*, FileType> ("htm", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("html", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("jpeg", JPEG));
      types.insert (types.end (), std::pair<const char*, FileType> ("jpg", JPEG));
      types.insert (types.end (), std::pair<const char*, FileType> ("mp3", MP3));
      types.insert (types.end (), std::pair<const char*, FileType> ("odg", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("odp", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("ods", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("odt", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("ogg", OGG));
      types.insert (types.end (), std::pair<const char*, FileType> ("pdf", PDF));
      types.insert (types.end (), std::pair<const char*, FileType> ("php", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("png", PNG));
      types.insert (types.end (), std::pair<const char*, FileType> ("ppt", MSOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("rtf", RTF));
      types.insert (types.end (), std::pair<const char*, FileType> ("sda", STAROFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sdc", STAROFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sdd", STAROFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sdw", STAROFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sht", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("shtm", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("shtml", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxc", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxd", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxi", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxm", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxw", OPENOFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("xls", MSOFFICE));

      TRACE9 ("FileTypeCheckerByName::getType (const char*) - Knowing types " << types.size ());
   }

   const char* extension (strrchr (file, '.'));
   if (extension) {
      TRACE3 ("FileTypeCheckerByName::getType (const char*) - Extension " << extension + 1);
      std::map<const char*, FileType>::const_iterator i (types.find (extension + 1));
      return i != types.end () ? i->second : UNKNOWN;
   }
   return UNKNOWN;
}


//-----------------------------------------------------------------------------
/// Returns the class of the file (e.g. MS Office document) according
/// to passed file-name
/// \param file: File to inspect
/// \returns FileType: Class of file
//-----------------------------------------------------------------------------
FileTypeChecker::FileType FileTypeCheckerByContent::getType (const char* file) {
   Check1 (file);
   TRACE1 ("FileTypeCheckerByContent::getType (const char*) - Checking " << file);

   // Create table of file-types, if not already done
   if (types.empty ()) {
      types[std::string (ID_PDF, sizeof (ID_PDF) - 1)] = PDF;
      types[std::string (ID_RTF, sizeof (ID_RTF) - 1)] = RTF;
      types[std::string (ID_ABIWORD, sizeof (ID_ABIWORD) - 1)] = ABIWORD;
      types[std::string (ID_ID3, sizeof (ID_ID3) - 1)] = MP3;
      types[std::string (ID_GIF87, sizeof (ID_GIF87) - 1)] = GIF;
      types[std::string (ID_GIF89, sizeof (ID_GIF89) - 1)] = GIF;
      types[std::string (ID_OGG, sizeof (ID_OGG) - 1)] = OGG;
      types[std::string (ID_JPEG, sizeof (ID_JPEG) - 1)] = JPEG;
      types[std::string (ID_PNG, sizeof (ID_PNG) - 1)] = PNG;
      TRACE9 ("FileTypeCheckerByContent::getType (const char*) - Known types " << types.size ());
   }

   std::ifstream stream (file);
   if (stream) {
      // Fill buffer with file
      char buffer[512];
      memset (buffer, 0, sizeof (buffer));
      stream.read (buffer, sizeof (buffer));

      // Check if first bytes identify the file
      for (std::map<std::string, FileType>::const_iterator i (types.begin ());
	   i != types.end (); ++i)
	 if (i->first == std::string (buffer, i->first.size ()))
	    return i->second;

      // Check for MP3
      if ((get2BytesLSB (buffer) & ID_MP3) == ID_MP3)
	 return MP3;

      // Check for OpenOffice
      if (get4BytesLSB (buffer) == ID_PKZIP_LOCALHDR) {
	 char buffer[80];
	 memset (buffer, 0, sizeof (buffer));
	 stream.seekg (-22, std::ios::end);
	 stream.read (buffer, 22);

	 if (get4BytesLSB (buffer) == ID_PKZIP_END_CDR) {
	    // Skip to central directory record
	    unsigned int cEntries (get4BytesLSB (buffer + 10));
	    stream.seekg (get4BytesLSB (buffer + 16), std::ios::beg);
	    TRACE7 ("FileTypeCheckerByContent::getType (const char*) - Start CDR: " << get4BytesLSB (buffer + 16) << " (" << cEntries << ')');

	    // Inspect all entries
	    while (cEntries--) {
	       stream.read (buffer, 46);
	       if (get4BytesLSB (buffer) == ID_PKZIP_CENTRALFILEHDR) {
		  unsigned int lenName (get2BytesLSB (buffer + 28));
		  unsigned int lenSkip (get2BytesLSB (buffer + 30) + get2BytesLSB (buffer + 32));
		  TRACE7 ("FileTypeCheckerByContent::getType (const char*) - Len of filename: " << lenName
			  << "; Skipping: " << lenSkip);

		  // Check if "meta.xml" entry has been found
		  if (lenName == 8) {
		     stream.read (buffer, 8);
		     if (!memcmp ("meta.xml", buffer, 8))
			return OPENOFFICE;
		     lenName -= 8;
		  }
		  stream.seekg (lenName + lenSkip, std::ios::cur);
	       }
	       else
		  break;
	    } // end-while
	 }
	 return UNKNOWN;
      }

      // Check for StarOffice/MS-Office (StarOffice up to V5 uses a
      // MS-compatible format additional for the information, additional to
      // their own format, so they could also be parsed as MS-Office document)
      if (!memcmp (buffer, ID_DOCOFFICE, sizeof (ID_DOCOFFICE) - 1)) {
	 char buffer[16];
	 stream.seekg (0x8c2, std::ios::beg);
	 stream.read (buffer, sizeof (buffer));
	 return (memcmp (buffer, ID_STAROFFICE, sizeof (ID_STAROFFICE) - 1)
		 ? MSOFFICE : STAROFFICE);
      }

      // Check for HTML-document; this must be the last test, as the buffer
      // might be updated
      skipHTMLComments (buffer, sizeof (buffer), stream);
      TRACE1 ("HTML: " << std::string (buffer, sizeof (ID_HTML) - 1));
      if (!memcmp (buffer, ID_HTML, sizeof (ID_HTML) - 1))
	 return HTML;
      // Don't add further tests; the buffer might have been overwritten
   }
   return UNKNOWN;
}

//-----------------------------------------------------------------------------
/// Skips over HTML-comments and white-spaces
/// \param buffer: Buffer to inspect (and maybe re-fill with new data from stream)
/// \param size: Size of buffer
/// \param stream: Stream to read from
/// \pre buffer must be at least 4 characters big
//-----------------------------------------------------------------------------
void FileTypeCheckerByContent::skipHTMLComments (char* buffer, unsigned int size,
						 std::ifstream& stream) {
   Check1 (buffer); Check1 (size > 3);

   const char* pos (buffer);
   unsigned int left (size);
   do {
      TRACE5 ("FileTypeCheckerByContent::skipHTMLComments (char*, unsigned int, std::ifstream&) - " << std::string (pos, 5));

      // Simply skip whitespaces
      if (isspace (*pos)) {
	 ++pos;
	 --left;
      }
      // Check for HTML comment (starting with <!--)
      else if ((*pos == '<') && (pos[1] == '!') && (pos[2] == '-') && (pos[3] == '-')) {
	 pos += 4;
	 bool cont (true);
	 do {
	    // Try to find the end of the comment (-->)
	    pos = static_cast<char*> (memchr (pos, '-', left));
	    if (pos) {
	       TRACE7 ("FileTypeCheckerByContent::skipHTMLComments (char*, unsigned int, std::ifstream&) - EOC: \"" << std::string (pos, (left > 4) ? 5 : left) << '"');
	       left = size - (pos - buffer);
	       if (left > 2) {
		  if ((pos[1] == '-') && (pos[2] == '>')) {
		     cont = false;
		     memcpy (buffer, pos + 3, left - 3);
		     pos = buffer;
		  }
		  else {
		     ++pos;
		     --left;
		     continue;
		  }
	       }
	       else {
		  memcpy (buffer, pos, left);
		  pos = buffer;
	       }
	    }
	    else {
	       left = 0;
	       pos = buffer;
	    }

	    TRACE9 ("FileTypeCheckerByContent::skipHTMLComments (char*, unsigned int, std::ifstream&) - Filling up: " << (size - left));
	    stream.read (buffer + left, size - left);
	    left += stream.gcount ();
	 } while (cont);
      } // endif HTML comment
      else {
	 memcpy (buffer, pos, left);
	 stream.read (buffer + left, size - left);
	 break;
      }
   } while (left);
}
