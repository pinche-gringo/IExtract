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

#include <YGP/Parse.h>

struct Properties;


// Class to extract the title of an HTML-file
class ParseHTML  {
 public:
   ParseHTML ();
   ~ParseHTML () { }

   void parse (YGP::Xistream& stream, Properties& result) throw (std::string) {
      prop = &result;
      htmlDoc.parse (stream); }

 private:
   // Callback-methods for type of parsed elementes
   int foundValue (const char*, unsigned int);
   int foundTitle (const char*, unsigned int);
   int foundAuthor (const char*, unsigned int);
   int foundComment (const char*, unsigned int);
   int foundEndOfHead (const char*, unsigned int);
   int foundEndScript (const char*, unsigned int);
   int foundScript (const char*, unsigned int);

   typedef YGP::OFParseText<ParseHTML> OMParseText;
   typedef YGP::OFParseSequence<ParseHTML> OMParseSequence;
   typedef YGP::OFParseUpperExact<ParseHTML> OMParseUpperExact;

   YGP::ParseExact      startTag;
   YGP::ParseExact      endTag;
   YGP::ParseUpperExact tagMeta;
   OMParseUpperExact    tagTitle;
   YGP::ParseUpperExact tagEndTitle;
   OMParseUpperExact    tagEndHead;
   OMParseText          title;
   OMParseText          value;
   YGP::ParseText       otherTag;
   YGP::ParseText       scriptType;
   YGP::ParseText       otherMetaEntry;
   YGP::ParseText       ignore;

   // Elements to parse meta contents
   YGP::ParseExact      quote;
   YGP::ParseExact      equal;
   YGP::ParseUpperExact name;
   YGP::ParseUpperExact content;
   YGP::ParseUpperExact script;
   OMParseUpperExact    endScript;

   // Supported meta-content
   OMParseUpperExact description;
   OMParseUpperExact author;
   OMParseUpperExact DCdescription;
   OMParseUpperExact DCauthor;
   OMParseUpperExact DCtitle;

   YGP::ParseSequence  seqTag;
   YGP::ParseSequence  seqTitle;
   YGP::ParseSequence  seqMetaCmd;
   YGP::ParseSequence  seqMetaName;
   OMParseSequence     seqScript;
   YGP::ParseSelection selMetaCmds;
   YGP::ParseSelection selMetaTags;
   YGP::ParseSelection selScriptContent;
   YGP::ParseSelection selCmd;
   YGP::ParseSelection htmlDoc;                                    // Startsequence

   YGP::ParseObject* _seqMetaName[11];
   YGP::ParseObject* _seqMetaCmd[3];
   YGP::ParseObject* _selMetaCmds[3];
   YGP::ParseObject* _seqScript[5];
   YGP::ParseObject* _selScriptContent[4];
   YGP::ParseObject* _seqTitle[6];
   YGP::ParseObject* _selCmd[6];
   YGP::ParseObject* _selMetaTags[7];
   YGP::ParseObject* _seqTag[4];
   YGP::ParseObject* _htmlDoc[3];

   Properties*  prop;
   enum { NONE = -1, TITLE = 0, AUTHOR, COMMENT } actEntry;
};

#endif
