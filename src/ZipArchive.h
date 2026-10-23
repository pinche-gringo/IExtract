#ifndef ZIPARCHIVE_H
#define ZIPARCHIVE_H

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

/**Helpers to decompress data (with the zlib) and to access the files of a
 * ZIP archive (only stored or deflated entries are supported).
 */
class ZipArchive {
  public:
    static std::optional<std::string> getFile(std::string_view archive, std::string_view name);
    static std::string inflate(std::string_view data, bool raw);

  private:
    ZipArchive() = delete;
    ZipArchive(const ZipArchive&) = delete;
    ZipArchive& operator=(const ZipArchive&) = delete;
};

#endif
