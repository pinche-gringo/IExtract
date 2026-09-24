//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseAbiword
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 4.06.2005
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
#include "ParseAbiword.h"


static const char* ID_TITLE      ("m key=\"dc.title\"");
static const char* ID_COMMENT    ("m key=\"dc.description\"");
static const char* ID_AUTHOR     ("m key=\"dc.creator\"");


//-----------------------------------------------------------------------------
/// Parses the AbiWord document
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid document
//-----------------------------------------------------------------------------
void ParseAbiword::parse (std::istream& stream, Properties& result) {
   namespace x3 = boost::spirit::x3;
   using SpiritParser::ws;

   std::string Properties::* entry (nullptr);

   auto tag = [&entry](auto& ctx) {
      const std::string& tag (x3::_attr (ctx));
      TRACE8 ("ParseAbiword::parse (std::istream&, Properties&) - Tag: " << tag);
      entry = ((tag == ID_TITLE) ? &Properties::strTitle
               : (tag == ID_COMMENT) ? &Properties::strComment
               : (tag == ID_AUTHOR) ? &Properties::strAuthor : nullptr); };
   auto value = [&entry, &result](auto& ctx) {
      if (entry)
         result.*entry = x3::_attr (ctx);
      entry = nullptr; };

   auto endMetadata = x3::lit ("</metadata>");
   auto metaEntry = x3::lit ('<') >> (*~x3::char_ ('>'))[tag] >> x3::lit ('>') >> ws
      >> (*~x3::char_ ('<'))[value];
   auto metadata = x3::lit ("<metadata>") >> ws >> *(!endMetadata >> metaEntry) >> endMetadata;
   auto document = *(x3::char_ - x3::lit ("<metadata>")) >> -metadata;

   SpiritParser::parse (SpiritParser::readStream (stream), document, _("AbiWord document"));
}
