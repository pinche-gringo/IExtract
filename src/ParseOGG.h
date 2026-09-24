#ifndef PARSEOGG_H
#define PARSEOGG_H

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

#include <YGP/XStream.h>

struct Properties;


/**Class to extract the comments (title, artist, album) of OGG/Vorbis files
 */
class ParseOGG {
 public:
   static void parse (YGP::Xistream& stream, Properties& result);

 private:
   // Prohibited manager functions
   ParseOGG ();
   ParseOGG (const ParseOGG& other);
   const ParseOGG& operator= (const ParseOGG& other);

   static void foundComment (const std::string& comment, Properties& result);
};

#endif
