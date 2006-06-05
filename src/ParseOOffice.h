#ifndef PARSEOOFICE_H
#define PARSEOOFICE_H

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


#include <string>

#include <YGP/Parse.h>


struct Properties;


class ParseOpenOffice {
 public:
   ParseOpenOffice ();
   ~ParseOpenOffice ();

   void parse (YGP::Xistream& stream, Properties& result) throw (YGP::ParseError) {
      prop = &result;
      selDocument.parse (stream);
   }

 private:
   // Prohibted manager functions
   ParseOpenOffice (const ParseOpenOffice& other);
   const ParseOpenOffice& operator= (const ParseOpenOffice& other);

   // Callback-methods for type of parsed elementes
   int foundTag (const char*, unsigned int);
   int foundValue (const char*, unsigned int);

   typedef YGP::OFParseText<ParseOpenOffice>    OMParseText;
   typedef YGP::OFParseQuoted<ParseOpenOffice>  OMParseQuoted;

   Properties*  prop;

   YGP::ParseExact   idMetadata;
   YGP::ParseExact   skipIDStart;
   YGP::ParseText    skipUnused;
   YGP::ParseText    skipLine;
   OMParseQuoted     tag;
   OMParseText       value;

   YGP::ParseSequence    seqMetadata;
   YGP::ParseSequence    seqEntry;

   YGP::ParseSelection   selDocument;                         // Startsequence

   YGP::ParseObject* _selDocument[4];
   YGP::ParseObject* _seqMetadata[4];
   YGP::ParseObject* _seqEntry[3];

   std::string Properties::* pEntry;
};

#endif
