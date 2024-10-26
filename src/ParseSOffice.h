#ifndef PARSSOFFICE_H
#define PARSESOFFICE_H

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


// Class to extract the properties of a StarOffice document
class ParseStarOffice  {
 public:
   ParseStarOffice ();
   ~ParseStarOffice () { }

   void parse (YGP::Xistream& stream, Properties& result) {
      prop = &result;
      seqDocument.parse (stream); }

 private:
   ParseStarOffice (const ParseStarOffice&);
   ParseStarOffice& operator= (const ParseStarOffice&);

   // Callback-methods for type of parsed elementes
   int foundLength (const char*, unsigned int);
   int foundValue (const char*, unsigned int);
   int foundProps (const char*, unsigned int);

   typedef YGP::OFParseAttomic<ParseStarOffice>  OMParseAttomic;
   typedef YGP::OFParseSequence<ParseStarOffice> OMParseSequence;

   YGP::ParseExact idOffice;
   YGP::ParseExact idDocInfo;
   YGP::ParseSkip  skip;
   YGP::ParseSkip  skip2;
   OMParseAttomic  length;
   OMParseAttomic  value;

   YGP::ParseSequence seqDocument;                            // Startsequence
   YGP::ParseSequence seqEntries;

   YGP::ParseObject* _seqDocument[5];
   YGP::ParseObject* _seqEntries[4];

   Properties*  prop;
   enum types { NONE = -1, CREATOR = 0, AUTHOR, TITLE, COMMENT } actEntry;
};

#endif
