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
   int foundValue (const char*, unsigned int);
   int foundTitle (const char*, unsigned int);
   int foundAuthor (const char*, unsigned int);
   int foundComment (const char*, unsigned int);
   int foundEndOfHead (const char*, unsigned int);

   typedef OFParseText<ParseHTML> OMParseText;
   typedef OFParseUpperExact<ParseHTML> OMParseUpperExact;

   ParseExact        startTag;
   ParseExact        endTag;
   ParseUpperExact   tagMeta;
   OMParseUpperExact tagTitle;
   ParseUpperExact   tagEndTitle;
   ParseUpperExact   tagEndHead;
   OMParseText       title;
   OMParseText       value;
   ParseText         otherTag;
   ParseText         ignore;

   // Elements to parse meta contents
   ParseExact      quote;
   ParseExact      equal;
   ParseUpperExact name;
   ParseUpperExact content;

   // Supported meta-content
   OMParseUpperExact description;
   OMParseUpperExact author;
   OMParseUpperExact DCdescription;
   OMParseUpperExact DCauthor;
   OMParseUpperExact DCtitle;

   ParseSequence  seqTag;
   ParseSequence  seqTitle;
   ParseSequence  seqMetaCmd;
   ParseSelection selMetaTags;
   ParseSelection selCmd;
   ParseSelection htmlDoc;                                    // Startsequence

   ParseObject* _seqMetaCmd[12];
   ParseObject* _seqTitle[6];
   ParseObject* _selCmd[5];
   ParseObject* _selMetaTags[6];
   ParseObject* _seqTag[4];
   ParseObject* _htmlDoc[3];

   Properties*  prop;
   enum { NONE = -1, TITLE = 0, AUTHOR, COMMENT } actEntry;
};

#endif
