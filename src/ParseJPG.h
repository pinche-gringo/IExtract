#ifndef PARSEJPG_H
#define PARSEJPG_H

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


#include <YGP/Parse.h>


struct Properties;


// Class to extract the properties of a JPEG image
class ParseJPEG  {
 public:
   ParseJPEG ();
   ~ParseJPEG () { }

   void parse (YGP::Xistream& stream, Properties& result) {
      prop = &result;
      jpegImage.parse (stream); }

 private:
   ParseJPEG (const ParseJPEG&);
   ParseJPEG& operator= (const ParseJPEG&);

   // Callback-methods for type of parsed elementes
   int foundType (const char*, unsigned int);
   int foundNumber (const char*, unsigned int);
   int foundTitle (const char*, unsigned int);
   int foundTitlePhotoshop (const char*, unsigned int);
   int foundCommentPhotoShop (const char*, unsigned int);
   int foundLengthLSB (const char*, unsigned int);
   int foundLengthMSB (const char*, unsigned int);
   int foundLengthLSB4 (const char*, unsigned int);
   int foundLengthMSB4 (const char*, unsigned int);
   int foundOffset (const char*, unsigned int);
   int foundByteOrder (const char*, unsigned int);
   int foundEndOfJPEG (const char*, unsigned int);
   int foundAPP1 (const char*, unsigned int);
   int foundAPP1Exif (const char*, unsigned int);
   int foundImage (const char*, unsigned int);

   unsigned long foundLength (unsigned int length);
   bool supportedLength4 (unsigned int length);

   typedef YGP::OFParseText<ParseJPEG>     OMParseText;
   typedef YGP::OFParseExact<ParseJPEG>    OMParseExact;
   typedef YGP::OFParseAttomic<ParseJPEG>  OMParseAttomic;
   typedef YGP::OFParseSequence<ParseJPEG> OMParseSequence;

   YGP::ParseExact idJPEG;
   OMParseExact    idEndJPEG;
   OMParseExact    idAPP1;
   YGP::ParseExact idAPPD;
   YGP::ParseExact idComment;

   YGP::ParseAttomic idMarker;
   OMParseAttomic    image;

   OMParseAttomic  title;
   OMParseAttomic  titlePhotoshop;
   OMParseAttomic  type;
   OMParseAttomic  number;
   OMParseAttomic  lengthLSB;
   OMParseAttomic  lengthMSB;
   OMParseAttomic  lengthLSB4;
   OMParseAttomic  lengthMSB4;
   OMParseAttomic  offset;
   OMParseAttomic  byteOrder;
   YGP::ParseSkip  skip6;
   YGP::ParseSkip  skipLen;

   YGP::ParseSelection  selMarker;
   YGP::ParseSequence   seqComment;
   OMParseSequence      seqEXIF;
   YGP::ParseSequence   seqAPPD;
   YGP::ParseSequence   seqOther;

   YGP::ParseSequence seqIFD;
   YGP::ParseSequence jpegImage;                              // Startsequence

   YGP::ParseObject*   _jpegImage[3];
   YGP::ParseObject*   _selMarker[7];
   YGP::ParseObject*   _seqComment[4];
   YGP::ParseObject*   _seqEXIF[10];
   YGP::ParseObject*   _seqAPPD[4];
   YGP::ParseObject*   _seqOther[4];

   YGP::ParseObject*   _seqIFD[4];

   unsigned int offsets[3];
   unsigned int lengths[3];

   unsigned int cRead;
   unsigned int actEntry;
   unsigned int cEntries;
   Properties*  prop;
};

#endif
