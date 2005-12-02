//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseJPEG
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 17.10.2002
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

// Note TRACELEVEL 9 is not recomended as the title very likely contains
// special characters!
#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "ParseJPG.h"
#include "Properties.h"

#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#endif

#ifdef WORDS_BIGENDIAN
#  define TYPE_TITLE     0x9b9c0001
#  define TYPE_COMMENT   0x9c9c0001
#  define TYPE_AUTHOR    0x9d9c0001

#  define TYPE_TITLE2    0x1c026900
#  define TYPE_COMMENT2  0x1c027800
#  define TYPE_AUTHOR2   0x1c026e00

#  define ENTRY_BLOCK    0x3842494d
#  define ENTRY_TYPE     0x6e000000

inline unsigned short get2BytesLSB (const char* pAddr) {
   return ((unsigned char)(*pAddr) << 8) + (unsigned char)pAddr[1];
}

inline unsigned short get2BytesMSB (const char* pAddr) {
   return *(unsigned short*)pAddr;
}

inline unsigned long get4BytesLSB (const char* pAddr) {
   return (((unsigned char)(*pAddr) << 24) + ((unsigned char)pAddr[1] << 16)
           + ((unsigned char)pAddr[2] << 8) + ((unsigned char)pAddr[3]));
}

inline unsigned long get4BytesMSB (const char* pAddr) {
   return *(unsigned char*)pAddr;
}

#else
#  define TYPE_TITLE     0x019c9b
#  define TYPE_COMMENT   0x019c9c
#  define TYPE_AUTHOR    0x019c9d

#  define TYPE_TITLE2    0x69021c
#  define TYPE_COMMENT2  0x78021c
#  define TYPE_AUTHOR2   0x6e021c

#  define ENTRY_BLOCK    0x4d494238
#  define ENTRY_TYPE     0x6e

inline unsigned short get2BytesLSB (const char* pAddr) {
   return *(unsigned short*)pAddr;
}

inline unsigned short get2BytesMSB (const char* pAddr) {
   return ((unsigned char)(*pAddr) << 8) + (unsigned char)pAddr[1];
}

inline unsigned long get4BytesLSB (const char* pAddr) {
   return *(unsigned int*)pAddr;
}

inline unsigned long get4BytesMSB (const char* pAddr) {
   return (((unsigned char)(*pAddr) << 24) + ((unsigned char)pAddr[1] << 16)
           + ((unsigned char)pAddr[2] << 8) + ((unsigned char)pAddr[3]));
}

#endif


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
ParseJPEG::ParseJPEG ()
   : idJPEG ("\xff\xd8", _("JPEG-ID"), false),
     idEndJPEG ("\xff\xd9", _("JPEG-Terminator"), *this, &ParseJPEG::foundEndOfJPEG, false),
     idAPP1 ("\xff\xe1", _("APP1 marker"), *this, &ParseJPEG::foundAPP1, false),
     idAPPD ("\xff\xed", _("APPD maker"), false),
     idComment ("\xff\xfe", _("Comment marker"), false),
     idMarker ("\xff\\*", _("Other marker"), 2, 2, false),
     image ("\\*", _("Image information"), *this, &ParseJPEG::foundImage, 1, 1, false),

     title ("\\*", _("Comment"), *this, &ParseJPEG::foundTitle, 1, 1, false),
     titlePhotoshop ("\\*", _("Comment values (Photoshop style)"), *this, &ParseJPEG::foundCommentPhotoShop, 1, 1, false),
     type ("\\*", _("Type of entry"), *this, &ParseJPEG::foundType, 4, 4, false),
     number ("\\*", _("Number of records"), *this, &ParseJPEG::foundNumber, 2, 2, false),
     lengthLSB ("\\*", _("Length (LSB first)"), *this, &ParseJPEG::foundLengthLSB, 2, 2, false),
     lengthMSB ("\\*", _("Length (MSB first)"), *this, &ParseJPEG::foundLengthMSB, 2, 2, false),
     lengthLSB4 ("\\*", _("Length (long - LSB first)"), *this, &ParseJPEG::foundLengthLSB4, 4, 4, false),
     lengthMSB4 ("\\*", _("Length (long - MSB first)"), *this, &ParseJPEG::foundLengthMSB4, 4, 4, false),
     offset ("\\*", _("Offset"), *this, &ParseJPEG::foundOffset, 4, 4, false),
     byteOrder ("\\*", _("Byte order"), *this, &ParseJPEG::foundByteOrder, 2, 2, false),
     skip6 (6),
     skipLen (0),

     selMarker (_selMarker, _("Marker"), -1U, 1, false),
     seqComment (_seqComment, _("Comment entry 1"), 1, 1, false),
     seqEXIF (_seqEXIF, _("APP1 (EXIF)"), *this, &ParseJPEG::foundAPP1Exif, 1, 1, false),
     seqAPPD (_seqAPPD, _("APPD entry"), 1, 1, false),
     seqOther (_seqOther, _("Other entry"), 1, 1, false),

     seqIFD (_seqIFD, _("Image file directory"), 1, 1, false),
     jpegImage (_jpegImage, _("JPEG image"), 1, 1),
     cRead (0), actEntry (TYPE_TITLE), cEntries (0) {

   _jpegImage[0] = &idJPEG;
   _jpegImage[1] = &selMarker;
   _jpegImage[2] = NULL;

   _selMarker[0] = &seqComment;
   _selMarker[1] = &seqEXIF;
   _selMarker[2] = &seqAPPD;
   _selMarker[3] = &seqOther;
   _selMarker[4] = &idEndJPEG;
   _selMarker[5] = &image;
   _selMarker[6] = NULL;

   _seqComment[0] = &idComment;
   _seqComment[1] = &lengthMSB;
   _seqComment[2] = &title;
   _seqComment[3] = NULL;

   _seqEXIF[0] = &idAPP1;
   _seqEXIF[1] = &lengthMSB;
   _seqEXIF[2] = &skip6;
   _seqEXIF[3] = &byteOrder;
   _seqEXIF[4] = &skip6;
   _seqEXIF[5] = &number;
   _seqEXIF[6] = &seqIFD;
   _seqEXIF[7] = &title;
   _seqEXIF[8] = &skipLen;
   _seqEXIF[9] = NULL;

   _seqAPPD[0] = &idAPPD;
   _seqAPPD[1] = &lengthLSB;
   _seqAPPD[2] = &titlePhotoshop;
   _seqAPPD[3] = NULL;

   _seqOther[0] = &idMarker;
   _seqOther[1] = &lengthMSB;
   _seqOther[2] = &skipLen;
   _seqOther[3] = NULL;

   _seqIFD[0] = &type;
   _seqIFD[1] = &lengthLSB4;
   _seqIFD[2] = &offset;
   _seqIFD[3] = NULL;

   Check3 ((sizeof (offsets) / sizeof (offsets[0]))
           == (sizeof (lengths) / sizeof (lengths[0])));
   for (unsigned int i (0);
        i < (sizeof (offsets) / sizeof (offsets[0])); ++i)
      offsets[i] = lengths[i] = 0;
}


//-----------------------------------------------------------------------------
/// Callback after a title was read
/// \param pTitle: Pointer to title
/// \param len: Length of title
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundTitle (const char* pTitle, unsigned int len) {
   Check3 (prop); Check3 (pTitle);
   TRACE9 ("ParseJPEG::foundTitle (const char*, unsigned int) - Title: "
          << std::string (pTitle, len) << " -> " << len << " chars");
   if (len) {
      TRACE8 ("ParseJPEG::foundTitle (const char*, unsigned int) - Skipping " << cRead);
      static std::string Properties::* values[] = { &Properties::strTitle,
						    &Properties::strComment,
						    &Properties::strAuthor };
      for (unsigned int i (0);
	   i < (sizeof (values) / sizeof (values[0])); ++i)
	 if (lengths[i]) {
	    TRACE7 ("ParseJPEG::foundTitle (const char*, unsigned int) - " << i
		    << ": Assigning " << offsets[i] - cRead << " - "
		    << offsets[i] - cRead + lengths[i]);
	    (prop->*(values[i])).assign (pTitle + offsets[i] - cRead, lengths[i]);
	 }
      selMarker.setMaxCard (1);
   }
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a comment entry (Photoshop stlye) was read
/// \param pTitle: Pointer to title
/// \param len: Length of title
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundCommentPhotoShop (const char* pTitle, unsigned int len) {
   Check3 (prop); Check3 (pTitle);
   TRACE9 ("ParseJPEG::foundCommentPhotoShop (const char*, unsigned int) - Title: "
           << std::string (pTitle, len) << " -> " << len << " chars");

   const char* pAct = pTitle + strlen (pTitle) + 1;    // Skip leading comment
   pTitle += len;

   TRACE8 ("ParseJPEG::foundCommentPhotoShop (const char*, unsigned int) - Header: *"
           << std::hex << (unsigned int)pAct << " = " << (*(unsigned int*)pAct) << std::dec);

   if (*(unsigned int*)pAct == ENTRY_BLOCK) {
      pAct += 6;
      pAct += (unsigned int)*pAct;

      TRACE8 ("ParseJPEG::foundCommentPhotoShop (const char*, unsigned int) - Entry: *"
              << std::hex << (unsigned int)pAct << " = " << (*(unsigned int*)pAct) << std::dec);

      if (*(unsigned int*)pAct == ENTRY_TYPE) {
         pAct += 4;
         pTitle = (unsigned int)(*pAct & 0xff) + pAct++;
         do {
            TRACE8 ("ParseJPEG::foundCommentPhotoShop (const char*, unsigned int) - Type: *"
                    << std::hex << (unsigned int)pAct << " = " << get4BytesLSB (pAct) << std::dec);

            static std::string Properties::* values[] = { &Properties::strTitle,
                                                          &Properties::strComment,
                                                          &Properties::strAuthor };
            static unsigned int aSupportedTypes[] = { TYPE_TITLE2, TYPE_COMMENT2,
                                                      TYPE_AUTHOR2 };

            for (unsigned int i (0);
                 i < (sizeof (aSupportedTypes) / sizeof (aSupportedTypes[0])); ++i) {
               if (*(unsigned int*)pAct == aSupportedTypes[i]) {
                  pAct += 4;
                  TRACE8 ("ParseJPEG::foundCommentPhotoShop (const char*, unsigned int) - " << i
                          << ": Assigning " << (unsigned int)(*pAct) << " chars = "
                          << pAct + 1);
                  (prop->*(values[i])).assign (pAct + 1, (unsigned int)*pAct);
                  pAct -= 4;
                  break;
               }
            } // end-for

            pAct += 4;
            TRACE8 ("ParseJPEG::foundCommentPhotoShop (const char*, unsigned int) - Skipping "
                    << ((unsigned int)*pAct) << " bytes");
            pAct += (unsigned int)*pAct + 1;
         } while (pAct < pTitle);
      } // end-if comment part found
   } // end-if header
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of the title was read (in LSB format)
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundLengthLSB (const char* length, unsigned int) {
   Check3 (length);
   TRACE8 ("ParseJPEG::foundLengthLSB (const char*, unsigned int): " << lengths[1]
	   << " = 0x" << std::hex << get2BytesLSB (length) << std::dec);
   foundLength (get2BytesLSB (length) - 2);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Sets the length of the title to read
/// \param length: Length of title
/// \returns \c unsigned long: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
unsigned long ParseJPEG::foundLength (unsigned int length) {
   skipLen.setOffset (lengths[1] = length);
   TRACE8 ("ParseJPEG::foundLength (unsigned int): " << length << " = 0x" << std::hex << length << std::dec);
   if (length) {
      title.setMaxCard (length);
      titlePhotoshop.setMaxCard (length);
   }
}

//-----------------------------------------------------------------------------
/// Callback after the length of the title was read (in MSB format)
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundLengthMSB (const char* length, unsigned int) {
   Check3 (length);
   TRACE8 ("ParseJPEG::foundLengthMSB (const char*, unsigned int): " << lengths[1]
	   << " = 0x" << std::hex << get2BytesMSB (length) << std::dec);
   foundLength (get2BytesMSB (length) - 2);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Checks if a supported type has been read and sets the length, if so
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
/// \pre: actEnty must have been set before
//-----------------------------------------------------------------------------
bool ParseJPEG::supportedLength4 (unsigned int length) {
   TRACE8 ("ParseJPEG::supportedLength4 (unsigned int) - " << length);
   static unsigned int aSupportedTypes[] = { TYPE_TITLE, TYPE_COMMENT, TYPE_AUTHOR };

   for (unsigned int i (0);
        i < (sizeof (aSupportedTypes) / sizeof (aSupportedTypes[0])); ++i)
      if (actEntry == aSupportedTypes[i]) {
         lengths[i] = length;
         actEntry = i;
         TRACE8 ("ParseJPEG::foundLength4 (unsigned int) - " << lengths[i]
                 << " (0x" << std::hex << lengths[i] << std::dec << ')');
	 return true;
      }
   actEntry = -1U;
   return false;
}

//-----------------------------------------------------------------------------
/// Callback after the length of the title was read (in LSB format)
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundLengthLSB4 (const char* length, unsigned int) {
   Check3 (length);
   TRACE8 ("ParseJPEG::foundLengthLSB4 (const char*, unsigned int): " << get4BytesLSB (length)
	   << " = 0x" << std::hex << get4BytesLSB (length) << std::dec);

   supportedLength4 (get4BytesLSB (length));
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of the title was read (in MSB format)
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundLengthMSB4 (const char* length, unsigned int) {
   Check3 (length);
   TRACE8 ("ParseJPEG::foundLengthMSB4 (const char*, unsigned int): " << lengths[1]
	   << " = 0x" << std::hex << get4BytesMSB (length) << std::dec);

   supportedLength4 (get4BytesMSB (length));
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number of entries has been read
/// \param nr: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundNumber (const char* nr, unsigned int) {
   Check3 (nr);
   cEntries = ((_seqIFD[1] == &lengthMSB4) ? get2BytesMSB (nr) : get2BytesLSB (nr));
   TRACE8 ("ParseJPEG::foundNumber (const char*, unsigned int): " << cEntries);

   skipLen.setOffset (4);
   seqIFD.setMaxCard (cEntries);
   seqIFD.setMinCard (cEntries);

   if (cEntries) {
      lengths[1] = 0;
      cRead = 10 + cEntries * 12;
      title.setMaxCard (title.getMaxCard () - cRead);
   }
   else {
      title.setMinCard (0);
      title.setMaxCard (0);
   }

   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the type of an entry has been parsed
/// \param pType: Pointer to found type
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundType (const char* pType, unsigned int) {
   Check3 (pType);
   actEntry = *(unsigned int*)pType;
   TRACE8 ("ParseJPEG::foundType (const char*, unsigned int) - " << std::hex << actEntry << std::dec);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the offset of an entry has been parsed
/// \param pOffset: Pointer to found offset
/// \param len: Length of data
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundOffset (const char* pOffset, unsigned int len) {
   Check3 (pOffset);
   if (actEntry != -1U) {
      offsets[actEntry] = get4BytesLSB (pOffset);
      TRACE8 ("ParseJPEG::foundOffset (const char*, unsigned int) - " << offsets[actEntry]
              << " (0x" << std::hex << offsets[actEntry] << std::dec << ')');
   }
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an end-of-JPEG marker has been parsed
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundEndOfJPEG (const char*, unsigned int) {
   TRACE8 ("ParseJPEG::foundEndOfJPEG (const char*, unsigned int)");
   selMarker.setMaxCard (1);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an APP1 marker has been found
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundAPP1 (const char*, unsigned int) {
   TRACE8 ("ParseJPEG::foundAPP1 (const char*, unsigned int)");
   skipLen.setOffset (0);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an APP1 marker has been found
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundImage (const char*, unsigned int) {
   TRACE8 ("ParseJPEG::foundImage (const char*, unsigned int)");
   selMarker.setMaxCard (1);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an APP1 marker has been found
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundByteOrder (const char* order, unsigned int) {
   TRACE8 ("ParseJPEG::foundByteOrder (const char*, unsigned int) - " << *order << order[1]);
   if ((*order == 'M') && (order[1] == 'M'))
      _seqIFD[1] = &lengthMSB4;
   else if ((*order != 'I') || (order[1] != 'I'))
      return YGP::ParseObject::PARSE_CB_ABORT;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an APP1 marker has been found
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseJPEG::foundAPP1Exif (const char*, unsigned int) {
   TRACE8 ("ParseJPEG::foundApp1Exif (const char*, unsigned int)");
   _seqIFD[1] = &lengthLSB4;
   return YGP::ParseObject::PARSE_OK;
}
