#ifndef PARSEMP3_H
#define PARSEMP3_H

//$Id$

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


#include <string>

#include <istream>

struct Properties;


// Extracts the information out of MP3 files
//  - The title is returned in strTitle
//  - The artist is returned in strAuthor
//  - The album is returned in strComment
class ParseMP3 {
 public:
   static void parse (std::istream& stream, Properties& result);

 private:
   ParseMP3 () = delete;
   ParseMP3 (const ParseMP3& other) = delete;
   const ParseMP3& operator= (const ParseMP3& other) = delete;

   static std::string strip (const std::string& value, unsigned int pos, unsigned int len);
   static unsigned int getLength (const char* value);
   static std::string getString (const char* value, unsigned int length);
};

#endif
