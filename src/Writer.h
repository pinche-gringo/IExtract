#ifndef WRITER_H
#define WRITER_H

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


#include <string.h>

#include <string>

#include <iosfwd>

struct File;
struct Properties;


// Baseclass of output classes
class Writer {
 public:
   Writer (const std::string& format, const std::string& New, unsigned long age = 0);
   virtual ~Writer ();

   virtual void printStart (std::ostream& out, const std::string& title) const { };
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const = 0;
   virtual void printMessage (std::ostream& out, const File& file,
                              const std::string& msg) const = 0;
   virtual void printEnd (std::ostream& out) const { };

   void printSeparator (std::ostream& out, const File& file,
                        const std::string& sep, const std::string& title = "") const;

 protected:
   virtual std::string changeSpecialChars (const std::string& val) const { return val; }

   bool isNew (const File& file) const {
      return file.time () > limit; }

   unsigned int columns () const;

   unsigned int      options;
   const std::string strNew;

   std::string getNextNode (const File& file, const Properties& prop) const;
   void getSubstitute (const char ctrl, std::string& substitute, const File& file,
                       const Properties& prop) const;

   static std::string convertToHumanString (unsigned long value);

 private:
   long limit;
   Tokenize columns_;
};


// Class to write fileinfo in HTML format
class HTMLWriter : public Writer {
 public:
   HTMLWriter (const std::string& format, const std::string& strNew, unsigned long age = 0)
      : Writer (format, strNew, age) { }
   virtual ~HTMLWriter ();

   virtual void printStart (std::ostream& out, const std::string& title) const;
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const std::string& msg) const;
   virtual void printEnd (std::ostream& out) const;

   static HTMLWriter* create (const std::string& format, const std::string& strNew,
                              unsigned long age = 0) {
      return new HTMLWriter (format, strNew, age); }

   virtual std::string changeSpecialChars (const std::string& value) const;
};


// Class to write fileinfo in XML format
class XMLWriter : public HTMLWriter {
 public:
   XMLWriter (const std::string& format, const std::string& strNew,
                 unsigned long age = 0) : HTMLWriter (format, strNew, age) { }
   virtual ~XMLWriter ();

   virtual void printStart (std::ostream& out, const std::string& title) const;
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const std::string& msg) const;
   virtual void printEnd (std::ostream& out) const;

   static XMLWriter* create (const std::string& format, const std::string& strNew,
                              unsigned long age = 0) {
      return new XMLWriter (format, strNew, age); }
};


// Class to write fileinfo in text format
class TextWriter : public Writer {
 public:
   TextWriter (const std::string& format, const std::string& strNew, unsigned long age = 0)
      : Writer (format, strNew, age) { }
   virtual ~TextWriter ();

   virtual void printStart (std::ostream& out, const std::string& title) const;
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const std::string& msg) const;

   static TextWriter* create (const std::string& format, const std::string& strNew,
                              unsigned long age = 0) {
      return new TextWriter (format, strNew, age); }
};



// Class to write fileinfo in LaTeX format
class LaTeXWriter : public Writer {
 public:
   LaTeXWriter (const std::string& format, const std::string& strNew, unsigned long age = 0)
      : Writer (format, strNew, age) { }
   virtual ~LaTeXWriter ();

   virtual void printStart (std::ostream& out, const std::string& title) const;
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const std::string& msg) const;
   virtual void printEnd (std::ostream& out) const;

   static LaTeXWriter* create (const std::string& format, const std::string& strNew,
                               unsigned long age = 0) {
      return new LaTeXWriter (format, strNew, age); }

 protected:
   virtual std::string changeSpecialChars (const std::string& val) const;
};


#endif
