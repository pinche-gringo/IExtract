#ifndef PARSEJPG_H
#define PARSEJPG_H

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


/**Class to extract the properties of JPEG images; out of either a comment,
 * the EXIF information (Windows XP tags) or the IPTC data (stored by Photoshop)
 */
class ParseJPEG  {
 public:
   ParseJPEG () { }
   ~ParseJPEG () { }

   void parse (YGP::Xistream& stream, Properties& result);

 private:
   ParseJPEG (const ParseJPEG&);
   ParseJPEG& operator= (const ParseJPEG&);

   static bool parseEXIF (const std::string& data, Properties& result);
   static void parsePhotoshop (const std::string& data, Properties& result);
   static void parseIPTC (const std::string& data, Properties& result);
};

#endif
