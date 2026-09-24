#ifndef PARSEMSOFFICE_H
#define PARSEMSOFFICE_H

//$Id$

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

#include <cstdint>

#include <vector>

#include <YGP/Exception.h>
#include <istream>

struct Properties;

// Class to extract the properties of Microsoft Office documents (OLE compound
// files)
class ParseMSOffice {
public:
  ParseMSOffice() = default;

  void parse(std::istream &stream, Properties &result);

private:
  ParseMSOffice(const ParseMSOffice &) = delete;
  ParseMSOffice &operator=(const ParseMSOffice &) = delete;

  using BAT = std::vector<std::uint32_t>; ///< Block allocation table

  static BAT readBAT(std::istream &stream, const char *pBATBlocks,
                     unsigned int cBlocks, unsigned int sizeBlock);
  static void readBlock(std::istream &stream, unsigned int offBlock,
                        char *block, unsigned int sizeBlock);
  static std::vector<char> readFile(std::istream &stream, unsigned int offBlock,
                                    const BAT &bat, unsigned int blocks,
                                    unsigned int sizeBlock);
  static unsigned int nextBlock(const BAT &bat, unsigned int block);
  static unsigned int getBlock(const BAT &bat, unsigned int start,
                               unsigned int nr);
};

#endif
