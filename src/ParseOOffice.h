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

#include <Parse.h>


struct Properties;


class ParseOpenOffice {
 public:
   ParseOpenOffice ();
   virtual ~ParseOpenOffice ();

   void parse (Xistream& stream, Properties& result) throw (std::string) {
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

   typedef OFParseText<ParseOpenOffice>    OMParseText;
   typedef OFParseQuoted<ParseOpenOffice>  OMParseQuoted;

   Properties*  prop;

   ParseExact        idMetadata;
   ParseExact        skipIDStart;
   ParseText         skipUnused;
   ParseText         skipLine;
   OMParseQuoted     tag;
   OMParseText       value;

   ParseSequence    seqMetadata;
   ParseSequence    seqEntry;

   ParseSelection   selDocument;                              // Startsequence

   ParseObject* _selDocument[4];
   ParseObject* _seqMetadata[4];
   ParseObject* _seqEntry[3];

   std::string Properties::* pEntry;
};

#endif
