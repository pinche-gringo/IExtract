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
   int foundTitle3 (const char*, unsigned int);
   int foundLength (const char*, unsigned int);
   int foundLength2 (const char*, unsigned int);
   int foundOffset (const char*, unsigned int);
   int foundPropertiesHeader (const char*, unsigned int);

   typedef OFParseText<ParseJPEG>     OMParseText;
   typedef OFParseExact<ParseJPEG>    OMParseExact;
   typedef OFParseAttomic<ParseJPEG>  OMParseAttomic;
   typedef OFParseSequence<ParseJPEG> OMParseSequence;

   ParseExact      idJPEG;
   ParseExact      idFormat1;
   ParseExact      idComment1;
   ParseExact      idComment2;
   ParseExact      idComment3;
   OMParseAttomic  title;
   OMParseAttomic  title3;
   OMParseAttomic  type;
   OMParseAttomic  number;
   OMParseAttomic  length1;
   OMParseAttomic  length2;
   OMParseAttomic  offset;
   ParseIgnore     skip;
   ParseTextIgnore ignore;

   ParseSelection  selFormat;
   ParseSequence   seqFormat1;
   ParseSelection  selProperties;
   ParseSequence   seqPropShort;
   ParseSequence   seqPropLong;
   ParseSequence   seqPropXXL;

   OMParseSequence seqEntries;
   ParseSequence   jpegImage;                                 // Startsequence

   ParseObject*   _jpegImage[3];
   ParseObject*   _selFormat[3];
   ParseObject*   _seqFormat1[4];
   ParseObject*   _selProperties[4];
   ParseObject*   _seqPropShort[4];
   ParseObject*   _seqPropLong[8];
   ParseObject*   _seqPropXXL[4];
   ParseObject*   _seqEntries[4];

   unsigned int offsets[2];
   unsigned int lengths[2];

   unsigned int cRead;
   unsigned int actEntry;
   unsigned int cEntries;
   Properties*  prop;
};

#endif
