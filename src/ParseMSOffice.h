#ifndef PARSEMSOFFICE_H
#define PARSEMSOFFICE_H

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


#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif


#include <map>

#include <YGP/XStream.h>
#include <YGP/Exception.h>

struct Properties;


// Class to extract the title (of the properties) of a Microsoft office document
class ParseMSOffice  {
 public:
   ParseMSOffice ();
   ~ParseMSOffice () { }

   void parse (YGP::Xistream& stream, Properties& result);

 private:
   static void readBAT (YGP::Xistream& stream, char* pBAT, const char* pBATBlocks,
			unsigned int cBlocks, unsigned int sizeBlock);
   static void readBlock (YGP::Xistream& stream, unsigned int offBlock,
			  char* block, unsigned int sizeBlock);
   static char* readFile (YGP::Xistream& stream, unsigned int offBlock,
			  void* pBAT, unsigned int blocks, unsigned int sizeBlock);
   static int getBlock (void* pBAT, unsigned int start, unsigned int nr);
};

#endif
