//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : Writer
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 13.10.2002
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
#include <time.h>

#include <iostream>

#include <File.h>

#include "Writer.h"
#include "Properties.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Konstructor
//Parameters: showOptions: Options how to display files
//            age: Maximal age for new files
//            pNew: Text to display for new files
/*--------------------------------------------------------------------------*/
Writer::Writer (unsigned int showOptions, unsigned long age, const char* pNew)
   : options (showOptions), pStrNew (pNew) {
   assert (pNew ? age : 1);

   limit = time (NULL) - age;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Writer::~Writer () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
HTMLWriter::~HTMLWriter () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Prints the start for an HTML-table
//Parameters: out: Stream where to put the output
/*--------------------------------------------------------------------------*/
void HTMLWriter::printStart (std::ostream& out) const {
   out << "<table>\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints the start for an HTML-table
//Parameters: out: Stream where to put the output
//            file: File whose data should be printed
//            prop: Properties of the file
/*--------------------------------------------------------------------------*/
void HTMLWriter::printFile (std::ostream& out, const File& file,
                            const Properties& prop) const {
   out << "<tr valign=top><td>";
   if (pStrNew && isNew (file))
      out << pStrNew;
   out << "</td><td><a href=\"" << file.path ()
       << file.name () << "\">";
   if (options & SHOW_PATH)
      out << file.path ();
   out << file.name () << "</td><td valign=top>-</td><td valign=top>"
       << (prop.strComment.empty () ? prop.strTitle : prop.strComment)
       << "</td></tr>\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints the endfor an HTML-table
//Parameters: out: Stream where to put the output
/*--------------------------------------------------------------------------*/
void HTMLWriter::printEnd (std::ostream& out) const {
   out << "</table>\n";
}


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
TextWriter::~TextWriter () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Prints the start for an Text-table
//Parameters: out: Stream where to put the output
//            file: File whose data should be printed
//            prop: Properties of the file
/*--------------------------------------------------------------------------*/
void TextWriter::printFile (std::ostream& out, const File& file,
                            const Properties& prop) const {
   if (options & SHOW_PATH)
      out << file.path ();
   if (pStrNew && isNew (file))
      out << pStrNew << ": ";
   out << file.name () << " - "
       << (prop.strComment.empty () ? prop.strTitle : prop.strComment) << '\n';
}
