//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseMSOffice
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 8.10.2002
//COPYRIGHT   : Copyright (C) 2002 - 2008

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


#include <cstring>

#include <IExtract-cfg.h>

#ifdef _MSC_VER
#pragma warning(disable:4355) // disable warning about this in initlist
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif


#include <iomanip>
#include <iostream>

#include <map>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "ParseMSOffice.h"
#include "Properties.h"


static const char SUMMARY[] = ("\x05\0\x53\0\x75\0\x6D\0\x6D\0\x61\0\x72\0\x79\0"
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
/// \throw YGP::ParseError: In case of an error an describing text
//-----------------------------------------------------------------------------
void ParseMSOffice::parse (YGP::Xistream& stream, Properties& result) throw (YGP::ParseError) {
   TRACE9 ("ParseMSOffice::parse (YGP::Xistream&, Properties&)");

   // Read the header (the first block)
   char header[512];
   stream.read (header, 512);
   if (!stream)
      throw (YGP::ParseError (_("Can't read document header!")));

   if (memcmp (header, "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1", 8))
      throw (YGP::ParseError (_("Office identifier not found!")));

   // Blocksizes
   UINT16 sizeBlock (YGP::get2BytesLSB (header + 0x1e));
   UINT16 sizeBlockSmall (YGP::get2BytesLSB (header + 0x20));
   TRACE8 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Sizes: " << (1 << sizeBlock) << '/' << (1 << sizeBlockSmall));
   if ((sizeBlockSmall > sizeBlock) || (sizeBlock > 512))
      throw (YGP::ParseError (_("Values for blocksizes are not plausible!")));

   // Count of BAT blocks and offset of properties
   UINT32 cBAT (YGP::get4BytesLSB (header + 0x2c));
   if (cBAT > 109)
      throw (YGP::ParseError (_("Number of blocks for BAT not plausible!")));

   UINT32 offProperties (YGP::get4BytesLSB (header + 0x30));
   TRACE6 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Properties: " << std::hex
	   << offProperties << " -> " << ((offProperties << sizeBlock) + 512));

   // Read the property-table
   char* block (new char [1 << sizeBlock]);
   readBlock (stream, offProperties, block, sizeBlock);

   // First entry must be the root-entry (which has a pointer to the small block area
   if ((YGP::get2BytesLSB (block + 0x40) != 0x16) || memcmp (block, ROOTENTRY, 0x16)) {
      delete [] block;
      snprintf (header, sizeof (header), "%p", (void*)((offProperties << sizeBlock) + 512));
      std::string error (_("Not a property block at offset %1!"));
      error.replace (error.find ("%1"), 2, header);
      throw (YGP::ParseError (error));
   }
   UINT32 offSBA (YGP::get4BytesLSB (block + 0x74));

   // Read the block array table
   UINT32* pBAT (new UINT32 [(cBAT << sizeBlock) >> 2]);
   try {
      readBAT (stream, (char*)pBAT, header + 0x4c, cBAT, sizeBlock);
   }
   catch (YGP::ParseError& e) {
      delete [] block;
      delete [] pBAT;
      throw (e);
   }

   const char* entry (block + 0x80);
   do {
      char* infoBlock (NULL);
      try {
	 while (entry < (block + (1 << sizeBlock))) {
	    TRACE9 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Entry with " << std::hex << YGP::get2BytesLSB (entry + 0x40) << " bytes");

	    // Check for InformationSummary-block
	    if ((YGP::get2BytesLSB (entry + 0x40) == 0x28) && !memcmp (entry, SUMMARY, 0x28)) {
	       INT32 length (YGP::get4BytesLSB (entry + 0x78));
	       UINT32 offBlock (YGP::get4BytesLSB (entry + 0x74));
	       TRACE8 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Info in " << std::hex
		       << offBlock << std::dec << "; " << length << " bytes");

	       char* infoBlock (NULL);
	       // Files less than 4K are stored in small blocks
	       if (length < 4096) {
		  UINT32* pSBAT (NULL);
		  std::map<unsigned int, char*> readBlocksSBA;

		  try {
		     // Read SBAT table
		     pSBAT = (UINT32*)readFile (stream, YGP::get4BytesLSB (header + 0x3c), pBAT,
						YGP::get4BytesLSB (header + 0x40), sizeBlock);

		     unsigned int cBlocks ((length & ((1 << sizeBlock) - 1))
					   ? ((((length >> sizeBlock)) + 1))
					   : length >> sizeBlock);
		     Check3 ((UINT32)length <= (cBlocks << sizeBlock));
		     infoBlock = new char[cBlocks << sizeBlock];
		     char* actPos (infoBlock);

		     // Read the small file
		     int bigBlock (0);
		     unsigned int cSmallBlocks (1 << (sizeBlock - sizeBlockSmall));
		     while (length > 0) {
			bigBlock = getBlock (pBAT, offSBA, offBlock >> (sizeBlock - sizeBlockSmall));
			if (readBlocksSBA.find (bigBlock) == readBlocksSBA.end ()) {
			   readBlocksSBA[bigBlock] = new char[1 << sizeBlock];
			   readBlock (stream, bigBlock, readBlocksSBA[bigBlock], sizeBlock);
			}
			memcpy (actPos, readBlocksSBA[bigBlock] + ((offBlock % cSmallBlocks) << sizeBlockSmall),
				1 << sizeBlockSmall);
			actPos += 1 << sizeBlockSmall;
			length -= 1 << sizeBlockSmall;
			offBlock = pSBAT[offBlock];
		     } // end-while
		  }
		  catch (YGP::ParseError& e) {
		     delete [] pSBAT;
		     for (std::map<unsigned int, char*>::iterator i (readBlocksSBA.begin ());
			  i != readBlocksSBA.end (); ++i)
			delete [] i->second;
		     throw;
		  }

		  // Cleanup
		  for (std::map<unsigned int, char*>::iterator i (readBlocksSBA.begin ());
		       i != readBlocksSBA.end (); ++i)
		     delete [] i->second;
		  delete [] pSBAT;
	       }
	       else {
		  unsigned int cBlocks ((length & ((1 << sizeBlock) - 1))
					? ((((length >> sizeBlock)) + 1))
					: length >> sizeBlock);
		  infoBlock = readFile (stream, offBlock, pBAT, cBlocks, sizeBlock);
	       }

	       // Check info-block
	       char* actPos (infoBlock);
	       TRACE9 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - InfoSummary: " << std::hex << YGP::get2BytesLSB (infoBlock) << std::dec);
	       if ((YGP::get2BytesLSB (infoBlock) != 0xfffe)
		   || memcmp (infoBlock + 0x18, SECTIONID, 0x14))
		  throw YGP::ParseError (_("Not an Information-Summary section!"));

	       TRACE9 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Start section: " << std::hex << YGP::get4BytesLSB (infoBlock + 0x2c) << std::dec);
	       actPos = infoBlock + YGP::get4BytesLSB (infoBlock + 0x2c);
	       unsigned int entries (YGP::get4BytesLSB (actPos + 4));
	       TRACE3 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Entries: " << entries);
	       if (entries > 50)
		  throw YGP::ParseError (_("Number of information entries not plausible!"));

	       entry = actPos + 8;
	       while (entries--) {
		  UINT32 offset (YGP::get4BytesLSB (entry + 4));
		  TRACE7 ("ParseMSOffice::parse (YGP::Xistream&, Properties&) - Entry: " << YGP::get4BytesLSB (entry) << ": Offset " << std::hex << offset << std::dec);

		  if ((offset >= YGP::get4BytesLSB (actPos))
		      || ((YGP::get4BytesLSB (actPos + offset) == 0x1e)
			  ? ((YGP::get4BytesLSB (actPos + offset + 4) >= YGP::get4BytesLSB (actPos))
			     || (offset < (YGP::get4BytesLSB (actPos + 4) << 3)))
			  : false))
		     throw YGP::ParseError (_("Values of entry not plausible!"));

		  switch (YGP::get4BytesLSB (entry)) {
		  case TYPE_TITLE:
		     Check3 (actPos[offset + 8 + YGP::get4BytesLSB (actPos + offset + 4) - 1] == '\0');
		     result.strTitle.assign (actPos + offset + 8,
					     YGP::get4BytesLSB (actPos + offset + 4) - 1);
		     break;

		  case TYPE_AUTHOR:
		     Check3 (actPos[offset + 8 + YGP::get4BytesLSB (actPos + offset + 4) - 1] == '\0');
		     result.strAuthor.assign (actPos + offset + 8,
					      YGP::get4BytesLSB (actPos + offset + 4) - 1);
		     break;

		  case TYPE_COMMENT:
		     Check3 (actPos[offset + 8 + YGP::get4BytesLSB (actPos + offset + 4) - 1] == '\0');
		     result.strComment.assign (actPos + offset + 8,
					       YGP::get4BytesLSB (actPos + offset + 4) - 1);

		     break;
		  } // end-switch
		  entry += 8;
	       } // end-while
	       goto end;
	    }
	    entry += 0x80;
	 } // end-while

	 Check3 (pBAT);
	 if ((offProperties = pBAT[offProperties]) > 0x80000000)
	    break;

	 readBlock (stream, offProperties, block, sizeBlock);
	 entry = block;
      }
      catch (YGP::ParseError& e) {
	 delete [] block;
	 delete [] pBAT;
	 delete [] infoBlock;
	 throw;
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
/// \param block: Block to read into
/// \param sizeBlock: Size (as exponent of 2) of the block
/// \throw YGP::ParseError: In case of error a describing text
/// \remarks - The first byte of block needs to be filled with the blocklength
//-----------------------------------------------------------------------------
void ParseMSOffice::readBlock (YGP::Xistream& stream, unsigned int offBlock, char* block,
			       unsigned int sizeBlock) throw (YGP::ParseError) {
   TRACE9 ("ParseMSOffice::readBlock (YGP::Xistream&, unsigned int, char*, unsigned int): " << offBlock);
   stream.seekg ((offBlock << sizeBlock) + 512, std::ios_base::beg);
   stream.read (block, 1 << sizeBlock);
   if (!stream) {
      snprintf (block, 1 << sizeBlock, "%p", (void*)((offBlock << sizeBlock) + 512));
      std::string error (_("Can't read block at offset %1"));
      error.replace (error.find ("%1"), 2, block);
      throw (YGP::ParseError (error));
   }
}


//-----------------------------------------------------------------------------
/// Reads the BAT
/// \param stream: Stream to read from
/// \param pBAT: Pointer to the block array table to fill
/// \param pBATBlocks: Pointer to the block-indexes of the BAT
/// \param sizeBlock: Size of the blocks
/// \param cBlocks: Number of BAT blocks
/// \throw YGP::ParseError: In case of error a describing text
//-----------------------------------------------------------------------------
void ParseMSOffice::readBAT (YGP::Xistream& stream, char* pBAT, const char* pBATBlocks,
			     unsigned int cBlocks, unsigned int sizeBlock) throw (YGP::ParseError) {
   TRACE9 ("ParseMSOffice::readBAT (YGP::Xistream&, 2x char*, 2x unsigned) - " << cBlocks);
   Check1 (pBAT);

   for (unsigned int i (0); i < cBlocks; ++i)
      readBlock (stream, YGP::get4BytesLSB (pBATBlocks + (i << 2)), pBAT + (i << sizeBlock), sizeBlock);
}

//-----------------------------------------------------------------------------
/// Reads a file starting at the passed block. A buffer holding the
/// read file is allocted.
/// \param stream: Stream to read from
/// \param offBlock: Index of startblock
/// \param pBAT: Pointer to used block array table
/// \param blocks: Number of block to read
/// \param sizeBlock: Size (as exponent of 2) of the block
/// \returns char*: Allocated buffer with file
/// \throw YGP::ParseError: In case of error a describing text
//-----------------------------------------------------------------------------
char* ParseMSOffice::readFile (YGP::Xistream& stream, unsigned int offBlock, void* pBAT,
			       unsigned int blocks, unsigned int sizeBlock) throw (YGP::ParseError) {
   char* pFile (new char [blocks << sizeBlock]);
   char* actPos (pFile);
   while (blocks--) {
      Check3 (offBlock < 0x80000000);
      readBlock (stream, offBlock, actPos, sizeBlock);
      actPos += 1 << sizeBlock;
      offBlock = ((UINT32*)pBAT)[offBlock];
   } // end-while
   return pFile;
}

//-----------------------------------------------------------------------------
/// Gets the nth block from the passed BAT
/// \param pBAT: Pointer to BAT to use
/// \param start: Start block
/// \param nr: Nth block to find
/// \returns unsigned int: Offset of block
/// \throw YGP::ParseError: In case of error a describing text
//-----------------------------------------------------------------------------
int ParseMSOffice::getBlock (void* pBAT, unsigned int start, unsigned int nr) throw (YGP::ParseError) {
   TRACE9 ("ParseMSOffice::getBlock (void*, 2x unsigned int) - " << start);
   while (nr--) {
      start = ((UINT32*)pBAT)[start];
      if (start > 0x80000000)
	 throw (YGP::ParseError (_("BAT not valid!")));
   } // end-while

   TRACE8 ("ParseMSOffice::getBlock (void*, 2x unsigned int) - Result: " << start);
   return start;
}
