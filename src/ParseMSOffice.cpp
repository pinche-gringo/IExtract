//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseWord
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

#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif


#include <iomanip>
#include <iostream>

#include <YGP/Check.h>
#include <YGP/Trace_.h>

#include "ParseWord.h"
#include "Properties.h"

static const unsigned LEN_CONTENT     = 1024;

#define ID1 "\xF9"
static const char* ID = ID1 "\x4F\x68\x10\xAB\x91\x08\x00\x2B\x27\xB3\xD9\x30\x00\x00\x00";

#define SEP1_1 "\x02"
static const char* SEP1 = SEP1_1 "\x00\x00\x00\xe4\x04\x00\x00\x1e\x00\x00\x00";
#define SEP2_1 "\x02"
static const char* SEP2 = SEP2_1 "\x00\x00\x00\x10\x27\x00\x00\x1e\x00\x00\x00";


#ifdef WORDS_BIGENDIAN
   static const unsigned int TYPE_TITLE   = 0x2000000;
   static const unsigned int TYPE_AUTHOR  = 0x4000000;
   static const unsigned int TYPE_COMMENT = 0x6000000;

inline unsigned int get4BytesLSB (const char* pAddr) {
   return (((unsigned char)(*pAddr) << 24) + ((unsigned char)pAddr[1] << 16)
           + ((unsigned char)pAddr[2] << 8) + ((unsigned char)pAddr[3]));
}

#else
   static const unsigned int TYPE_TITLE   = 2;
   static const unsigned int TYPE_AUTHOR  = 4;
   static const unsigned int TYPE_COMMENT = 6;

inline unsigned int get4BytesLSB (const char* pAddr) {
   return *(unsigned int*)pAddr;
}

#endif


static const unsigned int aTypes[] = { TYPE_TITLE, TYPE_AUTHOR, TYPE_COMMENT };


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
/*--------------------------------------------------------------------------*/
ParseWord::ParseWord()
   : id (ID, _("ID for title"), 16, 16, false)
     , idValue1 (SEP1, _("ID for values (I)"), *this, &ParseWord::foundValueStart, 12, 12, false)
     , idValue2 (SEP2, _("ID for values (II)"), *this, &ParseWord::foundValueStart, 12, 12, false)
     , skip (4)
     , nrEntries ("\\*", _("Number of entries"), *this, &ParseWord::foundNrEntries, 4, 4, false)
     , type ("\\*", _("Type of entry"), *this, &ParseWord::foundType, 4, 4, false)
     , offset ("\\*", _("Offset of Comment"), *this, &ParseWord::foundOffset, 4, 4, false)
     , length ("\\*", _("Length of title"), *this, &ParseWord::foundLength, 4, 4, false)
     , title ("\0", _("Title of document"), *this, &ParseWord::foundTitle, 1, 1, false)
     , skipIDStart (ID1, _("Other command"), 16, 1)
     , ignore (ID1, _("Content"), LEN_CONTENT, 1, false, false)
     , selValueStart (_selValueStart, _("Start of value ID"), -1U, 0)
     , seqTitle (_seqTitle, _("Title entry"), 1, 0, false)
     , seqEntries (_seqEntries, _("Entry description"), *this,
                   &ParseWord::foundPropertiesHeader, 1, 1, false)
     , seqProperties (_seqProperties, _("Properties"), 1, 1, false)
     , wordDoc (_wordDoc, _("Word document"), -1U, 1)
     , cRead (0), cEntries (0), actEntry (-1U), len (0), prop (NULL) {

   _seqProperties[0] = &id;
   _seqProperties[1] = &skip;
   _seqProperties[2] = &nrEntries;
   _seqProperties[3] = &seqEntries;
   _seqProperties[4] = &selValueStart;
   _seqProperties[5] = &seqTitle;
   _seqProperties[6] = NULL;

   _seqEntries[0] = &type;
   _seqEntries[1] = &skip;
   _seqEntries[2] = NULL;

   _seqTitle[0] = &length;
   _seqTitle[1] = &title;
   _seqTitle[2] = &skip;
   _seqTitle[3] = NULL;

   _selValueStart[0] = &idValue1;
   _selValueStart[1] = &idValue2;
   _selValueStart[2] = &skipIDStart;
   _selValueStart[3] = &ignore;
   _selValueStart[4] = NULL;

   _wordDoc[0] = &seqProperties;
   _wordDoc[1] = &skipIDStart;
   _wordDoc[2] = &ignore;
   _wordDoc[3] = NULL;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the number of entries has been parsed
//Parameters: pEntries: Pointer to number of entries
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseWord::foundNrEntries (const char* pEntries, unsigned int) {
   Check3 (pEntries);
   cEntries = get4BytesLSB (pEntries);
   TRACE4 ("ParseWord::foundNrEntries (const char*) - Entries: " << cEntries);

   seqEntries.setMaxCard (cEntries);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the number of entries has been parsed
//Parameters: pType: Pointer to found type
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the offset of the comment-entry was found
//Parameters: offset: Pointer to offset
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseWord::foundOffset (const char* offset, unsigned int) {
   Check3 (offset);
   Check3 (getTypeIndex (actEntry) != -1);

   unsigned int off (get4BytesLSB (offset));
   TRACE9 ("ParseWord::foundType (const char*) - Offset: " << off << " (0x"
           << hex << off << dec << ')');

   aOffsets[off] = actEntry;
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the length of the title was read
//Parameters: length: Pointer to length
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseWord::foundLength (const char* length, unsigned int) {
   Check3 (length);
   if ((len = *(int*)length))
      title.setMaxCard (len);
   TRACE8 ("ParseWord::foundLength (const char*, unsigned int): " << len);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a title was read
//Parameters: pTitle: Pointer to title
//            len: Length of title
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseWord::foundTitle (const char* pTitle, unsigned int len) {
   Check3 (pTitle);
   Check3 (aOffsets[actEntry] < 1000);

   TRACE1 ("ParseWord::foundTitle (const char*, unsigned int): " << pTitle
           << " (" << len << " bytes); Entries: " << seqTitle.getMaxCard ());

   static std::string Properties::* values[] =
      { &Properties::strTitle, &Properties::strAuthor, &Properties::strComment };

   Check3 (prop);
   Check3 (aOffsets.size ());
   Check3 (getTypeIndex (aOffsets[actEntry]) != -1);
   Check3 ((sizeof (values) / sizeof (values[0]))
            > getTypeIndex (aOffsets[actEntry]));
   (prop->*(values[getTypeIndex (aOffsets[actEntry])])) = pTitle;

   unsigned int off (aOffsets.begin ()->first);
   aOffsets.erase (aOffsets.begin ());

   if (aOffsets.size ()) {
      actEntry = aOffsets.begin ()->first;
      off = aOffsets.begin ()->first - off - len - 4;
      skip.setOffset (off);
      TRACE7 ("ParseWord::foundTitle (const char*) - Skipping " << off
              << " bytes for next entry");
   }

   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the header of the properthies has been read
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseWord::foundPropertiesHeader (const char*, unsigned int) {
   TRACE1 ("ParseWord::foundPropertiesHeader (const char*) - Bytes read: "
           << cRead << " (0x" << hex << cRead << dec << ')');

   // Check if there are any of the supported types in the document
   if (aOffsets.size ()) {
      actEntry = aOffsets.begin ()->first;
      seqTitle.setMaxCard (aOffsets.size ());
   }
   else
      _seqProperties[4] = NULL;

   // Set new values to search for ID of values
   skipIDStart.setValue (SEP1);
   ignore.setValue (SEP1);

   // Stop parsing of word document after this sequence terminates
   wordDoc.setMinCard (1);
   wordDoc.setMaxCard (1);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the ID for the start of the values has been read
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseWord::foundValueStart (const char*, unsigned int) {
   TRACE9 ("ParseWord::foundValueStart (const char*)");
   selValueStart.setMaxCard (0);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Retrieves the index of the passed type
//Parameters: type: Type to inspect
//Returns   : unsigned int: Offset; -1 if type is not valid
/*--------------------------------------------------------------------------*/
int ParseWord::getTypeIndex (unsigned int type) {
   for (unsigned int i (0); i < (sizeof (aTypes) / sizeof (aTypes[0])); ++i)
      if (type == aTypes[i])
         return i;

   return -1;
}
