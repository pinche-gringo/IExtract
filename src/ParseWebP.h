#ifndef PARSEWEBP_H
#define PARSEWEBP_H

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

/**Class to extract the properties of WebP images; out of the EXIF
 * information (see ParseTIFF) and the XMP packet (for still empty
 * properties).
 *
 * The format of WebP images is described at
 * https://developers.google.com/speed/webp/docs/riff_container
 */
class ParseWebP {
  public:
    static void parse(std::istream& stream, Properties& result);

  private:
    ParseWebP() = delete;
    ParseWebP(const ParseWebP&) = delete;
    ParseWebP& operator=(const ParseWebP&) = delete;
};

#endif
