//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseWord
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 08.10.2002
//COPYRIGHT   : Copyright (C) 2002 - 2005

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

#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif


#include <iomanip>
#include <iostream>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "ParseWord.h"
#include "Properties.h"


static const char SUMMARY[] = ("\x05\0\x53\0\x75\0\x6d\0\x6d\0\x61\0\x72\0\x79\0"
			       "\x49\0\x6E\0\x66\0\x6F\0\x72\0\x6D\0\x61\0\x74\0"
			       "\x69\0\x6F\0\x6E\0\0");
static const char CLASSID[] = { '\x01', '\0', '\0', '\0', '\xE0', '\x85', '\x9f', '\xF2', '\xF9', '\x4F',
				'\x68', '\x10', '\xAB', '\x91', '\x08', '\0', '\x2B', '\x27', '\xB3', '\xD9' };


static const unsigned LEN_CONTENT     = 1024;


#ifdef WORDS_BIGENDIAN
   static const unsigned int TYPE_TITLE   = 0x2000000;
   static const unsigned int TYPE_AUTHOR  = 0x4000000;
   static const unsigned int TYPE_COMMENT = 0x6000000;

inline unsigned int get4BytesLSB (const char* pAddr) {
   return (((unsigned char)(*pAddr) << 24) + ((unsigned char)pAddr[1] << 16)
           + ((unsigned char)pAddr[2] << 8) + ((unsigned char)pAddr[3]));

inline short int get2BytesLSB (const char* pAddr) {
   return  ((unsigned char)*pAddr << 8) + ((unsigned char)pAddr[1]));
}

#else
   static const unsigned int TYPE_TITLE   = 2;
   static const unsigned int TYPE_AUTHOR  = 4;
   static const unsigned int TYPE_COMMENT = 6;

inline unsigned int get4BytesLSB (const char* pAddr) {
   return *(unsigned int*)pAddr;
}

inline unsigned short get2BytesLSB (const char* pAddr) {
   return *(unsigned short*)pAddr;
}

#endif


static const unsigned int aTypes[] = { TYPE_TITLE, TYPE_AUTHOR, TYPE_COMMENT };


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
ParseWord::ParseWord()
   : poifsID ("\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1", _("Magic number")),
     skip (22),
     skipBeg (22, std::ios_base::beg),
     sizeBlock  ("\\*", _("Size of blocks"), *this, &ParseWord::foundBlockSize, 2, 2, false),
     blockIndex  ("\\*", _("Index of block"), *this, &ParseWord::foundBlockIndex, 4, 4, false),
     nameProperty ("\\*", _("Name of property"), *this, &ParseWord::foundNameProperty, 0x40, 0x40, false),
     lenNameProperty ("\\*", _("Length of name of property"), *this, &ParseWord::foundLenNameProperty, 2, 2, false),
     idPropStream ("\xfe\xff", _("ID of property set stream"), 2, 2, false),
     idClass (CLASSID, _("Class ID"), sizeof (CLASSID), sizeof (CLASSID), false),
     length ("\\*", _("Length"), *this, &ParseWord::foundLength, 4, 4, false),
     nrEntries ("\\*", _("Number of entries"), *this, &ParseWord::foundNrEntries, 4, 4, false),
     type ("\\*", _("Type of entry"), *this, &ParseWord::foundType, 4, 4, false),
     offset  ("\\*", _("Offset"), *this, &ParseWord::foundOffset, 4, 4, false),
     information ("\0", _("Information"), *this, &ParseWord::foundInformation, 1, 1, false),

     sectionOffset  ("\\*", _("Offset of section"), *this, &ParseWord::foundSectionOffset, 4, 4, false),
     seqProperties (_seqProperties, _("Properties"), -1U, 1, false),
     seqInformation (_seqInformation, _("Information summary"), 1, 0, false),

     seqEntries (_seqEntries, _("Entry description"), *this,
		 &ParseWord::foundPropertiesHeader, 1, 1, false),
     wordDoc (_wordDoc, _("Word document"), 1, 1),
     cRead (0), cEntries (0), actEntry (-1U), len (0),
     prop (NULL) {

   // Property table
   _seqProperties[0] = &nameProperty;
   _seqProperties[1] = &lenNameProperty;
   _seqProperties[2] = &skip;
   _seqProperties[3] = NULL;

   _seqEntries[0] = &type;
   _seqEntries[1] = &skip;
   _seqEntries[2] = NULL;

   _seqInformation[0] = &length;
   _seqInformation[1] = &information;
   _seqInformation[2] = &skip;
   _seqInformation[3] = NULL;

   _wordDoc[0] = &poifsID;
   _wordDoc[1] = &skip;
   _wordDoc[2] = &sizeBlock;
   _wordDoc[3] = &skip;
   _wordDoc[4] = &blockIndex;
   _wordDoc[5] = &skipBeg;
   _wordDoc[6] = &seqProperties;
   _wordDoc[7] = &blockIndex;       // Parsed only, if SummaryInformation found
   _wordDoc[8] = &skipBeg;
   _wordDoc[9] = &idPropStream;
   _wordDoc[10] = &skip;
   _wordDoc[11] = &idClass;
   _wordDoc[12] = &sectionOffset;
   _wordDoc[13] = &length;
   _wordDoc[14] = &nrEntries;
   _wordDoc[15] = &seqEntries;
   _wordDoc[16] = &skip;
   _wordDoc[17] = &seqInformation;
   _wordDoc[18] = NULL;
}


//-----------------------------------------------------------------------------
/// Callback after the number of blocks has been parsed
/// \param pBlockSize: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundBlockSize (const char* pBlockSize, unsigned int) {
   Check3 (pBlockSize);
   blockSize = 1 << get2BytesLSB (pBlockSize);
   TRACE9 ("ParseWord::foundBlockSize (const char*, unsigned int) - " << blockSize);
   skip.setOffset (16);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the index of a block has been parsed
/// \param pBlockID: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundBlockIndex (const char* pBlockID, unsigned int) {
   Check3 (pBlockID);
   skipBeg.setOffset ((get4BytesLSB (pBlockID) + 1) * blockSize);
   skip.setOffset (0x16);
   TRACE5 ("ParseWord::foundBlockIndex (const char*, unsigned int) - " << get4BytesLSB (pBlockID) << " -> " << skipBeg.getOffset ());
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the name of a property has been parsed
/// \param pName: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundNameProperty (const char* pName, unsigned int len) {
   Check2 (pName); Check2 (len <= sizeof (propertyName));
   memcpy (propertyName, pName, sizeof (propertyName));
   TRACE3 ("ParseWord::foundNameProperty (const char*, unsigned int) - " << pName << pName[2] << pName[4] << pName[6] << pName[8] << pName[10]);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of the name of a property has been parsed
/// \param pLen: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundLenNameProperty (const char* pLen, unsigned int) {
   unsigned int length (get2BytesLSB (pLen));
   TRACE7 ("ParseWord::foundLenNameProperty (const char*, unsigned int) - " << length);

   if (!length) {
      TRACE9 ("ParseWord::foundLenNameProperty (const char*, unsigned int) - End");
      seqProperties.setMinCard (0);
      seqProperties.setMaxCard (0);
      _wordDoc[7] = NULL;
   }
   else {
      TRACE9 (sizeof (CLASSID));
      Check3 (sizeof (SUMMARY) == 0x28);
      if ((length == 0x28) && !memcmp (propertyName, SUMMARY, 0x28)) {
	 TRACE9 ("ParseWord::foundLenNameProperty (const char*, unsigned int) - SI");
	 seqProperties.setMinCard (0);
	 seqProperties.setMaxCard (0);
	 skip.setOffset (0x32);
      }
      else
	 skip.setOffset (0x3e);
   }

   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the index to the next property has been parsed
/// \param pNext: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundNextProperty (const char* pNext, unsigned int) {
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the offset of the section-entry was found
/// \param offset: Pointer to offset
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundSectionOffset (const char* offset, unsigned int) {
   Check3 (offset);

   // Set offset to section (but substract section-offset)
   skip.setOffset (get4BytesLSB (offset) - 0x30);
   TRACE9 ("ParseWord::foundSectionOffset (const char*) - " << skip.getOffset () << " (0x"
           << std::hex << skip.getOffset () << std::dec << ')');
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number of entries has been parsed
/// \param pEntries: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundNrEntries (const char* pEntries, unsigned int) {
   Check3 (pEntries);
   cEntries = get4BytesLSB (pEntries);
   TRACE4 ("ParseWord::foundNrEntries (const char*) - Entries: " << cEntries);

   seqEntries.setMaxCard (cEntries);
   skip.setOffset (4);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number of entries has been parsed
/// \param pType: Pointer to found type
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundType (const char* pType, unsigned int) {
   Check3 (pType);
   actEntry = *(unsigned int*)pType;
   TRACE9 ("ParseWord::foundType (const char*) - Type: " << actEntry);

   _seqEntries[1] = ((getTypeIndex (actEntry) != -1)
                     ? static_cast<YGP::ParseObject*> (&offset)
                     : static_cast<YGP::ParseObject*> (&skip));

   cRead += 8;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the offset of the comment-entry was found
/// \param offset: Pointer to offset
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundOffset (const char* offset, unsigned int) {
   Check3 (offset);
   Check3 (getTypeIndex (actEntry) != -1);

   unsigned int off (get4BytesLSB (offset));
   TRACE9 ("ParseWord::foundOffset (const char*) - " << off << " (0x"
           << std::hex << off << std::dec << ')');

   aOffsets[off] = actEntry;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of an information was read
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundLength (const char* length, unsigned int) {
   Check3 (length);
   if ((len = get4BytesLSB (length)))
      information.setMaxCard (len);
   TRACE8 ("ParseWord::foundLength (const char*, unsigned int): " << len);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an information was read
/// \param pInfo: Pointer to information
/// \param len: Length of inforamtion
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundInformation (const char* pInfo, unsigned int len) {
   Check3 (pInfo);
   Check3 (aOffsets[actEntry] < 1000);

   TRACE1 ("ParseWord::foundInformation (const char*, unsigned int): " << pInfo
           << " (" << len << " bytes); Entries: " << seqInformation.getMaxCard ());

   static std::string Properties::* values[] =
      { &Properties::strTitle, &Properties::strAuthor, &Properties::strComment };

   Check3 (prop);
   Check3 (aOffsets.size ());
   Check3 (getTypeIndex (aOffsets[actEntry]) != -1);
   Check3 ((sizeof (values) / sizeof (values[0]))
            > getTypeIndex (aOffsets[actEntry]));
   (prop->*(values[getTypeIndex (aOffsets[actEntry])])) = pInfo;

   unsigned int off (aOffsets.begin ()->first);
   aOffsets.erase (aOffsets.begin ());

   if (aOffsets.size ()) {
      actEntry = aOffsets.begin ()->first;
      off = aOffsets.begin ()->first - off - len - 4;
      skip.setOffset (off);
      TRACE7 ("ParseWord::foundInformation (const char*) - Skipping " << off
              << " bytes for next entry");
   }

   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the header of the properthies has been read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseWord::foundPropertiesHeader (const char*, unsigned int) {
   TRACE1 ("ParseWord::foundPropertiesHeader (const char*) - Bytes read: "
           << cRead << " (0x" << std::hex << cRead << std::dec << ')');

   // Check if there are any of the supported types in the document
   if (aOffsets.size ()) {
      actEntry = aOffsets.begin ()->first;
      seqInformation.setMaxCard (aOffsets.size ());
      skip.setOffset (aOffsets.begin ()->first - cRead - 4); Check3 (skip.getOffset () > 0);
   }
   else
      _wordDoc[16] = NULL;

   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Retrieves the index of the passed type
/// \param type: Type to inspect
/// \returns \c unsigned int: Offset; -1 if type is not valid
//-----------------------------------------------------------------------------
int ParseWord::getTypeIndex (unsigned int type) {
   for (unsigned int i (0); i < (sizeof (aTypes) / sizeof (aTypes[0])); ++i)
      if (type == aTypes[i])
         return i;

   return -1;
}
