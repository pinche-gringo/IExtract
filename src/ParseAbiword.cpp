//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseAbiword
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 4.06.2005
//COPYRIGHT   : Copyright (C) 2005, 2006

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


#include <iostream>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <IExtract-cfg.h>

#include "Properties.h"
#include "ParseAbiword.h"


#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#endif


#define ID_METASTART      "<"
static const char* ID_METAINFO   (ID_METASTART "metadata>");
static const char* ID_TITLE      ("m key=\"dc.title\"");
static const char* ID_COMMENT    ("m key=\"dc.description\"");
static const char* ID_AUTHOR     ("m key=\"dc.creator\"");
static const char* ID_END        ("</metadata>");
static const char* ID_ABIWORD    ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE abiword PUBLIC \"-//ABISOURCE//DTD AWML");



//----------------------------------------------------------------------------
/// Default constructor
//----------------------------------------------------------------------------
ParseAbiword::ParseAbiword ()
   : prop (NULL),
     idAbiword (ID_ABIWORD, _("ID of an AbiWord document")),
     idMetadata (ID_METAINFO, _("Metadata information"), true, true),
     skipIDStart (ID_METASTART, _("Start of Abiword ID"), 256, 1, false, true),
     skipUnused (ID_METASTART, _("Unused contents"), 1024, 1, true, true),
     skipLine ("\n\r", _("Skip to end of line"), 1024, 1, true, true),
     tag ('<', _("Tag"), *this, &ParseAbiword::foundTag, 1024),
     value ("<", _("Value"), *this, &ParseAbiword::foundValue, 1024, 0),
     seqAbiWord (_seqAbiWord, _("AbiWord document")),
     seqMetadata (_seqMetadata, _("Metadata"), 1, 1, true),
     seqEntry (_seqEntry, _("Entries"), -1U, 1, true),
     selDocument (_selDocument, _("AbiWord meta-info"), -1U, 1, true),
     pEntry (NULL)
{
   _seqAbiWord[0] = &idAbiword;
   _seqAbiWord[1] = &selDocument;
   _seqAbiWord[2] = NULL;

   _selDocument[0] = &seqMetadata;
   _selDocument[1] = &skipIDStart;
   _selDocument[2] = &skipUnused;
   _selDocument[3] = NULL;

   _seqMetadata[0] = &idMetadata;
   _seqMetadata[1] = &skipLine;
   _seqMetadata[2] = &seqEntry;
   _seqMetadata[3] = NULL;

   _seqEntry[0] = &tag;
   _seqEntry[1] = &value;
   _seqEntry[2] = NULL;
}

//----------------------------------------------------------------------------
/// Destructor
//----------------------------------------------------------------------------
ParseAbiword::~ParseAbiword () {
}


//----------------------------------------------------------------------------
/// Callback after finding an XML tag in the document
/// \param pTag: Pointer to text holding the found tag
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseAbiword::foundTag (const char* pTag, unsigned int len) {
   TRACE1 ("ParseAbiword::foundTag (const char*, unsigned int) - Tag: "
           << pTag);
   Check1 (pTag); Check1 (len);

   Check3 (!pEntry);

   if (strcmp (pTag, ID_TITLE))
      if (strcmp (pTag, ID_COMMENT))
         if (strcmp (pTag, ID_AUTHOR)) {
             if (!strcmp (pTag, ID_END)) {
                seqEntry.setMaxCard (0);
                selDocument.setMaxCard (0);
                _seqEntry[1] = NULL;
             }
             pEntry = NULL;
         }
         else
            pEntry = &Properties::strAuthor;
      else
         pEntry = &Properties::strComment;
   else
      pEntry = &Properties::strTitle;

   return YGP::ParseObject::PARSE_OK;
}

//----------------------------------------------------------------------------
/// Callback after finding a value of an XML tag in the document.
/// \param pTag: Pointer to text holding the found value
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseAbiword::foundValue (const char* pValue, unsigned int len) {
   TRACE1 ("ParseAbiword::foundValue (const char*, unsigned int) - Value: "
           << pValue);
   Check1 (pValue);
   Check3 (prop);

   if (pEntry) {
      (prop->*pEntry).assign (pValue, len);
      pEntry = NULL;
   }
   return YGP::ParseObject::PARSE_OK;
}
