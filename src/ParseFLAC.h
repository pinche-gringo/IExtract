#ifndef PARSEFLAC_H
#define PARSEFLAC_H

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

#include <istream>

struct Properties;

/**Class to extract the comments (title, artist, album) of FLAC files; out of
 * the VORBIS_COMMENT metadata block
 *
 * The format of FLAC files is described at https://xiph.org/flac/format.html
 */
class ParseFLAC {
  public:
    static void parse(std::istream& stream, Properties& result);

  private:
    ParseFLAC() = delete;
    ParseFLAC(const ParseFLAC&) = delete;
    ParseFLAC& operator=(const ParseFLAC&) = delete;
};

#endif
