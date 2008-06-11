#ifndef PARSEOOFICE_H
#define PARSEOOFICE_H

//$Id$

// This file is part of IExtract.
//
// IExtract is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IExtract is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libYGP.  If not, see <http://www.gnu.org/licenses/>.


#include <string>

#include <YGP/Parse.h>


struct Properties;


class ParseOpenOffice {
 public:
   ParseOpenOffice ();
   ~ParseOpenOffice ();

   void parse (YGP::Xistream& stream, Properties& result) throw (YGP::ParseError) {
      prop = &result;
      seqDocument.parse (stream);
   }

 private:
   // Prohibted manager functions
   ParseOpenOffice (const ParseOpenOffice& other);
   const ParseOpenOffice& operator= (const ParseOpenOffice& other);

   // Callback-methods for type of parsed elementes
   int foundTag (const char*, unsigned int);
   int foundValue (const char*, unsigned int);
   int foundNrEntries (const char*, unsigned int);
   int foundName (const char*, unsigned int);
   int foundLength (const char*, unsigned int);
   int foundLenName (const char*, unsigned int);
   int foundOffsetCDR (const char*, unsigned int);
   int foundOffsetFile (const char*, unsigned int);

   typedef YGP::OFParseText<ParseOpenOffice>    OMParseText;
   typedef YGP::OFParseQuoted<ParseOpenOffice>  OMParseQuoted;
   typedef YGP::OFParseAttomic<ParseOpenOffice> OMParseAttomic;

   Properties*  prop;

   YGP::ParseExact   idZipEntry;
   YGP::ParseSkip    skip;
   YGP::ParseExact   idCDR;
   YGP::ParseSkip    skip2;
   OMParseAttomic    nrCDREntries;
   OMParseAttomic    offCDR;
   YGP::ParseExact   idCFileHdr;
   OMParseAttomic    lenName;
   OMParseAttomic    len;
   OMParseAttomic    name;
   OMParseAttomic    posFile;
   YGP::ParseSkip    posMetaInfo;

   YGP::ParseExact   idMetadata;
   YGP::ParseText    skipLine;
   OMParseQuoted     tag;
   OMParseText       value;

   YGP::ParseSequence    seqCDREntries;
   YGP::ParseSequence    seqMetadata;
   YGP::ParseSequence    seqEntry;

   YGP::ParseSequence    seqDocument;                         // Startsequence

   YGP::ParseObject* _seqDocument[14];
   YGP::ParseObject* _seqCDREntries[9];
   YGP::ParseObject* _seqMetadata[4];
   YGP::ParseObject* _seqEntry[3];

   std::string Properties::* pEntry;
};

#endif
