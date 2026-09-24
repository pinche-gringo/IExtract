#ifndef PARSERTF_H
#define PARSERTF_H

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

#include <string>

#include <istream>

struct Properties;

// Class to extract the title of a RTF-document
class ParseRTF {
public:
  ParseRTF() = default;

  void parse(std::istream &stream, Properties &result);

private:
  ParseRTF(const ParseRTF &) = delete;
  ParseRTF &operator=(const ParseRTF &) = delete;
};

#endif
