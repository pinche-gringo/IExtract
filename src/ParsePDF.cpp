//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParsePDFF
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


#include <stdlib.h>

#define TRACELEVEL 9
#include <Check.h>
#include <Trace_.h>

#include "ParsePDF.h"

#define ID1 "s"
#define ID ID1 "tartxref"


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
/*--------------------------------------------------------------------------*/
ParsePDF::ParsePDF () 
   : startObject (0)
     , startXRef (ID, "Tag for offset of cross reference table")
     , offXRef ("\\9", "Offset of cross reference table", *this, &ParsePDF::foundOffset, 10)
     , skipS (ID1, "Start of startxref-tag", 20)
     , skip (ID1, "Unused data", 20)
     , idXRef ("xref", "Tag for cross reference table")
     , nrStart ("\\9", "Number of cross reference entries", *this, &ParsePDF::foundStartNumber, 10)
     , count ("\\9", "Number of cross reference entries", *this, &ParsePDF::foundNumber, 10)
     , offObject ("\\9", "Offset of object", *this, &ParsePDF::foundObjOffset, 10)
     , tagTitle ("/Title (", "Tag for title", *this, &ParsePDF::foundTitle)
     , tagAuthor ("/Author (", "Tag for author", *this, &ParsePDF::foundAuthor)
     , tagComment ("/Subject", "Tag for comment (subject)", *this, &ParsePDF::foundComment)
     , value (")", "Value of entry", *this, &ParsePDF::foundTitle, 512)
     , selXRef (_selXRef, "Pointer to position of cross reference table", -1, 1)
     , seqXRef (_seqXRef, "Position of cross reference table", 1, 1)
     , seqXRefTable (_seqXRefTable, "Cross reference table", 1, 1)
     , seqXRefTableEntries (_seqXRefTableEntries, "Entries in cross reference table", 0, 0)
{
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
   _seqXRefTable[4] = NULL;

   _seqXRefTableEntries[0] = &offObject;
   _seqXRefTableEntries[1] = &skip;
   _seqXRefTableEntries[2] = NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
ParsePDF::~ParsePDF () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the offset of the cross reference table was read
//Parameters: pLength: Pointer to offset
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundOffset (const char* pOffset, unsigned int) {
   TRACE5 ("ParsePDF::foundOffset (const char*, unsigned int) - " << pOffset);
   Check3 (pOffset); Check3 (file);

   file->seekg (atoi (pOffset));
   selXRef.setMaxCard (1);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the start number of the entries in the cross
//            reference table was read
//Parameters: pNumber: Pointer to startnumber of objects
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundStartNumber (const char* pNumber, unsigned int) {
   TRACE5 ("ParsePDF::foundStartNumber (const char*, unsigned int) - " << pNumber);
   Check3 (pNumber); Check3 (file);

   startObject = atoi (pNumber);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the number of the entries in the cross reference
//            table was read
//Parameters: pNumber: Pointer to number of objects
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundNumber (const char* pNumber, unsigned int) {
   TRACE5 ("ParsePDF::foundNumber (const char*, unsigned int) - " << pNumber);
   Check3 (pNumber); Check3 (file);

   seqXRefTableEntries.setMaxCard (atoi (pNumber));
   skip.setValue ("\n");
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the offset of an object was read
//Parameters: pLength: Pointer to offset
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundObjOffset (const char* pOffset, unsigned int) {
   TRACE5 ("ParsePDF::foundObjOffset (const char*, unsigned int) - " << pOffset);
   Check3 (pOffset); Check3 (file);

   aOffsets.push_back (atoi (pOffset));
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the title was read
//Parameters: pLength: Pointer to offset
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundTitle (const char* pTitle, unsigned int) {
   TRACE5 ("ParsePDF::foundTitle (const char*, unsigned int) - " << pTitle);
   Check3 (pTitle); Check3 (file);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the author was read
//Parameters: pLength: Pointer to offset
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundAuthor (const char* pAuthor, unsigned int) {
   TRACE5 ("ParsePDF::foundAuthor (const char*, unsigned int) - " << pAuthor);
   Check3 (pAuthor); Check3 (file);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the comment was read
//Parameters: pLength: Pointer to offset
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParsePDF::foundComment (const char* pComment, unsigned int) {
   TRACE5 ("ParsePDF::foundComment (const char*, unsigned int) - " << pComment);
   Check3 (pComment); Check3 (file);
   return ParseObject::PARSE_OK;
}
