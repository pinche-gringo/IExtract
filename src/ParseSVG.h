#ifndef PARSESVG_H
#define PARSESVG_H

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

/**Class to extract the properties of SVG images:
 *  - The title is taken from the (first) <title>-element
 *  - The comment is taken from the (first) <desc>-element
 *  - The author (and still empty properties) are taken from the RDF
 *    metadata (Dublin Core; as written e.g. by Inkscape)
 */
class ParseSVG {
  public:
    static void parse(std::istream& stream, Properties& result);

  private:
    ParseSVG() = delete;
    ParseSVG(const ParseSVG&) = delete;
    ParseSVG& operator=(const ParseSVG&) = delete;
};

#endif
