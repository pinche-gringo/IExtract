//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseStarOffice
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 04.11.2002
//COPYRIGHT   : Copyright (C) 2002 - 2007

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

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "Properties.h"
#include "ParseSOffice.h"


#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#endif


// Tag for StarOffice document
static const char* ID ("\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1");
static const char* IDSTAROFFICE ("\x0F\0SfxDocumentInfo");


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
ParseStarOffice::ParseStarOffice ()
   : idOffice (ID, _("ID of office document"), false),
     idDocInfo (IDSTAROFFICE, _("ID of StarOffice document"), 17, 17, false),
     skip (0x8c0, std::ios::beg),
     skip2 (7),
     length ("\\*", _("Length of data-entry"), *this, &ParseStarOffice::foundLength, 2, 2, false),
     value ("\\*", _("Property-entry"), *this, &ParseStarOffice::foundValue, 1, 0, false),
     seqDocument (_seqDocument, _("StarOffice document"), 1, 1),
     seqEntries (_seqEntries, _("Entries of properties"), 4, 4, false),
     prop (NULL), actEntry (NONE) {

   _seqDocument[0] = &idOffice;
   _seqDocument[1] = &skip;
   _seqDocument[2] = &idDocInfo;
   _seqDocument[3] = &seqEntries;
   _seqDocument[4] = NULL;

   _seqEntries[0] = &skip2;
   _seqEntries[1] = &length;
   _seqEntries[2] = &value;
   _seqEntries[3] = NULL;
}


//-----------------------------------------------------------------------------
/// Callback after a value was read
/// \param pLength: Pointer to value
/// \param len: Length of data
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseStarOffice::foundValue (const char* pTitle, unsigned int len) {
   TRACE1 ("ParseStarOffice::foundValue (const char*, unsigned int) - "
           << len << " byte = " << pTitle);

   Check3 (actEntry != NONE);

   static struct {
      std::string Properties::* value;
      unsigned int offset;
   } entries[] = { { &Properties::strAuthor, 39 },
                   { &Properties::strAuthor, 80 },
                   { &Properties::strTitle,  128 },
                   { &Properties::strComment,  0 } };
   Check3 (actEntry <= (sizeof (entries) / sizeof (entries[0])));

   if (len) {
      Check3 (prop);
      (prop->*(entries[actEntry].value)).assign (pTitle, len);
   }

   skip2.setOffset (entries[actEntry].offset - len);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of the next value was read
/// \param pLength: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseStarOffice::foundLength (const char* pLength, unsigned int) {
   TRACE5 ("ParseStarOffice::foundLength (const char*, unsigned int) - "
           << YGP::get2BytesLSB (pLength));
   Check3 (pLength);
   actEntry = (enum types)((int)actEntry + 1);
   value.setMaxCard (YGP::get2BytesLSB (pLength));
   return YGP::ParseObject::PARSE_OK;
}
