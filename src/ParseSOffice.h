#ifndef PARSSOFFICE_H
#define PARSESOFFICE_H

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


// Class to extract the properties of a StarOffice document
class ParseStarOffice  {
 public:
   ParseStarOffice ();
   ~ParseStarOffice () { }

   void parse (Xistream& stream, Properties& result) throw (std::string) {
      prop = &result;
      stream.seekg (0x800);
      selDocument.parse (stream); }

 private:
   // Callback-methods for type of parsed elementes
   int foundLength (const char*, unsigned int);
   int foundValue (const char*, unsigned int);
   int foundProps (const char*, unsigned int);

   typedef OFParseAttomic<ParseStarOffice>  OMParseAttomic;
   typedef OFParseSequence<ParseStarOffice> OMParseSequence;

   ParseExact        idSOffice;
   ParseExact        skipIDStart;
   ParseSkip         skip;
   ParseText         skip2;
   OMParseAttomic    length;
   OMParseAttomic    value;

   ParseSelection   selDocument;                              // Startsequence
   OMParseSequence  seqEntries;
   ParseSequence    seqProperties;

   ParseObject* _selDocument[4];
   ParseObject* _seqEntries[4];
   ParseObject* _seqProperties[3];

   Properties*  prop;
   enum types { NONE = -1, CREATOR = 0, AUTHOR, TITLE, COMMENT } actEntry;
};

#endif
