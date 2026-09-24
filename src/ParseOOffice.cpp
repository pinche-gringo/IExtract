//$Id$

// PROJECT     : Extract
// SUBSYSTEM   : ParseOOffice
// REFERENCES  :
// TODO        :
// BUGS        :
// REVISION    : $Revision$
// AUTHOR      : Markus Schwab
// CREATED     : 17.06.2003
// COPYRIGHT   : Copyright (C) 2003 - 2008

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

#include "ParseOOffice.h"
#include "Properties.h"

static const char *ID_METAINFO("meta.xml");
static const char *ID_TITLE("dc:title");
static const char *ID_COMMENT("dc:description");
static const char *ID_AUTHOR("dc:creator");
static const char *ID_LOCALHDR("PK\03\04");
static const char *ID_ENDCDR("PK\05\06");
static const char *ID_CFILEHDR("PK\01\02");

static const std::size_t LEN_ENDCDR(22); // Length of end of central dir record

//-----------------------------------------------------------------------------
/// Parses the OpenOffice document (a ZIP archive) for its meta-information
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid document
//-----------------------------------------------------------------------------
void ParseOpenOffice::parse(std::istream &stream, Properties &result) {
  namespace x3 = boost::spirit::x3;
  using SpiritParser::bytes;
  using SpiritParser::skip;

  const std::string data(SpiritParser::readStream(stream));
  const char *const end(data.data() + data.size());
  const char *act(data.data());

  SpiritParser::parse(act, end, x3::lit(ID_LOCALHDR), _("ZIP archive"));
  if (data.size() < LEN_ENDCDR)
    throw YGP::ParseError(_("Invalid end of central directory record"));

  // End of central directory record: Number of entries and offset of the
  // directory
  std::size_t cEntries(0);
  std::size_t offset(0);
  auto setEntries = [&cEntries](auto &ctx) { cEntries = x3::_attr(ctx); };
  auto setOffset = [&offset](auto &ctx) { offset = x3::_attr(ctx); };

  act = end - LEN_ENDCDR;
  SpiritParser::parse(act, end,
                      x3::lit(ID_ENDCDR) >> x3::repeat(6)[x3::byte_] >>
                          x3::little_word[setEntries] >>
                          x3::repeat(4)[x3::byte_] >>
                          x3::little_dword[setOffset],
                      _("end of central directory record"));

  // Central directory: Search the entry for the meta-information
  std::size_t method(0), size(0), lenName(0), lenExtra(0), lenComment(0);
  std::size_t posEntry(0);
  bool found(false);
  auto set = [](std::size_t &value) {
    return [&value](auto &ctx) { value = x3::_attr(ctx); };
  };
  auto entry = x3::lit(ID_CFILEHDR) >> x3::repeat(6)[x3::byte_] >>
               x3::little_word[([&](auto &ctx) {
                 if (!found)
                   method = x3::_attr(ctx);
               })] >>
               x3::repeat(8)[x3::byte_] >> x3::little_dword[([&](auto &ctx) {
                 if (!found)
                   size = x3::_attr(ctx);
               })] >>
               x3::repeat(4)[x3::byte_] >> x3::little_word[set(lenName)] >>
               x3::little_word[set(lenExtra)] >>
               x3::little_word[set(lenComment)] >> x3::repeat(8)[x3::byte_] >>
               x3::little_dword[([&](auto &ctx) {
                 if (!found)
                   posEntry = x3::_attr(ctx);
               })] >>
               bytes(lenName)[([&](auto &ctx) {
                 if (x3::_attr(ctx) == ID_METAINFO)
                   found = true;
               })] >>
               skip(lenExtra) >> skip(lenComment);

  if (offset >= data.size())
    throw YGP::ParseError(_("Invalid offset of central directory"));
  act = data.data() + offset;
  SpiritParser::parse(act, end, SpiritParser::times(cEntries)[entry],
                      _("central directory"));
  if (!found)
    throw YGP::ParseError(_("Document does not contain meta-information"));
  if (method)
    throw YGP::ParseError(_("Compressed meta-information is not supported"));
  TRACE5("ParseOpenOffice::parse (std::istream&, Properties&) - Meta-info at "
         << posEntry << "; " << size << " bytes");

  // Local header of the meta-information, followed by its data
  if (posEntry >= data.size())
    throw YGP::ParseError(_("Invalid offset of meta-information"));
  std::string metaInfo;
  act = data.data() + posEntry;
  SpiritParser::parse(
      act, end,
      x3::lit(ID_LOCALHDR) >> x3::repeat(22)[x3::byte_] >>
          x3::little_word[set(lenName)] >> x3::little_word[set(lenExtra)] >>
          skip(lenName) >> skip(lenExtra) >>
          bytes(size)[([&metaInfo](auto &ctx) { metaInfo = x3::_attr(ctx); })],
      _("meta-information"));
  parseMetaInfo(metaInfo, result);
}

//-----------------------------------------------------------------------------
/// Parses the meta-information (an XML-file) of the document
/// \param metaInfo: Contents of the meta-information
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of invalid meta-information
//-----------------------------------------------------------------------------
void ParseOpenOffice::parseMetaInfo(const std::string &metaInfo,
                                    Properties &result) {
  namespace x3 = boost::spirit::x3;
  using SpiritParser::ws;

  std::string Properties::*entry(nullptr);

  auto tag = [&entry](auto &ctx) {
    const std::string &tag(x3::_attr(ctx));
    TRACE8("ParseOpenOffice::parseMetaInfo (const std::string&, Properties&) - "
           "Tag: "
           << tag);
    entry = ((tag == ID_TITLE)     ? &Properties::strTitle
             : (tag == ID_COMMENT) ? &Properties::strComment
             : (tag == ID_AUTHOR)  ? &Properties::strAuthor
                                   : nullptr);
  };
  auto value = [&entry, &result](auto &ctx) {
    if (entry)
      result.*entry = x3::_attr(ctx);
    entry = nullptr;
  };

  auto declaration =
      x3::lit("<?xml") >> *(x3::char_ - x3::lit("?>")) >> x3::lit("?>") >> ws;
  auto element = x3::lit('<') >> (*~x3::char_('>'))[tag] >> x3::lit('>') >>
                 ws >> (*~x3::char_('<'))[value];
  auto document = ws >> declaration >> *element;

  SpiritParser::parse(metaInfo, document, _("meta-information"));
}
