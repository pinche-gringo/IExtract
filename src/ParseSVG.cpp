// PROJECT     : Extract
// SUBSYSTEM   : ParseSVG
// REFERENCES  : https://www.w3.org/TR/SVG2/struct.html#DescriptionAndTitleElements
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 23.10.2026
// COPYRIGHT   : Copyright (C) 2026

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

#include <YGP/Exception.h>
#include <YGP/Trace.h>

#include "SpiritParser.h"

#include "ParseXMP.h"
#include "Properties.h"

#include "ParseSVG.h"

//-----------------------------------------------------------------------------
/// Parses the SVG image
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid image
//-----------------------------------------------------------------------------
void ParseSVG::parse(std::istream& stream, Properties& result) {
    result.strComment = result.strTitle = result.strAuthor = "";

    const std::string data(SpiritParser::readStream(stream));
    std::optional<std::string_view> svg(ParseXMP::getElement(data, "svg"));
    if (!svg)
        throw YGP::ParseError(_("Not an SVG image!"));

    // Ignore the metadata, while searching for the title and the description
    std::string image(*svg), rdf;
    if (std::optional<std::string_view> metadata = ParseXMP::getElement(image, "metadata")) {
        rdf = *metadata;
        image.erase(metadata->data() - image.data(), metadata->size());
    }

    if (std::optional<std::string_view> title = ParseXMP::getElement(image, "title"))
        result.strTitle = ParseXMP::getText(*title);
    if (std::optional<std::string_view> desc = ParseXMP::getElement(image, "desc"))
        result.strComment = ParseXMP::getText(*desc);
    if (!rdf.empty())
        ParseXMP::parse(rdf, result);
}
