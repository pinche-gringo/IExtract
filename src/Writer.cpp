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

#include <Trace_.h>

#include <File.h>
#include <ATStamp.h>
#include <Tokenize.h>

#include "Writer.h"
#include "Properties.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Konstructor
//Parameters: format: Format how to display entries
//            age: Maximal age for new files
//            pNew: Text to display for new files
/*--------------------------------------------------------------------------*/
Writer::Writer (const char* format, unsigned long age, const char* pNew)
   : pStrNew (pNew), pFormat (format) {
   assert (pNew ? age : 1);
   assert (pFormat);

   limit = time (NULL) - age;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
Writer::~Writer () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Counts the number of columns
//Returns   : unsigned int: Number of columns
/*--------------------------------------------------------------------------*/
unsigned int Writer::columns () const {
   unsigned int cols (0);
   OutIterator i (pFormat);
   while (i) {
      ++i;
      ++cols;
   }
   return cols;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Checks if the iterator is on a name entry (%n or %N)
//Returns   : bool: True if on a name entry
/*--------------------------------------------------------------------------*/
bool Writer::OutIterator::isAtName () const {
   int pos (0);
   std::string node (columns_.getActNode ());
                     
   while (((pos = node.find ('%', pos)) != std::string::npos)
          && (pos++ < node.size ()))
      if ((node[pos] == 'n') || (node[pos] == 'N'))
         return true;

   return false;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Returns the next token; special characters are expanded
//Returns   : std::string: Next (expanded) token
/*--------------------------------------------------------------------------*/
std::string Writer::OutIterator::operator* () const {
   assert (file);

   int pos (0);
   std::string token (columns_.getActNode ());

   TRACE2 ("Writer::OutIterator::operator* () - Node = '" << token << '\'');

   while (((pos = token.find ('%', pos)) != std::string::npos)
          && (pos < token.size ()))
      switch (token[pos + 1]) {
      case 'a': if (p) token.replace (pos, 2, p->strAuthor); break;

      case 'c': if (p) token.replace (pos, 2, p->strComment); break;

      case 'd': {
         ATimestamp stamp (file->time ());
         token.replace (pos, 2, stamp.toString ()); break; }

      case 'n': token.replace (pos, 2, file->name ()); break;

      case 'N': {
         std::string name (file->path ());
         name += file->name ();
         token.replace (pos, 2, name);
         break; }

      case 'p': token.replace (pos, 2, file->path ()); break;

      case 't': if (p) token.replace (pos, 2, p->strTitle); break;

      case '%': token.replace (pos, 1, 0, '\0'); break;
      }
   return token;
}


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
HTMLWriter::~HTMLWriter () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Prints the start for an HTML-table
//Parameters: out: Stream where to put the output
//            title: Title information
/*--------------------------------------------------------------------------*/
void HTMLWriter::printStart (std::ostream& out, const char* title) const {
   out << "<table>\n";

   if (title) {
      out << "<thead><tr>";
      if (pStrNew)
         out << "<td></td>";

      Tokenize titles (title);
      std::string node;
      while ((node = titles.getNextNode ('|')).size ())
         out << "<td>" << node << "</td>";
      out << "</tr></thead>";
   }
   out << "<tbody>";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints a file entry in HTML format
//Parameters: out: Stream where to put the output
//            file: File whose data should be printed
//            prop: Properties of the file
/*--------------------------------------------------------------------------*/
void HTMLWriter::printFile (std::ostream& out, const File& file,
                            const Properties& prop) const {
   out << "<tr valign=top>";
   if (pStrNew) {
      out << "<td>";
      if (isNew (file))
         out << pStrNew;
      out << "</td>";
   }

   OutIterator i (pFormat, file, prop);
   while (i) {
      out << "<td>";
      if (i.isAtName ())
         out << "<a href=\"" << file.path () << file.name () << "\">";
      out << *i;
      if (i.isAtName ())
         out << "</a>";
      out << "</td>";
      ++i;
   }
   out << "</tr>\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints a message in HTML-format (inside the table)
//Parameters: out: Stream where to put the output
//            file: File to which the message should be print
//            msg: Message to print (not NULL)
/*--------------------------------------------------------------------------*/
void HTMLWriter::printMessage (std::ostream& out, const File& file,
                               const char* msg) const {
   assert (msg);

   out << "<tr valign=top>";
   if (pStrNew) {
      out << "<td>";
      if (isNew (file))
         out << pStrNew;
      out << "</td>";
   }

   unsigned int cols (columns ());
   TRACE9 ("HTMLWriter::printMessage (ostream&, const File&, const char*) - "
           << cols << " Columns");
   OutIterator i (pFormat, file);
   while (i) {
      out << "<td>";
      if (i.isAtName ())
         out << "<a href=\"" << file.path () << file.name () << "\">" << *i
             << "</a>";
      out << "</td>";
      --cols;
      if (i.isAtName ())
         break;
      ++i;
   }

   TRACE5 ("HTMLWriter::printMessage (ostream&, const File&, const char*) - "
           << msg << " for " << cols << " Columns");
   out << "<td colspan=" << cols << '>' << msg << "</td></tr>\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints the endfor an HTML-table
//Parameters: out: Stream where to put the output
/*--------------------------------------------------------------------------*/
void HTMLWriter::printEnd (std::ostream& out) const {
   out << "</tbody></table>\n";
}


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
TextWriter::~TextWriter () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Prints the start for an HTML-table
//Parameters: out: Stream where to put the output
//            title: Title information
/*--------------------------------------------------------------------------*/
void TextWriter::printStart (std::ostream& out, const char* title) const {
   if (title) {
      Tokenize titles (title);
      std::string node;
      while ((node = titles.getNextNode ('|')).size ())
         out << node << " ";
   }
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints a file entry in text-format
//Parameters: out: Stream where to put the output
//            file: File whose data should be printed
//            prop: Properties of the file
/*--------------------------------------------------------------------------*/
void TextWriter::printFile (std::ostream& out, const File& file,
                            const Properties& prop) const {
   if (pStrNew && isNew (file))
      out << pStrNew << ": ";

   OutIterator i (pFormat, file, prop);
   std::string result;
   while (i) {
      result = *i;
      out << result;
      if (result.size ())
          out << ' ';
      ++i;
   }
   out << '\n';
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints a message
//Parameters: out: Stream where to put the output
//            file: File to which the message should be print
//            msg: Message to print (not NULL)
/*--------------------------------------------------------------------------*/
void TextWriter::printMessage (std::ostream& out, const File& file,
                               const char* msg) const {
   assert (msg);
   if (pStrNew && isNew (file))
      out << pStrNew << ": ";
   out << file.name () << " - " << msg << '\n';
}
