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

 private:
   FileTypeChecker ();
   FileTypeChecker (const FileTypeChecker& other);
   ~FileTypeChecker ();
   const FileTypeChecker& operator= (const FileTypeChecker& other);
};


/**Class to check for the class of file (e.g. MS Office document) by
   the name of the file
 */
class FileTypeCheckerByName : public FileTypeChecker {
 public:
   static FileType getType (const char* file);

 private:
   FileTypeCheckerByName ();
   FileTypeCheckerByName (const FileTypeCheckerByName& other);
   ~FileTypeCheckerByName ();
   const FileTypeCheckerByName& operator= (const FileTypeCheckerByName& other);

   static std::map<const char*, FileType, lessDereferenced> types;
};


/**Type to check for type of files by the content of the file
 */
class FileTypeCheckerByContent : public FileTypeChecker {
 public:
   static FileType getType (const char* file);

 private:
   FileTypeCheckerByContent ();
   FileTypeCheckerByContent (const FileTypeCheckerByContent& other);
   ~FileTypeCheckerByContent ();
   const FileTypeCheckerByContent& operator= (const FileTypeCheckerByContent& other);

   static void skipHTMLComments (char* buffer, unsigned int size, std::ifstream& stream);

   static std::map<std::string, FileType> types;
};

#endif
