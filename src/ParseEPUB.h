#ifndef PARSEEPUB_H
#define PARSEEPUB_H

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

/**Class to extract the properties of EPUB documents; out of the Dublin Core
 * metadata (dc:title, dc:creator, dc:description) of the package document
 * (whose location is read from META-INF/container.xml).
 */
class ParseEPUB {
  public:
    static void parse(std::istream& stream, Properties& result);

  private:
    ParseEPUB() = delete;
    ParseEPUB(const ParseEPUB&) = delete;
    ParseEPUB& operator=(const ParseEPUB&) = delete;
};

#endif
