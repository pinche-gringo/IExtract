// PROJECT     : Extract
// SUBSYSTEM   : ZipArchive
// REFERENCES  : https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
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

#include <zlib.h>

#include <array>
#include <memory>

#include <YGP/Exception.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "ZipArchive.h"

static constexpr unsigned int ID_LOCALHDR(0x04034b50);
static constexpr unsigned int ID_CFILEHDR(0x02014b50);
static constexpr unsigned int ID_ENDCDR(0x06054b50);

static constexpr std::size_t LEN_LOCALHDR(30);
static constexpr std::size_t LEN_CFILEHDR(46);
static constexpr std::size_t LEN_ENDCDR(22);
static constexpr std::size_t MAX_COMMENT(0xffff);

static constexpr unsigned int METHOD_STORED(0);
static constexpr unsigned int METHOD_DEFLATED(8);

//-----------------------------------------------------------------------------
/// Returns the (decompressed) contents of a file in a ZIP archive
/// \param archive: Contents of the archive
/// \param name: Name of the file (including its path)
/// \returns std::optional<std::string>: Contents of the file; nothing, if the
/// archive does not contain the file
/// \throw YGP::ParseError: In case of an invalid archive
//-----------------------------------------------------------------------------
std::optional<std::string> ZipArchive::getFile(std::string_view archive, std::string_view name) {
    TRACE5("ZipArchive::getFile (std::string_view, std::string_view) - " << name);

    // Search the end of central directory record (followed by a comment of at
    // most 64K)
    if (archive.size() < LEN_ENDCDR)
        throw YGP::ParseError(_("Invalid end of central directory record"));
    std::size_t pos(archive.size() - LEN_ENDCDR);
    const std::size_t stop((pos > MAX_COMMENT) ? (pos - MAX_COMMENT) : 0);
    while (YGP::get4BytesLSB(archive.data() + pos) != ID_ENDCDR)
        if (pos-- == stop)
            throw YGP::ParseError(_("Invalid end of central directory record"));

    unsigned int cEntries(YGP::get2BytesLSB(archive.data() + pos + 10));
    pos = YGP::get4BytesLSB(archive.data() + pos + 16);

    // Central directory: Search the entry of the file
    while (cEntries--) {
        if (((pos + LEN_CFILEHDR) > archive.size()) || (YGP::get4BytesLSB(archive.data() + pos) != ID_CFILEHDR))
            throw YGP::ParseError(_("Invalid central directory"));

        const char* entry(archive.data() + pos);
        std::size_t lenName(YGP::get2BytesLSB(entry + 28));
        pos += LEN_CFILEHDR + lenName + YGP::get2BytesLSB(entry + 30) + YGP::get2BytesLSB(entry + 32);
        if (pos > archive.size())
            throw YGP::ParseError(_("Invalid central directory"));
        if (archive.substr(entry + LEN_CFILEHDR - archive.data(), lenName) != name)
            continue;

        unsigned int method(YGP::get2BytesLSB(entry + 10));
        std::size_t size(YGP::get4BytesLSB(entry + 20));
        std::size_t offset(YGP::get4BytesLSB(entry + 42));
        TRACE5("ZipArchive::getFile (std::string_view, std::string_view) - Found @ " << offset << "; Method " << method << "; "
                                                                                     << size << " bytes");

        // Local header, followed by the data
        if (((offset + LEN_LOCALHDR) > archive.size()) || (YGP::get4BytesLSB(archive.data() + offset) != ID_LOCALHDR))
            throw YGP::ParseError(_("Archive contains an invalid file entry!"));
        offset +=
            LEN_LOCALHDR + YGP::get2BytesLSB(archive.data() + offset + 26) + YGP::get2BytesLSB(archive.data() + offset + 28);
        if ((offset + size) > archive.size())
            throw YGP::ParseError(_("Archive contains an invalid file entry!"));

        std::string_view data(archive.substr(offset, size));
        switch (method) {
        case METHOD_STORED:
            return std::string(data);
        case METHOD_DEFLATED:
            return inflate(data, true);
        default:
            throw YGP::ParseError(_("Unsupported compression method"));
        }
    }
    return std::nullopt;
}

//-----------------------------------------------------------------------------
/// Decompresses deflated data
/// \param data: Compressed data
/// \param raw: True, if the data is raw deflated data (as stored in ZIP
/// archives), false if the data has a zlib header
/// \returns std::string: Decompressed data
/// \throw YGP::ParseError: In case of invalid data
//-----------------------------------------------------------------------------
std::string ZipArchive::inflate(std::string_view data, bool raw) {
    z_stream zStream{};
    if (inflateInit2(&zStream, raw ? -MAX_WBITS : MAX_WBITS) != Z_OK)
        throw YGP::ParseError(_("Can't initialise zlib!"));
    const std::unique_ptr<z_stream, decltype(&inflateEnd)> cleanup(&zStream, &inflateEnd);

    zStream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
    zStream.avail_in = data.size();

    std::string result;
    std::array<char, 4096> output;
    int rc(Z_OK);
    do {
        zStream.avail_out = output.size();
        zStream.next_out = reinterpret_cast<Bytef*>(output.data());
        switch (rc = ::inflate(&zStream, Z_NO_FLUSH)) {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
            throw YGP::ParseError(_("Error inflating stream: Not a deflated file!"));

        case Z_MEM_ERROR:
            throw YGP::ParseError(_("Error inflating stream: Out of memory!"));

        case Z_BUF_ERROR:
            throw YGP::ParseError(_("Unexpected end of file!"));
        }
        result.append(output.data(), output.size() - zStream.avail_out);
    }
    while (rc != Z_STREAM_END);
    return result;
}
