#ifndef PARSEPDF_H
#define PARSEPDF_H

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


#include <map>
#include <string>

#include <YGP/XStream.h>

struct Properties;


/**Class to extract the properties of a PDF document.
 *
 * The PDF-specification can be found at
 * http://www.adobe.com/devnet/acrobat/pdfs/pdf_reference.pdf
 */
class ParsePDF {
 public:
   static void parse (YGP::Xistream& stream, Properties& result);

 private:
   ParsePDF ();
   ParsePDF (const ParsePDF& other);
   const ParsePDF& operator= (const ParsePDF& other);

   /// Values of a (top-level) dictionary
   struct Dictionary {
      Dictionary () : strings (), others () { }

      std::map<std::string, std::string> strings;    ///< Entries with string values
      std::map<std::string, std::string> others;     ///< Entries with other (simple) values
   };

   static Dictionary parseXRefTable (const std::string& data, std::size_t offset,
                                     std::map<unsigned int, std::size_t>& offsets);
   static Dictionary parseObject (const std::string& data, std::size_t offset, unsigned int id);
   static unsigned int parseReference (const std::string& reference);
};

#endif
