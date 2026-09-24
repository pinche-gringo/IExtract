// PROJECT     : Extract
// SUBSYSTEM   : ParseStarOffice
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 04.11.2002
// COPYRIGHT   : Copyright (C) 2002 - 2008, 2026

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

#include "ParseSOffice.h"
#include "Properties.h"

static const char ID[] = "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1";
static const char IDSTAROFFICE[] = "\x0F\0SfxDocumentInfo";
static const std::size_t POS_DOCINFO(0x8c0);

//-----------------------------------------------------------------------------
/// Parses the StarOffice document
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid document
//-----------------------------------------------------------------------------
void ParseStarOffice::parse(std::istream& stream, Properties& result) {
    namespace x3 = boost::spirit::x3;
    using SpiritParser::bytes;
    using SpiritParser::skip;

    // Entries of the document info; each followed by the offset of the next
    static const struct {
        std::string Properties::* value;
        std::size_t offset;
    } entries[] = {{&Properties::strAuthor, 39}, // Creator
                   {&Properties::strAuthor, 80},
                   {&Properties::strTitle, 128},
                   {&Properties::strComment, 0}};
    const std::size_t cEntries(std::size(entries));

    const std::size_t posDocInfo(POS_DOCINFO - (sizeof(ID) - 1));
    std::size_t offset(7);
    std::size_t length(0);
    std::size_t actEntry(0);

    auto setLength = [&length](auto& ctx) { length = x3::_attr(ctx); };
    auto value = [&](auto& ctx) {
        const std::string& val(x3::_attr(ctx));
        TRACE5("ParseStarOffice::parse (std::istream&, Properties&) - " << actEntry << ": " << val);
        if (val.size())
            result.*(entries[actEntry].value) = val;
        offset = entries[actEntry].offset - val.size();
        ++actEntry;
    };

    auto document = x3::lit(ID) >> skip(posDocInfo) >> x3::lit(std::string(IDSTAROFFICE, sizeof(IDSTAROFFICE) - 1)) >>
                    SpiritParser::times(cEntries)[skip(offset) >> x3::little_word[setLength] >> bytes(length)[value]];

    SpiritParser::parse(SpiritParser::readStream(stream), document, _("StarOffice document"));
}
