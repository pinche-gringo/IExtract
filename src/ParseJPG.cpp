//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseJPEG
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 17.10.2002
//COPYRIGHT   : Anticopyright (A) 2002

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

#include <assert.h>

#include <Trace_.h>

#include "ParseJPG.h"
#include "Properties.h"


/*--------------------------------------------------------------------------*/
//Purpose   : (Default-)Constructor
//Parameters: pClassname: Name of class containing parser-data
/*--------------------------------------------------------------------------*/
ParseJPEG::ParseJPEG ()
   : idJPEG ("\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49\x46\x00\x01", "JPEG-ID", 12, 12, false)
     , tagComment ("\xff\xfe", "ID for comment", *this, &ParseJPEG::foundID, 2, 0, false)
   , length ("\\*", "Length", *this, &ParseJPEG::foundLength, 2, 2, false)
   , title ("\0", "Comment", *this, &ParseJPEG::foundTitle, 1, 1, false)
   , ignore ("\\*", "Unused information", 8, 8, false)
   , jpegImage (_jpegImage, "JPEG image", 1, 1) {

   _jpegImage[0] = &idJPEG;
   _jpegImage[1] = &ignore;
   _jpegImage[2] = &tagComment;
   _jpegImage[3] = &length;
   _jpegImage[4] = &title;
   _jpegImage[5] = NULL;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a ID was found (or not found!)
//Parameters: len: Length of ID
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundID (const char*, unsigned int len) {
   assert (prop);
   if (len < 2)
      _jpegImage[3] = NULL;
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after a title was read
//Parameters: pTitle: Pointer to title
//            len: Length of title
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundTitle (const char* pTitle, unsigned int len) {
   assert (prop); assert (pTitle);
   prop->strComment.assign (pTitle, len - 2);
   jpegImage.setMaxCard (1);
   return ParseObject::PARSE_OK;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Callback after the length of the title was read
//Parameters: length: Pointer to length
//Returns   : int: Status: ParseObject::PARSE_OK
/*--------------------------------------------------------------------------*/
int ParseJPEG::foundLength (const char* length, unsigned int) {
   assert (length);
   unsigned int len ((*length << 8) + length[1]);
   if (len)
      title.setMaxCard (len);
   TRACE8 ("ParseJPEG::foundLength (const char*, unsigned int): " << len);
   return ParseObject::PARSE_OK;
}
