// PROJECT     : Extract
// SUBSYSTEM   : PNG-parser
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 2005-08-02
// COPYRIGHT   : Copyright (C) 2005, 2007, 2008, 2026

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
#include "ParseXMP.h"
#include "Properties.h"
#ifdef HAVE_LIBZ
#    include "ZipArchive.h"
#endif

static const unsigned int TYPE_TEXT(0x74455874);  // "tEXt"
static const unsigned int TYPE_ZTEXT(0x7a545874); // "zTXt"
static const unsigned int TYPE_ITEXT(0x69545874); // "iTXt"

// Keyword of the international text chunk holding XMP data
static const char* const KEY_XMP("XML:com.adobe.xmp");

//-----------------------------------------------------------------------------
/// Parses the PNG image
/// \param stream: Stream to parse
/// \throw YGP::ParseError: In case of an invalid image
//-----------------------------------------------------------------------------
void ParsePNG::parse(std::istream& stream) {
    namespace x3 = boost::spirit::x3;
    using SpiritParser::bytes;

    std::size_t length(0);
    unsigned int type(0);

    auto setLength = [&length](auto& ctx) { length = x3::_attr(ctx); };
    auto setType = [&type](auto& ctx) { type = x3::_attr(ctx); };
    auto chunkData = [this, &type](auto& ctx) { foundChunk(type, x3::_attr(ctx)); };

    // Chunk: Length (MSB first), type, data, CRC
    auto chunk = x3::big_dword[setLength] >> x3::big_dword[setType] >> bytes(length)[chunkData] >> x3::omit[x3::big_dword];
    auto image = x3::lit("\x89PNG\x0d\x0a\x1a\x0a") >> x3::repeat(2, x3::inf)[chunk];

    SpiritParser::parse(SpiritParser::readStream(stream), image, _("PNG image"));
    if (!xmp.empty())
        ParseXMP::parse(xmp, prop);
}

//-----------------------------------------------------------------------------
/// Handles a chunk of the image; stores the information of known text-chunks
/// \param type: Type of the chunk
/// \param data: Data of the chunk; for text-chunks: <keyword>\0<value> (see
/// the PNG specification for the details of compressed and international
/// text chunks)
//-----------------------------------------------------------------------------
void ParsePNG::foundChunk(unsigned int type, const std::string& data) {
    TRACE8("ParsePNG::foundChunk (unsigned int, const std::string&) - " << std::hex << type << std::dec << ": " << data.size()
                                                                        << " bytes");
    if ((type != TYPE_TEXT) && (type != TYPE_ZTEXT) && (type != TYPE_ITEXT))
        return;

    std::string::size_type pos(data.find('\0'));
    if (pos == std::string::npos)
        return;
    std::string key(data, 0, pos++);

    bool compressed(false);
    if (type == TYPE_ZTEXT) { // Compression method
        compressed = true;
        ++pos;
    }
    else if (type == TYPE_ITEXT) {
        // Compression flag, compression method, language tag, translated keyword
        if ((pos + 2) > data.size())
            return;
        compressed = data[pos];
        pos = data.find('\0', pos + 2);
        if (pos != std::string::npos)
            pos = data.find('\0', pos + 1);
        if (pos++ == std::string::npos)
            return;
    }

    std::string value(data, std::min(pos, data.size()));
    if (compressed) {
#ifdef HAVE_LIBZ
        try {
            value = ZipArchive::inflate(value, false);
        }
        catch (const YGP::ParseError& err) {
            TRACE1("ParsePNG::foundChunk (unsigned int, const std::string&) - " << err.what());
            return;
        }
#else
        return;
#endif
    }

    if (key == "Title")
        prop.strTitle = value;
    else if (key == "Author")
        prop.strAuthor = value;
    else if (key == "Description")
        prop.strComment = value;
    else if ((key == KEY_XMP) && (type == TYPE_ITEXT))
        xmp = value;
}
