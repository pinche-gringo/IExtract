// PROJECT     : Extract
// SUBSYSTEM   : ParsePDF
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 08.11.2002
// COPYRIGHT   : Copyright (C) 2002 - 2004, 2006 - 2008, 2011, 2024, 2026

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

#include <IExtract-cfg.h>

#include <set>

#include <YGP/Trace.h>

#include "SpiritParser.h"

#include "Properties.h"

#include "ParsePDF.h"

namespace {

namespace x3 = boost::spirit::x3;

/// State while parsing a dictionary
struct State {
    unsigned int depth; ///< Nesting level of dictionaries
    std::string key;    ///< Key of the actual top-level entry
    std::map<std::string, std::string>& strings;
    std::map<std::string, std::string>& others;
};
struct StateTag;

State& state(const auto& ctx) { return x3::get<StateTag>(ctx).get(); }

// Actions
auto append = [](auto& ctx) { x3::_val(ctx) += x3::_attr(ctx); };
auto appendOctal = [](auto& ctx) {
    unsigned int ch(0);
    for (char digit : x3::_attr(ctx))
        ch = (ch << 3) + (digit - '0');
    x3::_val(ctx) += static_cast<char>(ch);
};
auto appendEscaped = [](auto& ctx) {
    char ch(x3::_attr(ctx));
    switch (ch) {
    case 'n':
        ch = '\n';
        break;
    case 'r':
        ch = '\r';
        break;
    case 't':
        ch = '\t';
        break;
    case 'b':
        ch = '\b';
        break;
    case 'f':
        ch = '\f';
        break;
    }
    x3::_val(ctx) += ch;
};
auto decodeHex = [](auto& ctx) {
    std::string digits;
    for (char ch : x3::_attr(ctx))
        if (((ch >= '0') && (ch <= '9')) || ((ch >= 'a') && (ch <= 'f')) || ((ch >= 'A') && (ch <= 'F')))
            digits += ch;
    if (digits.size() & 1)
        digits += '0';

    std::string& value(x3::_val(ctx));
    for (std::size_t i(0); i < digits.size(); i += 2) {
        char ch(static_cast<char>(std::stoi(digits.substr(i, 2), nullptr, 16)));
        if (ch) // Drop zeros (of UTF-16 characters)
            value += ch;
    }
    if (!value.compare(0, 2, "\xfe\xff")) // Skip BOM of UTF-16
        value.erase(0, 2);
};
auto enter = [](auto& ctx) { ++state(ctx).depth; };
auto leave = [](auto& ctx) { --state(ctx).depth; };
auto setKey = [](auto& ctx) {
    if (state(ctx).depth == 1)
        state(ctx).key = x3::_attr(ctx);
};
auto storeString = [](auto& ctx) {
    State& st(state(ctx));
    if (st.depth == 1)
        st.strings[st.key] = x3::_attr(ctx);
};
auto storeOther = [](auto& ctx) {
    State& st(state(ctx));
    if (st.depth == 1)
        st.others[st.key] = std::string(x3::_attr(ctx).begin(), x3::_attr(ctx).end());
};

// Lexical elements
auto const white = x3::char_(" \t\r\n\f") | x3::lit('\0');
auto const ws = x3::omit[*(white | x3::lit('%') >> *~x3::char_("\r\n"))];
auto const regular = ~x3::char_(" \t\r\n\f()<>[]{}/%") - x3::lit('\0');
auto const digit = x3::char_('0', '9');
auto const number = -x3::char_("+-") >> (+digit >> -(x3::char_('.') >> *digit) | x3::char_('.') >> +digit);
auto const reference = +digit >> ws >> +digit >> ws >> x3::lit('R');
auto const name = x3::rule<class NameID, std::string>("Name") = x3::lit('/') >> *regular;
auto const keyword = +x3::char_("a-zA-Z");

x3::rule<class LiteralID, std::string> const literal("Literal string");
x3::rule<class NestedID, std::string> const nested("Nested string");
x3::rule<class HexID, std::string> const hex("Hexadecimal string");
x3::rule<class DictionaryID> const dictionary("Dictionary");
x3::rule<class ArrayID> const array("Array");
x3::rule<class ObjectID> const object("Object");

// Content of a literal string: Escape-sequences, balanced parenthesis, other
// characters
auto const content = *(x3::lit('\\') >> (x3::repeat(1, 3)[x3::char_('0', '7')][appendOctal] | x3::lit("\r\n") | x3::lit('\r') |
                                         x3::lit('\n') | x3::char_[appendEscaped]) |
                       nested[append] | (~x3::char_("()\\"))[append]);
auto const literal_def = x3::lit('(') >> content >> x3::lit(')');
auto const nested_def = x3::char_('(')[append] >> content >> x3::char_(')')[append];
auto const hex_def = x3::lit('<') >> !x3::lit('<') >> (*~x3::char_('>'))[decodeHex] >> x3::lit('>');

auto const value = literal[storeString] | hex[storeString] | x3::raw[reference][storeOther] | x3::raw[number][storeOther] |
                   x3::raw[name][storeOther] | object;
auto const dictionary_def = x3::lit("<<")[enter] >> ws >> *(name[setKey] >> ws >> value >> ws) >> x3::lit(">>")[leave];
auto const array_def = x3::lit('[') >> ws >> *(object >> ws) >> x3::lit(']');
auto const object_def = dictionary | array | x3::omit[literal] | x3::omit[hex] | reference | number | x3::omit[name] | keyword;

BOOST_SPIRIT_DEFINE(literal, nested, hex, dictionary, array, object)

} // namespace

//-----------------------------------------------------------------------------
/// Method to parse a PDF document
/// \param stream: Stream to parse
/// \param result: Result where to store found data
/// \throw YGP::ParseError: In case of an invalid document
//-----------------------------------------------------------------------------
void ParsePDF::parse(std::istream& stream, Properties& result) {
    const std::string data(SpiritParser::readStream(stream));
    if (data.compare(0, 5, "%PDF-"))
        throw YGP::ParseError(_("Not a PDF document!"));

    // Offset of the (last) cross reference table
    std::size_t pos(data.rfind("startxref"));
    if (pos == std::string::npos)
        throw YGP::ParseError(_("Document does not contain the offset of the cross reference table"));

    std::size_t offset(0);
    SpiritParser::Iterator act(data.data() + pos);
    SpiritParser::parse(act, data.data() + data.size(),
                        x3::lit("startxref") >> ws >> x3::ulong_[([&offset](auto& ctx) { offset = x3::_attr(ctx); })],
                        _("offset of cross reference table"));

    // Parse the cross reference tables (following the /Prev entries of the
    // trailers) until the offset of the /Info object is known
    std::map<unsigned int, std::size_t> offsets;
    std::set<std::size_t> parsed;
    unsigned int info(0);
    bool infoFound(false);
    while (!(infoFound && (offsets.find(info) != offsets.end()))) {
        if (!parsed.insert(offset).second)
            throw YGP::ParseError(_("Invalid /Prev entry"));

        Dictionary trailer(parseXRefTable(data, offset, offsets));
        if (!infoFound && (trailer.others.find("Info") != trailer.others.end())) {
            info = parseReference(trailer.others["Info"]);
            infoFound = true;
            TRACE5("ParsePDF::parse (std::istream&, Properties&) - Info: " << info);
        }

        if (infoFound && (offsets.find(info) != offsets.end()))
            break;
        if (trailer.others.find("Prev") == trailer.others.end())
            throw YGP::ParseError(_("Document does not contain neither an /Info nor a /Prev entry"));
        offset = std::stoul(trailer.others["Prev"]);
    }

    Dictionary values(parseObject(data, offsets[info], info));
    static const std::pair<const char*, std::string Properties::*> entries[] = {
        {"Title", &Properties::strTitle}, {"Author", &Properties::strAuthor}, {"Subject", &Properties::strComment}};
    for (const auto& entry : entries) {
        auto i(values.strings.find(entry.first));
        if (i != values.strings.end())
            result.*(entry.second) = i->second;
    }
}

//-----------------------------------------------------------------------------
/// Parses a cross reference table and its trailer
/// \param data: Contents of the document
/// \param offset: Offset of the cross reference table
/// \param offsets: In/Out: Offsets of the objects; already known ones are not
/// changed
/// \returns Dictionary: Values of the trailer
/// \throw YGP::ParseError: In case of an invalid table
//-----------------------------------------------------------------------------
ParsePDF::Dictionary ParsePDF::parseXRefTable(const std::string& data, std::size_t offset,
                                              std::map<unsigned int, std::size_t>& offsets) {
    TRACE5("ParsePDF::parseXRefTable (const std::string&, std::size_t, std::map&) - " << offset);
    if (offset >= data.size())
        throw YGP::ParseError(_("Invalid offset of cross reference table"));

    unsigned int actObject(0);
    std::size_t count(0);
    std::size_t objOffset(0);

    auto setStart = [&actObject](auto& ctx) { actObject = x3::_attr(ctx); };
    auto setCount = [&count](auto& ctx) { count = x3::_attr(ctx); };
    auto setOffset = [&objOffset](auto& ctx) { objOffset = x3::_attr(ctx); };
    auto entry = [&](auto& ctx) {
        if (x3::_attr(ctx) == 'n')
            offsets.emplace(actObject, objOffset);
        ++actObject;
    };

    Dictionary trailer;
    State st{0, std::string(), trailer.strings, trailer.others};
    auto table = x3::lit("xref") >> ws >>
                 +(x3::uint_[setStart] >> ws >> x3::uint_[setCount] >> ws >>
                   SpiritParser::times(count)[x3::ulong_[setOffset] >> ws >> +digit >> ws >> x3::char_("nf")[entry] >> ws]) >>
                 x3::lit("trailer") >> ws >> dictionary;

    SpiritParser::Iterator act(data.data() + offset);
    SpiritParser::parse(act, data.data() + data.size(), x3::with<StateTag>(std::ref(st))[table], _("cross reference table"));
    return trailer;
}

//-----------------------------------------------------------------------------
/// Parses an object, which must be a dictionary
/// \param data: Contents of the document
/// \param offset: Offset of the object
/// \param id: Expected number of the object
/// \returns Dictionary: Values of the object
/// \throw YGP::ParseError: In case of an invalid object
//-----------------------------------------------------------------------------
ParsePDF::Dictionary ParsePDF::parseObject(const std::string& data, std::size_t offset, unsigned int id) {
    TRACE5("ParsePDF::parseObject (const std::string&, std::size_t, unsigned int) - " << id << " @ " << offset);
    if (offset >= data.size())
        throw YGP::ParseError(_("Invalid offset of info object"));

    Dictionary values;
    State st{0, std::string(), values.strings, values.others};
    auto checkID = [id](auto& ctx) { x3::_pass(ctx) = (x3::_attr(ctx) == id); };
    auto obj = x3::uint_[checkID] >> ws >> x3::uint_ >> ws >> x3::lit("obj") >> ws >> dictionary;

    SpiritParser::Iterator act(data.data() + offset);
    SpiritParser::parse(act, data.data() + data.size(), x3::with<StateTag>(std::ref(st))[obj], _("info object"));
    return values;
}

//-----------------------------------------------------------------------------
/// Parses an indirect reference to an object
/// \param reference: Reference (in the format <object> <generation> R)
/// \returns unsigned int: Number of the referenced object
/// \throw YGP::ParseError: In case of an invalid reference
//-----------------------------------------------------------------------------
unsigned int ParsePDF::parseReference(const std::string& reference) {
    unsigned int id(0);
    SpiritParser::parse(reference,
                        x3::uint_[([&id](auto& ctx) { id = x3::_attr(ctx); })] >> ws >> x3::uint_ >> ws >> x3::lit('R'),
                        _("reference to /Info object"));
    return id;
}
