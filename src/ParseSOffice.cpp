//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseStarOffice
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 04.11.2002
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

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Properties.h"
#include "ParseSOffice.h"


#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#endif


#ifdef WORDS_BIGENDIAN
inline unsigned short get2BytesLSB (const char* pAddr) {
   return ((unsigned char)(*pAddr) << 8) + (unsigned char)pAddr[1];
}
#else
inline unsigned short get2BytesLSB (const char* pAddr) {
   return *(unsigned short*)pAddr;
}
#endif


// Tag for StarOffice document
#define ID1 "S"
#define ID ID1 "fxDocumentInfo"


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
/*--------------------------------------------------------------------------*/
ParseStarOffice::ParseStarOffice ()
   : idSOffice (ID, _("StarOffice ID"), false)
     , skipIDStart (ID1, _("Start of StarOffice IDs"), 256, 1, false)
     , skip (7)
     , skip2 (ID1, _("Unused contents 2"), 0x900, 1, false, false)
     , length ("\\*", _("Length of data-entry"), *this, &ParseStarOffice::foundLength, 2, 2, false)
     , value ("\\*", _("Property-entry"), *this, &ParseStarOffice::foundValue, 1, 0, false)
     , selDocument (_selDocument, _("StarOffice document"), -1U)
     , seqEntries (_seqEntries, _("Entries of properties"), *this, &ParseStarOffice::foundProps, 4, 4, false)
     , seqProperties (_seqProperties, _("Properties"), 1, 1, false)
     , prop (NULL), actEntry (NONE) {

   _selDocument[0] = &seqProperties;
   _selDocument[1] = &skipIDStart;
   _selDocument[2] = &skip2;
   _selDocument[3] = NULL;

   _seqProperties[0] = &idSOffice;
   _seqProperties[1] = &seqEntries;
   _seqProperties[2] = NULL;

   _seqEntries[0] = &skip;
   _seqEntries[1] = &length;
   _seqEntries[2] = &value;
   _seqEntries[3] = NULL;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a value was read
//Parameters: pLength: Pointer to value
//            len: Length of data
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
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

   skip.setOffset (entries[actEntry].offset - len);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the length of the next value was read
//Parameters: pLength: Pointer to length
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseStarOffice::foundLength (const char* pLength, unsigned int) {
   TRACE5 ("ParseStarOffice::foundLength (const char*, unsigned int) - "
           << get2BytesLSB (pLength));
   Check3 (pLength);
   actEntry = (enum types)((int)actEntry + 1);
   value.setMaxCard (get2BytesLSB (pLength));
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the property-entries have been parsed
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseStarOffice::foundProps (const char*, unsigned int) {
   TRACE9 ("ParseStarOffice::foundProps (const char*, unsigned int)");

   selDocument.setMaxCard (1);
   return YGP::ParseObject::PARSE_OK;
}
