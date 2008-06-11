#ifndef OPTIONS_H
#define OPTIONS_H

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


#include <string>

#include <YGP/Entity.h>


class Options : public YGP::Entity {
   friend class Application;

 public:
   Options ();
   ~Options ();

 private:
   Options (const Options& other);
   const Options& operator= (const Options& other);

   std::string  format;        // %attrib%; Format
   std::string  title;         // %attrib%; Title
   std::string  newText;       // %attrib%; TextForNewFiles
   unsigned int ageOfNewFiles; // %attrib%; MaxAgeForNewFiles; 0
   std::string  style;         // %attrib%; Style
   std::string  separate;      // %attrib%; DirSeparatorText
   unsigned int sort;          // %attrib%; SortFiles; 0
};


#include "Options.meta"

#endif
