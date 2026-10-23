#ifndef PARSEMP4_H
#define PARSEMP4_H

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
#include <string>

struct Properties;

/**Class to extract the properties of MPEG-4 (and QuickTime) files (e.g. M4A
 * audio files); the metadata (iTunes-style ilst or QuickTime user data) in
 * moov/udta are inspected:
 *  - The title (©nam) is returned in strTitle
 *  - The artist (©ART) is returned in strAuthor
 *  - The album (©alb; or the comment (©cmt), if there is no album) is
 *    returned in strComment
 * Furthermore an XMP packet is used for still empty properties.
 */
class ParseMP4 {
  public:
    static void parse(std::istream& stream, Properties& result);

  private:
    ParseMP4() = delete;
    ParseMP4(const ParseMP4&) = delete;
    ParseMP4& operator=(const ParseMP4&) = delete;

    struct Values;

    static void parseBoxes(const std::string& data, std::size_t start, std::size_t end, Values& values, unsigned int parent,
                           unsigned int depth);
    static void foundValue(unsigned int type, const std::string& value, Values& values);
};

#endif
