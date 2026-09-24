// PROJECT     : Extract
// SUBSYSTEM   : ParseMSOffice
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 8.10.2002
// COPYRIGHT   : Copyright (C) 2002 - 2008, 2011, 2024, 2026

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
#include <cstring>
#include <format>
#include <map>
#include <string_view>

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "ParseMSOffice.h"
#include "Properties.h"

static constexpr std::string_view SUMMARY("\x05\0\x53\0\x75\0\x6D\0\x6D\0\x61\0\x72\0\x79\0"
                                          "\x49\0\x6E\0\x66\0\x6F\0\x72\0\x6D\0\x61\0\x74\0"
                                          "\x69\0\x6F\0\x6E\0\0",
                                          0x28);
static constexpr std::string_view ROOTENTRY("R\0o\0o\0t\0 \0E\0n\0t\0r\0y\0\0", 0x16);
static constexpr std::string_view SECTIONID("\x01\0\0\0\xE0\x85\x9F\xF2\xF9\x4F"
                                            "\x68\x10\xAB\x91\x08\0\x2B\x27\xB3\xD9",
                                            0x14);

static constexpr std::uint32_t TYPE_TITLE = 2;
static constexpr std::uint32_t TYPE_AUTHOR = 4;
static constexpr std::uint32_t TYPE_COMMENT = 6;

static constexpr std::uint32_t LAST_BLOCK = 0x80000000; // Block-indices above are special

//-----------------------------------------------------------------------------
/// Tries to parse a Microsoft Office documnent
/// \param stream Stream to read from
/// \param result Structure to hold the found information
/// \throw YGP::ParseError In case of error a describing text
//-----------------------------------------------------------------------------
void ParseMSOffice::parse(std::istream& stream, Properties& result) {
    TRACE9("ParseMSOffice::parse (std::istream&, Properties&)");

    // Read the header (the first block)
    std::array<char, 512> header;
    stream.read(header.data(), header.size());
    if (!stream)
        throw YGP::ParseError(_("Can't read document header!"));

    if (std::string_view(header.data(), 8) != "\xD0\xCF\x11\xE0\xA1\xB1\x1A\xE1")
        throw YGP::ParseError(_("Office identifier not found!"));

    // Blocksizes
    const unsigned int sizeBlock(YGP::get2BytesLSB(header.data() + 0x1e));
    const unsigned int sizeBlockSmall(YGP::get2BytesLSB(header.data() + 0x20));
    TRACE8("ParseMSOffice::parse (std::istream&, Properties&) - Sizes: " << (1 << sizeBlock) << '/' << (1 << sizeBlockSmall));
    if ((sizeBlockSmall > sizeBlock) || (sizeBlock > 512))
        throw YGP::ParseError(_("Values for blocksizes are not plausible!"));
    const unsigned int lenBlock(1 << sizeBlock);

    // Count of BAT blocks and offset of properties
    const std::uint32_t cBAT(YGP::get4BytesLSB(header.data() + 0x2c));
    if (cBAT > 109)
        throw YGP::ParseError(_("Number of blocks for BAT not plausible!"));

    std::uint32_t offProperties(YGP::get4BytesLSB(header.data() + 0x30));
    TRACE6("ParseMSOffice::parse (std::istream&, Properties&) - Properties: " << std::hex << offProperties << " -> "
                                                                              << ((offProperties << sizeBlock) + 512));

    // Read the property-table
    std::vector<char> block(lenBlock);
    readBlock(stream, offProperties, block.data(), sizeBlock);

    // First entry must be the root-entry (which has a pointer to the small block
    // area
    if ((YGP::get2BytesLSB(block.data() + 0x40) != ROOTENTRY.size()) ||
        (std::string_view(block.data(), ROOTENTRY.size()) != ROOTENTRY)) {
        std::string error(_("Not a property block at offset %1!"));
        error.replace(error.find("%1"), 2, std::format("{:X}", (offProperties << sizeBlock) + 512));
        throw YGP::ParseError(error);
    }
    const std::uint32_t offSBA(YGP::get4BytesLSB(block.data() + 0x74));

    // Read the block allocation table
    const BAT bat(readBAT(stream, header.data() + 0x4c, cBAT, sizeBlock));

    const char* entry(block.data() + 0x80);
    while (true) {
        for (; entry < (block.data() + lenBlock); entry += 0x80) {
            TRACE9("ParseMSOffice::parse (std::istream&, Properties&) - Entry with "
                   << std::hex << YGP::get2BytesLSB(entry + 0x40) << " bytes");

            // Check for InformationSummary-block
            if ((YGP::get2BytesLSB(entry + 0x40) != SUMMARY.size()) || (std::string_view(entry, SUMMARY.size()) != SUMMARY))
                continue;

            std::int32_t length(YGP::get4BytesLSB(entry + 0x78));
            std::uint32_t offBlock(YGP::get4BytesLSB(entry + 0x74));
            TRACE8("ParseMSOffice::parse (std::istream&, Properties&) - Info in " << std::hex << offBlock << std::dec << "; "
                                                                                  << length << " bytes");

            const unsigned int cBlocks((length & (lenBlock - 1)) ? ((length >> sizeBlock) + 1) : (length >> sizeBlock));
            std::vector<char> infoBlock;
            // Files less than 4K are stored in small blocks
            if (length < 4096) {
                // Read SBAT table
                const std::vector<char> dataSBAT(readFile(stream, YGP::get4BytesLSB(header.data() + 0x3c), bat,
                                                          YGP::get4BytesLSB(header.data() + 0x40), sizeBlock));
                BAT sbat(dataSBAT.size() / sizeof(BAT::value_type));
                std::memcpy(sbat.data(), dataSBAT.data(), sbat.size() * sizeof(BAT::value_type));

                Check3(static_cast<std::uint32_t>(length) <= (cBlocks << sizeBlock));
                infoBlock.resize(cBlocks << sizeBlock);
                char* actPos(infoBlock.data());

                // Read the small file
                const unsigned int lenSmallBlock(1 << sizeBlockSmall);
                const unsigned int cSmallBlocks(1 << (sizeBlock - sizeBlockSmall));
                std::map<unsigned int, std::vector<char>> readBlocksSBA;
                while (length > 0) {
                    const unsigned int bigBlock(getBlock(bat, offSBA, offBlock >> (sizeBlock - sizeBlockSmall)));
                    auto [pos, inserted](readBlocksSBA.try_emplace(bigBlock));
                    if (inserted) {
                        pos->second.resize(lenBlock);
                        readBlock(stream, bigBlock, pos->second.data(), sizeBlock);
                    }
                    std::memcpy(actPos, pos->second.data() + ((offBlock % cSmallBlocks) << sizeBlockSmall), lenSmallBlock);
                    actPos += lenSmallBlock;
                    length -= lenSmallBlock;
                    offBlock = nextBlock(sbat, offBlock);
                } // end-while
            }
            else
                infoBlock = readFile(stream, offBlock, bat, cBlocks, sizeBlock);

            // Check info-block
            TRACE9("ParseMSOffice::parse (std::istream&, Properties&) - InfoSummary: "
                   << std::hex << YGP::get2BytesLSB(infoBlock.data()) << std::dec);
            if ((YGP::get2BytesLSB(infoBlock.data()) != 0xfffe) ||
                (std::string_view(infoBlock.data() + 0x18, SECTIONID.size()) != SECTIONID))
                throw YGP::ParseError(_("Not an Information-Summary section!"));

            TRACE9("ParseMSOffice::parse (std::istream&, Properties&) - Start section: "
                   << std::hex << YGP::get4BytesLSB(infoBlock.data() + 0x2c) << std::dec);
            const char* actPos(infoBlock.data() + YGP::get4BytesLSB(infoBlock.data() + 0x2c));
            unsigned int entries(YGP::get4BytesLSB(actPos + 4));
            TRACE3("ParseMSOffice::parse (std::istream&, Properties&) - Entries: " << entries);
            if (entries > 50)
                throw YGP::ParseError(_("Number of information entries not plausible!"));

            for (entry = actPos + 8; entries--; entry += 8) {
                const std::uint32_t offset(YGP::get4BytesLSB(entry + 4));
                TRACE7("ParseMSOffice::parse (std::istream&, Properties&) - Entry: " << YGP::get4BytesLSB(entry) << ": Offset "
                                                                                     << std::hex << offset << std::dec);

                if ((offset >= YGP::get4BytesLSB(actPos)) ||
                    ((YGP::get4BytesLSB(actPos + offset) == 0x1e)
                         ? ((YGP::get4BytesLSB(actPos + offset + 4) >= YGP::get4BytesLSB(actPos)) ||
                            (offset < (YGP::get4BytesLSB(actPos + 4) << 3)))
                         : false))
                    throw YGP::ParseError(_("Values of entry not plausible!"));

                std::string Properties::* value(nullptr);
                switch (YGP::get4BytesLSB(entry)) {
                case TYPE_TITLE:
                    value = &Properties::strTitle;
                    break;
                case TYPE_AUTHOR:
                    value = &Properties::strAuthor;
                    break;
                case TYPE_COMMENT:
                    value = &Properties::strComment;
                    break;
                } // end-switch
                if (value) {
                    const std::uint32_t len(YGP::get4BytesLSB(actPos + offset + 4));
                    Check3(actPos[offset + 8 + len - 1] == '\0');
                    (result.*value).assign(actPos + offset + 8, len - 1);
                }
            } // end-for
            return;
        } // end-for

        if ((offProperties = nextBlock(bat, offProperties)) > LAST_BLOCK)
            break;

        readBlock(stream, offProperties, block.data(), sizeBlock);
        entry = block.data();
    } // end-while
}

//-----------------------------------------------------------------------------
/// Reads the specified block from the stream
/// \param stream Stream to read from
/// \param offBlock Index of block to read
/// \param block Block to read into
/// \param sizeBlock Size (as exponent of 2) of the block
/// \throw YGP::ParseError In case of error a describing text
//-----------------------------------------------------------------------------
void ParseMSOffice::readBlock(std::istream& stream, unsigned int offBlock, char* block, unsigned int sizeBlock) {
    TRACE9("ParseMSOffice::readBlock (std::istream&, unsigned int, char*, "
           "unsigned int): "
           << offBlock);
    stream.seekg((offBlock << sizeBlock) + 512, std::ios_base::beg);
    stream.read(block, 1 << sizeBlock);
    if (!stream) {
        std::string error(_("Can't read block at offset %1"));
        error.replace(error.find("%1"), 2, std::format("{:X}", (offBlock << sizeBlock) + 512));
        throw YGP::ParseError(error);
    }
}

//-----------------------------------------------------------------------------
/// Reads the block allocation table
/// \param stream Stream to read from
/// \param pBATBlocks Pointer to the block-indexes of the BAT
/// \param cBlocks Number of BAT blocks
/// \param sizeBlock Size (as exponent of 2) of the blocks
/// \returns BAT Read block allocation table
/// \throw YGP::ParseError In case of error a describing text
//-----------------------------------------------------------------------------
ParseMSOffice::BAT ParseMSOffice::readBAT(std::istream& stream, const char* pBATBlocks, unsigned int cBlocks,
                                          unsigned int sizeBlock) {
    TRACE9("ParseMSOffice::readBAT (std::istream&, const char*, 2x unsigned) - " << cBlocks);

    BAT bat((cBlocks << sizeBlock) / sizeof(BAT::value_type));
    for (unsigned int i(0); i < cBlocks; ++i)
        readBlock(stream, YGP::get4BytesLSB(pBATBlocks + (i << 2)), reinterpret_cast<char*>(bat.data()) + (i << sizeBlock),
                  sizeBlock);
    return bat;
}

//-----------------------------------------------------------------------------
/// Reads a file starting at the passed block.
/// \param stream Stream to read from
/// \param offBlock Index of startblock
/// \param bat Used block allocation table
/// \param blocks Number of block to read
/// \param sizeBlock Size (as exponent of 2) of the block
/// \returns std::vector<char> Contents of the file
/// \throw YGP::ParseError In case of error a describing text
//-----------------------------------------------------------------------------
std::vector<char> ParseMSOffice::readFile(std::istream& stream, unsigned int offBlock, const BAT& bat, unsigned int blocks,
                                          unsigned int sizeBlock) {
    std::vector<char> file(blocks << sizeBlock);
    for (char* actPos(file.data()); blocks--; actPos += 1 << sizeBlock) {
        Check3(offBlock < LAST_BLOCK);
        readBlock(stream, offBlock, actPos, sizeBlock);
        if (blocks)
            offBlock = nextBlock(bat, offBlock);
    } // end-for
    return file;
}

//-----------------------------------------------------------------------------
/// Gets the block following the passed one
/// \param bat Block allocation table to use
/// \param block Actual block
/// \returns unsigned int Index of next block
/// \throw YGP::ParseError If the passed block is not in the table
//-----------------------------------------------------------------------------
unsigned int ParseMSOffice::nextBlock(const BAT& bat, unsigned int block) {
    if (block >= bat.size())
        throw YGP::ParseError(_("BAT not valid!"));
    return bat[block];
}

//-----------------------------------------------------------------------------
/// Gets the nth block from the passed BAT
/// \param bat Block allocation table to use
/// \param start Start block
/// \param nr Nth block to find
/// \returns unsigned int Offset of block
/// \throw YGP::ParseError In case of error a describing text
//-----------------------------------------------------------------------------
unsigned int ParseMSOffice::getBlock(const BAT& bat, unsigned int start, unsigned int nr) {
    TRACE9("ParseMSOffice::getBlock (const BAT&, 2x unsigned int) - " << start);
    while (nr--)
        if ((start = nextBlock(bat, start)) > LAST_BLOCK)
            throw YGP::ParseError(_("BAT not valid!"));

    TRACE8("ParseMSOffice::getBlock (const BAT&, 2x unsigned int) - Result: " << start);
    return start;
}
