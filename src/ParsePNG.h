#ifndef PARSEPNG_H
#define PARSEPNG_H

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


#include <YGP/XStream.h>


struct Properties;


/**Class to extract the comments out of PNG images
 *
 * The format of PNG images is described at
 *
 *  http://www.w3.org/TR/REC-png
 */
class ParsePNG {
 public:
   ParsePNG (Properties& result) : prop (result) { }
   ~ParsePNG () { }

   void parse (YGP::Xistream& stream);

 private:
   //@Section prohibited manager functions
   ParsePNG ();
   ParsePNG (const ParsePNG& other);
   const ParsePNG& operator= (const ParsePNG& other);

   void foundChunk (unsigned int type, const std::string& data);

   Properties&  prop;
};

#endif
