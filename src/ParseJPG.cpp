//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseJPEG
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 17.10.2002
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


// Note TRACELEVEL 9 is not recomended as the title very likely contains
// special characters!
#include <Trace_.h>

#include "ParseJPG.h"
#include "Properties.h"

#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#endif

#ifdef WORDS_BIGENDIAN
#  define TYPE_TITLE     0x9b9c0100
#  define TYPE_COMMENT   0x9c9c0100

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

inline unsigned int get4BytesLSB (const char* pAddr) {
   return (((unsigned char)(*pAddr) << 24) + ((unsigned char)pAddr[1] << 16)
           + ((unsigned char)pAddr[2] << 8) + ((unsigned char)pAddr[3]));
}

#else
#  define TYPE_TITLE     0x019c9b
#  define TYPE_COMMENT   0x019c9c

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

inline unsigned int get4BytesLSB (const char* pAddr) {
   return *(unsigned int*)pAddr;
}

#endif


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
//Parameters: pClassname: Name of class containing parser-data
/*--------------------------------------------------------------------------*/
ParseJPEG::ParseJPEG ()
   : idJPEG ("\xff\xd8", "JPEG-ID", false)
   , idFormat1 ("\xff\xe0", "ID of format 1", 2, 2, false)
   , idComment1 ("\xff\xfe", "ID of short comments", false)
   , idComment2 ("\xff\xe1", "ID of long comments", false)
   , idComment3 ("\xff\xed", "ID of XXL comments", false)
   , title ("\\*", "Comment", *this, &ParseJPEG::foundTitle, 1, 1, false)
   , title3 ("\\*", "Comment values", *this, &ParseJPEG::foundTitle3, 1, 1, false)
   , type ("\\*", "Type of entry", *this, &ParseJPEG::foundType, 4, 4, false)
   , number ("\\*", "Number of records", *this, &ParseJPEG::foundNumber, 2, 2, false)
   , length1 ("\\*", "Length (MSB first)", *this, &ParseJPEG::foundLength, 2, 2, false)
   , length2 ("\\*", "Length (LSB first)", *this, &ParseJPEG::foundLength2, 2, 2, false)
   , offset ("\\*", "Offset", *this, &ParseJPEG::foundOffset, 4, 4, false)
   , skip ("\\*", "Skipping chars", 16, false, false)
   , ignore ("\xff", "Ignore til special", 512, true, false)
   , selFormat (_selFormat, "Possible comments", 1, 0, false)
   , seqFormat1 (_seqFormat1, "Format style 1", 1, 1, false)
   , selProperties (_selProperties, "Properties", 1, 0, false)
   , seqPropShort (_seqPropShort, "Short properties", 1, 1, false)
   , seqPropLong (_seqPropLong, "Long properties", 1, 1, false)
   , seqPropXXL (_seqPropXXL, "XXL properties", 1, 1, false)
   , seqEntries (_seqEntries, "List of property entries", *this,
                 &ParseJPEG::foundPropertiesHeader, 1, 1, false)
   , jpegImage (_jpegImage, "JPEG image", 1, 1)
   , cRead (0), actEntry (TYPE_TITLE), cEntries (0) {

   _jpegImage[0] = &idJPEG;
   _jpegImage[1] = &selFormat;
   _jpegImage[2] = NULL;

   _selFormat[0] = &seqFormat1;
   _selFormat[1] = &seqPropLong;
   _selFormat[2] = NULL;

   _seqFormat1[0] = &idFormat1;
   _seqFormat1[1] = &skip;
   _seqFormat1[2] = &selProperties;
   _seqFormat1[3] = NULL;

   _selProperties[0] = &seqPropShort;
   _selProperties[1] = &seqPropLong;
   _selProperties[2] = &seqPropXXL;
   _selProperties[3] = NULL;

   _seqPropShort[0] = &idComment1;
   _seqPropShort[1] = &length1;
   _seqPropShort[2] = &title;
   _seqPropShort[3] = NULL;

   _seqPropLong[0] = &idComment2;
   _seqPropLong[1] = &length1;
   _seqPropLong[2] = &skip;
   _seqPropLong[3] = &number;
   _seqPropLong[4] = &seqEntries;
   _seqPropLong[5] = &skip;
   _seqPropLong[6] = &title;
   _seqPropLong[7] = NULL;

   _seqPropXXL[0] = &idComment3;
   _seqPropXXL[1] = &length1;
   _seqPropXXL[2] = &title3;
   _seqPropXXL[3] = NULL;

   _seqEntries[0] = &type;
   _seqEntries[1] = &length2;
   _seqEntries[2] = &offset;
   _seqEntries[3] = NULL;

   assert ((sizeof (offsets) / sizeof (offsets[0]))
           == (sizeof (lengths) / sizeof (lengths[0])));
   for (unsigned int i (0);
        i < (sizeof (offsets) / sizeof (offsets[0])); ++i)
      offsets[i] = lengths[i] = 0;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a title was read
//Parameters: pTitle: Pointer to title
//            len: Length of title
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundTitle (const char* pTitle, unsigned int len) {
   assert (prop); assert (pTitle);
   TRACE9 ("ParseJPEG::foundTitle (const char*, unsigned int) - Title: "
          << string (pTitle, len) << " -> " << len << " chars");

   static string Properties::* values[] = { &Properties::strTitle,
                                            &Properties::strComment };
   for (unsigned int i (0);
        i < (sizeof (lengths) / sizeof (lengths[0])); ++i)
      if (lengths[i]) {
         TRACE8 ("ParseJPEG::foundTitle (const char*, unsigned int) - " << i
                 << ": Assigning from " << (offsets[i] - cRead) << ' '
                 << lengths[i] << " chars");
         (prop->*(values[i])).assign (pTitle + offsets[i] - cRead, lengths[i] - 2);
      }

   TRACE9 ("ParseJPEG::foundTitle (const char*, unsigned int) - Out");
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a comment entry (Photoshop stlye) was read
//Parameters: pTitle: Pointer to title
//            len: Length of title
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundTitle3 (const char* pTitle, unsigned int len) {
   assert (prop); assert (pTitle);
   TRACE9 ("ParseJPEG::foundTitle3 (const char*, unsigned int) - Title: "
           << string (pTitle, len) << " -> " << len << " chars");

   const char* pAct = pTitle + strlen (pTitle) + 1;    // Skip leading comment
   pTitle += len;

   TRACE8 ("ParseJPEG::foundTitle3 (const char*, unsigned int) - Header: *"
           << hex << (unsigned int)pAct << " = " << (*(unsigned int*)pAct) << dec);

   if (*(unsigned int*)pAct == ENTRY_BLOCK) {
      pAct += 6;
      pAct += (unsigned int)*pAct;

      TRACE8 ("ParseJPEG::foundTitle3 (const char*, unsigned int) - Entry: *"
              << hex << (unsigned int)pAct << " = " << (*(unsigned int*)pAct) << dec);

      if (*(unsigned int*)pAct == ENTRY_TYPE) {
         pAct += 4;
         pTitle = pAct + (unsigned int)*pAct++;
         do {
            TRACE8 ("ParseJPEG::foundTitle3 (const char*, unsigned int) - Type: *"
                    << hex << (unsigned int)pAct << " = " << get4BytesLSB (pAct) << dec);

            static string Properties::* values[] = { &Properties::strTitle,
                                                     &Properties::strComment,
                                                     &Properties::strAuthor };
            static unsigned int aSupportedTypes[] = { TYPE_TITLE2, TYPE_COMMENT2,
                                                      TYPE_AUTHOR2 };

            for (unsigned int i (0);
                 i < (sizeof (aSupportedTypes) / sizeof (aSupportedTypes[0])); ++i) {
               if (*(unsigned int*)pAct == aSupportedTypes[i]) {
                  pAct += 4;
                  TRACE8 ("ParseJPEG::foundTitle3 (const char*, unsigned int) - " << i
                          << ": Assigning " << (unsigned int)(*pAct) << " chars = "
                          << pAct + 1);
                  (prop->*(values[i])).assign (pAct + 1, (unsigned int)*pAct);
                  pAct -= 4;
                  break;
               }
            } // end-for

            pAct += 4;
            TRACE8 ("ParseJPEG::foundTitle3 (const char*, unsigned int) - Skipping "
                    << ((unsigned int)*pAct) << " bytes");
            pAct += (unsigned int)*pAct + 1;
         } while (pAct < pTitle);
      } // end-if comment part found
   } // end-if header
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the length of the title was read
//Parameters: length: Pointer to length
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundLength (const char* length, unsigned int) {
   assert (length);
   lengths[1] = get2BytesMSB (length);
   TRACE8 ("ParseJPEG::foundLength (const char*, unsigned int): " << lengths[1]);
   if (lengths[1]) {
      title.setMaxCard (lengths[1]);
      title3.setMaxCard (lengths[1]);
   }

   skip.setMaxCard (14);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the length of the title was read
//Parameters: length: Pointer to length
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundLength2 (const char* length, unsigned int) {
   assert (length);

   static unsigned int aSupportedTypes[] = { TYPE_TITLE, TYPE_COMMENT };
   for (unsigned int i (0);
        i < (sizeof (lengths) / sizeof (lengths[0])); ++i)
      if (actEntry == aSupportedTypes[i]) {
         lengths[i] = get2BytesLSB (length);
         TRACE9 ("ParseJPEG::foundLength2 (const char*) - " << lengths[i]
                 << " (0x" << hex << lengths[i] << dec << ')');
         actEntry = i;
         return ParseObject::PARSE_OK;
      }

   actEntry = -1U;
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the number of entries has been read
//Parameters: nr: Pointer to number of entries
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundNumber (const char* nr, unsigned int) {
   assert (nr);
   seqEntries.setMaxCard (cEntries = (get2BytesLSB (nr)));
   seqEntries.setMinCard (cEntries);
   length2.setMaxCard (4);
   TRACE8 ("ParseJPEG::foundNumber (const char*, unsigned int): " << cEntries);
   lengths[1] = 0;
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the type of an entry has been parsed
//Parameters: pType: Pointer to found type
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundType (const char* pType, unsigned int) {
   assert (pType);
   actEntry = *(unsigned int*)pType;
   TRACE9 ("ParseJPEG::foundType (const char*) - " << hex << actEntry << dec);
   cRead += 12;
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the offset of an entry has been parsed
//Parameters: pOffset: Pointer to found offset
//            len: Length of data
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundOffset (const char* pOffset, unsigned int len) {
   assert (pOffset);
   if (actEntry != -1U) {
      offsets[actEntry] = get4BytesLSB (pOffset);
      TRACE9 ("ParseJPEG::foundOffset (const char*) - " << offsets[actEntry]
              << " (0x" << hex << offsets[actEntry] << dec << ')');
   }
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the header of the properthies has been read
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundPropertiesHeader (const char*, unsigned int) {
   TRACE1 ("ParseJPEG::foundPropertiesHeader (const char*) - Bytes read: "
           << cRead << " (0x" << hex << cRead << dec << ')');

   skip.setMinCard (4);
   skip.setMaxCard (4);
   cRead += 14;

   TRACE8 ("ParseJPEG::foundPropertiesHeader (const char*) - Title length: "
           << title.getMaxCard () - cRead - 8);
   if (title.getMaxCard () <= (cRead + 8))
      _seqPropLong[6] = NULL;
   else {
      assert (title.getMaxCard () > (cRead + 8));
      title.setMaxCard (title.getMaxCard () - cRead - 8);
   }
   return ParseObject::PARSE_OK;
}
