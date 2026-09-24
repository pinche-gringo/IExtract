//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseOGG
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 20.03.2005
//COPYRIGHT   : Copyright (C) 2005 - 2008, 2011, 2024

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

#include "ParseOGG.h"
#include "Properties.h"


//-----------------------------------------------------------------------------
/// Method to actually parse the OGG-file
/// \param stream: OGG-file to analyze
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid file
//-----------------------------------------------------------------------------
void ParseOGG::parse (YGP::Xistream& stream, Properties& result) {
   namespace x3 = boost::spirit::x3;
   using SpiritParser::bytes;
   using SpiritParser::skip;

   result.strComment = result.strTitle = result.strAuthor = "";

   std::size_t length (0);
   std::size_t count (0);

   auto setLength = [&length](auto& ctx) { length = x3::_attr (ctx); };
   auto setCount = [&count](auto& ctx) { count = x3::_attr (ctx); };
   auto comment = [&result](auto& ctx) { foundComment (x3::_attr (ctx), result); };

   // Skip the first page (holding the identification header) and the start
   // of the header of the second page, which holds the comment header
   auto ogg = x3::lit ("OggS") >> x3::repeat (0x50)[x3::byte_]
      >> x3::byte_[setLength] >> skip (length)                // Segment table
      >> x3::lit ("\x03vorbis")
      >> x3::little_dword[setLength] >> skip (length)         // Vendor string
      >> x3::little_dword[setCount]
      >> SpiritParser::times (count)[x3::little_dword[setLength] >> bytes (length)[comment]];

   SpiritParser::parse (SpiritParser::readStream (stream), ogg, _("OGG file"));
}

//-----------------------------------------------------------------------------
/// Stores the found comment-entry, if it is of interest
/// \param comment: Found comment entry (in the format <key>=<value>)
/// \param result: Out: Found information
//-----------------------------------------------------------------------------
void ParseOGG::foundComment (const std::string& comment, Properties& result) {
   TRACE8 ("ParseOGG::foundComment (const std::string&, Properties&) - " << comment);

   std::string::size_type pos (comment.find ('='));
   if ((pos == std::string::npos) || (pos == (comment.size () - 1)))
      return;

   std::string key (comment, 0, pos);
   if ((key == "TITLE") || (key == "title"))
      result.strTitle.append (comment, pos + 1);
   else if ((key == "ALBUM") || (key == "album"))
      result.strComment.append (comment, pos + 1);
   else if ((key == "ARTIST") || (key == "artist"))
      result.strAuthor.append (comment, pos + 1);
}
