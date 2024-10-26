#ifndef PARSERTF_H
#define PARSERTF_H

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

#include <YGP/Parse.h>

struct Properties;


// Class to extract the title of a RTF-document
class ParseRTF  {
 public:
   ParseRTF ();
   ~ParseRTF () { }

   void parse (YGP::Xistream& stream, Properties& result) {
      prop = &result;
      block.skipWS (stream);
      docRTF.parse (stream); }

 private:
   ParseRTF (const ParseRTF&);
   ParseRTF& operator= (const ParseRTF&);

   // Callback-methods for type of parsed elementes
   int foundTitle (const char*, unsigned int);
   int foundAuthor (const char*, unsigned int);
   int foundComment (const char*, unsigned int);
   int foundValue (const char*, unsigned int);
   int finish (const char*, unsigned int);

   typedef YGP::OFParseTextEsc<ParseRTF> OMParseTextEsc;
   typedef YGP::OFParseExact<ParseRTF> OMParseExact;

   YGP::ParseExact   idRTFDoc;
   YGP::ParseExact   startBlock;
   YGP::ParseExact   endBlock;
   YGP::ParseExact   info;
   OMParseExact      endInfoBlock;
   YGP::ParseExact   startCmd;
   YGP::ParseText    otherCmd;
   YGP::ParseAttomic noSpecialChar;

   // Supported content
   OMParseExact   author;
   OMParseExact   title;
   OMParseExact   description;
   OMParseTextEsc value;

   YGP::ParseSequence  seqInfo;
   YGP::ParseSequence  seqInfoValue;
   YGP::ParseSequence  seqOtherCmd;
   YGP::ParseSequence  seqValue;
   YGP::ParseSelection selEntry;
   YGP::ParseSelection selCmd;
   YGP::ParseSequence  block;
   YGP::ParseSequence  docRTF;

   YGP::ParseObject* _seqInfo[4];
   YGP::ParseObject* _seqInfoValue[5];
   YGP::ParseObject* _seqOtherCmd[4];
   YGP::ParseObject* _seqValue[3];
   YGP::ParseObject* _selEntry[5];
   YGP::ParseObject* _selCmd[4];
   YGP::ParseObject* _block[5];
   YGP::ParseObject* _docRTF[3];

   Properties*  prop;
   enum { NONE = -1, TITLE = 0, AUTHOR, COMMENT } actEntry;
};

#endif
