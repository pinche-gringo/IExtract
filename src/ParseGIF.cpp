//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : GIF-parser
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 2005-11-26
//COPYRIGHT   : Copyright (C) 2005

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

#include <cstring>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include "Properties.h"

#include "ParseGIF.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param result: Structure to store the parsed values
//-----------------------------------------------------------------------------
ParseGIF::ParseGIF (Properties& result)
   : idGIF ("GIF", _("GIF-ID"), false)
     , skip (7)
     , skip2 (2)
     , colourTable ("\\*", _("Info about colour table"), *this, &ParseGIF::skipColourTable, 1, 1, false)
     , idEndGIF ("\x3b", _("End of GIF image"), *this,  &ParseGIF::foundEndGIF, false)
     , idCommentExt ("\x21\xfe", _("Comment extension"), false)
     , idImage ("\x2c",  _("Image separator"), *this,  &ParseGIF::foundImage, false)
     , idExtension ("\x21", _("Extension ID"))
     , idTypeExtension ("\\*", _("Type of extension"), 1, 1, false)
     , comment ("\\*", _("Comment"), *this, &ParseGIF::foundComment, 0, 0, false)
     , lenBlock ("\\*", _("Length of sub-block"), *this, &ParseGIF::foundLength, 1, 1, false)
     , blocks (_blocks, _("Blocks"), -1U, -1U, false)
     , commentExt (_commentExt, _("Comment extension"), 1, 1, false)
     , imageDesc (_imageDesc, _("Image description"))
     , extension (_extension, _("Extension"),1, 1, false)
     , commentBlocks (_commentBlocks, _("Comment blocks"), *this, &ParseGIF::foundSubblock, -1U, 0, false)
     , subblocks (_subblocks, _("Subblocks"), *this, &ParseGIF::foundSubblock, -1U, 0, false)
     , gifImage (_gifImage, _("GIF image"), 1, 1, false)
     , prop (result) {
   TRACE9 ("ParseGIF::ParseGIF (Properties&)");

   _blocks[0] = &commentExt;
   _blocks[1] = &imageDesc;
   _blocks[2] = &extension;
   _blocks[3] = &idEndGIF;
   _blocks[4] = NULL;

   _subblocks[0] = &lenBlock;
   _subblocks[1] = &skip;
   _subblocks[2] = NULL;

   _extension[0] = &idExtension;
   _extension[1] = &idTypeExtension;
   _extension[2] = &subblocks;
   _extension[3] = NULL;

   _commentBlocks[0] = &lenBlock;
   _commentBlocks[1] = &comment;
   _commentBlocks[2] = NULL;

   _imageDesc[0] = &idImage;
   _imageDesc[1] = &skip;
   _imageDesc[2] = &colourTable;
   _imageDesc[3] = &skip2;
   _imageDesc[4] = &skip;
   _imageDesc[5] = &subblocks;
   _imageDesc[6] = NULL;

   _commentExt[0] = &idCommentExt;
   _commentExt[1] = &commentBlocks;
   _commentExt[2] = NULL;

   _gifImage[0] = &idGIF;
   _gifImage[1] = &skip;
   _gifImage[2] = &colourTable;
   _gifImage[3] = &skip2;
   _gifImage[4] = &skip;
   _gifImage[5] = &blocks;
   _gifImage[6] = NULL;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ParseGIF::~ParseGIF () {
   TRACE9 ("ParseGIF::~ParseGIF ()");
}


//-----------------------------------------------------------------------------
/// Callback after the packed flag specifying the global colour table has been found
/// \param flag: Pointer to colour-table flag
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseGIF::skipColourTable (const char* flag, unsigned int) {
   Check2 (flag);
   skip.setOffset ((*flag & 0x80) ? (1 << ((*flag & 0x7) + 1)) * 3 : 0);
   TRACE7 ("ParseGIF::skipColourTable (const char*, unsigned int) - " << std::hex
	   << (unsigned int)(*flag & 0xff) << std::dec << " -> Skipping " << skip.getOffset ());
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a comment was read
/// \param comment: Pointer to comment
/// \param len: Length of data
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseGIF::foundComment (const char* comment, unsigned int len) {
   Check2 (comment);
   TRACE8 ("ParseGIF::foundComment (const char*, unsigned int) - " << comment << " (" << len << ')');
   prop.strComment += comment;
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after an image-id was read
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseGIF::foundImage (const char*, unsigned int) {
   TRACE9 ("ParseGIF::foundImage (const char*, unsigned int)");
   skip.setOffset (8);
   skip2.setOffset (1);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of a sub-block
/// \param comment: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseGIF::foundLength (const char* length, unsigned int) {
   TRACE8 ("ParseGIF::foundLength (const char*, unsigned int) - " << (unsigned int)(*length & 0xff)
	   << " = 0x" << std::hex << (unsigned int)(*length & 0xff) << std::dec);
   skip.setOffset ((unsigned int)(*length & 0xff));
   comment.setMinCard ((unsigned int)(*length & 0xff));
   comment.setMaxCard ((unsigned int)(*length & 0xff));
   if (!*length) {
      subblocks.setMaxCard (1);
      commentBlocks.setMaxCard (1);
   }
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a sub-block was (completely) parsed
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseGIF::foundSubblock (const char*, unsigned int) {
   TRACE9 ("ParseGIF::foundSubblock (const char*, unsigned int)");
   subblocks.setMaxCard (-1U);
   commentBlocks.setMaxCard (-1U);
   return YGP::ParseObject::PARSE_OK;
}


//-----------------------------------------------------------------------------
/// Callback after the end-of-image ID was found
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseGIF::foundEndGIF (const char*, unsigned int) {
   TRACE9 ("ParseGIF::foundEndGIF (const char*, unsigned int)");
   blocks.setMaxCard (1);
   blocks.setMinCard (1);
   return YGP::ParseObject::PARSE_OK;
}
