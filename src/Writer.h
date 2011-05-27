#ifndef WRITER_H
#define WRITER_H

//$Id$

// This file is part of IExtract.
//
// IExtract is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IExtract is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with libYGP.  If not, see <http://www.gnu.org/licenses/>.


#include <string.h>

#include <string>

#include <YGP/TableWriter.h>


namespace YGP {
   struct File;
}
struct Properties;



/**Baseclass of output classes
 */
class Writer : public YGP::TableWriter {
 public:
   typedef std::string (*CHANGECHARS)(const std::string&);

   Writer (const std::string& format, const std::string& New, unsigned long age = 0,
	   const char* startRow = "", const char* endRow = "", const char* sepColumn = " ",
	   const char* startTab = "", const char* endTab = "", const char* sepTab = " ",
	   const char* rowStartHdr = NULL, const char* rowEndHdr = NULL,
	   const char* sepHdrCol = NULL, const char* defColumns = NULL);
   virtual ~Writer ();

   virtual std::string getSubstitute (char ctrl, bool extend = false) const;

   void printFile (std::ostream& out, const YGP::File& file, const Properties& prop);
   void printSeparator (std::ostream& out, const YGP::File& file,
			const std::string& data, const std::string& title) const;

   virtual void printMessage (std::ostream& out, const YGP::File& file, const std::string& msg) const = 0;
   virtual void printHeaderTail (std::ostream& out) const;

   virtual std::string changeSpecialChars (const std::string& value) const;
   virtual std::string changeSpecialFileChars (const std::string& value) const;

 protected:
   bool isNew (const YGP::File& file) const {
      return file.time () > limit; }

   const std::string strNew;
   long limit;

   static std::string convertToHumanString (unsigned long value);

   const YGP::File* file_;
   const Properties* prop_;

 private:
   Writer (const Writer&);
   Writer& operator= (const Writer&);
};


/**Class to write fileinfo in text format
 */
class TextWriter : public Writer {
 public:
   TextWriter (const std::string& format, const std::string& strNew, unsigned long age = 0);
   virtual ~TextWriter ();

   virtual void printMessage (std::ostream& out, const YGP::File& file, const std::string& msg) const;

   /// Creates a text writer
   /// \param format: Format how to display entries
   static TextWriter* create (const std::string& format, const std::string& strNew,
                              unsigned long age = 0) {
      return new TextWriter (format, strNew, age); }

 private:
   TextWriter (const TextWriter&);
   TextWriter& operator= (const TextWriter&);
};


/**Class to write fileinfo in text format
 */
class QuotedTextWriter : public Writer {
 public:
   QuotedTextWriter (const std::string& format, const std::string& strNew, unsigned long age = 0);
   virtual ~QuotedTextWriter ();

   virtual void printMessage (std::ostream& out, const YGP::File& file, const std::string& msg) const;

   virtual std::string changeSpecialChars (const std::string& value) const;

   /// Creates a text writer
   /// \param format: Format how to display entries
   static QuotedTextWriter* create (const std::string& format, const std::string& strNew,
				    unsigned long age = 0) {
      return new QuotedTextWriter (format, strNew, age); }

 private:
   QuotedTextWriter (const QuotedTextWriter&);
   QuotedTextWriter& operator= (const QuotedTextWriter&);
};


/**Class to write fileinfo in HTML format
 */
class HTMLWriter : public Writer {
 public:
   HTMLWriter (const std::string& format, const std::string& strNew, unsigned long age = 0);
   virtual ~HTMLWriter ();

   virtual void printMessage (std::ostream& out, const YGP::File& file, const std::string& msg) const;

   virtual std::string changeSpecialChars (const std::string& value) const;
   virtual std::string changeSpecialFileChars (const std::string& value) const;

   /// Creates an HTML writer
   /// \param format: Format how to display entries
   static HTMLWriter* create (const std::string& format, const std::string& strNew,
                              unsigned long age = 0) {
      return new HTMLWriter (format, strNew, age); }

 private:
   HTMLWriter (const HTMLWriter&);
   HTMLWriter& operator= (const HTMLWriter&);
};


/**Class to write fileinfo in XML format
 */
class XMLWriter : public HTMLWriter {
 public:
   XMLWriter (const std::string& format, const std::string& strNew,
	      unsigned long age = 0) : HTMLWriter (format, strNew, age) { }
   virtual ~XMLWriter ();

   virtual void printMessage (std::ostream& out, const YGP::File& file, const std::string& msg) const;

   /// Creates an XML writer
   /// \param format: Format how to display entries
   static XMLWriter* create (const std::string& format, const std::string& strNew,
                              unsigned long age = 0) {
      return new XMLWriter (format, strNew, age); }

 private:
   XMLWriter (const XMLWriter&);
   XMLWriter& operator= (const XMLWriter&);
};


/**Class to write fileinfo in LaTeX format
 */
class LaTeXWriter : public Writer {
 public:
   LaTeXWriter (const std::string& format, const std::string& strNew, unsigned long age = 0);
   virtual ~LaTeXWriter ();

   virtual void printMessage (std::ostream& out, const YGP::File& file, const std::string& msg) const;
   virtual void printHeaderLead (std::ostream& out) const;

   virtual std::string changeSpecialChars (const std::string& value) const;

   /// Creates a LaTeX writer
   /// \param format: Format how to display entries
   static LaTeXWriter* create (const std::string& format, const std::string& strNew,
                               unsigned long age = 0) {
      return new LaTeXWriter (format, strNew, age); }

 private:
   LaTeXWriter (const LaTeXWriter&);
   LaTeXWriter& operator= (const LaTeXWriter&);
};


#endif
