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

#include <iosfwd>

struct File;
struct Properties;


// Baseclass of output classes
class Writer {
 public:
   Writer (unsigned int showOptions = 0, unsigned long age = 0,
           const char* pNew = NULL);
   virtual ~Writer ();

   virtual void printStart (std::ostream& out) const { };
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const = 0;
   virtual void printMessage (std::ostream& out, const File& file,
                              const char* msg) const = 0;
   virtual void printEnd (std::ostream& out) const { };

   typedef enum { SHOW_PATH = 0x1 } showOptions;

 protected:
   bool isNew (const File& file) const {
      return file.time () > limit; }

   unsigned int  options;
   const char*   pStrNew;

 private:
   unsigned long limit;
};


// Class to write fileinfo in HTML format
class HTMLWriter : public Writer {
 public:
   HTMLWriter (unsigned int showOptions = 0, unsigned long age = 0,
               const char* pNew = NULL)
      : Writer (showOptions, age, pNew) { }
   virtual ~HTMLWriter ();

   virtual void printStart (std::ostream& out) const;
   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const char* msg) const;
   virtual void printEnd (std::ostream& out) const;

   static HTMLWriter* create (unsigned int options, unsigned long age = 0,
                              const char* pNew = NULL) {
      return new HTMLWriter (options, age, pNew); }
};


// Class to write fileinfo in text format
class TextWriter : public Writer {
 public:
   TextWriter (unsigned int showOptions = 0, unsigned long age = 0,
               const char* pNew = NULL)
      : Writer (showOptions, age, pNew) { }
   virtual ~TextWriter ();

   virtual void printFile (std::ostream& out, const File& file,
                           const Properties& prop) const;
   virtual void printMessage (std::ostream& out, const File& file,
                              const char* msg) const;

   static TextWriter* create (unsigned int options, unsigned long age = 0,
                              const char* pNew = NULL) {
      return new TextWriter (options, age, pNew); }
};


#endif
