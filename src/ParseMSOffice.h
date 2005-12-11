#ifndef PARSEMSOFFICE_H
#define PARSEMSOFFICE_H

//$Id$

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

#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif


#include <map>

#include <YGP/Parse.h>

struct Properties;


// Class to extract the title (of the properties) of a Microsoft office document
class ParseMSOffice  {
 public:
   ParseMSOffice ();
   ~ParseMSOffice () { delete [] pBAT; delete [] pSBAT; }

   void parse (YGP::Xistream& stream, Properties& result) throw (std::string) {
      prop = &result;
      officedoc.parse (stream); }

 private:
   // Callback-methods for type of parsed elementes
   int foundBlockSize (const char*, unsigned int);
   int foundBlockIndex (const char*, unsigned int);
   int foundNameProperty (const char*, unsigned int);
   int foundLenNameProperty (const char*, unsigned int);
   int foundSectionOffset (const char*, unsigned int);
   int foundBeginSBA (const char*, unsigned int);

   int foundNrEntries (const char*, unsigned int);
   int foundType (const char*, unsigned int);
   int foundOffset (const char*, unsigned int);
   int foundLength (const char*, unsigned int);
   int foundInformation (const char*, unsigned int);
   int foundPropertiesHeader (const char*, unsigned int);
   int foundBAT (const char*, unsigned int);

   int foundCountBAT (const char*, unsigned int);
   int foundCountSBAT (const char*, unsigned int);
   int foundCountXBAT (const char*, unsigned int);
   int foundBlockBAT (const char*, unsigned int);
   int foundBlockSBAT (const char*, unsigned int);
   int foundBlockXBAT (const char*, unsigned int);
   int foundBlock (const char*, unsigned int);
   int foundFileSize (const char*, unsigned int);

   static int getTypeIndex (unsigned int type);

   typedef YGP::OFParseText<ParseMSOffice>     OMParseText;
   typedef YGP::OFParseExact<ParseMSOffice>    OMParseExact;
   typedef YGP::OFParseAttomic<ParseMSOffice>  OMParseAttomic;
   typedef YGP::OFParseSequence<ParseMSOffice> OMParseSequence;

   YGP::ParseExact   poifsID;
   YGP::ParseSkip    skipBeg;
   YGP::ParseSkip    skip;
   OMParseAttomic    sizeBlock;
   OMParseAttomic    blockIndex;
   OMParseAttomic    nameProperty;
   OMParseAttomic    lenNameProperty;
   YGP::ParseExact   idPropStream;
   YGP::ParseExact   idClass;
   OMParseAttomic    sectionOffset;
   OMParseAttomic    length;
   OMParseAttomic    nrEntries;
   OMParseAttomic    type;
   OMParseAttomic    offset;
   OMParseText       information;
   OMParseAttomic    beginSBA;
   OMParseAttomic    countBAT;
   OMParseAttomic    countSBAT;
   OMParseAttomic    countXBAT;
   OMParseAttomic    blockBAT;
   OMParseAttomic    blockSBAT;
   OMParseAttomic    blockXBAT;
   OMParseAttomic    block;
   OMParseAttomic    sizeFile;

   YGP::ParseSequence  officedoc;                                // Startsequence
   YGP::ParseSequence  seqProperties;
   YGP::ParseSequence  seqInformation;
   OMParseSequence     seqEntries;
   OMParseSequence     seqBAT;

   YGP::ParseObject* _officedoc[22];
   YGP::ParseObject* _seqProperties[7];
   YGP::ParseObject* _seqInformation[4];
   YGP::ParseObject* _seqEntries[3];
   YGP::ParseObject* _seqBAT[4];

   // Map for offsets to supported type. 1st: Offset, 2nd: Type
   std::map<unsigned int, unsigned int> aOffsets;

   unsigned int cRead;
   unsigned int cEntries;
   unsigned int actEntry;
   unsigned int len;

   unsigned int indexSBA;
   unsigned int blockSize;
   unsigned int blockSizeSmall;
   char propertyName[0x40];

   unsigned int* pBAT, cBAT;
   unsigned int* pSBAT, cSBAT;
   unsigned int* pXBAT, cXBAT;

   Properties* prop;
};

#endif
