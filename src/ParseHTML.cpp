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

static const unsigned LEN_TAG         = 512;
static const unsigned LEN_TITLE       = 512;
static const unsigned LEN_COMMAND     = 1024;


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
//Parameters: pClassname: Name of class containing parser-data
/*--------------------------------------------------------------------------*/
ParseHTML::ParseHTML ()
   : prop (NULL) 
   , startTag ("<", "Start of HTML-tag"), endTag (">", "End of HTML-tag")
   , tagTitle ("TITLE", "title-tag")
   , title ("<", "Title of document", *this, &ParseHTML::foundTitle, LEN_TITLE)
   , otherTag (">", "Other HTML tag", LEN_TAG)
   , ignore ("<", "Content", LEN_COMMAND)
   , seqTitle (_seqTitle, "Title entry")
   , selCmd (_selCmd, "Valid HTML command")
   , seqTag (_seqTag, "Valid HTML tag")
   , htmlDoc (_htmlDoc, "HTML document", -1, 1) {

   _seqTitle[0] = &tagTitle;
   _seqTitle[1] = &endTag;
   _seqTitle[2] = &title;
   _seqTitle[3] = NULL;

   _selCmd[0] = &seqTitle;
   _selCmd[1] = &otherTag;
   _selCmd[2] = NULL;

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
int ParseHTML::foundTitle (const char* pTitle, unsigned int len) {
   assert (prop);
   prop->strTitle.assign (pTitle, len);
   htmlDoc.setMaxCard (1);
   return ParseObject::PARSE_OK;
}
