#ifndef PARSEPDF_H
#define PARSEPDF_H

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


#ifdef _MSC_VER
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif

#include <map>
#include <string>

#include <YGP/Parse.h>

struct Properties;


// Class to extract the properties of a PDF document
class ParsePDF {
 public:
   ParsePDF ();
   virtual ~ParsePDF ();

   static void parse (YGP::Xistream& stream, Properties& result) throw (std::string);

 private:
   ParsePDF (const ParsePDF& other);
   const ParsePDF& operator= (const ParsePDF& other);

   // Callback-methods for type of parsed elementes
   int foundValue (const char*, unsigned int);
   int foundTitle (const char*, unsigned int);
   int foundAuthor (const char*, unsigned int);
   int foundComment (const char*, unsigned int);
   int foundOffset (const char*, unsigned int);
   int foundPrevOffset (const char*, unsigned int);
   int foundNumber (const char*, unsigned int);
   int foundStartNumber (const char*, unsigned int);
   int foundObjOffset (const char*, unsigned int);
   int foundObjectID (const char*, unsigned int);
   int foundEndObj (const char*, unsigned int);
   int foundParenthesis (const char*, unsigned int);
   int foundBracket (const char*, unsigned int);

   void parseInfoObject ();

   typedef YGP::OFParseExact<ParsePDF>   OMParseExact;
   typedef YGP::OFParseTextEsc<ParsePDF> OMParseTextEsc;
   typedef YGP::OFParseAttomic<ParsePDF> OMParseAttomic;

   YGP::ParseExact startXRef;
   OMParseAttomic  offXRef;
   YGP::ParseExact skipS;
   YGP::ParseText  skip;

   YGP::ParseExact idXRef;
   OMParseAttomic  nrStart;
   OMParseAttomic  count;
   OMParseAttomic  offObject;

   YGP::ParseExact   tagTrailer;
   YGP::ParseExact   startObj;
   YGP::ParseExact   objInfo;
   YGP::ParseExact   objPrev;
   OMParseAttomic    objOffPrev;
   OMParseAttomic    idObject;
   YGP::ParseExact   idObj;
   YGP::ParseAttomic number;
   YGP::ParseExact   tagObj;
   OMParseExact      endObj;

   OMParseExact    tagTitle;
   OMParseExact    tagAuthor;
   OMParseExact    tagComment;
   OMParseTextEsc  value;

   OMParseExact    startOfValue1;
   OMParseExact    startOfValue2;
   YGP::ParseExact endOfValue;

   YGP::ParseSelection selXRef;
   YGP::ParseSequence  seqXRef;
   YGP::ParseSequence  seqXRefTable;
   YGP::ParseSequence  seqXRefTableEntries;
   YGP::ParseSequence  seqTrailer;
   YGP::ParseSelection selValues;
   YGP::ParseSequence  seqInfo;
   YGP::ParseSequence  seqPrev;
   YGP::ParseSequence  seqInfoObj;
   YGP::ParseSequence  seqInfoValue;
   YGP::ParseSelection selType;
   YGP::ParseSelection selStartOfValue;

   YGP::ParseObject* _selXRef[4];
   YGP::ParseObject* _seqXRef[4];
   YGP::ParseObject* _seqXRefTable[6];
   YGP::ParseObject* _seqXRefTableEntries[3];
   YGP::ParseObject* _seqTrailer[4];
   YGP::ParseObject* _selValues[5];
   YGP::ParseObject* _seqInfo[4];
   YGP::ParseObject* _seqPrev[3];
   YGP::ParseObject* _seqInfoObj[6];
   YGP::ParseObject* _seqInfoValue[5];
   YGP::ParseObject* _selType[6];
   YGP::ParseObject* _selStartOfValue[3];

   Properties* prop;

   enum { NONE = -1, TITLE = 0, AUTHOR, COMMENT } actEntry;
   YGP::Xistream* file;
   unsigned int offPrev;
   unsigned int actObject;
   unsigned int infoObject;
   std::map <unsigned int, unsigned int> aOffsets;
};

#endif
