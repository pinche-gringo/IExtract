//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : PNG-parser
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 2005-08-02
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

#include "Utility.h"
#include "ParsePNG.h"
#include "Properties.h"


#ifdef WORDS_BIGENDIAN

#define TYPE_TEXT 0x74584574

#else

#define TYPE_TEXT 0x74455874

#endif


//-----------------------------------------------------------------------------
/// Constructor
/// \param result: Structure to store the parsed values
//-----------------------------------------------------------------------------
ParsePNG::ParsePNG (Properties& result)
   : idPNG ("\x89PNG\x0d\x0a\x1a\x0a", _("PNG-ID"), false)
     , length ("\\*", _("Length (MSB first)"), *this, &ParsePNG::foundLength, 4, 4, false)
     , type ("\\*", _("Type of entry"), *this, &ParsePNG::foundType, 4, 4, false)
     , comment ("\\*", _("Comment"), *this, &ParsePNG::foundComment, 0, 0, false)
     , skip (0)
     , crc ("\\*", _("CRC"), 4, 4, false)
     , chunk (_chunk, _("Chunk"), -1U, 2, false)
     , pngImage (_pngImage, _("PNG image"), 1, 1, false)
     , prop (result) {
   TRACE9 ("ParsePNG::ParsePNG (Properties&)");

   _chunk[0] = &length;
   _chunk[1] = &type;
   _chunk[2] = &skip;
   _chunk[3] = &crc;
   _chunk[4] = NULL;

   _pngImage[0] = &idPNG;
   _pngImage[1] = &chunk;
   _pngImage[2] = NULL;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ParsePNG::~ParsePNG () {
   TRACE9 ("ParsePNG::~ParsePNG ()");
}


//-----------------------------------------------------------------------------
/// Callback after the type of a chunk was read
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParsePNG::foundType (const char* type, unsigned int) {
   Check2 (type);
   TRACE9 ("ParsePNG::foundType (const char*, unsigned int) - " << std::hex
	   << get4BytesMSB (type) << std::dec);
   Check2 (_chunk[2] == &skip);

   if (get4BytesMSB (type) == TYPE_TEXT) {
      _chunk[2] = &comment;
      comment.setMinCard (skip.getOffset ());
      comment.setMaxCard (skip.getOffset ());
   }
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the length of a chunk was read
/// \param length: Pointer to length
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParsePNG::foundLength (const char* length, unsigned int) {
   Check2 (length);
   Check3 (!(*length & 0x80));
   TRACE8 ("ParsePNG::foundLength (const char*, unsigned int) - " << get4BytesMSB (length));

   skip.setOffset (get4BytesMSB (length));
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a comment was read
/// \param comment: Pointer to comment in the format <keyword>\0<value>
/// \param len: Length of data
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParsePNG::foundComment (const char* comment, unsigned int len) {
   Check2 (comment);
   TRACE8 ("ParsePNG::foundComment (const char*, unsigned int) - " << comment << " (" << len << ')');

   _chunk[2] = &skip;
   if (!strcmp (comment, "Title"))
      prop.strTitle.assign (comment + 6, len - 6);
   else if (!strcmp (comment, "Author"))
      prop.strAuthor.assign (comment + 7, len - 7);
   else if (!strcmp (comment, "Description"))
      prop.strComment.assign (comment + 12, len - 12);

   return YGP::ParseObject::PARSE_OK;
}
