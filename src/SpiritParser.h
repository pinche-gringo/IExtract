#ifndef SPIRITPARSER_H
#define SPIRITPARSER_H

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


#include <cstddef>

#include <istream>
#include <iterator>
#include <string>
#include <type_traits>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/binary.hpp>

#include <istream>
#include <YGP/Exception.h>

#include <IExtract-cfg.h>


/// Helpers to parse (binary) files with Boost.Spirit X3
namespace SpiritParser {

namespace x3 = boost::spirit::x3;

/// Iterator the parsers work with
using Iterator = const char*;


/// Parser consuming a number of bytes, which is only known while parsing
/// (e.g. because it was read before). If \c Store is true, the bytes are
/// exposed as attribute (a std::string).
template <bool Store>
struct Bytes : x3::parser<Bytes<Store>> {
   using attribute_type = std::conditional_t<Store, std::string, x3::unused_type>;
   static constexpr bool has_attribute = Store;

   /// Constructor
   /// \param count Reference to the number of bytes to consume; evaluated while parsing
   explicit Bytes (const std::size_t& count) : count (count) { }
   Bytes (std::size_t&&) = delete;

   template <typename It, typename Context, typename RContext, typename Attribute>
   bool parse (It& first, const It& last, const Context&, RContext&, Attribute& attr) const {
      if (static_cast<std::size_t> (last - first) < count)
         return false;
      if constexpr (Store)
         x3::traits::move_to (first, first + count, attr);
      first += count;
      return true;
   }

   const std::size_t& count;
};

/// Returns a parser consuming (and exposing) \c count bytes
inline Bytes<true> bytes (const std::size_t& count) { return Bytes<true> (count); }
/// Returns a parser consuming (and ignoring) \c count bytes
inline Bytes<false> skip (const std::size_t& count) { return Bytes<false> (count); }
Bytes<true> bytes (std::size_t&&) = delete;
Bytes<false> skip (std::size_t&&) = delete;


/// Directive parsing its subject exactly a number of times, which is only
/// known while parsing. The attribute of the subject is ignored.
template <typename Subject>
struct Times : x3::unary_parser<Subject, Times<Subject>> {
   using base_type = x3::unary_parser<Subject, Times<Subject>>;
   using attribute_type = x3::unused_type;
   static constexpr bool has_attribute = false;

   Times (const Subject& subject, const std::size_t& count) : base_type (subject), count (count) { }

   template <typename It, typename Context, typename RContext, typename Attribute>
   bool parse (It& first, const It& last, const Context& context, RContext& rcontext, Attribute&) const {
      It start (first);
      for (std::size_t i (count); i; --i)
         if (!this->subject.parse (first, last, context, rcontext, x3::unused)) {
            first = start;
            return false;
         }
      return true;
   }

   const std::size_t& count;
};

/// Generator for the Times-directive: times (count)[subject]
struct TimesGen {
   explicit TimesGen (const std::size_t& count) : count (count) { }

   template <typename Subject>
   Times<typename x3::extension::as_parser<Subject>::value_type> operator[] (const Subject& subject) const {
      return { x3::as_parser (subject), count };
   }

   const std::size_t& count;
};

/// Returns a directive to parse its subject \c count times
inline TimesGen times (const std::size_t& count) { return TimesGen (count); }
TimesGen times (std::size_t&&) = delete;


/// Parser for a text, which is compared case-insensitive (only ASCII
/// characters are converted). Contrary to x3::no_case it can also be used
/// with text containing (signed) characters above 0x7f.
struct NoCase : x3::parser<NoCase> {
   using attribute_type = x3::unused_type;
   static constexpr bool has_attribute = false;

   /// Constructor
   /// \param text Text to parse (must be lower-case)
   explicit NoCase (const char* text) : text (text) { }

   template <typename It, typename Context, typename RContext, typename Attribute>
   bool parse (It& first, const It& last, const Context&, RContext&, Attribute&) const {
      It act (first);
      for (const char* pText (text); *pText; ++pText, ++act) {
         if (act == last)
            return false;
         char ch (*act);
         if ((ch >= 'A') && (ch <= 'Z'))
            ch += 'a' - 'A';
         if (ch != *pText)
            return false;
      }
      first = act;
      return true;
   }

   const char* text;
};

/// Returns a parser for the passed text, compared case-insensitive
/// \param text Text to parse (must be lower-case)
inline NoCase nocase (const char* text) { return NoCase (text); }


/// Parser skipping (optional) whitespaces. Contrary to x3::space it can also
/// be used with text containing (signed) characters above 0x7f.
inline const auto ws = x3::omit[*x3::char_ (" \t\n\r\f\v")];


/// Reads the (rest of the) stream into memory
/// \param stream Stream to read
/// \returns std::string Read data
inline std::string readStream (std::istream& stream) {
   return std::string (std::istreambuf_iterator<char> (stream), std::istreambuf_iterator<char> ());
}

/// Parses the passed data with the passed parser
/// \param first Start of the data to parse; set to the end of the parsed data
/// \param last End of data
/// \param parser Parser to use
/// \param description Description of the parsed data (for error messages)
/// \throw YGP::ParseError If the data does not match the parser
template <typename Parser>
void parse (Iterator& first, Iterator last, const Parser& parser, const char* description) {
   bool ok (false);
   try {
      ok = x3::parse (first, last, parser);
   }
   catch (const x3::expectation_failure<Iterator>& err) {
      std::string error (_("Invalid %1; expected: %2"));
      error.replace (error.find ("%2"), 2, err.which ());
      error.replace (error.find ("%1"), 2, description);
      throw YGP::ParseError (error);
   }
   if (!ok) {
      std::string error (_("Invalid %1"));
      error.replace (error.find ("%1"), 2, description);
      throw YGP::ParseError (error);
   }
}

/// Parses the passed data with the passed parser
/// \param data Data to parse
/// \param parser Parser to use
/// \param description Description of the parsed data (for error messages)
/// \throw YGP::ParseError If the data does not match the parser
template <typename Parser>
void parse (const std::string& data, const Parser& parser, const char* description) {
   Iterator first (data.data ());
   parse (first, data.data () + data.size (), parser, description);
}

}

#endif
