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

#include <ctime>
#include <cstring>
#include <clocale>

#include <iostream>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#include <YGP/File.h>
#include <YGP/ATStamp.h>
#include <YGP/ANumeric.h>
#include <YGP/Tokenize.h>

#include "Writer.h"
#include "Properties.h"


//-----------------------------------------------------------------------------
/// Konstructor
/// \param format: Format how to display entries
/// \param New: Text to display for new files
/// \param age: Maximal age for new files
//-----------------------------------------------------------------------------
Writer::Writer (const std::string& format, const std::string& New,
                unsigned long age)
   : strNew (New), columns_ (format) {
   Check3 (strNew.size () ? age : 1);

   limit = time (NULL) - age;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Writer::~Writer () {
}


//-----------------------------------------------------------------------------
/// Counts the number of columns
/// \returns \c unsigned int: Number of columns
//-----------------------------------------------------------------------------
unsigned int Writer::columns () const {
   unsigned int cols (1);
   YGP::Tokenize t (columns_);
   while (!t.getNextNode ('|').empty ())
      ++cols;
   return cols;
}

//-----------------------------------------------------------------------------
/// Prints a separating text between directories. The following characters are
/// substituted:
///   - 'e': With the end-of-data as defined by the writer class
///   - 's': With the start-of-data as defined by the writer class
///   - 'n': With the name of the file
///   - 'N': With path and name of the file
///   - 'p': With the path of the file
///   - 'P': With the path of the file in UNIX style (separated by /)
///   - 'U': With path and name of the file in UNIX style (separated by /)
/// \param out: Stream where to put the output
/// \param file: File specifying directory
/// \param data: Text to print for separation
/// \param title: Text to print as header for every new dir
//-----------------------------------------------------------------------------
void Writer::printSeparator (std::ostream& out, const YGP::File& file,
                             const std::string& data, const std::string& title) const {
   unsigned int pos (0);
   unsigned int oldPos (0);
   while ((pos < data.size ())
          && ((pos = data.find ('%', oldPos)) != std::string::npos)) {
      TRACE9 ("printSeparator (ostream&, const YGP::File, const string&) - Inspecting "
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
#if SYSTEM != UNIX
         unsigned int pos (0);
         while ((pos = path.find (YGP::File::DIRSEPARATOR, pos)) != std::string::npos)
            path.replace (pos, 1, 1, '/');
#endif

         if (data[pos] == 'U')
            path += file.name ();
         break; }
      }
      oldPos = pos + 1;
   }
   out << data.substr (oldPos, pos - oldPos);
}


//-----------------------------------------------------------------------------
/// Returns the passed string with special characters changed.
/// \returns \c std::string: String with changed special characters
/// \remarks: To be implemented by derived classes
//-----------------------------------------------------------------------------
std::string Writer::changeSpecialChars (const std::string& val) const {
   return val;
 }

//-----------------------------------------------------------------------------
/// Returns the passed string with special filename characters changed.
/// \returns \c std::string: String with changed special characters
/// \remarks: To be implemented by derived classes
//-----------------------------------------------------------------------------
std::string Writer::changeSpecialFileChars (const std::string& val) const { 
   return changeSpecialChars (val);
}

//-----------------------------------------------------------------------------
/// Returns the substitute for a control character Substitutes:
///   - 'a': With the author (of the properties)
///   - 'c': With the comment (of the properties)
///   - 'd': With the timestamp of the file
///   - 'D': With the date of the file
///   - 'e': With the extension of the file
///   - 'E': With the name of the file without extension
///   - 'n': With the name of the file
///   - 'N': With path and name of the file
///   - 'p': With the path of the file
///   - 'P': With the path of the file in UNIX style (separated by /)
///   - 's': With the size of the file in bytes
///   - 'S': With the size of the file (human readable)
///   - 't': With the title (of the properties)
///   - 'U': With path and name of the file in UNIX style (separated by /)
///   - Other chars: With the character itself
/// \param ctrl: Control character
/// \param subst: String with which to replace the character
/// \param file: File subsituting various placeholders
/// \param prop: Properties subsituting various placeholders
/// \param extend: Flag, if special formatting of substitute is wanted
//-----------------------------------------------------------------------------
void Writer::getSubstitute (const char ctrl, std::string& subst, const YGP::File& file,
                            const Properties& prop, bool extend) const {
   switch (ctrl) {
   case 'a':
      subst = extend ? prop.strAuthor : changeSpecialChars (prop.strAuthor);
      break;

   case 'c':
      subst = extend ? prop.strAuthor : changeSpecialChars (prop.strComment);
      break;

   case 'D':
   case 'd': {
      YGP::ATimestamp stamp (file.time ());
      subst = (ctrl == 'D') ? stamp.ADate::toString () : stamp.toString ();
      break; }

   case 'e': {
      const char* ext (strrchr (file.name (), '.'));
      ext = ext ? ext + 1 : "";
      subst = extend ? changeSpecialFileChars (ext) : ext;
      break;
   }

   case 'E': {
      std::string tmp;
      const char* ext (strrchr (file.name (), '.'));
      if (ext) {
         tmp = file.name ();
         tmp.erase (ext - file.name ());
         ext = tmp.c_str ();
      }
      else
         ext = file.name ();
      subst = extend ? changeSpecialFileChars (ext) : ext;
      break;
   }

   case 'n': subst = changeSpecialFileChars (file.name ()); break;

   case 'N':
      subst = extend ? changeSpecialFileChars (file.path ()) : file.path ();
      subst += extend ? changeSpecialFileChars (file.name ()) : file.name ();
      break;

   case 'p':
      subst = extend ? changeSpecialFileChars (file.path ()) : file.path ();
      break;

   case 't':
      subst = extend ? prop.strTitle : changeSpecialChars (prop.strTitle);
      break;

   case 'P':
   case 'U': {
      subst = extend ? changeSpecialFileChars (file.path ()) : file.path ();
#if SYSTEM != UNIX
      unsigned int pos (0);
      while ((pos = subst.find (YGP::File::DIRSEPARATOR, pos)) != std::string::npos)
         subst.replace (pos, 1, 1, '/');
#endif

      if (ctrl == 'U')
         subst += extend ? changeSpecialFileChars (file.name ()) : file.name ();
         break; }

   case 's':
   case 'S': {
      YGP::ANumeric size (file.size ());
      subst = "";
      subst = ((ctrl == 'S') ? convertToHumanString (file.size ())
               : YGP::ANumeric::toString (file.size ()));
      break; }

   default:
      subst = ctrl;
   }

   TRACE9 ("Writer::getSubstitute (const char, std::string&) - Replacing '"
           << ctrl << "' with " << subst);
}

//-----------------------------------------------------------------------------
/// Returns the next token; special characters are expanded
/// \returns \c std::string: Next (expanded) token
//-----------------------------------------------------------------------------
std::string Writer::convertToHumanString (unsigned long value) {
   if (value < 1000)
      return YGP::ANumeric::toString (value);

   std::string tString (1, 'k');

   if (value > 1000000) {
      tString = 'M';
      value >>= 10;
   }

   if (value < 10000) {
      static struct lconv* loc = localeconv ();
      value += 50;
      double temp (value);
      temp /= 102.4;
      tString = (char ((int (temp) % 10) + '0')) + tString;
      value = (unsigned long)(temp / 10);
      tString = loc->decimal_point + tString;
   }
   else
      value >>= 10;
   return YGP::ANumeric::toString (value) + tString;
}

//-----------------------------------------------------------------------------
/// Returns the next token; special characters are expanded
/// \param file: File subsituting various placeholders
/// \param prop: Properties subsituting various placeholders
/// \returns \c std::string: Next (expanded) token
//-----------------------------------------------------------------------------
std::string Writer::getNextNode (const YGP::File& file, const Properties& prop) const {
   unsigned int pos (0);
   std::string token (const_cast<Writer*> (this)->columns_.getNextNode ('|'));
   if (token.empty ()) {
      const_cast<Writer*> (this)->columns_.reset ();
      return token;
   }

   TRACE2 ("Writer::getNextNode (const YGP::File&, const Properties&) - Node = '"
           << token << '\'');

   std::string substitute;
   unsigned int nPos (0);
   while (((pos = token.find ('%', nPos)) != std::string::npos)
          && (pos < token.size ())) {
      if (token[pos + 1] == '*') {
         substitute = "";
         if ((pos + 1) < token.size ())
            getSubstitute (token[nPos = pos + 2], substitute, file, prop, true);
      }
      else if (token[pos + 1] != '(')
         getSubstitute (token[nPos = pos + 1], substitute, file, prop);
      else {
         nPos = pos + 1;
         do {
            getSubstitute (token[nPos], substitute, file, prop);
         } while (substitute.empty () && (token[++nPos] != ')') && token[nPos]);

         // Now skip to next closing bracket
         if (token[nPos])
            if ((nPos = token.find (')', nPos)) != std::string::npos)
               ++nPos;
            else
               nPos = token.size ();
      } // end-else '(' found

      token.replace (pos, nPos - pos + 1, substitute);
      nPos = pos + substitute.length ();
   }
   return token.empty () ? " " : token;
}


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
HTMLWriter::~HTMLWriter () {
}


//-----------------------------------------------------------------------------
/// Prints the start of an HTML-table
/// \param out: Stream where to put the output
/// \param title: Title information
//-----------------------------------------------------------------------------
void HTMLWriter::printStart (std::ostream& out, const std::string& title) const {
   out << "<table>\n";

   if (title.size ()) {
      out << "<thead><tr>";
      if (strNew.size ())
         out << "<td></td>";

      YGP::Tokenize titles (title);
      std::string node;
      while ((node = titles.getNextNode ('|')).size ())
         out << "<td>" << node << "</td>";
      out << "</tr></thead>\n";
   }
   out << "<tbody>";
}

//-----------------------------------------------------------------------------
/// Prints a file entry in HTML format
/// \param out: Stream where to put the output
/// \param file: File whose data should be printed
/// \param prop: Properties of the file
//-----------------------------------------------------------------------------
void HTMLWriter::printFile (std::ostream& out, const YGP::File& file,
                            const Properties& prop) const {
   out << "<tr valign=top>";
   if (strNew.size ()) {
      out << "<td>";
      if (isNew (file))
         out << strNew;
      out << "</td>";
   }

   std::string value;
   while (!((value = getNextNode (file, prop)).empty ()))
      out << "<td>" << value << "</td>";
   out << "</tr>\n";
}

//-----------------------------------------------------------------------------
/// Changes the HTML special characters quote ("), ampersand (&), apostrophe
/// ('), less (<) and greater (>) to HTML-values
/// \param value: String to change
/// \returns \c Changed string
//-----------------------------------------------------------------------------
std::string HTMLWriter::changeSpecialChars (const std::string& value) const {
   TRACE5 ("HTMLWriter::changeSpecialChars (const std::string&) - Changing: " << value);

   std::string chg (value);
   static const char toChange[] = { '&', '<', '>', '\'', '"' };
   static const char* changeTo[] = { "&amp;", "&lt;", "&gt;", "&apos;", "&quot;" };
   Check3 (sizeof (toChange) == (sizeof (changeTo) / sizeof (changeTo[0])));

   for (unsigned int i (0); i < chg.size (); ++i)
      for (unsigned int j (0); j < sizeof (toChange); ++j)
         if (chg[i] == toChange[j]) {
            TRACE9 ("HTMLWriter::changeSpecialChars (const std::string&) - Changing "
                    << chg[i] << " with " << changeTo[j]);
            chg.replace (i, 1, changeTo[j]);
            i += strlen (changeTo[j]);
         }
   return chg;
}

//-----------------------------------------------------------------------------
/// Changes the blank in filenames to %20
/// \param value: String to change
/// \returns \c Changed string
//-----------------------------------------------------------------------------
std::string HTMLWriter::changeSpecialFileChars (const std::string& value) const {
   std::string chg (value);
   for (unsigned int i (0); i < chg.size (); ++i)
      if (chg[i] == ' ') {
         chg.replace (i, 1, "\%20");
         i += 3;
      }
   return chg;
}

//-----------------------------------------------------------------------------
/// Prints a message in HTML-format (inside the table)
/// \param out: Stream where to put the output
/// \param file: File to which the message should be print
/// \param msg: Message to print (not NULL)
//-----------------------------------------------------------------------------
void HTMLWriter::printMessage (std::ostream& out, const YGP::File& file,
                               const std::string& msg) const {
   Check3 (!msg.empty ());

   out << "<tr valign=top>";
   if (strNew.size ()) {
      out << "<td>E</td>";
   }

   out << "<td><a href=\"" << file.path () << file.name () << "\">" << file.name ()
       << "</a></td><td colspan=" << (columns () - 1) << '>' << msg
       << "</td></tr>\n";
}

//-----------------------------------------------------------------------------
/// Prints the end of an HTML-table
/// \param out: Stream where to put the output
//-----------------------------------------------------------------------------
void HTMLWriter::printEnd (std::ostream& out) const {
   out << "</tbody></table>\n";
}


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
TextWriter::~TextWriter () {
}


//-----------------------------------------------------------------------------
/// Prints the start of a text-table
/// \param out: Stream where to put the output
/// \param title: Title information
//-----------------------------------------------------------------------------
void TextWriter::printStart (std::ostream& out, const std::string& title) const {
   if (title.size ()) {
      YGP::Tokenize titles (title);
      std::string node;
      while ((node = titles.getNextNode ('|')).size ())
         out << node << " ";
      out << '\n';
   }
}

//-----------------------------------------------------------------------------
/// Prints a file entry in text-format
/// \param out: Stream where to put the output
/// \param file: File whose data should be printed
/// \param prop: Properties of the file
//-----------------------------------------------------------------------------
void TextWriter::printFile (std::ostream& out, const YGP::File& file,
                            const Properties& prop) const {
   if (strNew.size () && isNew (file))
      out << strNew << ": ";

   std::string value;
   while (!((value = getNextNode (file, prop)).empty ()))
      if (value.size ()) {
         out << value;;
          out << ' ';
      }
   out << '\n';
}

//-----------------------------------------------------------------------------
/// Prints a message
/// \param out: Stream where to put the output
/// \param file: File to which the message should be print
/// \param msg: Message to print (not NULL)
//-----------------------------------------------------------------------------
void TextWriter::printMessage (std::ostream& out, const YGP::File& file,
                               const std::string& msg) const {
   Check3 (!msg.empty ());
   if (strNew.size () && isNew (file))
      out << "E: " << ": ";
   out << file.name () << " - " << msg << '\n';
}


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
LaTeXWriter::~LaTeXWriter () {
}


//-----------------------------------------------------------------------------
/// Prints the start of a LaTeX-table (tabular)
/// \param out: Stream where to put the output
/// \param title: Title information
//-----------------------------------------------------------------------------
void LaTeXWriter::printStart (std::ostream& out, const std::string& title) const {
   out << "\\begin{tabular}{";
   for (unsigned int i (0); i < columns (); ++i)
      out << 'l';
   out << "}\n";

   if (title.size ()) {
      if (strNew.size ())
         out << "&";

      YGP::Tokenize titles (title);
      std::string node;
      node = titles.getNextNode ('|');
      out << "{\\textbf " << node << '}';
      while ((node = titles.getNextNode ('|')).size ())
         out << "&{\\textbf " << node << "}" << node;

      out << "\\\\\n";
   }
 }

//-----------------------------------------------------------------------------
/// Prints a file entry in LaTeX format
/// \param out: Stream where to put the output
/// \param file: File whose data should be printed
/// \param prop: Properties of the file
//-----------------------------------------------------------------------------
void LaTeXWriter::printFile (std::ostream& out, const YGP::File& file,
                             const Properties& prop) const {
   if (strNew.size ()) {
      if (isNew (file))
         out << strNew;
      out << '&';
   }

   std::string value;
   if (!((value = getNextNode (file, prop)).empty ()))
      out << value;
   while (!((value = getNextNode (file, prop)).empty ()))
      out << '&' << value;
   out << "\\\\\n";
}

//-----------------------------------------------------------------------------
/// Prints a message in LaTeX-format (inside the table)
/// \param out: Stream where to put the output
/// \param file: File to which the message should be print
/// \param msg: Message to print (not NULL)
//-----------------------------------------------------------------------------
void LaTeXWriter::printMessage (std::ostream& out, const YGP::File& file,
                                const std::string& msg) const {
   Check3 (!msg.empty ());

   if (strNew.size ()) {
      if (isNew (file))
         out << "E:";
      out << '&';
   }

   out << file.name () << "&{\\multicolumn{" << (columns () - 1) << "}l{"
       << msg << "}\\\\\n";
}

//-----------------------------------------------------------------------------
/// Prints the end of a LaTeX-table
/// \param out: Stream where to put the output
//-----------------------------------------------------------------------------
void LaTeXWriter::printEnd (std::ostream& out) const {
   out << "\\end{tabular}\n";
}

//-----------------------------------------------------------------------------
/// Changes the LaTeX special characters quote ("), ampersand (&), apostrophe
/// ('), less (<) and greater (>) to HTML-values
/// \param value: String to change
/// \returns \c Changed string
//-----------------------------------------------------------------------------
std::string LaTeXWriter::changeSpecialChars (const std::string& value) const {
   std::string chg (value);
   static const char toChange[] = { '#', '$', '%', '&', '~', '_', '^', '\\',
                                    '{', '}' };
   static const char* changeTo[] = { "\\#", "\\$", "\\%", "\\&", "\\~", "\\_",
                                    "\\^", "$\\backslash$", "\\{", "\\}" };
   Check3 (sizeof (toChange) == (sizeof (changeTo) / sizeof (changeTo[0])));

   for (unsigned int i (0); i < chg.size (); ++i)
      for (unsigned int j (0); j < sizeof (toChange); ++j)
         if (chg[i] == toChange[j]) {
            TRACE9 ("LaTeXWriter::changeSpecialChars (const std::string&) - Changing "
                    << chg[i] << " with " << changeTo[j]);
            chg.replace (i, 1, changeTo[j]);
            i += strlen (changeTo[j]);
         }


   return chg;
}


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
XMLWriter::~XMLWriter () {
}


//-----------------------------------------------------------------------------
/// Prints the start of an XML-table
/// \param out: Stream where to put the output
/// \param title: Title information
//-----------------------------------------------------------------------------
void XMLWriter::printStart (std::ostream& out, const std::string& title) const {
   out << "<table>\n";

   if (title.size ()) {
      YGP::Tokenize titles (title);
      std::string node;
      while ((node = titles.getNextNode ('|')).size ())
         out << node;
      out << '\n';
   }
}

//-----------------------------------------------------------------------------
/// Prints a file entry in XML format
/// \param out: Stream where to put the output
/// \param file: File whose data should be printed
/// \param prop: Properties of the file
//-----------------------------------------------------------------------------
void XMLWriter::printFile (std::ostream& out, const YGP::File& file,
                            const Properties& prop) const {
   if (strNew.size ()) {
      if (isNew (file))
         out << "    " << strNew;
   }

   std::string value;
   while (!((value = getNextNode (file, prop)).empty ()))
      out << "    " << value;
   out << '\n';
}

//-----------------------------------------------------------------------------
/// Prints a message in XML-format (inside the table)
/// \param out: Stream where to put the output
/// \param file: File to which the message should be print
/// \param msg: Message to print (not NULL)
//-----------------------------------------------------------------------------
void XMLWriter::printMessage (std::ostream& out, const YGP::File& file,
                               const std::string& msg) const {
   Check3 (!msg.empty ());

   out << "<Error><File>" << file.path () << file.name () << "</File>"
       << "<Name>" << file.name () << "<Name>"
       << "<Description>" << msg << "</Desription></Error>\n";
}

//-----------------------------------------------------------------------------
/// Prints the end of an XML-table
/// \param out: Stream where to put the output
//-----------------------------------------------------------------------------
void XMLWriter::printEnd (std::ostream& out) const {
   out << "</table>\n";
}
