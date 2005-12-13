//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseMSOffice
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 8.10.2002
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

#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif


#include <iomanip>
#include <iostream>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Utility.h"
#include "ParseMSOffice.h"
#include "Properties.h"


static const char SUMMARY[] = ("\x05\0\x53\0\x75\0\x6d\0\x6d\0\x61\0\x72\0\x79\0"
			       "\x49\0\x6E\0\x66\0\x6F\0\x72\0\x6D\0\x61\0\x74\0"
			       "\x69\0\x6F\0\x6E\0\0");
static const char ROOTENTRY[] = "R\0o\0o\0t\0 \0E\0n\0t\0r\0y\0\0";
static const char SECTIONID[] = { '\x01', '\0', '\0', '\0', '\xE0', '\x85', '\x9F', '\xF2', '\xF9', '\x4F',
				  '\x68', '\x10', '\xAB', '\x91', '\x08', '\0', '\x2B', '\x27', '\xB3', '\xD9' };


static const UINT32 TYPE_TITLE   = 2;
static const UINT32 TYPE_AUTHOR  = 4;
static const UINT32 TYPE_COMMENT = 6;


//-----------------------------------------------------------------------------
/// (Default-)Constructor
//-----------------------------------------------------------------------------
ParseMSOffice::ParseMSOffice() {
}


//-----------------------------------------------------------------------------
/// Tries to parse a Microsoft Office documnent
/// \param stream: Stream to read from
/// \param result: Structure to hold the found information
/// \throw std::string: In case of an error an describing text
//-----------------------------------------------------------------------------
void ParseMSOffice::parse (YGP::Xistream& stream, Properties& result) throw (std::string) {
   TRACE9 ("ParseMSOffice::parse (YGP::Xistream&, Properties&)");

   // Read the header (the first block)
   char header[512];
   stream.read (header, 512);
   if (!stream)
      throw std::string (_("Can't read document header!"));

   if (memcmp (header, "\xd0\xcf\x11\xe0\xa1\xb1\x1a\xe1", 8))
      throw std::string (_("Office identifier not found!"));

   // Blocksizes
   UINT16 sizeBlock (get2BytesLSB (header + 0x1e));
   UINT16 sizeBlockSmall (get2BytesLSB (header + 0x20));
   TRACE8 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Sizes: " << (1 << sizeBlock) << '/' << (1 << sizeBlockSmall));
   if ((sizeBlockSmall > sizeBlock) || (sizeBlock > 512))
      throw std::string (_("Values for blocksizes are not plausible!"));

   // Count of BAT blocks and offset of properties
   UINT32 cBAT (get4BytesLSB (header + 0x2c));
   if (cBAT > 109)
      throw std::string (_("Number of blocks for BAT not plausible!"));

   UINT32 offProperties (get4BytesLSB (header + 0x30));
   TRACE6 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Properties: " << std::hex
	   << offProperties << " -> " << ((offProperties << sizeBlock) + 512));

   // Read the property-table
   char* block (new char [1 << sizeBlock]);
   readBlock (stream, offProperties, block, sizeBlock);

   // First entry must be the root-entry (which has a pointer to the small block area
   if ((get2BytesLSB (block + 0x40) != 0x16) || memcmp (block, ROOTENTRY, 0x16)) {
      delete [] block;
      snprintf (header, sizeof (header), "%p", (void*)((offProperties << sizeBlock) + 512));
      std::string error (_("Not a property block at offset %1!"));
      error.replace (error.find ("%1"), 2, header);
      throw error;
   }
   UINT32 offSBA (get4BytesLSB (block + 0x74));

   // Read the block array table
   UINT32* pBAT (new UINT32 [(cBAT << sizeBlock) >> 2]);
   try {
      readBAT (stream, (char*)pBAT, header + 0x4c, cBAT, sizeBlock);
   }
   catch (std::string& e) {
      delete [] block;
      delete [] pBAT;
      throw e;
   }

   const char* entry (block + 0x80);
   do {
      char* infoBlock (NULL);
      try {
	 while (entry < (block + (1 << sizeBlock))) {
	    TRACE9 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Entry with " << std::hex << get2BytesLSB (entry + 0x40) << " bytes");

	    // Check for InformationSummary-block
	    if ((get2BytesLSB (entry + 0x40) == 0x28) && !memcmp (entry, SUMMARY, 0x28)) {
	       INT32 length (get4BytesLSB (entry + 0x78));
	       char* infoBlock (new char [length]);
	       UINT32 offBlock (get4BytesLSB (entry + 0x74));
	       TRACE8 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Info in " << std::hex
		       << offBlock << std::dec << "; " << length << " bytes");

	       infoBlock = new char [length];
	       char* actPos (infoBlock);
	       // Files less than 4K are stored in small blocks
	       if (length < 4096) {
		  UINT32* pSBAT (NULL);
		  UINT32 offSBAT (get4BytesLSB (header + 0x3c));
	       }
	       else {
		  while (length > 0) {
		     Check3 (offBlock < 0x80000000);
		     readBlock (stream, offBlock, block, sizeBlock);
		     memcpy (actPos, block, length < (1 << sizeBlock) ? length : 1 << sizeBlock);
		     actPos += 1 << sizeBlock;
		     length -= 1 << sizeBlock;
		     offBlock = pBAT[offBlock];
		  } // end-while
	       }

	       // Check info-block
	       TRACE9 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - InfoSummary: " << std::hex << get2BytesLSB (infoBlock) << std::dec);
	       if ((get2BytesLSB (infoBlock) != 0xfffe)
		   || memcmp (infoBlock + 0x18, SECTIONID, 0x14))
		  throw std::string (_("Not an Information-Summary section!"));

	       TRACE9 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Start section: " << std::hex << get4BytesLSB (infoBlock + 0x2c) << std::dec);
	       actPos = infoBlock + get4BytesLSB (infoBlock + 0x2c);
	       unsigned int entries (get4BytesLSB (actPos + 4));
	       TRACE3 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Entries: " << entries);
	       if (entries > 50)
		  throw std::string (_("Number of information entries not plausible!"));

	       entry = actPos + 8;
	       while (entries--) {
		  UINT32 offset (get4BytesLSB (entry + 4));
		  TRACE7 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Entry: " << get4BytesLSB (entry) << ": Offset " << std::hex << offset << std::dec);

		  if ((offset >= get4BytesLSB (actPos))
		      || ((get4BytesLSB (actPos + offset) == 0x1e)
			  ? ((get4BytesLSB (actPos + offset + 4) >= get4BytesLSB (actPos))
			     || (offset < (get4BytesLSB (actPos + 4) << 3)))
			  : false))
		     throw std::string (_("Values of entry not plausible!"));

		  switch (get4BytesLSB (entry)) {
		  case TYPE_TITLE:
		     Check3 (actPos[offset + 8 + get4BytesLSB (actPos + offset + 4) - 1] == '\0');
		     result.strTitle.assign (actPos + offset + 8,
					     get4BytesLSB (actPos + offset + 4) - 1);
		     break;

		  case TYPE_AUTHOR:
		     Check3 (actPos[offset + 8 + get4BytesLSB (actPos + offset + 4) - 1] == '\0');
		     result.strAuthor.assign (actPos + offset + 8,
					      get4BytesLSB (actPos + offset + 4) - 1);
		     break;

		  case TYPE_COMMENT:
		     Check3 (actPos[offset + 8 + get4BytesLSB (actPos + offset + 4) - 1] == '\0');
		     result.strComment.assign (actPos + offset + 8,
					       get4BytesLSB (actPos + offset + 4) - 1);

		     break;
		  } // end-switch
		  entry += 8;
	       } // end-while
	       goto end;
	    }
	    entry += 0x80;
	 } // end-while

	 Check3 (pBAT);
	 if ((offProperties = pBAT[offProperties]) < 0x80000000)
	    break;

	 readBlock (stream, offProperties, block, sizeBlock);
	 entry = block;
      }
      catch (std::string& e) {
	 delete [] block;
	 delete [] pBAT;
	 delete [] infoBlock;
	 throw e;
      }
   } while (true); // end-do

 end:
   delete [] block;
   delete [] pBAT;
}

//-----------------------------------------------------------------------------
/// Reads the specified block from the stream
/// \param stream: Stream to read from
/// \param offBlock: Index of block to read
/// \param block: Block to read into; On input the first byte contains the length (as
///              exponent of 2) of the block
/// \throw std::string: In case of error a describing text
/// \remarks - The first byte of block needs to be filled with the blocklength
//-----------------------------------------------------------------------------
void ParseMSOffice::readBlock (YGP::Xistream& stream, unsigned int offBlock, char* block,
			       unsigned int sizeBlock) throw (std::string) {
   TRACE9 ("ParseMSOffice::readBlock (YGP::Xistream&, unsigned int, char*, unsigned int): " << offBlock);
   stream.seekg ((offBlock << sizeBlock) + 512, std::ios_base::beg);
   stream.read (block, 1 << sizeBlock);
   if (!stream) {
      snprintf (block, 1 << sizeBlock, "%p", (void*)((offBlock << sizeBlock) + 512));
      std::string error (_("Can't read block at offset %1"));
      error.replace (error.find ("%1"), 2, block);
      throw error;
   }
}


//-----------------------------------------------------------------------------
/// Reads the BAT
/// \param stream: Stream to read from
/// \param pBAT: Pointer to the block array table to fill
/// \param pBATBlocks: Pointer to the block-indexes of the BAT
/// \param sizeBlock: Size of the blocks
/// \param cBlocks: Number of BAT blocks
/// \throw std::string: In case of error a describing text
//-----------------------------------------------------------------------------
void ParseMSOffice::readBAT (YGP::Xistream& stream, char* pBAT, const char* pBATBlocks,
			     unsigned int cBlocks, unsigned int sizeBlock) throw (std::string) {
   TRACE9 ("ParseMSOffice::readBAT (YGP::Xistream&, 2x char*, 2x unsigned) - " << cBlocks);
   Check1 (pBAT);

   for (unsigned int i (0); i < cBlocks; ++i)
      readBlock (stream, get4BytesLSB (pBATBlocks + (i << 2)), pBAT + (i << sizeBlock), sizeBlock);
}
