//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseHTML
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 08.10.2002
//COPYRIGHT   : Anticopyright (A) 2002

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

#include <assert.h>

#include "ParseHTML.h"
#include "Properties.h"

static const unsigned LEN_TAG         = 512;
static const unsigned LEN_TITLE       = 512;
static const unsigned LEN_COMMAND     = 1024;


#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#endif


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
/*--------------------------------------------------------------------------*/
ParseHTML::ParseHTML ()
   : prop (NULL), actEntry (NONE)
     , startTag ("<", "Start of HTML-tag"), endTag (">", "End of HTML-tag")
     , tagMeta ("META", "Meta tag")
     , tagTitle ("TITLE", "Title-tag", *this, &ParseHTML::foundTitle)
     , tagEndTitle ("/TITLE", "Title-tag")
     , tagEndHead ("/HEAD", "End of header")
     , title ("<", "Title of document", *this, &ParseHTML::foundValue, LEN_TITLE)
     , value ("\">", "Value of entry", *this, &ParseHTML::foundValue, LEN_TITLE)
     , otherTag (">", "Other HTML tag", LEN_TAG)
     , ignore ("<", "Unused information", LEN_COMMAND)
     , quote ("\"", "Quote", 1, 0, true)
     , equal ("=", "Equal sign", 1, 0, true)
     , name ("NAME", "Name of meta tag")
     , content ("CONTENT", "Content specifier")
     , description ("DESCRIPTION", "Description", *this, &ParseHTML::foundComment)
     , author ("AUTHOR", "Author", *this, &ParseHTML::foundAuthor)
     , DCdescription  ("DC.DESCRIPTION", "Description in Dublin Core", *this, &ParseHTML::foundComment)
     , DCauthor ("DC.CREATOR", "Author in Dublin Core", *this, &ParseHTML::foundAuthor)
     , DCtitle ("DC.TITLE", "Title in Dublin Core", *this, &ParseHTML::foundTitle)
     , seqMetaCmd (_seqMetaCmd, "Meta entry")
     , seqMetaName (_seqMetaName, "Name entry for meta tag")
     , seqTitle (_seqTitle, "Title entry")
     , selMetaTags (_selMetaTags, "Recogniced meta tags", 1, 0)
     , selMetaCmds (_selMetaCmds, "Meta entries")
     , selCmd (_selCmd, "Valid HTML command")
     , seqTag (_seqTag, "Valid HTML tag")
     , htmlDoc (_htmlDoc, "HTML document", -1, 1) {

   _seqMetaCmd[0] = &tagMeta;
   _seqMetaCmd[1] = &selMetaCmds;
   _seqMetaCmd[2] = NULL;

   _selMetaCmds[0] = &seqMetaName;
   _selMetaCmds[1] = &otherTag;
   _selMetaCmds[2] = NULL;

   _seqMetaName[0] = &name;
   _seqMetaName[1] = &equal;
   _seqMetaName[2] = &quote;
   _seqMetaName[3] = &selMetaTags;
   _seqMetaName[4] = &quote;
   _seqMetaName[5] = &content;
   _seqMetaName[6] = &equal;
   _seqMetaName[7] = &quote;
   _seqMetaName[8] = &value;
   _seqMetaName[9] = &quote;
   _seqMetaName[10] = NULL;

   _selMetaTags[0] = &description;
   _selMetaTags[1] = &author;
   _selMetaTags[2] = &DCdescription;
   _selMetaTags[3] = &DCauthor;
   _selMetaTags[4] = &DCtitle;
   _selMetaTags[5] = &value;
   _selMetaTags[6] = NULL;

   _seqTitle[0] = &tagTitle;
   _seqTitle[1] = &endTag;
   _seqTitle[2] = &title;
   _seqTitle[3] = &startTag;
   _seqTitle[4] = &tagEndTitle;
   _seqTitle[5] = NULL;

   _selCmd[0] = &seqTitle;
   _selCmd[1] = &seqMetaCmd;
   _selCmd[2] = &tagEndHead;
   _selCmd[3] = &otherTag;
   _selCmd[4] = NULL;

   _seqTag[0] = &startTag;
   _seqTag[1] = &selCmd;
   _seqTag[2] = &endTag;
   _seqTag[3] = NULL;

   _htmlDoc[0] = &seqTag;
   _htmlDoc[1] = &ignore;
   _htmlDoc[2] = NULL;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a title was read
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseHTML::foundValue (const char* pTitle, unsigned int len) {
   if (actEntry != NONE) {
      static string Properties::* values[] =
         { &Properties::strTitle, &Properties::strAuthor, &Properties::strComment };

      assert (prop);
      (prop->*(values[actEntry])).assign (pTitle, len);
   }
   actEntry = NONE;
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a title tag was read
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseHTML::foundTitle (const char*, unsigned int) {
   actEntry = TITLE;
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after an author-tag was read
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseHTML::foundAuthor (const char*, unsigned int) {
   actEntry = AUTHOR;
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a comment tag was read
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseHTML::foundComment (const char*, unsigned int) {
   actEntry = COMMENT;
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a title tag was read
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseHTML::foundEndOfHead (const char*, unsigned int) {
   htmlDoc.setMaxCard (1);
   return ParseObject::PARSE_OK;
}
