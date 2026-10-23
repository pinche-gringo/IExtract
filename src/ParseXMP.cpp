// PROJECT     : Extract
// SUBSYSTEM   : ParseXMP
// REFERENCES  : https://www.adobe.com/devnet/xmp.html
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

#include <YGP/Trace.h>
#include <YGP/Utility.h>

#include "Properties.h"

#include "ParseXMP.h"

// Elements (of the Dublin Core), which might contain a dc:title of their own
// (e.g. the cc:Agent written by Inkscape)
static constexpr std::string_view NESTED[] = {"dc:creator", "dc:contributor", "dc:publisher", "dc:rights"};

//-----------------------------------------------------------------------------
/// Extracts title, creator and description out of the passed XMP data. Only
/// empty properties are set.
/// \param xmp: XMP packet (or other XML-data holding RDF)
/// \param result: In/Out: Found information
//-----------------------------------------------------------------------------
void ParseXMP::parse(std::string_view xmp, Properties& result) {
    TRACE5("ParseXMP::parse (std::string_view, Properties&) - " << xmp.size() << " bytes");

    if (result.strAuthor.empty())
        if (auto creator(getElement(xmp, "dc:creator")); creator)
            result.strAuthor = getList(*creator);

    // Remove elements which might hold their own title, before searching the
    // title and the description
    std::string data(xmp);
    for (auto tag : NESTED)
        while (auto element = getElement(data, tag)) {
            std::size_t start(data.rfind('<', element->data() - data.data()));
            std::size_t end(element->data() - data.data() + element->size());
            end = data.find('>', end);
            data.erase(start, (end == std::string::npos) ? end : (end - start + 1));
        }

    if (result.strTitle.empty())
        if (auto title(getElement(data, "dc:title")); title)
            result.strTitle = getAlternative(*title);
    if (result.strComment.empty())
        if (auto description(getElement(data, "dc:description")); description)
            result.strComment = getAlternative(*description);
}

//-----------------------------------------------------------------------------
/// Returns the content of the first element with the passed name
/// \param xml: XML data to inspect
/// \param tag: Name of the element (including its namespace-prefix)
/// \returns std::optional<std::string_view>: Content of the element (empty
/// for empty elements), if found
//-----------------------------------------------------------------------------
std::optional<std::string_view> ParseXMP::getElement(std::string_view xml, std::string_view tag) {
    std::size_t pos(0);
    while ((pos = xml.find(tag, pos)) != std::string_view::npos) {
        std::size_t start(pos);
        pos += tag.size();
        if ((start == 0) || (xml[start - 1] != '<') || (pos >= xml.size()) || !std::string_view(" \t\r\n/>").contains(xml[pos]))
            continue;

        std::size_t end(xml.find('>', pos));
        if (end == std::string_view::npos)
            break;
        if (xml[end - 1] == '/')
            return xml.substr(end, 0);

        std::string closing("</");
        closing += tag;
        std::size_t close(xml.find(closing, ++end));
        if (close == std::string_view::npos)
            break;
        return xml.substr(end, close - end);
    }
    return std::nullopt;
}

//-----------------------------------------------------------------------------
/// Returns the text of the passed XML data; elements and comments are
/// removed, CDATA sections are kept, entities are decoded and leading and
/// trailing whitespaces are removed
/// \param xml: XML data
/// \returns std::string: Text of the data
//-----------------------------------------------------------------------------
std::string ParseXMP::getText(std::string_view xml) {
    std::string text;
    std::size_t pos(0);
    while (pos < xml.size()) {
        std::size_t start(xml.find('<', pos));
        if (start == std::string_view::npos)
            start = xml.size();

        std::string part(xml.substr(pos, start - pos));
        YGP::convertHTML2UTF8(part);
        text += part;
        if (start == xml.size())
            break;

        if (xml.substr(start, 9) == "<![CDATA[") {
            std::size_t end(xml.find("]]>", start));
            text += xml.substr(start + 9, (end == std::string_view::npos) ? end : (end - start - 9));
            pos = (end == std::string_view::npos) ? xml.size() : (end + 3);
        }
        else if (xml.substr(start, 4) == "<!--") {
            std::size_t end(xml.find("-->", start));
            pos = (end == std::string_view::npos) ? xml.size() : (end + 3);
        }
        else {
            std::size_t end(xml.find('>', start));
            pos = (end == std::string_view::npos) ? xml.size() : (end + 1);
        }
    }

    static constexpr const char* WHITESPACE(" \t\r\n");
    std::size_t start(text.find_first_not_of(WHITESPACE));
    if (start == std::string::npos)
        return "";
    return text.substr(start, text.find_last_not_of(WHITESPACE) - start + 1);
}

//-----------------------------------------------------------------------------
/// Searches an (uncompressed) XMP packet in the passed data
/// \param data: Data to inspect
/// \returns std::string_view: Found packet (empty, if there is none)
//-----------------------------------------------------------------------------
std::string_view ParseXMP::findPacket(std::string_view data) {
    std::size_t start(data.find("<x:xmpmeta"));
    if (start == std::string_view::npos)
        return {};
    std::size_t end(data.find("</x:xmpmeta>", start));
    if (end == std::string_view::npos)
        return {};
    return data.substr(start, end - start + 12);
}

//-----------------------------------------------------------------------------
/// Returns the value of a language alternative (rdf:Alt); the default
/// language ("x-default") is preferred, else the first entry is used. If the
/// value is no alternative, its text is returned.
/// \param value: Content of the property
/// \returns std::string: Text of the value
//-----------------------------------------------------------------------------
std::string ParseXMP::getAlternative(std::string_view value) {
    std::optional<std::string_view> first;
    std::size_t pos(0);
    while (auto entry = getElement(value.substr(pos), "rdf:li")) {
        std::size_t start(entry->data() - value.data());
        if (!first)
            first = entry;
        std::size_t tag(value.rfind("<rdf:li", start));
        if (value.substr(tag, start - tag).contains("x-default"))
            return getText(*entry);
        pos = start + entry->size();
    }
    return getText(first.value_or(value));
}

//-----------------------------------------------------------------------------
/// Returns the values of a list (rdf:Seq or rdf:Bag), separated by
/// commas. If the value is no list, its text is returned.
/// \param value: Content of the property
/// \returns std::string: Text of the values
//-----------------------------------------------------------------------------
std::string ParseXMP::getList(std::string_view value) {
    std::string result;
    std::size_t pos(0);
    bool found(false);
    while (auto entry = getElement(value.substr(pos), "rdf:li")) {
        found = true;
        if (std::string text(getText(*entry)); !text.empty()) {
            if (!result.empty())
                result += ", ";
            result += text;
        }
        pos = entry->data() - value.data() + entry->size();
    }
    return found ? result : getText(value);
}
