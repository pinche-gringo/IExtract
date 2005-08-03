#ifndef PARSEPNG_H
#define PARSEPNG_H

//$Id$

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <YGP/Parse.h>


struct Properties;


/**Class to extract the comments out of PNG images
 *
 * The format of PNG images is described at
 *
 *  http://www.w3.org/TR/REC-png
 */
class ParsePNG {
 public:
   ParsePNG (Properties& result);
   virtual ~ParsePNG ();

   void parse (YGP::Xistream& stream) throw (std::string) {
      pngImage.parse (stream); }

 private:
   //@Section prohibited manager functions
   ParsePNG ();
   ParsePNG (const ParsePNG& other);
   const ParsePNG& operator= (const ParsePNG& other);

   int foundType (const char*, unsigned int);
   int foundLength (const char*, unsigned int);
   int foundComment (const char*, unsigned int);

   typedef YGP::OFParseAttomic<ParsePNG>  OMParseAttomic;

   YGP::ParseExact   idPNG;
   OMParseAttomic    length;
   OMParseAttomic    type;
   OMParseAttomic    comment;
   YGP::ParseSkip    skip;
   YGP::ParseAttomic crc;

   YGP::ParseSequence chunk;
   YGP::ParseObject*  _chunk[5];

   YGP::ParseSequence pngImage;
   YGP::ParseObject*  _pngImage[3];

   Properties&  prop;
};

#endif
