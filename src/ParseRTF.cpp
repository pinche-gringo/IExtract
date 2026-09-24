// PROJECT     : Extract
// SUBSYSTEM   : ParseRTF
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 26.12.2003
// COPYRIGHT   : Copyright (C) 2003, 2004, 2006, 2008, 2026

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

#include <YGP/Trace.h>

#include "SpiritParser.h"

#include "ParseRTF.h"
#include "Properties.h"

namespace {

namespace x3 = boost::spirit::x3;
using SpiritParser::ws;

/// State while parsing a document
struct State {
    Properties& prop;
    std::string Properties::* entry; ///< Entry to store the next value
    bool done;                       ///< Info block completely parsed
};
struct StateTag;

State& state(const auto& ctx) { return x3::get<StateTag>(ctx).get(); }

// Actions
auto notDone = [](auto& ctx) { x3::_pass(ctx) = !state(ctx).done; };
auto isDone = [](auto& ctx) { x3::_pass(ctx) = state(ctx).done; };
auto finish = [](auto& ctx) { state(ctx).done = true; };
auto title = [](auto& ctx) { state(ctx).entry = &Properties::strTitle; };
auto author = [](auto& ctx) { state(ctx).entry = &Properties::strAuthor; };
auto comment = [](auto& ctx) { state(ctx).entry = &Properties::strComment; };
auto value = [](auto& ctx) {
    State& st(state(ctx));
    TRACE8("ParseRTF::parse (std::istream&, Properties&) - Value: " << x3::_attr(ctx));
    if (st.entry)
        st.prop.*(st.entry) = x3::_attr(ctx);
    st.entry = nullptr;
};

// Value of a command (escaped characters are taken literally)
auto const text = x3::rule<class TextID, std::string>("Value of entry") = *(x3::lit('\\') >> x3::char_ | ~x3::char_("}>")) >> ws;
// Text following a command (its first character is ignored)
auto const cmdValue = ~x3::char_("\\{}") >> ws >> text;
auto const otherCmd = x3::rule<class CommandID>("Command") = x3::lit('\\') >> +~x3::char_(" \t\n\r\\{}") >> ws >> -cmdValue;

auto const entry = x3::lit('{') >> ws >>
                   (x3::lit("\\author")[author] | x3::lit("\\title")[title] | x3::lit("\\doccomm")[comment] | otherCmd) >> ws >>
                   text[value] >> x3::lit('}') >> ws;
auto const info = x3::lit("\\info") >> ws >> +entry >> x3::lit('}')[finish] >> ws;

x3::rule<class CommandsID> const commands("RTF commands");
x3::rule<class BlockID> const block("RTF block");

auto const commands_def = *(x3::eps[notDone] >> (info | block | otherCmd | (+~x3::char_("\\{}") >> ws)));
auto const block_def =
    x3::lit('{') >> ws >> commands >> (x3::eps[isDone] | x3::lit('}') >> ws >> -(x3::eps[notDone] >> cmdValue));

BOOST_SPIRIT_DEFINE(commands, block)

auto const document = ws >> x3::lit("{\\rtf1") >> ws >> commands;

} // namespace

//-----------------------------------------------------------------------------
/// Parses the RTF document
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid document
//-----------------------------------------------------------------------------
void ParseRTF::parse(std::istream& stream, Properties& result) {
    State st{result, nullptr, false};
    SpiritParser::parse(SpiritParser::readStream(stream), x3::with<StateTag>(std::ref(st))[document], _("RTF document"));
}
