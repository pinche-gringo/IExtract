//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : Plugins
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 26.01.2007
//COPYRIGHT   : Copyright (C) 2007

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
#include <string>

#include <istream>
#include <YGP/Exception.h>

#include <IExtract/Properties.h>


extern "C" {
   void processFile (std::istream& hFile, Properties& result);
   bool getFileType (char* buffer, const char* text,
		     unsigned int length, std::ifstream& stream);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a text-file. The first 3 lines are
/// considered to contain the properties.
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void processFile (std::istream& hFile, Properties& result) {
   static constexpr std::string::size_type MAX_LENGTH (79);

   static const std::pair<std::string Properties::*, const char*> lines[] =
      { { &Properties::strTitle, "Title too long!" },
        { &Properties::strComment, "Comment too long!" },
        { &Properties::strAuthor, "Author too long!" } };
   for (const auto& [value, error] : lines) {
      std::string line;
      if (!std::getline (hFile, line) || (line.size () > MAX_LENGTH))
         throw YGP::ParseError (error);
      result.*value = line;
   }
}

//-----------------------------------------------------------------------------
/// Checks if the first bytes in buffer are alphanumeric
/// \param buffer: Buffer to inspect
/// \param text: Text to match
/// \param len: Number of bytes to check (length of buffer, if *text == \0)
/// \param stream: Stream from where to read more characters
/// \returns bool: True, if the text matches
//-----------------------------------------------------------------------------
bool getFileType (char* buffer, const char*, unsigned int, std::ifstream&) {
   return std::isalnum (static_cast<unsigned char> (*buffer));
}
