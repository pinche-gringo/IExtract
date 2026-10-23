// PROJECT     : Extract
// SUBSYSTEM   : ParseWebP
// REFERENCES  : https://developers.google.com/speed/webp/docs/riff_container
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

#include "ParseWebP.h"

static const unsigned int CHUNK_EXIF(0x46495845); // "EXIF"
static const unsigned int CHUNK_XMP(0x20504d58);  // "XMP "

//-----------------------------------------------------------------------------
/// Parses the WebP image (a RIFF container); its chunks are inspected for
/// EXIF and XMP information
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid image
//-----------------------------------------------------------------------------
void ParseWebP::parse(std::istream& stream, Properties& result) {
    namespace x3 = boost::spirit::x3;
    using SpiritParser::bytes;
    using SpiritParser::skip;

    result.strComment = result.strTitle = result.strAuthor = "";

    std::size_t length(0), padding(0);
    unsigned int type(0);
    std::string exif, xmp;

    auto setType = [&type](auto& ctx) { type = x3::_attr(ctx); };
    auto setLength = [&length, &padding](auto& ctx) {
        length = x3::_attr(ctx);
        padding = length & 1;
    };
    auto chunkData = [&](auto& ctx) {
        TRACE8("ParseWebP::parse (std::istream&, Properties&) - Chunk " << std::hex << type << std::dec << ": "
                                                                        << x3::_attr(ctx).size() << " bytes");
        if (type == CHUNK_EXIF)
            exif = x3::_attr(ctx);
        else if (type == CHUNK_XMP)
            xmp = x3::_attr(ctx);
    };

    // Chunk: Type, length (LSB first), data, padding to an even size
    auto chunk = x3::little_dword[setType] >> x3::little_dword[setLength] >> bytes(length)[chunkData] >> skip(padding);
    auto image = x3::lit("RIFF") >> x3::little_dword >> x3::lit("WEBP") >> +chunk;

    SpiritParser::parse(SpiritParser::readStream(stream), image, _("WebP image"));

    // The EXIF information might be prefixed with the identifier used in JPEG
    // images
    if (!exif.compare(0, 6, std::string("Exif\0\0", 6)))
        exif.erase(0, 6);
    if (!exif.empty())
        ParseTIFF::parseIFD(exif, result);
    if (!xmp.empty())
        ParseXMP::parse(xmp, result);
}
