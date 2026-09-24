#ifndef SELECTION_H
#define SELECTION_H

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

#include <IExtract-cfg.h>

#include <YGP/Parse.h>


/**Selection, which tries all its entries as optional.
 *
 * YGP::ParseSelection passes the optional-flag of the selection to its last
 * entry, so a mismatch of the last entry throws an exception instead of just
 * terminating the selection. The parsers of IExtract rely on the latter.
 */
class Selection : public YGP::ParseSelection {
 public:
   Selection (YGP::ParseObject* apObjectList[], const char* description,
              unsigned int max = 1, unsigned int min = 1, bool skipWhitespace = true)
      : YGP::ParseSelection (apObjectList, description, max, min, skipWhitespace) { }

 protected:
   int doParse (YGP::Xistream& stream, bool optional) override {
      int rc (YGP::ParseSelection::doParse (stream, true));
      if (rc && !optional) {
         std::string error (_("Expected selection %1"));
         error.replace (error.find ("%1"), 2, getDescription ());
         throw YGP::ParseError (error);
      }
      return rc;
   }
};

#endif
