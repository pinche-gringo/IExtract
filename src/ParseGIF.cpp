//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : GIF-parser
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 2005-11-26
//COPYRIGHT   : Copyright (C) 2005, 2006, 2008

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

#include "Properties.h"

#include "ParseGIF.h"


//-----------------------------------------------------------------------------
/// Parses the GIF image
/// \param stream: Stream to parse
/// \throw YGP::ParseError: In case of an invalid image
//-----------------------------------------------------------------------------
void ParseGIF::parse (YGP::Xistream& stream) {
   namespace x3 = boost::spirit::x3;
   using SpiritParser::bytes;
   using SpiritParser::skip;

   std::size_t length (0);

   // Packed flags specifying a colour table: Sets its length
   auto colourTable = x3::byte_[([&length](auto& ctx) {
         unsigned int flag (x3::_attr (ctx));
         length = (flag & 0x80) ? (1 << ((flag & 0x7) + 1)) * 3 : 0;
         TRACE8 ("ParseGIF::parse (YGP::Xistream&) - Colour table: " << length << " bytes"); })];
   auto setLength = [&length](auto& ctx) { length = x3::_attr (ctx); };
   auto addComment = [this](auto& ctx) { prop.strComment += x3::_attr (ctx).c_str (); };

   // Data sub-blocks (length and data); terminated by an empty block
   auto lenBlock = (x3::byte_ - x3::byte_ (0))[setLength];
   auto subblocks = *(lenBlock >> skip (length)) >> x3::byte_ (0);
   auto commentBlocks = *(lenBlock >> bytes (length)[addComment]) >> x3::byte_ (0);

   auto commentExt = x3::lit ("\x21\xfe") >> commentBlocks;
   auto imageDesc = x3::lit ('\x2c') >> x3::repeat (8)[x3::byte_] >> colourTable
      >> x3::byte_ >> skip (length) >> subblocks;
   auto extension = x3::lit ('\x21') >> x3::byte_ >> subblocks;

   auto image = x3::lit ("GIF") >> x3::repeat (7)[x3::byte_] >> colourTable
      >> x3::repeat (2)[x3::byte_] >> skip (length)
      >> *(commentExt | imageDesc | extension) >> x3::lit ('\x3b');

   SpiritParser::parse (SpiritParser::readStream (stream), image, _("GIF image"));
}
