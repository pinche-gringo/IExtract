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


#include <YGP/Check.h>
#include <YGP/Trace.h>


#include "FileTypeChk.h"


std::map<const char*, FileTypeChecker::FileType, FileTypeCheckerByName::lessDereferenced> FileTypeCheckerByName::types;


//-----------------------------------------------------------------------------
/// Returns the type of the file according to passed file-name
/// \param file: File to inspect
/// \returns FileType: Type of file
//-----------------------------------------------------------------------------
FileTypeChecker::FileType FileTypeCheckerByName::getType (const char* file) {
   TRACE1 ("FileTypeCheckerByName::getType (const char*) - Checking " << file);
   Check3 (file);

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
      types.insert (types.end (), std::pair<const char*, FileType> ("sxc", STAROFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxd", STAROFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxi", STAROFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxm", STAROFFICE));
      types.insert (types.end (), std::pair<const char*, FileType> ("sxw", STAROFFICE));
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
/// Returns the type of the file according to the content of the passed file-name
/// \param file: File to inspect
/// \returns FileType: Type of file
//-----------------------------------------------------------------------------
FileTypeChecker::FileType FileTypeCheckerByContent::getType (const char* file) {
   TRACE1 ("FileTypeCheckerByName::getType (const char*) - Checking " << file);
   Check3 (file);

   // Create table of file-types, if not already done
   return UNKNOWN;
}
