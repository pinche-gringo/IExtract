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

#include <string>

#include <Parse.h>

struct Properties;


// Class to extract the title of an HTML-file
class ParseHTML  {
 public:
   ParseHTML ();
   ~ParseHTML () { }

   void parse (Xistream& stream, Properties& result) throw (std::string) {
      prop = &result;
      htmlDoc.parse (stream); }

 private:
   // Callback-methods for type of parsed elementes
   int foundTitle (const char*, unsigned int);

   typedef OFParseText<ParseHTML> OMParseText;

   ParseExact      startTag;
   ParseExact      endTag;
   ParseUpperExact tagTitle;
   ParseUpperExact tagEndTitle;
   OMParseText     title;
   ParseText       otherTag;
   ParseText       ignore;

   ParseSequence  seqTag;
   ParseSequence  seqTitle;
   ParseSelection selCmd;
   ParseSelection htmlDoc;                                    // Startsequence

   ParseObject* _seqTitle[6];
   ParseObject* _selCmd[3];
   ParseObject* _seqTag[4];
   ParseObject* _htmlDoc[3];

   Properties*  prop;
};

#endif
