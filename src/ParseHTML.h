#ifndef PARSEHTML_H
#define PARSEHTML_H

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


// Class to extract the title of an HTML-file
class ParseHTML  {
 public:
   ParseHTML ();
   ~ParseHTML () { if (pTitle) free (pTitle); }

   const char* parse (Xistream& stream) throw (std::string) {
      htmlDoc.parse (stream);
      return pTitle; }

 private:
   // Callback-methods for type of parsed elementes
   int foundTitle (const char*);

   typedef OFParseText<ParseHTML> OMParseText;

   ParseExact  startTag;
   ParseExact  endTag;
   ParseExact  tagTitle;
   ParseExact  tagEndTitle;
   OMParseText title;
   ParseText   otherTag;
   ParseText   ignore;

   ParseSequence  seqTag;
   ParseSequence  seqTitle;
   ParseSelection selCmd;
   ParseSelection htmlDoc;                                    // Startsequence

   ParseObject* _seqTitle[6];
   ParseObject* _selCmd[3];
   ParseObject* _seqTag[4];
   ParseObject* _htmlDoc[3];

   char* pTitle;
};

#endif
