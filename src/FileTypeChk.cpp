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

   const char* extension (strchr (file, '.'));
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
      types["<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE abiword PUBLIC \"-//ABISOURCE//DTD AWML"] = ABIWORD;
      types["GIF87a"] = GIF;
      types["GIF89a"] = GIF;
      types["\xFF\xD8"] = JPEG;
      types["\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"] = MSOFFICE;
      types["ID3"] = MP3;
      types["OggS"] = OGG;
      types["\x89PNG\x0D\x0A\x1A\x0A"] = PNG;

      TRACE9 ("FileTypeCheckerByContent::getType (const char*) - Knowing types " << types.size ());
   }

   std::ifstream stream (file);
   if (stream) {
      char buffer[512];
      memset (buffer, '\0', sizeof (buffer));
      stream.read (buffer, sizeof (buffer));

      for (std::map<std::string, FileType>::const_iterator i (types.begin ());
	   i != types.end (); ++i)
	 if (i->first == std::string (buffer, i->first.size ()))
	    return i->second;

      if ((get2BytesLSB (buffer) & 0xE0FF) == 0xE0FF)
	 return MP3;

      // Check for HTML-document
      const char* start (buffer);
      const char* temp;
      while ((temp = skipWS (start, sizeof (buffer) - (start - buffer)))
	     || (temp = skipHTMLComment (start, sizeof (buffer) - (start - buffer))))
	 start = temp;

      if ((static_cast<unsigned int> (start - buffer) < (sizeof (buffer) - 13))
	  && !memcmp (start, "<!DOCTYPE HTML", 14))
	 return HTML;
   }
   return UNKNOWN;
}

//-----------------------------------------------------------------------------
/// Skips over white-spaces
/// \param buffer: Buffer to inspect
/// \param size: Size of buffer
/// \returns const char*: Pointer to end of skipped characters or NULL
//-----------------------------------------------------------------------------
const char* FileTypeCheckerByContent::skipWS (const char* buffer, unsigned int size) {
   Check1 (buffer);
   TRACE5 ("FileTypeCheckerByContent::skipWS (const char*, unsigned int) - " << std::string (buffer, 5));

   if (size && isspace (*buffer)) {
      while (isspace (*++buffer) && --size)
	 ;
      return buffer;
   }
   return NULL;
}

//-----------------------------------------------------------------------------
/// Skips over white-spaces
/// \param buffer: Buffer to inspect
/// \param size: Size of buffer
/// \returns const char*: Pointer to end of skipped characters or NULL
//-----------------------------------------------------------------------------
const char* FileTypeCheckerByContent::skipHTMLComment (const char* buffer, unsigned int size) {
   Check1 (buffer);
   TRACE5 ("FileTypeCheckerByContent::skipHTMLComment (const char*, unsigned int) - " << size << ": " << std::string (buffer, 5));

   if ((size > 3)
       && (*buffer == '<') && (buffer[1] == '!')
       && (buffer[2] == '-') && (buffer[3] == '-')) {
      const char* temp;
      while ((size > 2) && (temp = static_cast<char*> (memchr (buffer, '-', size)))) {
	 if ((temp[1] == '-') && (temp[2] == '>'))
	    return temp + 3;

	 size -= ++temp - buffer;
	 buffer = temp;
      } // end-while
   }
   return NULL;
}
