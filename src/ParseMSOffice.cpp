//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseMSOffice
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 8.10.2002
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

#define CHECK 9
#define TRACELEVEL 9
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Utility.h"
#include "ParseMSOffice.h"
#include "Properties.h"


static const char SUMMARY[] = ("\x05\0\x53\0\x75\0\x6d\0\x6d\0\x61\0\x72\0\x79\0"
			       "\x49\0\x6E\0\x66\0\x6F\0\x72\0\x6D\0\x61\0\x74\0"
			       "\x69\0\x6F\0\x6E\0\0");
static const char ROOTENTRY[] = "R\0o\0o\0t\0 \0E\0n\0t\0r\0y\0\0";
static const char CLASSID[] = { '\x01', '\0', '\0', '\0', '\xE0', '\x85', '\x9F', '\xF2', '\xF9', '\x4F',
				'\x68', '\x10', '\xAB', '\x91', '\x08', '\0', '\x2B', '\x27', '\xB3', '\xD9' };


static const unsigned LEN_CONTENT     = 1024;


#ifdef MSOFFICES_BIGENDIAN
   static const unsigned int TYPE_TITLE   = 0x2000000;
   static const unsigned int TYPE_AUTHOR  = 0x4000000;
   static const unsigned int TYPE_COMMENT = 0x6000000;

#else
   static const unsigned int TYPE_TITLE   = 2;
   static const unsigned int TYPE_AUTHOR  = 4;
   static const unsigned int TYPE_COMMENT = 6;

#endif


static const unsigned int aTypes[] = { TYPE_TITLE, TYPE_AUTHOR, TYPE_COMMENT };


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
ParseMSOffice::ParseMSOffice()
   : poifsID ("\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1", _("Magic number")),
     skipBeg (22, std::ios_base::beg),
     skip (22),
     sizeBlock  ("\\*", _("Size of blocks"), *this, &ParseMSOffice::foundBlockSize, 4, 4, false),
     blockIndex  ("\\*", _("Index of block"), *this, &ParseMSOffice::foundBlockIndex, 4, 4, false),
     nameProperty ("\\*", _("Name of property"), *this, &ParseMSOffice::foundNameProperty, 0x40, 0x40, false),
     lenNameProperty ("\\*", _("Length of name of property"), *this, &ParseMSOffice::foundLenNameProperty, 2, 2, false),
     idPropStream ("\xfe\xff", _("ID of property set stream"), 2, 2, false),
     idClass (CLASSID, _("Class ID"), sizeof (CLASSID), sizeof (CLASSID), false),
     sectionOffset  ("\\*", _("Offset of section"), *this, &ParseMSOffice::foundSectionOffset, 4, 4, false),
     length ("\\*", _("Length"), *this, &ParseMSOffice::foundLength, 4, 4, false),
     nrEntries ("\\*", _("Number of entries"), *this, &ParseMSOffice::foundNrEntries, 4, 4, false),
     type ("\\*", _("Type of entry"), *this, &ParseMSOffice::foundType, 4, 4, false),
     offset  ("\\*", _("Offset"), *this, &ParseMSOffice::foundOffset, 4, 4, false),
     information ("\0", _("Information"), *this, &ParseMSOffice::foundInformation, 1, 1, false),
     beginSBA   ("\\*", _("Index of SBA"), *this, &ParseMSOffice::foundBeginSBA, 4, 4, false),
     countBAT ("\\*", _("Number of elements in BAT array"), *this, &ParseMSOffice::foundCountBAT, 4, 4, false),
     countSBAT ("\\*", _("Blocks of SBAT array"), *this, &ParseMSOffice::foundCountSBAT, 4, 4, false),
     countXBAT ("\\*", _("Blocks of XBAT array"), *this, &ParseMSOffice::foundCountXBAT, 4, 4, false),
     blockBAT  ("\\*", _("Index of of BAT"), *this, &ParseMSOffice::foundBlockBAT, 4, 4, false),
     blockSBAT  ("\\*", _("Index of of SBAT"), *this, &ParseMSOffice::foundBlockSBAT, 4, 4, false),
     blockXBAT  ("\\*", _("Index of of XBAT"), *this, &ParseMSOffice::foundBlockXBAT, 4, 4, false),
     block ("\\*", _("block"), *this, &ParseMSOffice::foundBlock, 512, 512, false),
     sizeFile ("\\*", _("Filesize"), *this, &ParseMSOffice::foundFileSize, 4, 4, false),

     officedoc (_officedoc, _("Office document"), 1, 1),
     seqProperties (_seqProperties, _("Properties"), -1U, 1, false),
     seqInformation (_seqInformation, _("Information summary"), 1, 0, false),
     seqEntries (_seqEntries, _("Entry description"), *this,
		 &ParseMSOffice::foundPropertiesHeader, 1, 1, false),
     seqBAT (_seqBAT, _("BAT"), *this, &ParseMSOffice::foundBAT, 1, 1),
     cRead (0), cEntries (0), actEntry (-1U), len (0), pBAT (NULL), cBAT (0),
     pSBAT (NULL), cSBAT (0), pXBAT (NULL), cXBAT (0), prop (NULL) {

   // Property table
   _seqProperties[0] = &nameProperty;
   _seqProperties[1] = &lenNameProperty;
   _seqProperties[2] = &skip;
   _seqProperties[3] = &blockIndex;
   _seqProperties[4] = &sizeFile;
   _seqProperties[5] = &skip;
   _seqProperties[6] = NULL;

   _seqBAT[0] = &blockIndex;
   _seqBAT[1] = &skipBeg;
   _seqBAT[2] = &block;
   _seqBAT[3] = NULL;

   _seqEntries[0] = &type;
   _seqEntries[1] = &skip;
   _seqEntries[2] = NULL;

   _seqInformation[0] = &length;
   _seqInformation[1] = &information;
   _seqInformation[2] = &skip;
   _seqInformation[3] = NULL;

   _officedoc[0] = &poifsID;
   _officedoc[1] = &skip;
   _officedoc[2] = &sizeBlock;
   _officedoc[3] = &skip;
   _officedoc[4] = &countBAT;
   _officedoc[5] = &skipBeg;
   _officedoc[6] = &seqBAT;
   _officedoc[7] = &skipBeg;
   _officedoc[8] = &blockIndex;
   _officedoc[9] = &skipBeg;
   _officedoc[10] = &seqProperties;
   _officedoc[11] = &skipBeg;
   _officedoc[12] = &idPropStream;
   _officedoc[13] = &skip;
   _officedoc[14] = &idClass;
   _officedoc[15] = &sectionOffset;
   _officedoc[16] = &length;
   _officedoc[17] = &nrEntries;
   _officedoc[18] = &seqEntries;
   _officedoc[19] = &skip;
   _officedoc[20] = &seqInformation;
   _officedoc[21] = NULL;
}


//-----------------------------------------------------------------------------
/// Callback after the sizes of blocks has been parsed
/// \param pBlockSize: Pointer to sizes
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundBlockSize (const char* pBlockSize, unsigned int) {
   Check3 (pBlockSize);
   blockSize = get2BytesLSB (pBlockSize);
   blockSizeSmall = get2BytesLSB (pBlockSize + 2);
   TRACE9 ("ParseMSOffice::foundBlockSize (const char*, unsigned int) - " << (1 << blockSize) << '/' << (1 << blockSizeSmall));
   skip.setOffset (10);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the index of a block has been parsed
/// \param pBlockID: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundBlockIndex (const char* pBlockID, unsigned int) {
   Check3 (pBlockID);
   skipBeg.setOffset ((get4BytesLSB (pBlockID) + 1) << blockSize);
   skip.setOffset (0x16);
   TRACE5 ("ParseMSOffice::foundBlockIndex (const char*, unsigned int) - " << get4BytesLSB (pBlockID) << " -> " << skipBeg.getOffset ());
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the begin of the "small block area"  has been parsed
/// \param pBlockID: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundBeginSBA (const char* pBlockID, unsigned int) {
   Check3 (pBlockID);
   indexSBA = get4BytesLSB (pBlockID);
   skip.setOffset (4);
   TRACE5 ("ParseMSOffice::foundBeginSBA (const char*, unsigned int) - "
	   << std::hex << indexSBA << " -> 0x" << ((indexSBA + 1) << blockSize) << std::dec);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the name of a property has been parsed
/// \param pName: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundNameProperty (const char* pName, unsigned int len) {
   Check2 (pName); Check2 (len <= sizeof (propertyName));
   memcpy (propertyName, pName, sizeof (propertyName));
   TRACE3 ("ParseMSOffice::foundNameProperty (const char*, unsigned int) - " << pName << pName[2] << pName[4] << pName[6] << pName[8] << pName[10]);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of the name of a property has been parsed
/// \param pLen: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundLenNameProperty (const char* pLen, unsigned int) {
   unsigned int length (get2BytesLSB (pLen));
   TRACE7 ("ParseMSOffice::foundLenNameProperty (const char*, unsigned int) - " << length);

   if (length > 0x40)
      return YGP::ParseObject::PARSE_CB_ABORT;

   if (!length) {
      TRACE9 ("ParseMSOffice::foundLenNameProperty (const char*, unsigned int) - End");
      seqProperties.setMinCard (0);
      seqProperties.setMaxCard (0);
      _officedoc[7] = NULL;
   }
   else {
      Check3 (sizeof (SUMMARY) == 0x28);
      Check3 (sizeof (ROOTENTRY) == 0x16);

      if ((length == 0x28) && !memcmp (propertyName, SUMMARY, 0x28)) {
	 TRACE9 ("ParseMSOffice::foundLenNameProperty (const char*, unsigned int) - SI");
	 seqProperties.setMinCard (0);
	 seqProperties.setMaxCard (0);
	 skip.setOffset (0x32);
	 _seqProperties[3] = &blockIndex;
      }
      else if ((length == 0x16) && !memcmp (propertyName, ROOTENTRY, 0x16)) {
	 TRACE9 ("ParseMSOffice::foundLenNameProperty (const char*, unsigned int) - RE");
	 skip.setOffset (0x32);
	 _seqProperties[3] = &beginSBA;
      }
      else {
	 _seqProperties[3] = NULL;
	 skip.setOffset (0x3e);
      }
   }
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number of BAT array entries has been parsed
/// \param count: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundCountBAT (const char* count, unsigned int) {
   TRACE8 ("ParseMSOffice::foundCountBAT (const char*, unsigned int) - " << get4BytesLSB (count));
   if ((cBAT = get4BytesLSB (count)) > 109)
      return YGP::ParseObject::PARSE_CB_ABORT;

   seqBAT.setMinCard (cBAT);
   seqBAT.setMaxCard (cBAT);
   skipBeg.setOffset (0x4c);

   pBAT = new unsigned int [(cBAT << blockSize) >> 2];
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the size of the file has been parsed
/// \param size: Pointer to size
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundFileSize  (const char* size, unsigned int) {
   TRACE8 ("ParseMSOffice::foundFileSize (const char*, unsigned int) - " << get4BytesLSB (size));
   if (get4BytesLSB (size) < 4096) {
      unsigned int cBlocks (1 << (blockSize - blockSizeSmall));
      unsigned int block ((skipBeg.getOffset () >> blockSize) - 1);
      unsigned int index (indexSBA);
      while (block > cBlocks) {
	 TRACE9  ("ParseMSOffice::foundFileSize (const char*, unsigned int) - Missing blocks " << block);
	 Check3 (index < cBAT);
	 TRACE9  ("ParseMSOffice::foundFileSize (const char*, unsigned int) - Block " << index
		  << " -> " << pBAT[index]);
	 index = pBAT[index]; Check3 (pBAT[index] > 0);
	 block -= cBlocks;
      } // end-while

      skipBeg.setOffset (((index + 1) << blockSize) + (block << blockSizeSmall));
      TRACE9 ("ParseMSOffice::foundFileSize (const char*, unsigned int) - Skip to " << std::hex << skipBeg.getOffset () << std::dec);
   }
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number of SBAT blocks has been parsed
/// \param cBlocks: Pointer to number of SBAT blocks
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundCountSBAT (const char* cBlocks, unsigned int) {
   TRACE9 ("ParseMSOffice::foundCountSBAT (const char*, unsigned int) - " << get4BytesLSB (cBlocks));
   if (get4BytesLSB (cBlocks) > 20000)
      return YGP::ParseObject::PARSE_CB_ABORT;

   pSBAT = new unsigned int [get4BytesLSB (cBlocks)];
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number of XBAT blocks has been parsed
/// \param cBlocks: Pointer to number of SBAT blocks
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundCountXBAT (const char* cBlocks, unsigned int) {
   TRACE9 ("ParseMSOffice::foundCountXBAT (const char*, unsigned int) - " << get4BytesLSB (cBlocks));
   if (get4BytesLSB (cBlocks) > 20000)
      return YGP::ParseObject::PARSE_CB_ABORT;
   pXBAT = new unsigned int [get4BytesLSB (cBlocks)];
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the BAT has been read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundBAT (const char*, unsigned int) {
   skipBeg.setOffset (0x30);
   return YGP::ParseObject::PARSE_OK;
}

int ParseMSOffice::foundBlockBAT (const char*, unsigned int) {
   return YGP::ParseObject::PARSE_OK;
}

int ParseMSOffice::foundBlockSBAT (const char*, unsigned int) {
   return YGP::ParseObject::PARSE_OK;
}

int ParseMSOffice::foundBlockXBAT (const char*, unsigned int) {
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a block was found
/// \param count: Pointer to block
/// \param len: Length of block
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundBlock (const char* block, unsigned int len) {
   TRACE9 ("ParseMSOffice::foundBlock (const char*, unsigned int) - " << len);
   Check2 (len == 512); Check3 (pBAT);

   for (unsigned int i (0); i < (len >> 2); ++i) {
      pBAT[i] = get4BytesLSB (block + (i << 2));
      if (pBAT[i] < 0x80000000)
	 TRACE9 ("Block " << std::hex << i << ": " << pBAT[i] << std::dec);
   }
   cBAT += len >> 2;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the offset of the section-entry was found
/// \param offset: Pointer to offset
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundSectionOffset (const char* offset, unsigned int) {
   Check3 (offset);

   // Set offset to section (but substract section-offset)
   skip.setOffset (get4BytesLSB (offset) - 0x30);
   TRACE9 ("ParseMSOffice::foundSectionOffset (const char*) - " << skip.getOffset () << " (0x"
           << std::hex << skip.getOffset () << std::dec << ')');
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number of entries has been parsed
/// \param pEntries: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundNrEntries (const char* pEntries, unsigned int) {
   Check3 (pEntries);
   cEntries = get4BytesLSB (pEntries);
   TRACE4 ("ParseMSOffice::foundNrEntries (const char*) - Entries: " << cEntries);

   seqEntries.setMaxCard (cEntries);
   skip.setOffset (4);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number of entries has been parsed
/// \param pType: Pointer to found type
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundType (const char* pType, unsigned int) {
   Check3 (pType);
   actEntry = *(unsigned int*)pType;
   TRACE9 ("ParseMSOffice::foundType (const char*) - Type: " << actEntry);

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
int ParseMSOffice::foundOffset (const char* offset, unsigned int) {
   Check3 (offset);
   Check3 (getTypeIndex (actEntry) != -1);

   unsigned int off (get4BytesLSB (offset));
   TRACE9 ("ParseMSOffice::foundOffset (const char*) - " << off << " (0x"
           << std::hex << off << std::dec << ')');

   aOffsets[off] = actEntry;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of an information was read
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundLength (const char* length, unsigned int) {
   Check3 (length);
   if ((len = get4BytesLSB (length)))
      information.setMaxCard (len);
   TRACE8 ("ParseMSOffice::foundLength (const char*, unsigned int): " << len);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an information was read
/// \param pInfo: Pointer to information
/// \param len: Length of inforamtion
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundInformation (const char* pInfo, unsigned int len) {
   Check3 (pInfo);
   Check3 (aOffsets[actEntry] < 1000);

   TRACE1 ("ParseMSOffice::foundInformation (const char*, unsigned int): " << pInfo
           << " (" << len << " bytes); Entries: " << seqInformation.getMaxCard ());

   static std::string Properties::* values[] =
      { &Properties::strTitle, &Properties::strAuthor, &Properties::strComment };

   Check3 (prop);
   Check3 (aOffsets.size ());
   Check3 (getTypeIndex (aOffsets[actEntry]) != -1);
   Check3 ((sizeof (values) / sizeof (values[0]))
            > (unsigned int)getTypeIndex (aOffsets[actEntry]));
   (prop->*(values[getTypeIndex (aOffsets[actEntry])])) = pInfo;

   unsigned int off (aOffsets.begin ()->first);
   aOffsets.erase (aOffsets.begin ());

   if (aOffsets.size ()) {
      actEntry = aOffsets.begin ()->first;
      off = aOffsets.begin ()->first - off - len - 4;
      skip.setOffset (off);
      TRACE7 ("ParseMSOffice::foundInformation (const char*) - Skipping " << off
              << " bytes for next entry");
   }

   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the header of the properthies has been read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseMSOffice::foundPropertiesHeader (const char*, unsigned int) {
   TRACE1 ("ParseMSOffice::foundPropertiesHeader (const char*) - Bytes read: "
           << cRead << " (0x" << std::hex << cRead << std::dec << ')');

   // Check if there are any of the supported types in the document
   if (aOffsets.size ()) {
      actEntry = aOffsets.begin ()->first;
      seqInformation.setMaxCard (aOffsets.size ());
      skip.setOffset (aOffsets.begin ()->first - cRead - 4); Check3 (skip.getOffset () > 0);
   }
   else
      _officedoc[16] = NULL;

   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Retrieves the index of the passed type
/// \param type: Type to inspect
/// \returns \c unsigned int: Offset; -1 if type is not valid
//-----------------------------------------------------------------------------
int ParseMSOffice::getTypeIndex (unsigned int type) {
   for (unsigned int i (0); i < (sizeof (aTypes) / sizeof (aTypes[0])); ++i)
      if (type == aTypes[i])
         return i;

   return -1;
}
