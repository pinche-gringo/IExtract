#ifndef PARSEOGG_H
#define PARSEOGG_H

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
#include <YGP/XStream.h>

struct Properties;


// Extracts the information out of OGG files
//  - The title is returned in strTitle
//  - The artist is returned in strAuthor
//  - The album is returned in strComment
class ParseOGG {
 public:
   static void parse (YGP::Xistream& stream, Properties& result);

 private:
   ParseOGG (Properties& result);
   ~ParseOGG ();

   // Prohibited manager functions
   ParseOGG ();
   ParseOGG (const ParseOGG& other);
   const ParseOGG& operator= (const ParseOGG& other);

   int foundNrSegments (const char* nr, unsigned int);
   int foundLenVendorString (const char* nr, unsigned int);
   int foundNrComments (const char* nr, unsigned int);
   int foundComment (const char* comment, unsigned int len);
   int foundLenComment (const char* nr, unsigned int);

   Properties& prop;

   typedef YGP::OFParseAttomic<ParseOGG>  OMParseAttomic;

   YGP::ParseExact     txtOGG;
   YGP::ParseExact     idCommentHeader;
   YGP::ParseSkip      skip;
   OMParseAttomic      nrSegments;
   OMParseAttomic      lenVendorStr;
   OMParseAttomic      nrComments;
   OMParseAttomic      lenEntry;
   OMParseAttomic      txtEntry;

   YGP::ParseSequence  seqComment;
   YGP::ParseSequence  seqOGG;

   YGP::ParseObject*   _seqComment[3];
   YGP::ParseObject*   _seqOGG[10];
};

#endif
