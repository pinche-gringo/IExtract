//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseWord
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

#include <iomanip>
#include <iostream>

#include "ParseWord.h"

static const unsigned LEN_CONTENT     = 1024;


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
//Parameters: pClassname: Name of class containing parser-data
/*--------------------------------------------------------------------------*/
ParseWord::ParseWord() : len (0), pTitle (NULL)
   , id ("\x02\x00\x00\x00\xe4\x04\x00\x00\x1e\x00\x00\x00", "ID for title", 12, 12, false)
   , length ("\\*", "Length of title", *this, &ParseWord::foundLength, 4, 4, false)
   , title ("\0", "Title of document", *this, &ParseWord::foundTitle, 1, 1, false)
   , skipIDStart ("\x02", "Other command", 1, 1)
   , ignore ("\x02", "Content", LEN_CONTENT)
   , seqTitle (_seqTitle, "Title entry", 1, 1)
   , wordDoc (_wordDoc, "Word document", -1, 1) {

   _seqTitle[0] = &id;
   _seqTitle[1] = &length;
   _seqTitle[2] = &title;
   _seqTitle[3] = NULL;

   _wordDoc[0] = &seqTitle;
   _wordDoc[1] = &skipIDStart;
   _wordDoc[2] = &ignore;
   _wordDoc[3] = NULL;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the length of the title was read
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseWord::foundLength (const char* length) {
   if (len = *(int*)length)
      title.setMaxCard (len);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//purpose   : callback after a title was read
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseWord::foundTitle (const char* ptitle) {
   if (len) {
      pTitle = strdup (ptitle);
      wordDoc.setMaxCard (1);
   }
   return ParseObject::PARSE_OK;
}
