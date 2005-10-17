//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : Writer
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 13.10.2002
//COPYRIGHT   : Copyright (C) 2002 - 2005

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
/// Constructor
/// \param New: Text to display for new files
/// \param age: Maximal age for new files
/// \param startRow: String starting rows
/// \param endRow: String terminating rows
/// \param sepColumn: String separating columns
/// \param startTab: String starting table
/// \param endTab: String terminating table
/// \param sepTab: String separating the tableheader from the tablebody
/// \param startRowHdr: String starting header of the table
/// \param endRowHdr: String terminating header of the table
/// \param sepHdrCol: String terminating columns of the header of the table
/// \param defColumns: Definition of the columns
//-----------------------------------------------------------------------------
Writer::Writer (const std::string& format, const std::string& New, unsigned long age,
		const char* startRow, const char* endRow, const char* sepColumn,
		const char* startTab, const char* endTab, const char* sepTab,
		const char* rowStartHdr, const char* rowEndHdr,
		const char* sepHdrCol, const char* defColumns)
   : YGP::TableWriter (format, startRow, endRow, sepColumn, startTab, endTab, sepTab, rowStartHdr, rowEndHdr, sepHdrCol, defColumns),
     strNew (New), file_ (NULL), prop_ (NULL) {
   Check3 (strNew.size () ? age : 1);

   limit = time (NULL) - age;
}

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
Writer::~Writer () {
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
/// Returns the substitute for a control character.
///
///  Substitutes:
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
/// \param extend: Flag, if special formatting of substitute is wanted
/// \returns std::string: String with which to replace the character
//-----------------------------------------------------------------------------
std::string Writer::getSubstitute (char ctrl, bool extend) const {
   Check3 (prop_); Check3 (file_); Check1 (writer);

   std::string subst;
   switch (ctrl) {
   case 'a':
      subst = extend ? prop_->strAuthor : changeSpecialChars (prop_->strAuthor);
      break;

   case 'c':
      subst = extend ? prop_->strAuthor : changeSpecialChars (prop_->strComment);
      break;

   case 'D':
   case 'd': {
      YGP::ATimestamp stamp (file_->time ());
      subst = (ctrl == 'D') ? stamp.ADate::toString () : stamp.toString ();
      break; }

   case 'e': {
	  const char* ext = strrchr (file_->name (), '.');
      ext = ext ? ext + 1 : "";
      subst = extend ? changeSpecialFileChars (ext) : ext;
      break;
   }

   case 'E': {
      std::string tmp;
      const char* ext = strrchr (file_->name (), '.');
      if (ext) {
         tmp = file_->name ();
         tmp.erase (ext - file_->name ());
         ext = tmp.c_str ();
      }
      else
         ext = file_->name ();
      subst = extend ? changeSpecialFileChars (ext) : ext;
      break;
   }

   case 'n': subst = changeSpecialFileChars (file_->name ()); break;

   case 'N':
      subst = extend ? changeSpecialFileChars (file_->path ()) : file_->path ();
      subst += extend ? changeSpecialFileChars (file_->name ()) : file_->name ();
      break;

   case 'p':
      subst = extend ? changeSpecialFileChars (file_->path ()) : file_->path ();
      break;

   case 't':
      subst = extend ? prop_->strTitle : changeSpecialChars (prop_->strTitle);
      break;

   case 'P':
   case 'U': {
      subst = extend ? changeSpecialFileChars (file_->path ()) : file_->path ();
#if SYSTEM != UNIX
      unsigned int pos (0);
      while ((pos = subst.find (YGP::File::DIRSEPARATOR, pos)) != std::string::npos)
         subst.replace (pos, 1, 1, '/');
#endif

      if (ctrl == 'U')
         subst += extend ? changeSpecialFileChars (file_->name ()) : file_->name ();
         break; }

   case 's':
   case 'S': {
      YGP::ANumeric size (file_->size ());
      subst = "";
      subst = ((ctrl == 'S') ? convertToHumanString (file_->size ())
               : YGP::ANumeric::toString (file_->size ()));
      break; }

   default:
      subst = ctrl;
   }

   TRACE9 ("Writer::getSubstitute (char, const YGP::TableWriter*, std::string&) - Replacing '"
           << ctrl << "' with " << subst);
   return subst;
}

//-----------------------------------------------------------------------------
/// Change characters with special meanings to ones understood by the writer
/// \param value: Value to change
/// \returns std::string: Changed valaue
//-----------------------------------------------------------------------------
std::string Writer::changeSpecialChars (const std::string& value) const {
   return value;
}

//-----------------------------------------------------------------------------
/// Change characters with special meanings to ones understood by the writer
/// \param value: Value to change
/// \returns std::string: Changed valaue
//-----------------------------------------------------------------------------
std::string Writer::changeSpecialFileChars (const std::string& value) const {
   return changeSpecialChars (value);
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
/// Prints the start of the table header
/// \param out: Stream where to put the output
/// \param title: Title information
//-----------------------------------------------------------------------------
void Writer::printHeaderTail (std::ostream& out) const {
   if (strNew.size ())
      out << colHdrSeparator;
}

//-----------------------------------------------------------------------------
/// Prints a file entry in HTML format
/// \param out: Stream where to put the output
/// \param file: File whose data should be printed
/// \param prop: Properties of the file
//-----------------------------------------------------------------------------
void Writer::printFile (std::ostream& out, const YGP::File& file, const Properties& prop) {
   TRACE9 ("Writer::printFile (std::ostream&, const std::string&) const");

   out << rowStart;
   if (strNew.size ()) {
      if (isNew (file))
         out << strNew;
      out << colHdrSeparator;
   }

   file_ = &file;
   prop_ = &prop;

   std::string node (getNextNode ());
   out << node;
   while (!((node = getNextNode ()).empty ()))
      out << colSeparator << node;
   out << rowEnd;
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param format: Format how to display entries
/// \param strNew: String to display for new entries
/// \param age: Maximal age (in days) for entries to be considered as new
//-----------------------------------------------------------------------------
TextWriter::TextWriter (const std::string& format, const std::string& strNew, unsigned long age)
   : Writer (format, strNew, age, TBLW_TEXT_PARAMS) {
 }

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
TextWriter::~TextWriter () {
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
      out << "!!" << ": ";
   out << file.name () << " - " << msg << '\n';
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param format: Format how to display entries
/// \param strNew: String to display for new entries
/// \param age: Maximal age (in days) for entries to be considered as new
//-----------------------------------------------------------------------------
HTMLWriter::HTMLWriter (const std::string& format, const std::string& strNew, unsigned long age)
   : Writer (format, strNew, age, TBLW_HTML_PARAMS) {
 }

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
HTMLWriter::~HTMLWriter () {
}


//-----------------------------------------------------------------------------
/// Change characters with special meanings to ones understood by the writer
/// \param value: Value to change
/// \returns std::string: Changed valaue
//-----------------------------------------------------------------------------
std::string HTMLWriter::changeSpecialChars (const std::string& value) const {
   return YGP::TableWriter::changeHTMLSpecialChars (value);
}

//-----------------------------------------------------------------------------
/// Change characters with special meanings to ones understood by the writer
/// \param value: Value to change
/// \returns std::string: Changed valaue
//-----------------------------------------------------------------------------
std::string HTMLWriter::changeSpecialFileChars (const std::string& value) const {
   return YGP::TableWriter::changeHTMLSpecialFileChars (value);
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

   out << rowStart;
   if (strNew.size ()) {
      out << "!!" << colSeparator;
   }

   out << "<a href=\"" << file_->path () << file_->name () << "\">" << file_->name ()
       << "</a></td><td colspan=" << (columns () - 1) << '>' << msg << rowEnd;
}


//-----------------------------------------------------------------------------
/// Constructor
/// \param format: Format how to display entries
/// \param strNew: String to display for new entries
/// \param age: Maximal age (in days) for entries to be considered as new
//-----------------------------------------------------------------------------
LaTeXWriter::LaTeXWriter (const std::string& format, const std::string& strNew, unsigned long age)
   : Writer (format, strNew, age, TBLW_LATEX_PARAMS) {
 }

//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
LaTeXWriter::~LaTeXWriter () {
}


//-----------------------------------------------------------------------------
/// Prints the start of the table header
/// \param out: Stream where to put the output
/// \param title: Title information
//-----------------------------------------------------------------------------
void LaTeXWriter::printHeaderLead (std::ostream& out) const {
   YGP::TableWriter::printLaTeXHeaderLead (out, columns ());
}

//-----------------------------------------------------------------------------
/// Change characters with special meanings to ones understood by the writer
/// \param value: Value to change
/// \returns std::string: Changed valaue
//-----------------------------------------------------------------------------
std::string LaTeXWriter::changeSpecialChars (const std::string& value) const {
   return YGP::TableWriter::changeLaTeXSpecialChars (value);
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
         out << "!!";
      out << '&';
   }

   out << file.name () << "&{\\multicolumn{" << (columns () - 1) << "}l{"
       << msg << rowEnd;
}


//-----------------------------------------------------------------------------
/// Destructor
//-----------------------------------------------------------------------------
XMLWriter::~XMLWriter () {
}

//-----------------------------------------------------------------------------
/// Prints a message in XML-format (inside the table)
/// \param out: Stream where to put the output
/// \param file: File to which the message should be print
/// \param msg: Message to print (not NULL)
//-----------------------------------------------------------------------------
void XMLWriter::printMessage (std::ostream& out, const YGP::File& file, const std::string& msg) const {
   Check3 (!msg.empty ());

   out << "<Error><File>" << file.path () << file.name () << "</File>"
       << "<Name>" << file.name () << "<Name>"
       << "<Description>" << msg << "</Desription></Error>\n";
}
