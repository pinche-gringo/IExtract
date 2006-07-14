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


#include <fstream>

#include <YGP/Check.h>
#include <YGP/Trace.h>


#include "FileTypeChk.h"


std::map<const char*, FileTypeChecker::FileType, FileTypeCheckerByName::lessDereferenced> FileTypeCheckerByName::types;
std::map<std::string, FileTypeChecker::FileType> FileClassCheckerByContent::types;


//-----------------------------------------------------------------------------
/// Returns the type of the file (e.g. MS Word document) according to
/// passed file-name
/// \param file: File to inspect
/// \returns FileType: Type of file
//-----------------------------------------------------------------------------
FileTypeChecker::FileType FileTypeCheckerByName::getType (const char* file) {
   TRACE1 ("FileTypeCheckerByName::getType (const char*) - Checking " << file);
   Check3 (file);

   // Create table of file-types, if not already done
   if (types.empty ()) {
      types.insert (types.end (), std::pair<const char*, FileType> ("abw", ABIWORD));
      types.insert (types.end (), std::pair<const char*, FileType> ("doc", MSWORD));
      types.insert (types.end (), std::pair<const char*, FileType> ("gif", GIF));
      types.insert (types.end (), std::pair<const char*, FileType> ("htm", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("html", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("jpeg", JPEG));
      types.insert (types.end (), std::pair<const char*, FileType> ("jpg", JPEG));
      types.insert (types.end (), std::pair<const char*, FileType> ("mp3", MP3));
      types.insert (types.end (), std::pair<const char*, FileType> ("odg", OODRAW));
      types.insert (types.end (), std::pair<const char*, FileType> ("odp", OOIMPRESS));
      types.insert (types.end (), std::pair<const char*, FileType> ("ods", OOCALC));
      types.insert (types.end (), std::pair<const char*, FileType> ("odt", OOWRITE));
      types.insert (types.end (), std::pair<const char*, FileType> ("ogg", OGG));
      types.insert (types.end (), std::pair<const char*, FileType> ("pdf", PDF));
      types.insert (types.end (), std::pair<const char*, FileType> ("php", PHP));
      types.insert (types.end (), std::pair<const char*, FileType> ("png", PNG));
      types.insert (types.end (), std::pair<const char*, FileType> ("ppt", MSPOWERPT));
      types.insert (types.end (), std::pair<const char*, FileType> ("rtf", RTF));
      types.insert (types.end (), std::pair<const char*, FileType> ("sda", SODRAW));
      types.insert (types.end (), std::pair<const char*, FileType> ("sdc", SOCALC));
      types.insert (types.end (), std::pair<const char*, FileType> ("sdd", SOIMPRESS));
      types.insert (types.end (), std::pair<const char*, FileType> ("sdw", SOWRITE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sht", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("shtm", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("shtml", HTML));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxc", OOCALC));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxd", OODRAW));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxi", OOIMPRESS));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxm", OOMATH));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxw", OOWRITE));
      types.insert (types.end (), std::pair<const char*, FileType> ("xls", MSEXCEL));

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
FileTypeChecker::FileType FileClassCheckerByName::getType (const char* file) {
   TRACE1 ("FileClassCheckerByName::getType (const char*) - Checking " << file);
   Check3 (file);

   FileType rc (FileTypeCheckerByName::getType (file));
   switch (rc) {
   case MSWORD:
   case MSEXCEL:
   case MSPOWERPT:
      return MSOFFICE;

   case OOWRITE:
   case OOCALC:
   case OODRAW:
   case OOMATH:
   case OOIMPRESS:
      return OOOFFICE;

   case SOWRITE:
   case SOCALC:
   case SODRAW:
   case SOIMPRESS:
      return SOOFFICE;

   case PHP:
      return HTML;

   default:
      return rc;
   } // end-switch
}


//-----------------------------------------------------------------------------
/// Returns the class of the file (e.g. MS Office document) according
/// to passed file-name
/// \param file: File to inspect
/// \returns FileType: Class of file
//-----------------------------------------------------------------------------
FileTypeChecker::FileType FileClassCheckerByContent::getType (const char* file) {
   TRACE1 ("FileClassCheckerByContent::getType (const char*) - Checking " << file);
   Check3 (file);

   // Create table of file-types, if not already done
   if (types.empty ()) {
      types["<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE abiword PUBLIC \"-//ABISOURCE//DTD AWML"] = ABIWORD;
      types["GIF87a"] = GIF;
      types["GIF89a"] = GIF;
      types["\xFF\xD8"] = JPEG;
      types["\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1"] = MSOFFICE;
      types["OggS"] = OGG;
      types["\x89PNG\x0D\x0A\x1A\x0A"] = PNG;

      TRACE9 ("FileClassCheckerByContent::getType (const char*) - Knowing types " << types.size ());
   }

   std::ifstream stream (file);
   if (stream) {
      char buffer[512];
      stream.read (buffer, sizeof (buffer));

      for (std::map<std::string, FileType>::const_iterator i (types.begin ());
	   i != types.end (); ++i)
	 if (i->first == std::string (buffer, i->first.size ()))
	    return i->second;
   }
   return UNKNOWN;
}
