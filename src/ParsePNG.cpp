//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : PNG-parser
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 2005-08-02
//COPYRIGHT   : Copyright (C) 2005, 2007, 2008

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


#include <IExtract-cfg.h>

#include <YGP/Trace.h>

#include "SpiritParser.h"

#include "ParsePNG.h"
#include "Properties.h"


static const unsigned int TYPE_TEXT (0x74455874);                    // "tEXt"


//-----------------------------------------------------------------------------
/// Parses the PNG image
/// \param stream: Stream to parse
/// \throw YGP::ParseError: In case of an invalid image
//-----------------------------------------------------------------------------
void ParsePNG::parse (std::istream& stream) {
   namespace x3 = boost::spirit::x3;
   using SpiritParser::bytes;

   std::size_t length (0);
   unsigned int type (0);

   auto setLength = [&length](auto& ctx) { length = x3::_attr (ctx); };
   auto setType = [&type](auto& ctx) { type = x3::_attr (ctx); };
   auto chunkData = [this, &type](auto& ctx) { foundChunk (type, x3::_attr (ctx)); };

   // Chunk: Length (MSB first), type, data, CRC
   auto chunk = x3::big_dword[setLength] >> x3::big_dword[setType]
      >> bytes (length)[chunkData] >> x3::omit[x3::big_dword];
   auto image = x3::lit ("\x89PNG\x0d\x0a\x1a\x0a") >> x3::repeat (2, x3::inf)[chunk];

   SpiritParser::parse (SpiritParser::readStream (stream), image, _("PNG image"));
}

//-----------------------------------------------------------------------------
/// Handles a chunk of the image; stores the information of known text-chunks
/// \param type: Type of the chunk
/// \param data: Data of the chunk; for text-chunks: <keyword>\0<value>
//-----------------------------------------------------------------------------
void ParsePNG::foundChunk (unsigned int type, const std::string& data) {
   TRACE8 ("ParsePNG::foundChunk (unsigned int, const std::string&) - " << std::hex
           << type << std::dec << ": " << data.size () << " bytes");
   if (type != TYPE_TEXT)
      return;

   std::string::size_type pos (data.find ('\0'));
   if (pos == std::string::npos)
      return;

   std::string key (data, 0, pos);
   if (key == "Title")
      prop.strTitle.assign (data, pos + 1);
   else if (key == "Author")
      prop.strAuthor.assign (data, pos + 1);
   else if (key == "Description")
      prop.strComment.assign (data, pos + 1);
}
