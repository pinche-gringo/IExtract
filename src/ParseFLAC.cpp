// PROJECT     : Extract
// SUBSYSTEM   : ParseFLAC
// REFERENCES  : https://xiph.org/flac/format.html
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

#include <array>
#include <string_view>

#include <YGP/Exception.h>
#include <YGP/Trace.h>

#include "ParseOGG.h"
#include "Properties.h"

#include "ParseFLAC.h"

static constexpr unsigned int BLOCK_VORBIS_COMMENT(4);

//-----------------------------------------------------------------------------
/// Parses the FLAC file; the metadata blocks are inspected for the (first)
/// VORBIS_COMMENT block. The audio data is not read.
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid file
//-----------------------------------------------------------------------------
void ParseFLAC::parse(std::istream& stream, Properties& result) {
    result.strComment = result.strTitle = result.strAuthor = "";

    std::array<char, 4> buffer{};
    if (!stream.read(buffer.data(), buffer.size()) || std::string_view(buffer.data(), buffer.size()) != "fLaC")
        throw YGP::ParseError(_("Invalid FLAC file"));

    // Metadata blocks: Flag for the last block and type (1 byte), length (3 bytes MSB first)
    bool last(false);
    while (!last) {
        if (!stream.read(buffer.data(), buffer.size()))
            throw YGP::ParseError(_("Invalid FLAC file"));
        last = buffer[0] & 0x80;
        unsigned int type(buffer[0] & 0x7f);
        std::size_t length((static_cast<unsigned char>(buffer[1]) << 16) + (static_cast<unsigned char>(buffer[2]) << 8) +
                           static_cast<unsigned char>(buffer[3]));
        TRACE8("ParseFLAC::parse (std::istream&, Properties&) - Block " << type << ": " << length << " bytes");

        if (type == BLOCK_VORBIS_COMMENT) {
            std::string data(length, '\0');
            if (!stream.read(data.data(), length))
                throw YGP::ParseError(_("Invalid FLAC file"));
            ParseOGG::parseComments(data, result);
            return;
        }
        stream.seekg(length, std::ios::cur);
    }
}
