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

#include <Parse.h>


// Class to extract the title (of the properties) of a Word document
class ParseWord  {
 public:
   ParseWord ();
   ~ParseWord () { if (pTitle) free (pTitle); }

   const char* parse (Xistream& stream) throw (std::string) {
      wordDoc.parse (stream);
      return pTitle; }

 private:
   // Callback-methods for type of parsed elementes
   int foundLength (const char*);
   int foundTitle (const char*);

   typedef OFParseText<ParseWord> OMParseText;
   typedef OFParseAttomic<ParseWord> OMParseAttomic;

   ParseExact     id;
   OMParseAttomic length;
   OMParseText    title;
   ParseAttomic   skipIDStart;
   ParseText      ignore;

   ParseSequence  seqTitle;
   ParseSelection wordDoc;                                   // Startsequence

   ParseObject* _wordDoc[4];
   ParseObject* _seqTitle[4];

   unsigned int len;
   char*  pTitle;
};

#endif
