//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseOOffice
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 17.06.2003
//COPYRIGHT   : Copyright (C) 2003 - 2007

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
#include <YGP/Utility.h>

#include "Properties.h"
#include "ParseOOffice.h"


#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#endif


// IDs of OpenOffice's meta-info
static const char* ID_METAINFO ("meta.xml<?xml version=\"");
static const char* ID_TITLE    ("dc:title");
static const char* ID_COMMENT  ("dc:description");
static const char* ID_AUTHOR   ("dc:creator");
static const char* ID_END      ("/office:document-meta");

// IDs of ZIP fiele
static const char* ID_LOCALHDR ("PK\03\04");
static const char* ID_ENDCDR   ("PK\05\06");
static const char* ID_CFILEHDR ("PK\01\02");


//----------------------------------------------------------------------------
/// Default constructor
//----------------------------------------------------------------------------
ParseOpenOffice::ParseOpenOffice ()
   : prop (NULL),
     idZipEntry (ID_LOCALHDR, _("ID of local header"), false),
     skip (-22, std::ios::end),
     idCDR (ID_ENDCDR, _("ID of end of central directory record"), false),
     skip2 (6),
     nrCDREntries ("\\*", _("Entries in the central directory record"), *this,
		   &ParseOpenOffice::foundNrEntries, 2, 2, false),
     offCDR ("\\*", _("Offset of central directory record"), *this,
	     &ParseOpenOffice::foundOffsetCDR, 4, 4, false),
     idCFileHdr (ID_CFILEHDR, _("ID of central file header"), false),
     lenName ("\\*", _("Length of file-name"), *this, &ParseOpenOffice::foundLenName, 2, 2, false),
     len ("\\*", _("Length"), *this, &ParseOpenOffice::foundLength, 2, 2, false),
     name ("\\*", _("Filename"), *this, &ParseOpenOffice::foundName, 1, 1, false),
     posFile ("\\*", _("Offset of meta-info"), *this,
	     &ParseOpenOffice::foundOffsetFile, 4, 4, false),
     posMetaInfo (0, std::ios::beg),

     idMetadata (ID_METAINFO, _("Metadata information"), true, true),
     skipLine ("\n\r", _("Skip to end of line"), 1024, 1, true, true),
     tag ('<', _("Tag"), *this, &ParseOpenOffice::foundTag, 1024),
     value ("<", _("Value"), *this, &ParseOpenOffice::foundValue, 1024, 0),

     seqCDREntries (_seqCDREntries, _("Central directory record entries"), 1, 1, false),
     seqMetadata (_seqMetadata, _("Metadata"), 1, 1, true),
     seqEntry (_seqEntry, _("Entries"), -1U, 1, true  ),
     seqDocument (_seqDocument, _("OpenOffice document"), 1, 1, true),
     pEntry (NULL)
{
   _seqDocument[0] = &idZipEntry;
   _seqDocument[1] = &skip;
   _seqDocument[2] = &idCDR;
   _seqDocument[3] = &skip2;
   _seqDocument[4] = &nrCDREntries;
   _seqDocument[5] = &skip2;
   _seqDocument[6] = &offCDR;
   _seqDocument[7] = &skip;
   _seqDocument[8] = &seqCDREntries;
   _seqDocument[9] = &posMetaInfo;
   _seqDocument[10] = &idZipEntry;
   _seqDocument[11] = &skip;
   _seqDocument[12] = &seqMetadata;
   _seqDocument[13] = NULL;

   _seqCDREntries[0] = &idCFileHdr;
   _seqCDREntries[1] = &skip2;
   _seqCDREntries[2] = &lenName;
   _seqCDREntries[3] = &len;
   _seqCDREntries[4] = &len;
   _seqCDREntries[5] = &skip;
   _seqCDREntries[6] = &posFile;
   _seqCDREntries[7] = &name;
   _seqCDREntries[8] = NULL;

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
ParseOpenOffice::~ParseOpenOffice () {
}


//----------------------------------------------------------------------------
/// Callback after finding an XML tag in the document
/// \param pTag: Pointer to text holding the found tag
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseOpenOffice::foundTag (const char* pTag, unsigned int len) {
   TRACE1 ("ParseOpenOffice::foundTag (const char*, unsigned int) - Tag: "
           << pTag);
   Check1 (pTag); Check1 (len);

   Check3 (!pEntry);

   if (strcmp (pTag, ID_TITLE))
      if (strcmp (pTag, ID_COMMENT))
         if (strcmp (pTag, ID_AUTHOR)) {
             if (!strcmp (pTag, ID_END)) {
                seqEntry.setMaxCard (0);
                seqDocument.setMaxCard (0);
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
int ParseOpenOffice::foundValue (const char* pValue, unsigned int len) {
   TRACE1 ("ParseOpenOffice::foundValue (const char*, unsigned int) - Value: "
           << pValue);
   Check1 (pValue);
   Check3 (prop);

   if (pEntry) {
      (prop->*pEntry).assign (pValue, len);
      pEntry = NULL;
   }
   return YGP::ParseObject::PARSE_OK;
}

//----------------------------------------------------------------------------
/// Callback after finding the offset of the CDR
/// \param pTag: Pointer to text holding the found value
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseOpenOffice::foundOffsetCDR (const char* pValue, unsigned int len) {
   Check1 (pValue);
   TRACE5 ("ParseOpenOffice::foundOffsetCBR (const char*, unsigned int) - Value: "
           << std::hex << YGP::get4BytesLSB (pValue) << std::dec);

   skip.setOffset (YGP::get4BytesLSB (pValue));
   skip.setWay (std::ios::beg);
   skip2.setOffset (24);
   skip2.setWay (std::ios::cur);
   return YGP::ParseObject::PARSE_OK;
}

//----------------------------------------------------------------------------
/// Callback after finding the number of entries in the CDR
/// \param pTag: Pointer to text holding the found value
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseOpenOffice::foundNrEntries (const char* pValue, unsigned int len) {
   Check1 (pValue);
   TRACE5 ("ParseOpenOffice::foundNrEntries (const char*, unsigned int) - Value: " << YGP::get2BytesLSB (pValue));
   seqCDREntries.setMaxCard (YGP::get2BytesLSB (pValue));
   skip2.setOffset (4);
   return YGP::ParseObject::PARSE_OK;
}

//----------------------------------------------------------------------------
/// Callback after finding the length of a file-name
/// \param pTag: Pointer to text holding the found value
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseOpenOffice::foundLenName (const char* pValue, unsigned int len) {
   Check1 (pValue);
   TRACE5 ("ParseOpenOffice::foundLenName (const char*, unsigned int) - Value: " << YGP::get2BytesLSB (pValue));
   len = YGP::get2BytesLSB (pValue);
   name.setMinCard (len);
   name.setMaxCard (len);
   skip.setOffset (8);
   skip.setWay (std::ios::cur);
   return YGP::ParseObject::PARSE_OK;
}

//----------------------------------------------------------------------------
/// Callback after finding a length
/// \param pTag: Pointer to text holding the found value
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseOpenOffice::foundLength (const char* pValue, unsigned int len) {
   Check1 (pValue);
   TRACE5 ("ParseOpenOffice::foundLength (const char*, unsigned int) - Value: " << YGP::get2BytesLSB (pValue));
   skip.setOffset (YGP::get2BytesLSB (pValue) + skip.getOffset ());
   return YGP::ParseObject::PARSE_OK;
}

//----------------------------------------------------------------------------
/// Callback after finding a file-name
/// \param pTag: Pointer to text holding the found value
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseOpenOffice::foundName (const char* pValue, unsigned int len) {
   Check1 (pValue);
   TRACE5 ("ParseOpenOffice::foundName (const char*, unsigned int) - Value: " << pValue);
   if ((len == 8) && !memcmp (pValue, "meta.xml", len)) {
      seqCDREntries.setMinCard (1);
      seqCDREntries.setMaxCard (1);
      skip.setOffset (26);
   }
   return YGP::ParseObject::PARSE_OK;
}

//----------------------------------------------------------------------------
/// Callback after finding the offset of a file in the ZIP-archive
/// \param pTag: Pointer to text holding the found value
/// \param len: Length of text
/// \returns \c YGP::ParseObject::ParseOK
//----------------------------------------------------------------------------
int ParseOpenOffice::foundOffsetFile (const char* pValue, unsigned int len) {
   Check1 (pValue);
   TRACE5 ("ParseOpenOffice::foundOffsetFile (const char*, unsigned int) - "
           << std::hex << YGP::get4BytesLSB (pValue) << std::dec);
   posMetaInfo.setOffset (YGP::get4BytesLSB (pValue));
   return YGP::ParseObject::PARSE_OK;
}
