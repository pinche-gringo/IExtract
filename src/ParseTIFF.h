#ifndef PARSETIFF_H
#define PARSETIFF_H

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

#include <string>

#include <istream>

struct Properties;

/**Class to extract the properties of TIFF images (or of TIFF structures, as
 * used by the EXIF information of JPEG or WebP images).
 *
 * The first image file directory is inspected for
 *   - the Windows XP tags (XPTitle, XPAuthor, XPComment)
 *   - ImageDescription (as comment) and Artist (as author), if the
 *     corresponding Windows XP tag is not available
 *   - an embedded XMP packet (used for still empty properties)
 */
class ParseTIFF {
  public:
    static void parse(std::istream& stream, Properties& result);
    static bool parseIFD(const std::string& tiff, Properties& result);

  private:
    ParseTIFF() = delete;
    ParseTIFF(const ParseTIFF&) = delete;
    ParseTIFF& operator=(const ParseTIFF&) = delete;
};

#endif
