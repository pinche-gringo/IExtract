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

#include <Parse.h>

struct Properties;


// Class to extract the title (of the properties) of a Word document
class ParseWord  {
 public:
   ParseWord ();
   ~ParseWord () { }

   void parse (Xistream& stream, Properties& result) throw (std::string) {
      prop = &result;
      wordDoc.parse (stream); }

 private:
   // Callback-methods for type of parsed elementes
   int foundNrEntries (const char*, unsigned int);
   int foundType (const char*, unsigned int);
   int foundOffset (const char*, unsigned int);
   int foundLength (const char*, unsigned int);
   int foundTitle (const char*, unsigned int);

   int foundPropertiesHeader (const char*, unsigned int);

   static int getTypeIndex (unsigned int type);

   typedef OFParseText<ParseWord>     OMParseText;
   typedef OFParseAttomic<ParseWord>  OMParseAttomic;
   typedef OFParseSequence<ParseWord> OMParseSequence;

   ParseExact      id;
   ParseSkip       skip;
   OMParseAttomic  nrEntries;
   OMParseAttomic  type;
   OMParseAttomic  offset;
   OMParseAttomic  length;
   OMParseText     title;
   ParseAttomic    skipIDStart;
   ParseText       ignore;

   ParseSequence   seqTitle;
   OMParseSequence seqEntries;
   ParseSequence   seqProperties;
   ParseSelection  wordDoc;                                    // Startsequence

   ParseObject* _wordDoc[5];
   ParseObject* _seqProperties[6];
   ParseObject* _seqTitle[4];
   ParseObject* _seqEntries[3];

   // Map for offsets to supported type. 1st: Offset, 2nd: Type
   map<unsigned int, unsigned int> aOffsets;

   unsigned int cRead;
   unsigned int cEntries;
   unsigned int actEntry;
   unsigned int len;

   Properties* prop;
};

#endif
