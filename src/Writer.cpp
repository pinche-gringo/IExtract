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

#include <ostream>

#include <File.h>

#include "Writer.h"


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
//            pDescription: Description to the file
//Requires  : pDescription not NULL
/*--------------------------------------------------------------------------*/
void HTMLWriter::printFile (std::ostream& out, const File& file,
                            const char* pDescription) const {
   out << "<tr><td>&nbsp;&nbsp;<a href=" << file.path () << file.name ()
       << '>';
   if (options & SHOW_PATH)
      out << file.path ();
   out << file.name () << "</td><td> - ";
   if (pDescription)
      out << pDescription;
   out << "</td></tr>\n";
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
//            pDescription: Description to the file
/*--------------------------------------------------------------------------*/
void TextWriter::printFile (std::ostream& out, const File& file,
                            const char* pDescription) const {
   if (options & SHOW_PATH)
      out << file.path ();
   out << file.name () << " - ";
   if (pDescription)
       out << pDescription;
   out << '\n';
}
