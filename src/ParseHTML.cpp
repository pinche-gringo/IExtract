// PROJECT     : Extract
// SUBSYSTEM   : ParseHTML
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 08.10.2002
// COPYRIGHT   : Copyright (C) 2002 - 2005, 2008, 2026

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

#include <algorithm>

#include <YGP/Trace.h>

#include "SpiritParser.h"

#include "ParseHTML.h"
#include "Properties.h"

//-----------------------------------------------------------------------------
/// Parses the header of the HTML document for its title and meta-information
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid document
//-----------------------------------------------------------------------------
void ParseHTML::parse(std::istream& stream, Properties& result) {
    namespace x3 = boost::spirit::x3;
    using SpiritParser::nocase;
    using SpiritParser::ws;

    std::string Properties::* entry(nullptr);
    bool endOfHead(false);

    auto metaName = [&entry](auto& ctx) {
        std::string name(x3::_attr(ctx));
        std::transform(name.begin(), name.end(), name.begin(),
                       [](char ch) { return ((ch >= 'A') && (ch <= 'Z')) ? ch + 'a' - 'A' : ch; });
        TRACE8("ParseHTML::parse (std::istream&, Properties&) - Meta: " << name);
        entry = (((name == "description") || (name == "dc.description")) ? &Properties::strComment
                 : ((name == "author") || (name == "dc.creator"))        ? &Properties::strAuthor
                 : (name == "dc.title")                                  ? &Properties::strTitle
                                                                         : nullptr);
    };
    auto metaValue = [&entry, &result](auto& ctx) {
        if (entry)
            result.*entry = x3::_attr(ctx);
        entry = nullptr;
    };
    auto title = [&result](auto& ctx) { result.strTitle = x3::_attr(ctx); };
    auto notEndOfHead = [&endOfHead](auto& ctx) { x3::_pass(ctx) = !endOfHead; };
    auto foundEndOfHead = [&endOfHead](auto&) { endOfHead = true; };

    auto restOfTag = x3::omit[*~x3::char_('>')];
    auto equal = -(x3::lit('=') >> ws);
    auto quote = -(x3::lit('"') >> ws);

    auto tagTitle =
        nocase("title") >> restOfTag >> x3::lit('>') >> ws >> (*~x3::char_('<'))[title] >> x3::lit('<') >> ws >> nocase("/title");

    // <META NAME="name" CONTENT="value">; other meta-tags are ignored
    auto nameValue = x3::rule<class MetaNameID, std::string>("Name of meta tag") =
        x3::lit('"') >> ws >> *~x3::char_('"') >> x3::lit('"') | +~x3::char_(" \t\n\r>");
    auto metaEntry = nocase("name") >> ws >> equal >> nameValue[metaName] >> ws >> nocase("content") >> ws >> equal >> quote >>
                     (*~x3::char_("\">"))[metaValue] >> quote;
    auto tagMeta = nocase("meta") >> ws >> -metaEntry >> restOfTag;

    auto tagEndHead = nocase("/head")[foundEndOfHead] >> restOfTag;
    auto tagScript = nocase("script") >> restOfTag >> x3::lit('>') >> ws >> x3::omit[*(x3::char_ - nocase("</script"))] >>
                     nocase("</script") >> restOfTag;

    auto tag = x3::lit('<') >> ws >> (tagTitle | tagMeta | tagEndHead | tagScript | restOfTag) >> x3::lit('>') >> ws;
    auto document = *(x3::eps[notEndOfHead] >> (tag | x3::omit[+~x3::char_('<')]));

    SpiritParser::parse(SpiritParser::readStream(stream), document, _("HTML document"));
}
