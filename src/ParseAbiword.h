#ifndef PARSEABIWORD_H
#define PARSEABIWORD_H

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


class ParseAbiword {
 public:
   ParseAbiword ();
   ~ParseAbiword ();

   void parse (YGP::Xistream& stream, Properties& result) {
      prop = &result;
      selDocument.parse (stream);
   }

 private:
   // Prohibted manager functions
   ParseAbiword (const ParseAbiword& other);
   const ParseAbiword& operator= (const ParseAbiword& other);

   // Callback-methods for type of parsed elementes
   int foundTag (const char*, unsigned int);
   int foundValue (const char*, unsigned int);

   typedef YGP::OFParseText<ParseAbiword>    OMParseText;
   typedef YGP::OFParseQuoted<ParseAbiword>  OMParseQuoted;

   Properties*  prop;

   YGP::ParseExact   idAbiword;
   YGP::ParseExact   idMetadata;
   YGP::ParseExact   skipIDStart;
   YGP::ParseText    skipUnused;
   YGP::ParseText    skipLine;
   OMParseQuoted     tag;
   OMParseText       value;

   YGP::ParseSequence    seqAbiWord;
   YGP::ParseSequence    seqMetadata;
   YGP::ParseSequence    seqEntry;

   YGP::ParseSelection   selDocument;                         // Startsequence

   YGP::ParseObject* _seqAbiWord[3];
   YGP::ParseObject* _selDocument[4];
   YGP::ParseObject* _seqMetadata[4];
   YGP::ParseObject* _seqEntry[3];

   std::string Properties::* pEntry;
};

#endif
