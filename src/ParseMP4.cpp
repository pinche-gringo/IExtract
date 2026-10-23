// PROJECT     : Extract
// SUBSYSTEM   : ParseMP4
// REFERENCES  : ISO/IEC 14496-12; QuickTime File Format
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

#include <YGP/Exception.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "ParseXMP.h"
#include "Properties.h"

#include "ParseMP4.h"

// Types of boxes
static constexpr unsigned int BOX_MOOV(0x6d6f6f76); // "moov"
static constexpr unsigned int BOX_UDTA(0x75647461); // "udta"
static constexpr unsigned int BOX_META(0x6d657461); // "meta"
static constexpr unsigned int BOX_ILST(0x696c7374); // "ilst"
static constexpr unsigned int BOX_DATA(0x64617461); // "data"
static constexpr unsigned int BOX_HDLR(0x68646c72); // "hdlr"
static constexpr unsigned int BOX_UUID(0x75756964); // "uuid"
static constexpr unsigned int BOX_XMP(0x584d505f);  // "XMP_"
static constexpr unsigned int BOX_NAME(0xa96e616d); // "©nam"
static constexpr unsigned int BOX_ART(0xa9415254);  // "©ART"
static constexpr unsigned int BOX_ALB(0xa9616c62);  // "©alb"
static constexpr unsigned int BOX_CMT(0xa9636d74);  // "©cmt"

// UUID of the box holding XMP data
static const std::string UUID_XMP("\xbe\x7a\xcf\xcb\x97\xa9\x42\xe8\x9c\x71\x99\x94\x91\xe3\xaf\xac", 16);

// Maximal nesting of boxes
static constexpr unsigned int MAX_DEPTH(8);

/// Values found while parsing the boxes
struct ParseMP4::Values {
    Properties& result;
    std::string comment;
    std::string xmp;
};

//-----------------------------------------------------------------------------
/// Parses the MPEG-4 file. The top-level boxes are read; only the movie box
/// (and a box with XMP data) is read into memory and inspected
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid file
//-----------------------------------------------------------------------------
void ParseMP4::parse(std::istream& stream, Properties& result) {
    result.strComment = result.strTitle = result.strAuthor = "";

    stream.seekg(0, std::ios::end);
    const std::size_t sizeFile(stream.tellg());
    stream.seekg(0, std::ios::beg);

    Values values{result, {}, {}};
    bool moov(false);
    std::size_t pos(0);
    std::array<char, 16> header;
    while ((pos + 8) <= sizeFile) {
        // Box: Size (MSB first; 1: 64 bit size following the type; 0: up to the
        // end of the file), type
        stream.seekg(pos, std::ios::beg);
        if (!stream.read(header.data(), 8))
            break;
        std::size_t size(YGP::get4BytesMSB(header.data()));
        unsigned int type(YGP::get4BytesMSB(header.data() + 4));
        std::size_t lenHeader(8);
        if (size == 1) {
            if (!stream.read(header.data() + 8, 8))
                break;
            size = (static_cast<std::size_t>(YGP::get4BytesMSB(header.data() + 8)) << 32) + YGP::get4BytesMSB(header.data() + 12);
            lenHeader = 16;
        }
        else if (!size)
            size = sizeFile - pos;
        TRACE5("ParseMP4::parse (std::istream&, Properties&) - Box " << std::string(header.data() + 4, 4) << ": " << size
                                                                     << " bytes");
        if ((size < lenHeader) || (size > (sizeFile - pos)))
            throw YGP::ParseError(_("Invalid size of box"));

        if (!pos && (type != 0x66747970)) // "ftyp"
            throw YGP::ParseError(_("Invalid MPEG-4 file"));

        if ((type == BOX_MOOV) || (type == BOX_UUID)) {
            std::string data(size - lenHeader, '\0');
            if (!stream.read(data.data(), data.size()))
                throw YGP::ParseError(_("Unexpected end of file!"));
            if (type == BOX_MOOV) {
                moov = true;
                parseBoxes(data, 0, data.size(), values, BOX_MOOV, 0);
            }
            else if (!data.compare(0, UUID_XMP.size(), UUID_XMP))
                values.xmp = data.substr(UUID_XMP.size());
        }
        pos += size;
    }
    if (!moov)
        throw YGP::ParseError(_("MPEG-4 file does not contain a movie box"));

    if (result.strComment.empty())
        result.strComment = values.comment;
    if (!values.xmp.empty())
        ParseXMP::parse(values.xmp, result);
}

//-----------------------------------------------------------------------------
/// Parses the boxes in the passed range of the data (recursively for the
/// containers of interest)
/// \param data: Data holding the boxes
/// \param start: Start of the boxes
/// \param end: End of the boxes
/// \param values: In/Out: Found values
/// \param parent: Type of the box containing the boxes
/// \param depth: Nesting level of the boxes
/// \throw YGP::ParseError: In case of an invalid box
//-----------------------------------------------------------------------------
void ParseMP4::parseBoxes(const std::string& data, std::size_t start, std::size_t end, Values& values, unsigned int parent,
                          unsigned int depth) {
    if (depth > MAX_DEPTH)
        throw YGP::ParseError(_("Boxes are nested too deeply"));

    while ((start + 8) <= end) {
        std::size_t size(YGP::get4BytesMSB(data.data() + start));
        unsigned int type(YGP::get4BytesMSB(data.data() + start + 4));
        if (!size)
            size = end - start;
        if ((size < 8) || (size > (end - start)))
            throw YGP::ParseError(_("Invalid size of box"));
        TRACE8("ParseMP4::parseBoxes (...) - " << depth << ": " << data.substr(start + 4, 4) << ": " << size << " bytes");

        std::size_t posData(start + 8);
        switch (type) {
        case BOX_UDTA:
        case BOX_ILST:
            parseBoxes(data, posData, start + size, values, type, depth + 1);
            break;

        case BOX_META:
            // In MPEG-4 files the box has a version and flags (4 bytes); in
            // QuickTime files the children follow directly
            if (((posData + 12) <= (start + size)) && (YGP::get4BytesMSB(data.data() + posData + 4) != BOX_HDLR))
                posData += 4;
            parseBoxes(data, posData, start + size, values, type, depth + 1);
            break;

        case BOX_XMP:
            if (parent == BOX_UDTA)
                values.xmp = data.substr(posData, size - 8);
            break;

        case BOX_NAME:
        case BOX_ART:
        case BOX_ALB:
        case BOX_CMT:
            if (parent == BOX_ILST) {
                // Item holding a data box: Size, type, type of the value, locale
                // (4 bytes each), value
                if (((posData + 16) <= (start + size)) && (YGP::get4BytesMSB(data.data() + posData + 4) == BOX_DATA)) {
                    std::size_t lenData(YGP::get4BytesMSB(data.data() + posData));
                    if ((lenData >= 16) && (lenData <= (start + size - posData)))
                        foundValue(type, data.substr(posData + 16, lenData - 16), values);
                }
            }
            else if ((parent == BOX_UDTA) && ((posData + 4) <= (start + size))) {
                // QuickTime user data text: Length, language (2 bytes each), text
                std::size_t len(YGP::get2BytesMSB(data.data() + posData));
                if ((posData + 4 + len) <= (start + size))
                    foundValue(type, data.substr(posData + 4, len), values);
            }
            break;
        }
        start += size;
    }
}

//-----------------------------------------------------------------------------
/// Stores a found value; the first found is used
/// \param type: Type of the value
/// \param value: Value
/// \param values: In/Out: Found values
//-----------------------------------------------------------------------------
void ParseMP4::foundValue(unsigned int type, const std::string& value, Values& values) {
    TRACE8("ParseMP4::foundValue (unsigned int, const std::string&, Values&) - " << std::hex << type << std::dec << ": "
                                                                                 << value);
    std::string& entry((type == BOX_NAME)  ? values.result.strTitle
                       : (type == BOX_ART) ? values.result.strAuthor
                       : (type == BOX_ALB) ? values.result.strComment
                                           : values.comment);
    if (entry.empty())
        entry = value;
}
