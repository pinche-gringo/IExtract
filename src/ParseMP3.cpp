//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseMP3
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 15.01.2003
//COPYRIGHT   : Anticopyright (A) 2003

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


#include <Trace_.h>

#include "Properties.h"

#include "ParseMP3.h"


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
/*--------------------------------------------------------------------------*/
ParseMP3::ParseMP3 () {
   
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ParseMP3::~ParseMP3 () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Method to actually parse the MP3-file
//Parameters: stream: MP3-file to analyze
//            result: Out: Found information
/*--------------------------------------------------------------------------*/
void ParseMP3::parse (Xistream& stream, Properties& result) throw (std::string) {
   stream.seekg (-0x80, ios::end);
   std::string value;

   getline (stream, value, '\xff');
   TRACE9 ("ParseMP3::parse (Xistream&, Properties&) - Found: " << value
           << "; Length: " << value.size ());
   if ((value[0] == 'T') && (value[1] == 'A') && (value[2] == 'G')) {
      result.strComment = strip (value, 63, 29);
      result.strTitle = strip (value, 3, 29);
      result.strAuthor = strip (value, 33, 29);
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the specified substring, removed from trailing spaces
//Parameters: value: String to manipulate
//            pos: Starting pos inside the string
//            len: Maximal length of string
/*--------------------------------------------------------------------------*/
std::string ParseMP3::strip (std::string& value, unsigned int pos, unsigned int len) {
   len += pos;
   while (len > pos) {
      TRACE9 ("ParseMP3::strip (std::string&, unsigned int, unsigned int) - "
              << value[len]);
      if (value[len] != ' ')
         break;
      --len;
   }
   return value.substr (pos, len - pos + 1);
}
