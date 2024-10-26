//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseMP3
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 15.01.2003
//COPYRIGHT   : Copyright (C) 2003 - 2008, 2024

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


#include <cstring>

#include <IExtract-cfg.h>

#include <YGP/Trace.h>
#include <YGP/Exception.h>
#include <YGP/Utility.h>

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
//-----------------------------------------------------------------------------
void ParseMP3::parse (YGP::Xistream& stream, Properties& result) {
   result.strTitle.clear ();
   result.strAuthor.clear ();
   result.strComment.clear ();

   char buffer[10] = "\0";
   stream.read (buffer, sizeof (buffer));
   if (memcmp (buffer, "ID3", 3)) {
      if ((YGP::get2BytesLSB (buffer) & ID_MP3) != ID_MP3)
	 throw YGP::ParseError (_("MP3-ID not found"));
   }
   else {
      unsigned int lenID3 (getLength (buffer + 6));
      TRACE5 ("ParseMP3::parse (Xistream&, Properties&) - Len of ID3: " << lenID3);
      char* id3 (new char [lenID3]);
      const char* pos (id3);

      // Check for ID3v2.3 or above
      if (YGP::get2BytesLSB (buffer + 3) > 0x02) {
	 // Check if an extended header is present
	 if ((buffer[5] & 0x40) == 0x40) {
	    stream.read (buffer, 4);
	    unsigned int cExtHdr (getLength (buffer) - 4);
	    TRACE9 ("ParseMP3::parse (Xistream&, Properties&) - Skipping ext. header: " << cExtHdr);

	    // Only handle extended header, if its size is plausible, else ignore
	    // to be able to handle badly written ID3 tags
	    if ((cExtHdr >= 6) && (cExtHdr < lenID3)) {
	       lenID3 -= cExtHdr;
	       pos += cExtHdr;
	    }
	    else
	       stream.seekg (-4, std::ios::cur);
	 }

	 stream.read (id3, lenID3);

	 while (static_cast<unsigned int> (pos - id3) < lenID3) {
	    unsigned int len (YGP::get4BytesMSB (pos + 4));
	    // Sometimes the length seems to be 7bit encoded, so correct, if so
	    if (len > (lenID3 - (pos - id3)))
	       break;

	    switch (YGP::get4BytesLSB (pos)) {
	    case 0x32544954:                                        // TIT2-tag
	       result.strTitle = getString (pos + 10, len);
	       break;

	    case 0x31455054:                                        // TPE1-tag
	       result.strAuthor = getString (pos + 10, len);
	       break;

	    case 0x424C4154:                                  // TALB-tag
	       result.strComment = getString (pos + 10, len);
	       break;
	    } // end-switch

	    pos += len + 10;
	    TRACE7 ("ParseMP3::parse (Xistream&, Properties&) - Used: " << (pos - id3) << "; Left: " << (lenID3 - (pos - id3)));
	 } // end-while
      } // endif ID3v2.3 or above
      else {
	 stream.read (id3, lenID3);

	 while (static_cast<unsigned int> (pos - id3) < lenID3) {
	    unsigned int len ((pos[3] << 14) + (pos[4] << 7) + pos[5]);
	    TRACE7 ("ParseMP3::parse (Xistream&, Properties&) - Frame: " << std::string (pos, 3));
	    TRACE3 ("ParseMP3::parse (Xistream&, Properties&) - Len of frame: " << std::hex << len << std::dec << " (" << len << ')');
	    if (len > (lenID3 - (pos - id3)))
	       break;

	    if (memcmp (pos, "TP1", 3))
	       if (memcmp (pos, "TAL", 3)) {
		  if (!memcmp (pos, "TT2", 3))
		     result.strTitle = getString (pos + 6, len);
	       }
	       else
		  result.strComment = getString (pos + 6, len);
	    else
	       result.strAuthor = getString (pos + 6, len);

	    pos += len + 6;
	 } // end-while
      }
      delete id3;
      if (result.strTitle.size () || result.strAuthor.size () || result.strComment.size ())
	 return;
   }
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

//-----------------------------------------------------------------------------
/// Returns the value of a "32 bit synchsafe integer", an integer having the
/// 7th bit of every byte cleared and ignored
/// \param value: Pointer to 4 bytes
/// \returns unsigned int: Integer value
//-----------------------------------------------------------------------------
unsigned int ParseMP3::getLength (const char* value) {
   Check1 (value);
   TRACE5 ("ParseMP3::getLength (const char*) - " << std::hex << YGP::get4BytesLSB (value) << std::dec);

   unsigned int rc ((unsigned char)*value);
   for (unsigned int i (0); i < 3; ++i) {
      rc <<= 7;
      rc += (unsigned char)*++value;
   }
   return rc;
}

//-----------------------------------------------------------------------------
/// Gets a string; according the codification in ID3 tags, where the first byte
/// of the buffer identifies, how the value is encoded
/// \param value: Start of string
/// \param length: Length of string
/// \returns std::string: The extracted string
//-----------------------------------------------------------------------------
std::string ParseMP3::getString (const char* value, unsigned int length) {
   Check1 (value);
   TRACE7 ("ParseMP3::getString (const char*, unsigned int) - " << std::hex << YGP::get4BytesMSB (value) << std::dec);

   switch (*value) {
   case '\0':
      if (YGP::get2BytesLSB (value + 1)) {
	 ++value;
	 --length;
      }
      else {
	 unsigned int newLen (YGP::get2BytesLSB (value + 3) - 1);
	 if (newLen < length) {
	    length = newLen;
	    value += 5;
	 }
      }
      break;

   case '\x03':
      ++value;
      --length;
      break;
   } // end-switch
   TRACE9 ("ParseMP3::getString (const char*, unsigned int): " << length << ": " << std::string (value, length));
   return std::string (value, length);
}
