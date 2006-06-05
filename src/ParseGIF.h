#ifndef PARSEGIF_H
#define PARSEGIF_H

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


/**Class to extract the comments out of GIF images
 */
class ParseGIF {
 public:
   ParseGIF (Properties& result);
   virtual ~ParseGIF ();

   void parse (YGP::Xistream& stream) throw (YGP::ParseError) {
      gifImage.parse (stream); }

 private:
   //@Section prohibited manager functions
   ParseGIF ();
   ParseGIF (const ParseGIF& other);
   const ParseGIF& operator= (const ParseGIF& other);

   int foundComment (const char*, unsigned int);
   int skipColourTable (const char*, unsigned int);
   int foundImage (const char*, unsigned int);
   int foundLength (const char*, unsigned int);
   int foundEndGIF (const char*, unsigned int);

   int foundSubblock (const char*, unsigned int);

   typedef YGP::OFParseExact<ParseGIF>    OMParseExact;
   typedef YGP::OFParseAttomic<ParseGIF>  OMParseAttomic;
   typedef YGP::OFParseSequence<ParseGIF> OMParseSequence;

   YGP::ParseExact   idGIF;
   YGP::ParseSkip    skip;
   YGP::ParseSkip    skip2;
   OMParseAttomic    colourTable;
   OMParseExact      idEndGIF;

   YGP::ParseExact   idCommentExt;
   OMParseExact      idImage;
   YGP::ParseExact   idExtension;
   YGP::ParseAttomic idTypeExtension;
   OMParseAttomic    comment;
   OMParseAttomic    lenBlock;

   YGP::ParseSelection blocks;
   YGP::ParseObject*   _blocks[5];

   YGP::ParseSequence commentExt;
   YGP::ParseObject*  _commentExt[3];

   YGP::ParseSequence imageDesc;
   YGP::ParseObject*  _imageDesc[7];
   YGP::ParseSequence extension;
   YGP::ParseObject*  _extension[4];

   OMParseSequence    commentBlocks;
   YGP::ParseObject*  _commentBlocks[3];
   OMParseSequence    subblocks;
   YGP::ParseObject*  _subblocks[3];

   YGP::ParseSequence gifImage;
   YGP::ParseObject*  _gifImage[7];

   Properties&  prop;
};

#endif
