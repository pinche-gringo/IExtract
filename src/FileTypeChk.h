#ifndef FILETYPECHK_H
#define FILETYPECHK_H

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


#include <map>
#include <string>
#include <iosfwd>


/**Base-class to check for file-types
 */
class FileTypeChecker {
 public:
   typedef enum { UNKNOWN, ABIWORD, GIF, HTML, JPEG, MP3, MSOFFICE, OGG,
		  OPENOFFICE, PDF, PNG, RTF, STAROFFICE, LAST } FileType;

   virtual FileType getType (const char* file) const = 0;
   virtual ~FileTypeChecker ();

protected:
   FileTypeChecker () { }

 private:
   FileTypeChecker (const FileTypeChecker& other);
   const FileTypeChecker& operator= (const FileTypeChecker& other);
};


/**Class to check for the type of a file (e.g. MS Office document) by
   the extension of the file
 */
class FileTypeCheckerByExtension : public FileTypeChecker {
 public:
   FileTypeCheckerByExtension ();
   ~FileTypeCheckerByExtension ();

   /// Gets the type of the passed file
   /// \param file: File to analyse
   /// \returns FileType: Type of file
   virtual FileType getType (const char* file) const;

 protected:
   struct lessDereferenced : public std::binary_function<const char*, const char*, bool> {
   /// Access to the functor
   /// \param x: First object to compare
   /// \param y: Second object to compare
   /// \returns bool: True, if the string the  object x points to is
   ///     smaller than the string the second objects points to.
   bool operator() (const char* x, const char* y) const {
      return strcmp (x, y) < 0; }
   };

   /// Gets the file-type defined by the passed extension
   /// \param file: Extension of file to analyse
   /// \returns FileType: Type of file
   FileType getType4Extension (const char* extension) const;

 private:
   FileTypeCheckerByExtension (const FileTypeCheckerByExtension& other);
   const FileTypeCheckerByExtension& operator= (const FileTypeCheckerByExtension& other);

   std::map<const char*, FileType, lessDereferenced> types;
};


/**Class to check for the type of a file (e.g. MS Office document) by
   the extension  of the file (ignoring the case)
 */
class FileTypeCheckerByCaseExt : public FileTypeCheckerByExtension {
 public:
   /// Defaultconstructor
   FileTypeCheckerByCaseExt () : FileTypeCheckerByExtension () { }
   ~FileTypeCheckerByCaseExt ();

   /// Gets the type of the passed file
   /// \param file: File to analyse
   /// \returns FileType: Type of file
   virtual FileType getType (const char* file) const;

 private:
   FileTypeCheckerByCaseExt (const FileTypeCheckerByCaseExt& other);
   const FileTypeCheckerByCaseExt& operator= (const FileTypeCheckerByCaseExt& other);
};


/**Type to check for type of files by the content of the file
 */
class FileTypeCheckerByContent : public FileTypeChecker {
 public:
   FileTypeCheckerByContent ();
   ~FileTypeCheckerByContent ();

   /// Gets the type of the passed file
   /// \param file: File to analyse
   /// \returns FileType: Type of file
   virtual FileType getType (const char* file) const;

 private:
   FileTypeCheckerByContent (const FileTypeCheckerByContent& other);
   const FileTypeCheckerByContent& operator= (const FileTypeCheckerByContent& other);

   static void skipHTMLComments (char* buffer, unsigned int size, std::ifstream& stream);

   std::map<std::string, FileType> types;
};

#endif
