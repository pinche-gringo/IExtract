#ifndef PARSEJPG_H
#define PARSEJPG_H

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


#include <Parse.h>


struct Properties;


// Class to extract the properties of a JPEG image
class ParseJPEG  {
 public:
   ParseJPEG ();
   ~ParseJPEG () { }

   void parse (Xistream& stream, Properties& result) throw (std::string) {
      prop = &result;
      jpegImage.parse (stream); }

 private:
   // Callback-methods for type of parsed elementes
   int foundType (const char*, unsigned int);
   int foundNumber (const char*, unsigned int);
   int foundTitle (const char*, unsigned int);
   int foundLength (const char*, unsigned int);
   int foundLength2 (const char*, unsigned int);
   int foundOffset (const char*, unsigned int);

   typedef OFParseText<ParseJPEG>    OMParseText;
   typedef OFParseExact<ParseJPEG>   OMParseExact;
   typedef OFParseAttomic<ParseJPEG> OMParseAttomic;

   ParseExact     idJPEG;
   ParseExact     idComment2;
   ParseExact     tagComment1;
   ParseExact     tagComment2;
   OMParseText    title;
   OMParseAttomic type;
   OMParseAttomic number;
   OMParseAttomic length1;
   OMParseAttomic length2;
   OMParseAttomic offset;
   ParseAttomic   ignore;

   ParseSelection selComment;
   ParseSequence  seqComment1;
   ParseSequence  seqComment2;
   ParseSequence  seqEntries;
   ParseSequence  jpegImage;                                  // Startsequence

   ParseObject*   _selComment[3];
   ParseObject*   _seqEntries[4];
   ParseObject*   _seqComment1[4];
   ParseObject*   _seqComment2[7];
   ParseObject*   _jpegImage[4];

   unsigned int offTitle;
   unsigned int offComment;

   unsigned int cRead;
   unsigned int actEntry;
   unsigned int cEntries;
   Properties*  prop;
};

#endif
