//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseOGG
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 20.03.2005
//COPYRIGHT   : Copyright (C) 2005

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


#include <IExtract-cfg.h>

#include <cstring>

#include <YGP/Trace.h>

#include "Utility.h"
#include "ParseOGG.h"
#include "Properties.h"


//-----------------------------------------------------------------------------
/// Constructor
/// \param result: Properties, where the found data is stored
//-----------------------------------------------------------------------------
ParseOGG::ParseOGG (Properties& result)
   : prop (result)
     , txtOGG ("OggS", _("OGG-ID"), false)
     , skip (0x69)
     , lenVendorStr ("\\*", _("Length of vendor string"), *this, &ParseOGG::foundLenVendorString, 4, 4, false)
     , nrComments ("\\*", _("Number of comments"), *this, &ParseOGG::foundNrComments, 4, 4, false)
     , lenEntry ("\\*", _("Length of comment entry"), *this, &ParseOGG::foundLenComment, 4, 4, false)
     , txtEntry ("\\*", _("Comment entry"), *this, &ParseOGG::foundComment, 0, 0, false)
     , seqComment (_seqComment, _("Comment entry"), 0, 0, false)
     , seqOGG (_seqOGG, _("OGG file")) {
   _seqOGG[0] = &txtOGG;
   _seqOGG[1] = &skip;
   _seqOGG[2] = &lenVendorStr;
   _seqOGG[3] = &skip;
   _seqOGG[4] = &nrComments;
   _seqOGG[5] = &seqComment;
   _seqOGG[6] = NULL;

   _seqComment[0] = &lenEntry;
   _seqComment[1] = &txtEntry;
   _seqComment[2] = NULL;

   result.strComment = result.strTitle = result.strAuthor = "";
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
ParseOGG::~ParseOGG () {
}


//-----------------------------------------------------------------------------
/// Method to actually parse the OGG-file
/// \param stream: OGG-file to analyze
/// \param result: Out: Found information
//-----------------------------------------------------------------------------
void ParseOGG::parse (YGP::Xistream& stream, Properties& result) throw (std::string) {
   ParseOGG obj (result);

   obj.seqOGG.parse (stream);
}

//-----------------------------------------------------------------------------
/// Callback after the length-entry of vendor string has been parsed
/// \param nr: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseOGG::foundLenVendorString (const char* nr, unsigned int) {
   Check1 (nr);
   TRACE8 ("ParseOGG::foundLenVendorString (const char*, unsigned int): " << get4BytesLSB (nr));
   skip.setOffset (get4BytesLSB (nr));
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after the number-of-comments entry has been parsed
/// \param nr: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseOGG::foundNrComments (const char* nr, unsigned int) {
   Check1 (nr);
   unsigned int len (get4BytesLSB (nr));
   TRACE8 ("ParseOGG::foundNrComments (const char*, unsigned int): " << len);
   seqComment.setMaxCard (len);
   seqComment.setMinCard (len);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a lenght-of-comment entry has been parsed
/// \param nr: Pointer to number of entries
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseOGG::foundLenComment (const char* nr, unsigned int) {
   Check1 (nr);
   unsigned int len (get4BytesLSB (nr));
   TRACE8 ("ParseOGG::foundLenComment (const char*, unsigned int): " << len);
   txtEntry.setMaxCard (len);
   txtEntry.setMinCard (len);
   return YGP::ParseObject::PARSE_OK;
}

//-----------------------------------------------------------------------------
/// Callback after a comment-entry has been found
/// \param comment: Found comment entry
/// \returns \c int: Status: YGP::ParseObject::PARSE_OK
//-----------------------------------------------------------------------------
int ParseOGG::foundComment (const char* comment, unsigned int len) {
   TRACE9 ("ParseOGG::foundComment (const char*, unsigned int) - " << comment);
   Check1 (comment); Check1 (len);

   if (len > 6) {
      if (!(strncmp (comment, "TITLE=", 6) && strncmp (comment, "title=", 6)))
	 prop.strTitle.append (comment + 6, len - 6);
      else if (!(strncmp (comment, "ALBUM=", 6) && strncmp (comment, "album=", 6)))
	 prop.strComment.append (comment + 6, len - 6);
      else if (!(strncmp (comment, "ARTIST=", 7) && strncmp (comment, "artist=", 7)))
	 prop.strAuthor.append (comment + 7, len - 7);
   }
   txtEntry.setMaxCard (0);
   txtEntry.setMinCard (0);
   return YGP::ParseObject::PARSE_OK;
}
