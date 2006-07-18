//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseMP3
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 15.01.2003
//COPYRIGHT   : Copyright (C) 2003 - 2006

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


#include <YGP/Trace.h>
#include <YGP/Exception.h>

#include "Utility.h"
#include "Properties.h"

#include "ParseMP3.h"


static const unsigned int ID_MP3 (0xE0FF);


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
ParseMP3::ParseMP3 () {
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ParseMP3::~ParseMP3 () {
}


//-----------------------------------------------------------------------------
/// Method to actually parse the MP3-file
/// \param stream: MP3-file to analyze
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void ParseMP3::parse (YGP::Xistream& stream, Properties& result) throw (YGP::ParseError) {
   char buffer[10];
   stream.read (buffer, sizeof (buffer));
   if (memcmp (buffer, "ID3", 3)) {
      if ((get2BytesLSB (buffer) & ID_MP3) != ID_MP3)
	 throw YGP::ParseError (_("MP3-ID not found"));

      stream.seekg (-0x80, std::ios::end);

      std::string value;
      std::getline (stream, value, '\xff');
      TRACE9 ("ParseMP3::parse (Xistream&, Properties&) - Found: " << value
	      << "; Length: " << value.size ());
      if ((value[0] == 'T') && (value[1] == 'A') && (value[2] == 'G')) {
	 result.strComment = strip (value, 63, 29);
	 result.strTitle = strip (value, 3, 29);
	 result.strAuthor = strip (value, 33, 29);
      }
   }
   else {
      unsigned int lenID3 (0);
      for (unsigned int i (6); i < 10; ++i) {
	 lenID3 <<= 7;
	 lenID3 += buffer[i];
      }
      TRACE5 ("ParseMP3::parse (Xistream&, Properties&) - Len of ID3: " << lenID3);

      char* id3 (new char [lenID3]);
      const char* pos (id3);
      stream.read (id3, lenID3);

      while (static_cast<unsigned int> (pos - id3) < lenID3) {
	 unsigned int len ((pos[4] << 21) + (pos[5] << 14) + (pos[6] << 7) + pos[7]);

	 TRACE7 ("ParseMP3::parse (Xistream&, Properties&) - Frame: " << std::string (pos, 4));
	 TRACE3 ("ParseMP3::parse (Xistream&, Properties&) - Len of frame: " << len);

	 switch (get4BytesLSB (pos)) {
	 case 0x32544954:                                           // TIT2-tag
	    result.strTitle = std::string (pos + 11, len - 1);
	    break;

	 case 0x31455054:                                           // TPE1-tag
	    result.strAuthor = std::string (pos + 11, len - 1);
	    break;

	 case 0x424C4154:                                           // TPE1-tag
	    result.strComment = std::string (pos + 11, len - 1);
	    break;

	 case 0:
	    return;
	 } // end-switch

	 pos += len + 10;
      } // end-while
      delete id3;
   }
}

//-----------------------------------------------------------------------------
/// Returns the specified substring removed from trailing spaces
/// \param value: String to manipulate
/// \param pos: Starting pos inside the string
/// \param len: Maximal length of string
//-----------------------------------------------------------------------------
std::string ParseMP3::strip (std::string& value, unsigned int pos, unsigned int len) {
   len += pos;
   while (len > pos) {
      TRACE9 ("ParseMP3::strip (std::string&, unsigned int, unsigned int) - "
              << value[len]);
      if ((value[len] != ' ') && (value[len]))
         break;
      --len;
   }
   return (pos == len) ? " " : value.substr (pos, len - pos + 1);
}
