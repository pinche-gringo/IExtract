#ifndef PARSEGIF_H
#define PARSEGIF_H

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


/**Class to extract the comments out of GIF images
 */
class ParseGIF {
 public:
   ParseGIF (Properties& result) : prop (result) { }
   ~ParseGIF () { }

   void parse (YGP::Xistream& stream);

 private:
   //@Section prohibited manager functions
   ParseGIF ();
   ParseGIF (const ParseGIF& other);
   const ParseGIF& operator= (const ParseGIF& other);

   Properties&  prop;
};

#endif
