//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseRTF
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


#include <IExtract-cfg.h>

#define CHECK 9
#include <YGP/Check.h>

#include "ParseRTF.h"
#include "Properties.h"

static const unsigned LEN_VALUE         = 256;
static const unsigned LEN_COMMAND       = 64;


#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#endif


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
ParseRTF::ParseRTF ()
   : startBlock ("{", _("Start of block"))
     , endBlock ("}", _("End of block"))
     , info ("\\info", _("Info block"))
     , endInfoBlock ("}", _("End of info block"), *this, &ParseRTF::finish)
     , startCmd ("\\", _("Start of command"))
     , otherCmd (" \t\n\r\\{}", _("Command"), LEN_COMMAND)
     , noSpecialChar ("\\!\\\\{}", _("No special char"))
     , author ("\\author", _("Author-entry"), *this, &ParseRTF::foundAuthor)
     , title ("\\title", _("Title-entry"), *this, &ParseRTF::foundTitle)
     , description ("\\doccomm", _("Description-entry"), *this, &ParseRTF::foundComment)
     , value ("}>", _("Value of entry"), *this, &ParseRTF::foundValue, LEN_VALUE, 0)
     , seqInfo (_seqInfo, _("Information block"))
     , seqInfoValue (_seqInfoValue, _("Information block value"), -1U)
     , seqOtherCmd (_seqOtherCmd, _("Other command"))
     , seqValue (_seqValue, _("Value of command"), 1, 0)
     , selEntry (_selEntry, _("Entry"))
     , selCmd (_selCmd, _("Valid RTF command"), -1U)
     , block (_block, _("RTF block"), -1U)
     , prop (NULL), actEntry (NONE) {

   _seqInfo[0] = &info;
   _seqInfo[1] = &seqInfoValue;
   _seqInfo[2] = &endInfoBlock;
   _seqInfo[3] = NULL;

   _seqInfoValue[0] = &startBlock;
   _seqInfoValue[1] = &selEntry;
   _seqInfoValue[2] = &value;
   _seqInfoValue[3] = &endBlock;
   _seqInfoValue[4] = NULL;

   _seqOtherCmd[0] = &startCmd;
   _seqOtherCmd[1] = &otherCmd;
   _seqOtherCmd[2] = &seqValue;
   _seqOtherCmd[3] = NULL;

   _seqValue[0] = &noSpecialChar;
   _seqValue[1] = &value;
   _seqValue[2] = NULL;

   _selEntry[0] = &author;
   _selEntry[1] = &title;
   _selEntry[2] = &description;
   _selEntry[3] = &seqOtherCmd;
   _selEntry[4] = NULL;

   _selCmd[0] = &seqInfo;
   _selCmd[1] = &block;
   _selCmd[2] = &seqOtherCmd;
   _selCmd[3] = NULL;

   _block[0] = &startBlock;
   _block[1] = &selCmd;
   _block[2] = &endBlock;
   _block[3] = NULL;
}


//-----------------------------------------------------------------------------
/// Callback after a title was read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseRTF::foundValue (const char* pValue, unsigned int len) {
   if (actEntry != NONE) {
      static std::string Properties::* values[] =
         { &Properties::strTitle, &Properties::strAuthor, &Properties::strComment };

      Check3 (prop);
      (prop->*(values[actEntry])).assign (pValue, len);
   }
   actEntry = NONE;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a title tag was read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseRTF::foundTitle (const char*, unsigned int) {
   actEntry = TITLE;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an author-tag was read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseRTF::foundAuthor (const char*, unsigned int) {
   actEntry = AUTHOR;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a comment tag was read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseRTF::foundComment (const char*, unsigned int) {
   actEntry = COMMENT;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a title tag was read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseRTF::finish (const char*, unsigned int) {
   block.setMaxCard (1);
   _block[2] = NULL;
   selCmd.setMaxCard (1);
   return YGP::ParseObject::PARSE_OK;
}
