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


#include <string>
#include <vector>

#include <Parse.h>

struct Properties;


// Class to extract the properties of a PDF document
class ParsePDF {
 public:
   ParsePDF ();
   virtual ~ParsePDF ();

   static void parse (Xistream& stream, Properties& result) throw (std::string) {
      ParsePDF obj;
      stream.seekg (-40, ios::end);
      obj.prop = &result;
      obj.file = &stream;
      obj.selXRef.parse (stream); }

 private:
   ParsePDF (const ParsePDF& other);
   const ParsePDF& operator= (const ParsePDF& other);

   // Callback-methods for type of parsed elementes
   int foundValue (const char*, unsigned int);
   int foundTitle (const char*, unsigned int);
   int foundAuthor (const char*, unsigned int);
   int foundComment (const char*, unsigned int);
   int foundOffset (const char*, unsigned int);
   int foundNumber (const char*, unsigned int);
   int foundStartNumber (const char*, unsigned int);
   int foundObjOffset (const char*, unsigned int);

   typedef OFParseText<ParsePDF>    OMParseText;
   typedef OFParseExact<ParsePDF>   OMParseExact;
   typedef OFParseAttomic<ParsePDF> OMParseAttomic;

   ParseExact      startXRef;
   OMParseAttomic  offXRef;
   ParseExact      skipS;
   ParseTextIgnore skip;

   ParseExact      idXRef;
   OMParseAttomic  nrStart;
   OMParseAttomic  count;
   OMParseAttomic  offObject;

   OMParseExact tagTitle;
   OMParseExact tagAuthor;
   OMParseExact tagComment;
   OMParseText  value;

   ParseSelection selXRef;
   ParseSequence  seqXRef;
   ParseSequence  seqXRefTable;
   ParseSequence  seqXRefTableEntries;

   ParseObject* _selXRef[4];
   ParseObject* _seqXRef[4];
   ParseObject* _seqXRefTable[5];
   ParseObject* _seqXRefTableEntries[3];

   Properties* prop;

   Xistream* file;
   unsigned int startObject;
   vector<unsigned int> aOffsets;
};

#endif
