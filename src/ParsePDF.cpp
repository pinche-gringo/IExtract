//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParsePDF
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 08.11.2002
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

#include <cstdio>
#include <cstdlib>

#include <sstream>

#include <YGP/Check.h>
#include <YGP/Trace_.h>

#include "Properties.h"

#include "ParsePDF.h"

#define ID1 "s"
#define ID ID1 "tartxref"


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
/*--------------------------------------------------------------------------*/
ParsePDF::ParsePDF ()
   : startXRef (ID, _("Tag for offset of cross reference table"))
     , offXRef ("\\9", _("Offset of cross reference table"), *this, &ParsePDF::foundOffset, 10, 1)
     , skipS (ID1, _("Start of startxref-tag"), 20, 1, true)
     , skip (ID1, _("Unused data"), 256, 1, true, false)
     , idXRef ("xref", _("Tag for cross reference table"))
     , nrStart ("\\9", _("Number of cross reference entries"), *this, &ParsePDF::foundStartNumber, 10)
     , count ("\\9", _("Number of cross reference entries"), *this, &ParsePDF::foundNumber, 10)
     , offObject ("\\9", _("Offset of object"), *this, &ParsePDF::foundObjOffset, 10, 1, false)
     , tagTrailer ("trailer", _("Tag for trailer"))
     , startObj ("<<", _("Start of object"))
     , objInfo ("/Info", _("Reference to info object"))
     , objPrev ("/Prev", _("Reference to other trailer"))
     , objOffPrev ("\\9", _("Offset of /Prev entry"), *this, &ParsePDF::foundPrevOffset, 10, 1)
     , idObject ("\\9", _("ID of object"), *this, &ParsePDF::foundObjectID, 10)
     , idObj ("1", _("ID of object (repeated)"))
     , number ("\\9", _("Generation"), 10)
     , tagObj ("obj", _("Tag for an object"))
     , endObj (">>", _("End of object"), *this, &ParsePDF::foundEndObj)
     , tagTitle ("/Title", _("Tag for title"), *this, &ParsePDF::foundTitle)
     , tagAuthor ("/Author", _("Tag for author"), *this, &ParsePDF::foundAuthor)
     , tagComment ("/Subject", _("Tag for comment (subject)"), *this, &ParsePDF::foundComment)
     , value (")>", _("Value of entry"), *this, &ParsePDF::foundValue, 512)
     , startOfValue1 ("(", _("Start of value ('(')"), *this, &ParsePDF::foundParenthesis)
     , startOfValue2 ("<", _("Start of value ('<')"), *this, &ParsePDF::foundBracket)
     , endOfValue (")", _("End of value"))
     , selXRef (_selXRef, _("Pointer to position of cross reference table"), -1U, 0)
     , seqXRef (_seqXRef, _("Position of cross reference table"))
     , seqXRefTable (_seqXRefTable, _("Cross reference table"))
     , seqXRefTableEntries (_seqXRefTableEntries, _("Entries in cross reference table"), 0, 0)
     , seqTrailer (_seqTrailer, _("Trailer"))
     , selValues (_selValues, _("Trailer values"), -1U, 0)
     , seqInfo (_seqInfo, _("Info entry"))
     , seqPrev (_seqPrev, _("Prev entry"))
     , seqInfoObj (_seqInfoObj, _("Info object"))
     , seqInfoValue (_seqInfoValue, _("Info values"), -1U, 0)
     , selType (_selType, _("Valid type"))
     , selStartOfValue (_selStartOfValue, _("Start of values"))
     , actEntry (NONE), offPrev (0), actObject (0), infoObject (-1U) {
   _selXRef[0] = &seqXRef;
   _selXRef[1] = &skipS;
   _selXRef[2] = &skip;
   _selXRef[3] = NULL;

   _seqXRef[0] = &startXRef;
   _seqXRef[1] = &offXRef;
   _seqXRef[2] = &seqXRefTable;
   _seqXRef[3] = NULL;

   _seqXRefTable[0] = &idXRef;
   _seqXRefTable[1] = &nrStart;
   _seqXRefTable[2] = &count;
   _seqXRefTable[3] = &seqXRefTableEntries;
   _seqXRefTable[4] = &seqTrailer;
   _seqXRefTable[5] = NULL;

   _seqXRefTableEntries[0] = &offObject;
   _seqXRefTableEntries[1] = &skip;
   _seqXRefTableEntries[2] = NULL;

   _seqTrailer[0] = &tagTrailer;
   _seqTrailer[1] = &startObj;
   _seqTrailer[2] = &selValues;
   _seqTrailer[3] = NULL;

   _selValues[0] = &seqInfo;
   _selValues[1] = &seqPrev;
   _selValues[2] = &endObj;
   _selValues[3] = &skip;
   _selValues[4] = NULL;

   _seqInfo[0] = &objInfo;
   _seqInfo[1] = &idObject;
   _seqInfo[2] = &skip;
   _seqInfo[3] = NULL;

   _seqPrev[0] = &objPrev;
   _seqPrev[1] = &objOffPrev;
   _seqPrev[2] = NULL;

   _seqInfoObj[0] = &idObj;
   _seqInfoObj[1] = &number;
   _seqInfoObj[2] = &tagObj;
   _seqInfoObj[3] = &startObj;
   _seqInfoObj[4] = &seqInfoValue;
   _seqInfoObj[5] = NULL;

   _seqInfoValue[0] = &selType;
   _seqInfoValue[1] = &selStartOfValue;
   _seqInfoValue[2] = &value;
   _seqInfoValue[3] = &endOfValue;
   _seqInfoValue[4] = NULL;

   _selType[0] = &tagTitle;
   _selType[1] = &tagAuthor;
   _selType[2] = &tagComment;
   _selType[3] = &endObj;
   _selType[4] = &skip;
   _selType[5] = NULL;

   _selStartOfValue[0] = &startOfValue1;
   _selStartOfValue[1] = &startOfValue2;
   _selStartOfValue[2] = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ParsePDF::~ParsePDF () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the offset of /Prev entry was read
//Parameters: pLength: Pointer to offset
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundPrevOffset (const char* pOffset, unsigned int) {
   TRACE9 ("ParsePDF::foundPrevOffset (const char*, unsigned int) - " << pOffset);
   Check3 (pOffset);
   offPrev = atoi (pOffset);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the offset of the cross reference table was read
//Parameters: pLength: Pointer to offset
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundOffset (const char* pOffset, unsigned int) {
   TRACE9 ("ParsePDF::foundOffset (const char*, unsigned int) - " << pOffset);
   Check3 (pOffset); Check3 (file);

   file->seekg (atoi (pOffset), std::ios::beg);
   selXRef.setMaxCard (0);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the start number of the entries in the cross
//            reference table was read
//Parameters: pNumber: Pointer to startnumber of objects
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundStartNumber (const char* pNumber, unsigned int) {
   TRACE5 ("ParsePDF::foundStartNumber (const char*, unsigned int) - " << pNumber);
   Check3 (pNumber); Check3 (file);

   actObject = atoi (pNumber);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the number of the entries in the cross reference
//            table was read
//Parameters: pNumber: Pointer to number of objects
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundNumber (const char* pNumber, unsigned int) {
   TRACE5 ("ParsePDF::foundNumber (const char*, unsigned int) - " << pNumber);
   Check3 (pNumber); Check3 (file);

   seqXRefTableEntries.setMaxCard (atoi (pNumber));
   skip.setValue ("\x0d\x0a");
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the offset of an object was read
//Parameters: pLength: Pointer to offset
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundObjOffset (const char* pOffset, unsigned int) {
   TRACE9 ("ParsePDF::foundObjOffset (const char*, unsigned int) - " << pOffset
           << " for object " << actObject);
   Check3 (pOffset); Check3 (file);

   aOffsets[actObject++] = atoi (pOffset);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Parses the info object
/*--------------------------------------------------------------------------*/
void ParsePDF::parseInfoObject () {
   Check3 (infoObject != -1U);
   Check3 (aOffsets.find (infoObject) != aOffsets.end ());

   TRACE5 ("ParsePDF::parseInfoObject () - Going to pos " << aOffsets[infoObject]
           << "; searching for " << infoObject);

   file->seekg (aOffsets[infoObject], std::ios::beg);

   std::ostringstream str;
   str << infoObject;
   idObj.setValue (str.str ().c_str ());
   idObj.setMaxCard (str.str ().size ());
   idObj.setMinCard (str.str ().size ());
   skip.setValue ("\\ ");

   Check3 (file);
   seqInfoObj.parse (*file);
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the ID of an object was read
//Parameters: pLength: Pointer to ID
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundObjectID (const char* pID, unsigned int len) {
   TRACE5 ("ParsePDF::foundObjectID (const char*, unsigned int) - " << pID);
   Check3 (pID); Check3 (file);

   infoObject = atoi (pID);
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after an end-of-object tag was read
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundEndObj (const char*, unsigned int) {
   TRACE9 ("ParsePDF::foundEndObj (const char*, unsigned int)");
   if (selValues.getMaxCard ()) {
      selValues.setMaxCard (0);
      skip.setValue ("(<");
   }
   else {
      seqInfoValue.setMaxCard (0);
      _seqInfoValue[1] = NULL;
   }
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the title was read
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundTitle (const char* pTitle, unsigned int) {
   TRACE5 ("ParsePDF::foundTitle (const char*, unsigned int)");
   actEntry = TITLE;
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the author was read
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundAuthor (const char*, unsigned int) {
   TRACE5 ("ParsePDF::foundAuthor (const char*, unsigned int)");
   actEntry = AUTHOR;
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the comment was read
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundComment (const char*, unsigned int) {
   TRACE5 ("ParsePDF::foundComment (const char*, unsigned int)");
   actEntry = COMMENT;
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the start of a hex-value was read
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundBracket (const char*, unsigned int) {
   TRACE5 ("ParsePDF::foundBracket (const char*, unsigned int)");
   endOfValue.setValue (">");
   value.setValue (">");
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the start of a string value was read
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundParenthesis (const char*, unsigned int) {
   TRACE5 ("ParsePDF::foundParenthesis (const char*, unsigned int)");
   endOfValue.setValue (")");
   value.setValue (")");
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the value for an entry was read
//Parameters: pLength: Pointer to value
//Returns   : int: Status: YGP::ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundValue (const char* pValue, unsigned int len) {
   TRACE9 ("ParsePDF::foundValue (const char*, unsigned int) - " << pValue);
   Check3 (pValue);

   if (actEntry != NONE) {
      TRACE9 ("ParsePDF::foundValue (const char*, unsigned int) - Assigning: "
              << pValue + 1);

      static std::string Properties::* values[] =
         { &Properties::strTitle, &Properties::strAuthor, &Properties::strComment };

      Check3 (prop);
      Check3 ((sizeof (values) / sizeof (values[0])) > actEntry);
      Check1 ((*value.getValue () == ')') || (*value.getValue () == '>'));

      if (*value.getValue () == ')')
         (prop->*(values[actEntry])).assign (pValue, len);
      else {
         prop->*(values[actEntry]) = "";

         Check1 (!(len & 1));
         if (*(unsigned int*)pValue == 'FFEF') {      // Skip MS-header for ???
            pValue += 4;
            len -= 4;
         }
         while (len) {
            unsigned int ch;
            sscanf (pValue, "%2X", &ch);
            if (ch)
               prop->*(values[actEntry]) += (char)ch;
            pValue += 2;
            len -= 2;
         }
      }
      actEntry = NONE;
   }
   return YGP::ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Method to parse a PDF object
//Parameters: stream: Stream to parse
//            result: Result where to store found data
/*--------------------------------------------------------------------------*/
void ParsePDF::parse (YGP::Xistream& stream, Properties& result) throw (std::string) {
   ParsePDF obj;
   stream.seekg (-40, std::ios::end);
   obj.prop = &result;
   obj.file = &stream;

   // Now parse first cross reference table (including trailer)
   unsigned int rc (obj.selXRef.parse (stream));
   while (!(rc || obj.aOffsets[obj.infoObject])) {
      if (!obj.offPrev)
         throw (std::string (_("Document does not contain neither an /Info"
                               " nor a /Prev entry")));

      stream.seekg (obj.offPrev, std::ios::beg);
      obj.offPrev = 0;
      rc = obj.seqXRefTable.parse (stream);
   }
   if (obj.aOffsets[obj.infoObject])
      obj.parseInfoObject ();
}

