// PROJECT     : Extract
// SUBSYSTEM   : ParseOGG
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 20.03.2005
// COPYRIGHT   : Copyright (C) 2005 - 2008, 2011, 2024, 2026

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

#include <string_view>
#include <vector>

#include <YGP/Trace.h>

#include "SpiritParser.h"

#include "ParseOGG.h"
#include "Properties.h"

//-----------------------------------------------------------------------------
/// Method to actually parse the OGG-file. The first two packets (of the first
/// logical stream) are inspected; supported are Vorbis, Opus and FLAC streams.
/// \param stream: OGG-file to analyze
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid file
//-----------------------------------------------------------------------------
void ParseOGG::parse(std::istream& stream, Properties& result) {
    result.strComment = result.strTitle = result.strAuthor = "";

    const std::string data(SpiritParser::readStream(stream));
    std::vector<std::string> packets(1);
    std::string serial;
    std::size_t pos(0);
    while (packets.size() < 3) {
        // Page header: "OggS", version, type, granule position (8 bytes), serial
        // number, page number, checksum (4 bytes each), number of segments,
        // segment table
        if (((pos + 27) > data.size()) || data.compare(pos, 4, "OggS"))
            throw YGP::ParseError(_("Invalid OGG file"));
        std::size_t cSegments(static_cast<unsigned char>(data[pos + 26]));
        std::size_t posData(pos + 27 + cSegments);
        if (posData > data.size())
            throw YGP::ParseError(_("Invalid OGG file"));

        const std::string_view segments(data.data() + pos + 27, cSegments);
        bool own(serial.empty() || !data.compare(pos + 14, 4, serial));
        if (serial.empty())
            serial = data.substr(pos + 14, 4);

        for (unsigned char length : segments) {
            if ((posData + length) > data.size())
                throw YGP::ParseError(_("Invalid OGG file"));
            if (own) {
                packets.back().append(data, posData, length);
                if (length < 255)
                    packets.emplace_back();
            }
            posData += length;
        }
        pos = posData;
    }

    TRACE5("ParseOGG::parse (std::istream&, Properties&) - Packets: " << packets[0].size() << ", " << packets[1].size());
    std::size_t offset(0);
    if (!packets[0].compare(0, 7, "\x01vorbis") && !packets[1].compare(0, 7, "\x03vorbis"))
        offset = 7;
    else if (!packets[0].compare(0, 8, "OpusHead") && !packets[1].compare(0, 8, "OpusTags"))
        offset = 8;
    else if (!packets[0].compare(0, 5,
                                 "\x7f"
                                 "FLAC") &&
             (packets[1].size() > 4) && ((packets[1][0] & 0x7f) == 4))
        offset = 4; // Header of the VORBIS_COMMENT metadata block
    else
        throw YGP::ParseError(_("Unsupported OGG stream"));
    parseComments(packets[1].substr(offset), result);
}

//-----------------------------------------------------------------------------
/// Parses a Vorbis comment header (vendor string and the comments); as used
/// by Vorbis, Opus and FLAC
/// \param data: Comment header
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid header
//-----------------------------------------------------------------------------
void ParseOGG::parseComments(const std::string& data, Properties& result) {
    namespace x3 = boost::spirit::x3;
    using SpiritParser::bytes;
    using SpiritParser::skip;

    std::size_t length(0);
    std::size_t count(0);

    auto setLength = [&length](auto& ctx) { length = x3::_attr(ctx); };
    auto setCount = [&count](auto& ctx) { count = x3::_attr(ctx); };
    auto comment = [&result](auto& ctx) { foundComment(x3::_attr(ctx), result); };

    auto comments = x3::little_dword[setLength] >> skip(length) // Vendor string
                    >> x3::little_dword[setCount] >>
                    SpiritParser::times(count)[x3::little_dword[setLength] >> bytes(length)[comment]];

    SpiritParser::parse(data, comments, _("comment header"));
}

//-----------------------------------------------------------------------------
/// Stores the found comment-entry, if it is of interest
/// \param comment: Found comment entry (in the format <key>=<value>)
/// \param result: Out: Found information
//-----------------------------------------------------------------------------
void ParseOGG::foundComment(const std::string& comment, Properties& result) {
    TRACE8("ParseOGG::foundComment (const std::string&, Properties&) - " << comment);

    std::string::size_type pos(comment.find('='));
    if ((pos == std::string::npos) || (pos == (comment.size() - 1)))
        return;

    std::string key(comment, 0, pos);
    if ((key == "TITLE") || (key == "title"))
        result.strTitle.append(comment, pos + 1);
    else if ((key == "ALBUM") || (key == "album"))
        result.strComment.append(comment, pos + 1);
    else if ((key == "ARTIST") || (key == "artist"))
        result.strAuthor.append(comment, pos + 1);
}
