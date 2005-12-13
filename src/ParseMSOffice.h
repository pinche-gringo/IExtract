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
   ~ParseMSOffice () { }

   void parse (YGP::Xistream& stream, Properties& result) throw (std::string);

 private:
   static void readBAT (YGP::Xistream& stream, char* pBAT, const char* pBATBlocks,
			unsigned int cBlocks, unsigned int sizeBlock) throw (std::string);
   static void readBlock (YGP::Xistream& stream, unsigned int offBlock,
			  char* block, unsigned int sizeBlock) throw (std::string);
};

#endif
