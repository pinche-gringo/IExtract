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
   Writer (const char* format, unsigned long age = 0, const char* pNew = NULL);
   virtual ~Writer ();

   virtual void printStart (std::ostream& out, const char* title = NULL) const { };
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const = 0;
   virtual void printMessage (std::ostream& out, const File& file,
                              const char* msg) const = 0;
   virtual void printEnd (std::ostream& out) const { };

 protected:
   bool isNew (const File& file) const {
      return file.time () > limit; }

   unsigned int columns () const;

   unsigned int  options;
   const char*   pStrNew;
   const char*   pFormat;

   class OutIterator {
      friend class Writer;

    public:
      OutIterator (const char* format, const File& outfile)
         : file (&outfile), p (NULL), columns_ (format) { columns_.getNextNode ('|'); }
      OutIterator (const char* format, const File& outfile, 
                   const struct Properties& prop)
         : file (&outfile), p (&prop), columns_ (format) { columns_.getNextNode ('|'); }
      ~OutIterator () { }

      std::string operator++ () {
         return columns_.getNextNode ('|');
      }
      std::string operator++ (int) {
         std::string old (columns_.getActNode ());
         columns_.getNextNode ('|');
         return old;
      }

      operator void*() const { return (void*)columns_.getActNode ().size (); }
      bool operator! () const { return columns_.getActNode ().empty (); }

      std::string operator* () const;

      bool isAtName () const;

    private:
      OutIterator (const char* format) : columns_ (format), p (NULL), file (NULL) {
         columns_.getNextNode ('|'); }

      Tokenize columns_;

      const File* file;
      const struct Properties* p;

      OutIterator (const OutIterator&);
      OutIterator& operator= (const OutIterator&);
   };

 private:
   unsigned long limit;
};


// Class to write fileinfo in HTML format
class HTMLWriter : public Writer {
 public:
   HTMLWriter (const char* format, unsigned long age = 0, const char* pNew = NULL)
      : Writer (format, age, pNew) { }
   virtual ~HTMLWriter ();

   virtual void printStart (std::ostream& out, const char* title = NULL) const;
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const char* msg) const;
   virtual void printEnd (std::ostream& out) const;

   static HTMLWriter* create (const char* format, unsigned long age = 0,
                              const char* pNew = NULL) {
      return new HTMLWriter (format, age, pNew); }
};


// Class to write fileinfo in text format
class TextWriter : public Writer {
 public:
   TextWriter (const char* format, unsigned long age = 0, const char* pNew = NULL)
      : Writer (format, age, pNew) { }
   virtual ~TextWriter ();

   virtual void printStart (std::ostream& out, const char* title = NULL) const;
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const char* msg) const;

   static TextWriter* create (const char* format, unsigned long age = 0,
                              const char* pNew = NULL) {
      return new TextWriter (format, age, pNew); }
};



// Class to write fileinfo in LaTeX format
class LaTeXWriter : public Writer {
 public:
   LaTeXWriter (const char* format, unsigned long age = 0, const char* pNew = NULL)
      : Writer (format, age, pNew) { }
   virtual ~LaTeXWriter ();

   virtual void printStart (std::ostream& out, const char* title = NULL) const;
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const char* msg) const;
   virtual void printEnd (std::ostream& out) const;

   static LaTeXWriter* create (const char* format, unsigned long age = 0,
                              const char* pNew = NULL) {
      return new LaTeXWriter (format, age, pNew); }
};


#endif
