#ifndef PARSEWORD_H
#define PARSEWORD_H

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
#pragma warning(disable:4355) // disable warning about this in initlist
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif


#include <map>

#include <YGP/Parse.h>

struct Properties;


// Class to extract the title (of the properties) of a Word document
class ParseWord  {
 public:
   ParseWord ();
   ~ParseWord () { }

   void parse (YGP::Xistream& stream, Properties& result) throw (std::string) {
      prop = &result;
      wordDoc.parse (stream); }

 private:
   // Callback-methods for type of parsed elementes
   int foundNrEntries (const char*, unsigned int);
   int foundType (const char*, unsigned int);
   int foundOffset (const char*, unsigned int);
   int foundLength (const char*, unsigned int);
   int foundTitle (const char*, unsigned int);
   int foundValueStart (const char*, unsigned int);
   int foundPropertiesHeader (const char*, unsigned int);

   static int getTypeIndex (unsigned int type);

   typedef YGP::OFParseText<ParseWord>     OMParseText;
   typedef YGP::OFParseExact<ParseWord>    OMParseExact;
   typedef YGP::OFParseAttomic<ParseWord>  OMParseAttomic;
   typedef YGP::OFParseSequence<ParseWord> OMParseSequence;

   YGP::ParseExact   id;
   OMParseExact      idValue1;
   OMParseExact      idValue2;
   YGP::ParseSkip    skip;
   OMParseAttomic    nrEntries;
   OMParseAttomic    type;
   OMParseAttomic    offset;
   OMParseAttomic    length;
   OMParseText       title;
   YGP::ParseAttomic skipIDStart;
   YGP::ParseText    ignore;

   YGP::ParseSelection selValueStart;
   YGP::ParseSequence  seqTitle;
   OMParseSequence     seqEntries;
   YGP::ParseSequence  seqProperties;
   YGP::ParseSelection wordDoc;                                // Startsequence

   YGP::ParseObject* _wordDoc[4];
   YGP::ParseObject* _seqProperties[7];
   YGP::ParseObject* _seqTitle[4];
   YGP::ParseObject* _seqEntries[3];
   YGP::ParseObject* _selValueStart[5];

   // Map for offsets to supported type. 1st: Offset, 2nd: Type
   std::map<unsigned int, unsigned int> aOffsets;

   unsigned int cRead;
   unsigned int cEntries;
   unsigned int actEntry;
   unsigned int len;

   Properties* prop;
};

#endif
