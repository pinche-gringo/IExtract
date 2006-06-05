#ifndef PARSEMP3_H
#define PARSEMP3_H

//$Id$

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


#include <string>

#include <YGP/XStream.h>

struct Properties;


// Extracts the information out of MP3 files
//  - The title is returned in strTitle
//  - The artist is returned in strAuthor
//  - The album is returned in strComment
class ParseMP3 {
 public:
   static void parse (YGP::Xistream& stream, Properties& result);

 private:
   ParseMP3 ();
   ParseMP3 (const ParseMP3& other);
   ~ParseMP3 ();
   const ParseMP3& operator= (const ParseMP3& other);

   static std::string strip (std::string& value, unsigned int pos, unsigned int len);
};

#endif
