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


#include <YGP/Parse.h>


struct Properties;


// Class to extract the properties of a JPEG image
class ParseJPEG  {
 public:
   ParseJPEG ();
   ~ParseJPEG () { }

   void parse (YGP::Xistream& stream, Properties& result) throw (std::string) {
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

   typedef YGP::OFParseText<ParseJPEG>     OMParseText;
   typedef YGP::OFParseExact<ParseJPEG>    OMParseExact;
   typedef YGP::OFParseAttomic<ParseJPEG>  OMParseAttomic;
   typedef YGP::OFParseSequence<ParseJPEG> OMParseSequence;

   YGP::ParseExact idJPEG;
   YGP::ParseExact idFormat1;
   YGP::ParseExact idComment1;
   YGP::ParseExact idComment2;
   YGP::ParseExact idComment3;
   OMParseAttomic  title;
   OMParseAttomic  title3;
   OMParseAttomic  type;
   OMParseAttomic  number;
   OMParseAttomic  length1;
   OMParseAttomic  length2;
   OMParseAttomic  offset;
   YGP::ParseSkip  skip;

   YGP::ParseSelection  selFormat;
   YGP::ParseSequence   seqFormat1;
   YGP::ParseSelection  selProperties;
   YGP::ParseSequence   seqPropShort;
   YGP::ParseSequence   seqPropLong;
   YGP::ParseSequence   seqPropXXL;

   OMParseSequence    seqEntries;
   YGP::ParseSequence jpegImage;                              // Startsequence

   YGP::ParseObject*   _jpegImage[3];
   YGP::ParseObject*   _selFormat[3];
   YGP::ParseObject*   _seqFormat1[4];
   YGP::ParseObject*   _selProperties[4];
   YGP::ParseObject*   _seqPropShort[4];
   YGP::ParseObject*   _seqPropLong[8];
   YGP::ParseObject*   _seqPropXXL[4];
   YGP::ParseObject*   _seqEntries[4];

   unsigned int offsets[3];
   unsigned int lengths[3];

   unsigned int cRead;
   unsigned int actEntry;
   unsigned int cEntries;
   Properties*  prop;
};

#endif
