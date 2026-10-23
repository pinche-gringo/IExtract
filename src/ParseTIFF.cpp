// PROJECT     : Extract
// SUBSYSTEM   : ParseTIFF
// REFERENCES  : TIFF 6.0 specification; EXIF 2.3 specification
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 23.10.2026
// COPYRIGHT   : Copyright (C) 2026

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

#include "ParseTIFF.h"
#include "ParseXMP.h"
#include "Properties.h"

// Tags in the image file directory
static const unsigned int TAG_DESCRIPTION(0x010e);
static const unsigned int TAG_ARTIST(0x013b);
static const unsigned int TAG_XMP(0x02bc);
static const unsigned int TAG_XPTITLE(0x9c9b);
static const unsigned int TAG_XPCOMMENT(0x9c9c);
static const unsigned int TAG_XPAUTHOR(0x9c9d);

// Types of the values
static const unsigned int TYPE_BYTE(1);
static const unsigned int TYPE_ASCII(2);
static const unsigned int TYPE_UNDEFINED(7);

//-----------------------------------------------------------------------------
/// Parses the TIFF image
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid image
//-----------------------------------------------------------------------------
void ParseTIFF::parse(std::istream& stream, Properties& result) {
    result.strTitle.clear();
    result.strAuthor.clear();
    result.strComment.clear();
    parseIFD(SpiritParser::readStream(stream), result);
}

//-----------------------------------------------------------------------------
/// Parses the (first) image file directory of TIFF data for title, comment and
/// author. Only empty properties are set.
/// \param tiff: TIFF data (starting with the byte order)
/// \param result: In/Out: Found information
/// \returns bool: True, if the image file directory contains entries
/// \throw YGP::ParseError: In case of invalid data
//-----------------------------------------------------------------------------
bool ParseTIFF::parseIFD(const std::string& tiff, Properties& result) {
    namespace x3 = boost::spirit::x3;

    // Values of the Windows XP tags and of the "standard" (ASCII) tags
    Properties xp, ascii;
    std::string xmp;

    // Parses the image file directory with the passed parsers for words and
    // double words (of the byte order of the data)
    auto parse = [&](auto word, auto dword) {
        std::size_t offset(0), count(0);
        unsigned int tag(0), type(0);
        std::size_t cValues(0);

        auto setValue = [](auto& value) { return [&value](auto& ctx) { value = x3::_attr(ctx); }; };
        auto entry = [&](auto& ctx) {
            // The value is stored in the entry, if it fits into the 4 bytes of the
            // offset
            std::size_t posValue((cValues > 4) ? static_cast<std::size_t>(x3::_attr(ctx))
                                               : static_cast<std::size_t>(x3::_where(ctx).begin() - tiff.data()));
            TRACE8("ParseTIFF::parseIFD (const std::string&, Properties&) - Tag " << std::hex << tag << std::dec << " (" << type
                                                                                  << "): " << cValues << " @ " << posValue);
            if (posValue >= tiff.size())
                return;

            std::string value(tiff.substr(posValue, cValues));
            switch (tag) {
            case TAG_XPTITLE:
            case TAG_XPCOMMENT:
            case TAG_XPAUTHOR:
                if (type == TYPE_BYTE)
                    xp.*((tag == TAG_XPTITLE)    ? &Properties::strTitle
                         : (tag == TAG_XPAUTHOR) ? &Properties::strAuthor
                                                 : &Properties::strComment) = value;
                break;

            case TAG_DESCRIPTION:
            case TAG_ARTIST:
                if (type == TYPE_ASCII) {
                    value.erase(value.find_last_not_of('\0') + 1);
                    ascii.*((tag == TAG_ARTIST) ? &Properties::strAuthor : &Properties::strComment) = value;
                }
                break;

            case TAG_XMP:
                if ((type == TYPE_BYTE) || (type == TYPE_UNDEFINED))
                    xmp = value;
                break;
            }
        };

        SpiritParser::Iterator act(tiff.data());
        SpiritParser::parse(act, tiff.data() + tiff.size(), x3::repeat(2)[x3::byte_] >> word >> dword[setValue(offset)],
                            _("TIFF header"));
        if (offset >= tiff.size())
            throw YGP::ParseError(_("Invalid offset of image file directory"));

        act = tiff.data() + offset;
        SpiritParser::parse(
            act, tiff.data() + tiff.size(),
            word[setValue(count)] >>
                SpiritParser::times(
                    count)[word[setValue(tag)] >> word[setValue(type)] >> dword[setValue(cValues)] >> dword[entry]],
            _("image file directory"));
        return count > 0;
    };

    bool found(false);
    if (!tiff.compare(0, 2, "II"))
        found = parse(x3::little_word, x3::little_dword);
    else if (!tiff.compare(0, 2, "MM"))
        found = parse(x3::big_word, x3::big_dword);
    else
        throw YGP::ParseError(_("Invalid byte order of TIFF data"));

    for (auto member : {&Properties::strTitle, &Properties::strAuthor, &Properties::strComment})
        if ((result.*member).empty())
            result.*member = (xp.*member).empty() ? ascii.*member : xp.*member;
    if (!xmp.empty())
        ParseXMP::parse(xmp, result);
    return found;
}
