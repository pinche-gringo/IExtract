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

#include <Trace_.h>

#include "ParseJPG.h"
#include "Properties.h"


#define TYPE_TITLE    0x019b9c
#define TYPE_COMMENT  0x019c9c

unsigned int ParseJPEG::aSupportedTypes[] = { TYPE_TITLE, TYPE_COMMENT };


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
//Parameters: pClassname: Name of class containing parser-data
/*--------------------------------------------------------------------------*/
ParseJPEG::ParseJPEG ()
   : idJPEG ("\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49\x46\x00\x01", "JPEG-ID", 12, 12, false)
   , idComment2 ("Exif\0\0", "Tag of comment style 2", 6, 6, false)
   , tagComment1 ("\xff\xfe", "ID of comment style 1", 2, 2, false)
   , tagComment2 ("\xff\xe1", "ID of comment style 2", 2, 2, false)
   , number ("\\*", "Number of records", *this, &ParseJPEG::foundNumber, 2, 2, false)
   , length1 ("\\*", "Length", *this, &ParseJPEG::foundLength, 2, 2, false)
   , length2 ("\\*", "Length", *this, &ParseJPEG::foundLength2, 2, 2, false)
   , type ("\\*", "Type of entry", *this, &ParseJPEG::foundType, 4, 4, false)
   , title ("\\*", "Comment", *this, &ParseJPEG::foundTitle, 1, 1, false)
   , offset("\\*", "Offset", *this, &ParseJPEG::foundOffset, 4, 4, false)
   , ignore ("\\*", "Unused information", 8, 8, false)
   , selComment (_selComment, "Possible comments", 1, 0, false)
   , seqComment1 (_seqComment1, "Comment style 1", 1, 1, false)
   , seqComment2 (_seqComment2, "Comment style 2", 1, 1, false)
   , seqEntries (_seqEntries, "List of property entries", *this,
                 &ParseJPEG::foundPropertiesHeader, 1, 1, false)
   , jpegImage (_jpegImage, "JPEG image", 1, 1)
   , cRead (0), actEntry (TYPE_TITLE), cEntries (0) {

   _jpegImage[0] = &idJPEG;
   _jpegImage[1] = &ignore;
   _jpegImage[2] = &selComment;
   _jpegImage[3] = NULL;

   _selComment[0] = &seqComment1;
   _selComment[1] = &seqComment2;
   _selComment[2] = NULL;

   _seqComment1[0] = &tagComment1;
   _seqComment1[1] = &length1;
   _seqComment1[2] = &title;
   _seqComment1[3] = NULL;

   _seqComment2[0] = &tagComment2;
   _seqComment2[1] = &length1;
   _seqComment2[2] = &idComment2;
   _seqComment2[3] = &ignore;
   _seqComment2[4] = &number;
   _seqComment2[5] = &seqEntries;
   _seqComment2[6] = &ignore;
   _seqComment2[7] = &title;
   _seqComment2[8] = NULL;

   _seqEntries[0] = &type;
   _seqEntries[1] = &length2;
   _seqEntries[2] = &offset;
   _seqEntries[3] = NULL;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a title was read
//Parameters: pTitle: Pointer to title
//            len: Length of title
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundTitle (const char* pTitle, unsigned int len) {
   assert (prop); assert (pTitle);
   prop->strComment.assign (pTitle, len - 2);
   jpegImage.setMaxCard (1);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the length of the title was read
//Parameters: length: Pointer to length
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundLength (const char* length, unsigned int) {
   assert (length);
   unsigned int len (((unsigned char)(*length) << 8) + (unsigned char)length[1]);
   if (len)
      title.setMaxCard (len);
   TRACE8 ("ParseJPEG::foundLength (const char*, unsigned int): " << len);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the length of the title was read
//Parameters: length: Pointer to length
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundLength2 (const char* length, unsigned int) {
   assert (length);
   unsigned int offset (getTypeIndex (actEntry));
   if (offset != -1) {
      lengths[offset] = *(unsigned int*)length;
      TRACE9 ("ParseWord::foundLength2 (const char*) - " << lengths[offset]
              << " (0x" << hex << lengths[offset] << dec << ')');
   }
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the number of entries has been read
//Parameters: nr: Pointer to number of entries
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundNumber (const char* nr, unsigned int) {
   assert (nr);
   seqEntries.setMaxCard (cEntries = (*(unsigned int*)nr));
   length2.setMaxCard (4);
   TRACE8 ("ParseJPEG::foundNumber (const char*, unsigned int): " << cEntries);
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
   TRACE9 ("ParseWord::foundType (const char*) - " << hex << actEntry << dec);
   cRead += 10;
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
   unsigned int offset (getTypeIndex (actEntry));
   if (offset != -1) {
      offsets[offset] = *(unsigned int*)pOffset;
      TRACE9 ("ParseWord::foundOffset (const char*) - " << offsets[offset]
              << " (0x" << hex << offsets[offset] << dec << ')');
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

   ignore.setMinCard (4);
   ignore.setMaxCard (4);

   TRACE8 ("ParseJPEG::foundPropertiesHeader (const char*) - Setting title length to "
           << title.getMaxCard () - cRead - 32);
   assert (title.getMaxCard () > cRead);
   title.setMaxCard (title.getMaxCard () - cRead - 32);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Retrieves the index of the passed type
//Parameters: type: Type to inspect
//Returns   : unsigned int: Offset; -1 if type is not valid
/*--------------------------------------------------------------------------*/
int ParseJPEG::getTypeIndex (unsigned int type) {
   for (unsigned int i (0);
        i < (sizeof (aSupportedTypes) / sizeof (aSupportedTypes[0])); ++i)
      if (type == aSupportedTypes[i])
         return i;

   return -1;
}
