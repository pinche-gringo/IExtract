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

#include <time.h>

#include <iostream>

#include <Check.h>
#include <Trace_.h>

#include <File.h>
#include <ATStamp.h>
#include <Tokenize.h>

#include "Writer.h"
#include "Properties.h"


/*--------------------------------------------------------------------------*/
//Purpose   : Konstructor
//Parameters: format: Format how to display entries
//            New: Text to display for new files
//            age: Maximal age for new files
/*--------------------------------------------------------------------------*/
Writer::Writer (const std::string& format, const std::string& New,
                unsigned long age)
   : strNew (New), format (format) {
   Check3 (strNew.size () ? age : 1);

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
   OutIterator i (format);
   while (i) {
      ++i;
      ++cols;
   }
   return cols;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints a separating text between directories. The following
//            characters are substituted:
//              'e': With the end-of-data as defined by the writer class
//              's': With the start-of-data as defined by the writer class
//              'n': With the name of the file
//              'N': With path and name of the file
//              'p': With the path of the file
//              'P': With the path of the file in UNIX style (separated by /)
//              'U': With path and name of the file in UNIX style (separated by /)
//Parameters: out: Stream where to put the output
//            file: File specifying directory
//            data: Text to print for separation
//            title: Text to print as header for every new dir
/*--------------------------------------------------------------------------*/
void Writer::printSeparator (std::ostream& out, const File& file,
                             const std::string& data, const std::string& title) const {
   unsigned int pos (0);
   unsigned int oldPos (0);
   while ((pos < data.size ())
          && ((pos = data.find ('%', oldPos)) != std::string::npos)) {
      TRACE9 ("printSeparator (ostream&, const File, const string&) - Inspecting "
             << data[pos + 1]);
      out << data.substr (oldPos, pos - oldPos );

      switch (data[++pos]) {
      case 'e': printEnd (out); break;

      case 's': printStart (out, title); break;

      case 'n': out << file.name (); break;

      case 'N': out << file.path () << file.name (); break;

      case 'p': out << file.path (); break;

      case 'P':
      case 'U': {
         std::string path (file.path ());
         unsigned int pos (0);
         while ((pos = path.find (File::DIRSEPARATOR, pos)) != std::string::npos)
            path.replace (pos, 1, '/');

         if (data[pos] == 'U')
            path += file.name ();
         break; }
      }
      oldPos = pos + 1;
   }
   out << data.substr (oldPos, pos - oldPos);
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
//Purpose   : Returns the substitute for a control character
//            Substitutes:
//              'a': With the author (of the properties)
//              'c': With the comment (of the properties)
//              'd': With the timestamp of the file
//              'D': With the date of the file
//              'n': With the name of the file
//              'N': With path and name of the file
//              'p': With the path of the file
//              'P': With the path of the file in UNIX style (separated by /)
//              't': With the title (of the properties)
//              'U': With path and name of the file in UNIX style (separated by /)
//              Other chars: With the character itself
//Parameters: ctrl: Control character
//            subst: String with which to replace the character
/*--------------------------------------------------------------------------*/
void Writer::OutIterator::getSubstitute (const char ctrl, std::string& subst) const {
   switch (ctrl) {
   case 'a': if (p) subst = p->strAuthor; break;

   case 'c': if (p) subst = p->strComment; break;

   case 'D':
   case 'd': {
      Check3 (file);
      ATimestamp stamp (file->time ());
      subst = (ctrl == 'D') ? stamp.ADate::toString () : stamp.toString (); break; }

   case 'n': Check3 (file); subst = file->name (); break;

   case 'N':
      Check3 (file);
      subst = file->path ();
      subst += file->name ();
      break;

   case 'p': Check3 (file); subst = file->path (); break;

   case 't': subst = p->strTitle; break;

   case 'P':
   case 'U': {
      Check3 (file);
      subst = file->path ();
      unsigned int pos (0);
      while ((pos = subst.find (File::DIRSEPARATOR, pos)) != std::string::npos)
         subst.replace (pos, 1, '/');

      if (ctrl == 'U')
         subst += file->name ();
         break; }

   default:
      subst = ctrl;
   }

   TRACE9 ("Writer::OutIterator::getSubstitute (const char, std::string&) - Replacing '"
           << ctrl << "' with " << subst);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns the next token; special characters are expanded
//Returns   : std::string: Next (expanded) token
/*--------------------------------------------------------------------------*/
std::string Writer::OutIterator::operator* () const {
   Check3 (file);

   int pos (0);
   std::string token (columns_.getActNode ());

   TRACE2 ("Writer::OutIterator::operator* () - Node = '" << token << '\'');

   std::string substitute;
   unsigned int nPos (0);
   while (((pos = token.find ('%', nPos)) != std::string::npos)
          && (pos < token.size ())) {
      if (token[pos + 1] != '(')
         getSubstitute (token[nPos = pos + 1], substitute);
      else {
         nPos = pos + 1;
         do {
            getSubstitute (token[nPos], substitute);
         } while (substitute.empty () && (token[++nPos] != ')') && token[nPos]);

         // Now skip to next closing bracket
         if (token[nPos])
            if ((nPos = token.find (')', nPos)) != std::string::npos)
               ++nPos;
            else
               nPos = token.size ();
      } // end-else '(' found

      token.replace (pos, nPos - pos + 1, substitute);
      ++pos;
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
void HTMLWriter::printStart (std::ostream& out, const std::string& title) const {
   out << "<table>\n";

   if (title.size ()) {
      out << "<thead><tr>";
      if (strNew.size ())
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
   if (strNew.size ()) {
      out << "<td>";
      if (isNew (file))
         out << strNew;
      out << "</td>";
   }

   OutIterator i (format, file, prop);
   while (i) {
      out << "<td>" << *i << "</a></td>";
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
                               const std::string& msg) const {
   Check3 (msg);

   out << "<tr valign=top>";
   if (strNew.size ()) {
      out << "<td>E</td>";
   }

   out << "<a href=\"" << file.path () << file.name () << "\">" << file.name ()
       << "</a></td><td colspan=" << (columns () - 1) << '>' << msg
       << "</td></tr>\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints the end for an HTML-table
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
void TextWriter::printStart (std::ostream& out, const std::string& title) const {
   if (title.size ()) {
      Tokenize titles (title);
      std::string node;
      while ((node = titles.getNextNode ('|')).size ())
         out << node << " ";
      out << '\n';
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
   if (strNew.size () && isNew (file))
      out << strNew << ": ";

   OutIterator i (format, file, prop);
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
                               const std::string& msg) const {
   Check3 (msg);
   if (strNew.size () && isNew (file))
      out << "E: " << ": ";
   out << file.name () << " - " << msg << '\n';
}


/*--------------------------------------------------------------------------*/
//Purpose   : Destructor
/*--------------------------------------------------------------------------*/
LaTeXWriter::~LaTeXWriter () {
}


/*--------------------------------------------------------------------------*/
//Purpose   : Prints the start for a LaTeX-table (tabular)
//Parameters: out: Stream where to put the output
//            title: Title information
/*--------------------------------------------------------------------------*/
void LaTeXWriter::printStart (std::ostream& out, const std::string& title) const {
   out << "\\begin{tabular}{";
   for (unsigned int i (0); i < columns (); ++i)
      out << 'l';
   out << "}\n";

   if (title.size ()) {
      if (strNew.size ())
         out << "&";

      Tokenize titles (title);
      std::string node;
      node = titles.getNextNode ('|');
      out << "{\\textbf " << node << '}';
      while ((node = titles.getNextNode ('|')).size ())
         out << "&{\\textbf " << node << "}" << node;

      out << "\\\\\n";
   }
 }

/*--------------------------------------------------------------------------*/
//Purpose   : Prints a file entry in LaTeX format
//Parameters: out: Stream where to put the output
//            file: File whose data should be printed
//            prop: Properties of the file
/*--------------------------------------------------------------------------*/
void LaTeXWriter::printFile (std::ostream& out, const File& file,
                             const Properties& prop) const {
   if (strNew.size ()) {
      if (isNew (file))
         out << strNew;
      out << '&';
   }

   OutIterator i (format, file, prop);
   while (i) {
      out << *i;
      ++i;
      if (i)
         out << '&';
   }
   out << "\\\\\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints a message in LaTeX-format (inside the table)
//Parameters: out: Stream where to put the output
//            file: File to which the message should be print
//            msg: Message to print (not NULL)
/*--------------------------------------------------------------------------*/
void LaTeXWriter::printMessage (std::ostream& out, const File& file,
                                const std::string& msg) const {
   Check3 (msg);

   if (strNew.size ()) {
      if (isNew (file))
         out << "E:";
      out << '&';
   }

   out << file.name () << "&{\\multicolumn{" << (columns () - 1) << "}l{"
       << msg << "}\\\\\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Prints the endfor an LaTeX-table
//Parameters: out: Stream where to put the output
/*--------------------------------------------------------------------------*/
void LaTeXWriter::printEnd (std::ostream& out) const {
   out << "\\end{tabular}\n";
}
