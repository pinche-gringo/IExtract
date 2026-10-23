// PROJECT     : Extract
// SUBSYSTEM   : ParseEPUB
// REFERENCES  : https://www.w3.org/TR/epub-33/
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
#include "ZipArchive.h"

#include "ParseEPUB.h"

//-----------------------------------------------------------------------------
/// Parses the EPUB document (a ZIP archive)
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid document
//-----------------------------------------------------------------------------
void ParseEPUB::parse(std::istream& stream, Properties& result) {
    result.strComment = result.strTitle = result.strAuthor = "";

    const std::string data(SpiritParser::readStream(stream));
    std::optional<std::string> container(ZipArchive::getFile(data, "META-INF/container.xml"));
    if (!container)
        throw YGP::ParseError(_("Not an EPUB document!"));

    // Location of the package document: <rootfile full-path="..." .../>
    std::size_t pos(container->find("<rootfile"));
    if (pos != std::string::npos)
        pos = container->find("full-path=", pos);
    if ((pos == std::string::npos) || ((pos + 11) > container->size()))
        throw YGP::ParseError(_("EPUB document does not contain the location of the package document"));
    pos += 10;
    std::size_t end(container->find((*container)[pos], pos + 1));
    if (end == std::string::npos)
        throw YGP::ParseError(_("EPUB document does not contain the location of the package document"));
    std::string file(ParseXMP::getText(container->substr(pos + 1, end - pos - 1)));
    TRACE5("ParseEPUB::parse (std::istream&, Properties&) - Package document: " << file);

    std::optional<std::string> package(ZipArchive::getFile(data, file));
    if (!package)
        throw YGP::ParseError(_("EPUB document does not contain the package document"));

    // The metadata are stored like XMP data (with the Dublin Core elements)
    std::optional<std::string_view> metadata(ParseXMP::getElement(*package, "metadata"));
    if (!metadata)
        metadata = ParseXMP::getElement(*package, "opf:metadata");
    ParseXMP::parse(metadata.value_or(*package), result);
}
