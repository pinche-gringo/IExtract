#ifndef PARSEXMP_H
#define PARSEXMP_H

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

#include <optional>
#include <string>
#include <string_view>

struct Properties;

/**Class to extract the Dublin Core properties (dc:title, dc:creator and
 * dc:description) out of XMP packets (or other RDF/XML data).
 *
 * As XMP is embedded in many file formats, it is used as fallback: only
 * properties, which are still empty, are set.
 *
 * Furthermore some (very simple) helpers to access XML data are provided.
 */
class ParseXMP {
  public:
    static void parse(std::string_view xmp, Properties& result);

    static std::optional<std::string_view> getElement(std::string_view xml, std::string_view tag);
    static std::string getText(std::string_view xml);
    static std::string_view findPacket(std::string_view data);

  private:
    ParseXMP() = delete;
    ParseXMP(const ParseXMP&) = delete;
    ParseXMP& operator=(const ParseXMP&) = delete;

    static std::string getAlternative(std::string_view value);
    static std::string getList(std::string_view value);
};

#endif
