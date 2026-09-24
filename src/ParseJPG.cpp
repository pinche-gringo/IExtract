//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : ParseJPEG
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 17.10.2002
//COPYRIGHT   : Copyright (C) 2002 - 2008, 2011

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

#include "ParseJPG.h"
#include "Properties.h"


// Markers
static const unsigned char M_TEM  (0x01);
static const unsigned char M_RST0 (0xd0);
static const unsigned char M_RST7 (0xd7);
static const unsigned char M_EOI  (0xd9);
static const unsigned char M_SOS  (0xda);
static const unsigned char M_APP1 (0xe1);
static const unsigned char M_APPD (0xed);
static const unsigned char M_COM  (0xfe);

// Tags in the EXIF information (Windows XP)
static const unsigned int TAG_TITLE    (0x9c9b);
static const unsigned int TAG_COMMENT  (0x9c9c);
static const unsigned int TAG_AUTHOR   (0x9c9d);
static const unsigned int TYPE_BYTE    (1);

// Resource-ID of the IPTC information in Photoshop's image resource blocks
static const unsigned int RES_IPTC     (0x0404);

// Datasets (of record 2) in the IPTC information
static const unsigned int IPTC_TITLE   (105);
static const unsigned int IPTC_AUTHOR  (110);
static const unsigned int IPTC_COMMENT (120);


//-----------------------------------------------------------------------------
/// Parses the JPEG image; the markers up to the image data are inspected
/// \param stream: Stream to parse
/// \param result: Out: Found information
/// \throw YGP::ParseError: In case of an invalid image
//-----------------------------------------------------------------------------
void ParseJPEG::parse (YGP::Xistream& stream, Properties& result) {
   namespace x3 = boost::spirit::x3;
   using SpiritParser::bytes;

   unsigned int marker (0);
   std::size_t length (0);
   bool done (false);

   auto setMarker = [&marker](auto& ctx) { marker = x3::_attr (ctx); };
   // Checks if the marker has a length (and data) following
   auto hasLength = [&marker](auto& ctx) {
      x3::_pass (ctx) = (marker != M_TEM) && ((marker < M_RST0) || (marker > M_RST7)); };
   auto isEnd = [&marker, &done](auto& ctx) {
      x3::_pass (ctx) = done || (marker == M_EOI) || (marker == M_SOS); };
   auto setLength = [&length](auto& ctx) {
      length = x3::_attr (ctx);
      x3::_pass (ctx) = (length >= 2);
      length -= 2; };
   auto segment = [&](auto& ctx) {
      const std::string& data (x3::_attr (ctx));
      TRACE8 ("ParseJPEG::parse (YGP::Xistream&, Properties&) - Marker " << std::hex << marker
              << std::dec << ": " << data.size () << " bytes");
      switch (marker) {
      case M_COM:
         result.strComment = data;
         done = !data.empty ();
         break;

      case M_APP1:
         done = parseEXIF (data, result);
         break;

      case M_APPD:
         parsePhotoshop (data, result);
         break;
      } };

   auto markerID = x3::lit ('\xff') >> *x3::lit ('\xff') >> x3::byte_[setMarker];
   auto markerData = x3::eps[hasLength] >> x3::big_word[setLength] >> bytes (length)[segment];
   auto image = x3::lit ("\xff\xd8")
      >> *(markerID >> !x3::eps[isEnd] >> -markerData);

   SpiritParser::parse (SpiritParser::readStream (stream), image, _("JPEG image"));
}

//-----------------------------------------------------------------------------
/// Parses the EXIF information (of an APP1 marker) for the Windows XP tags
/// for title, comment and author
/// \param data: Data of the APP1 marker
/// \param result: Out: Found information
/// \returns bool: True, if the (first) image file directory contains entries
//-----------------------------------------------------------------------------
bool ParseJPEG::parseEXIF (const std::string& data, Properties& result) {
   namespace x3 = boost::spirit::x3;
   using SpiritParser::skip;

   static const std::string ID_EXIF ("Exif\0\0", 6);
   if (data.compare (0, ID_EXIF.size (), ID_EXIF))
      return false;
   const std::string tiff (data, ID_EXIF.size ());

   // Parses the image file directory with the passed parsers for words and
   // double words (of the byte order of the data)
   auto parseIFD = [&tiff, &result](auto word, auto dword) {
      std::size_t offset (0), count (0);
      unsigned int tag (0), type (0);
      std::size_t cValues (0);

      auto setValue = [](auto& value) { return [&value](auto& ctx) { value = x3::_attr (ctx); }; };
      auto entry = [&](auto& ctx) {
         static const std::pair<unsigned int, std::string Properties::*> tags[] =
            { { TAG_TITLE, &Properties::strTitle }, { TAG_COMMENT, &Properties::strComment },
              { TAG_AUTHOR, &Properties::strAuthor } };

         // The value is stored in the entry, if it fits into the 4 bytes of the offset
         std::size_t posValue ((cValues > 4) ? static_cast<std::size_t> (x3::_attr (ctx))
                               : static_cast<std::size_t> (x3::_where (ctx).begin () - tiff.data ()));
         TRACE8 ("ParseJPEG::parseEXIF (const std::string&, Properties&) - Tag " << std::hex << tag
                 << std::dec << " (" << type << "): " << cValues << " @ " << posValue);
         if (type != TYPE_BYTE)
            return;
         for (const auto& t : tags)
            if ((t.first == tag) && (posValue < tiff.size ()))
               result.*(t.second) = tiff.substr (posValue, cValues);
      };

      SpiritParser::Iterator act (tiff.data ());
      SpiritParser::parse (act, tiff.data () + tiff.size (),
                           x3::repeat (2)[x3::byte_] >> word >> dword[setValue (offset)],
                           _("TIFF header"));
      if (offset >= tiff.size ())
         throw YGP::ParseError (_("Invalid offset of image file directory"));

      act = tiff.data () + offset;
      SpiritParser::parse (act, tiff.data () + tiff.size (),
                           word[setValue (count)]
                           >> SpiritParser::times (count)[word[setValue (tag)] >> word[setValue (type)]
                                                          >> dword[setValue (cValues)] >> dword[entry]],
                           _("image file directory"));
      return count > 0;
   };

   if (!tiff.compare (0, 2, "II"))
      return parseIFD (x3::little_word, x3::little_dword);
   if (!tiff.compare (0, 2, "MM"))
      return parseIFD (x3::big_word, x3::big_dword);
   throw YGP::ParseError (_("Invalid byte order of EXIF information"));
}

//-----------------------------------------------------------------------------
/// Parses the image resource blocks of Photoshop (of an APPD marker) for the
/// IPTC information
/// \param data: Data of the APPD marker
/// \param result: Out: Found information
//-----------------------------------------------------------------------------
void ParseJPEG::parsePhotoshop (const std::string& data, Properties& result) {
   namespace x3 = boost::spirit::x3;
   using SpiritParser::bytes;
   using SpiritParser::skip;

   unsigned int id (0);
   std::size_t length (0), padding (0);

   auto setID = [&id](auto& ctx) { id = x3::_attr (ctx); };
   // Name: Pascal string, padded to an even size (including its length)
   auto setNameLength = [&length, &padding](auto& ctx) {
      length = x3::_attr (ctx);
      padding = !(length & 1); };
   auto setLength = [&length, &padding](auto& ctx) {
      length = x3::_attr (ctx);
      padding = length & 1; };
   auto resource = [&id, &result](auto& ctx) {
      TRACE8 ("ParseJPEG::parsePhotoshop (const std::string&, Properties&) - Resource " << std::hex
              << id << std::dec << ": " << x3::_attr (ctx).size () << " bytes");
      if (id == RES_IPTC)
         parseIPTC (x3::_attr (ctx), result); };

   auto block = x3::lit ("8BIM") >> x3::big_word[setID]
      >> x3::byte_[setNameLength] >> skip (length) >> skip (padding)
      >> x3::big_dword[setLength] >> bytes (length)[resource] >> skip (padding);
   auto resources = x3::lit (std::string ("Photoshop 3.0\0", 14)) >> *block;

   SpiritParser::Iterator act (data.data ());
   x3::parse (act, data.data () + data.size (), resources);
}

//-----------------------------------------------------------------------------
/// Parses the IPTC information for title, author and comment
/// \param data: IPTC information
/// \param result: Out: Found information
//-----------------------------------------------------------------------------
void ParseJPEG::parseIPTC (const std::string& data, Properties& result) {
   namespace x3 = boost::spirit::x3;
   using SpiritParser::bytes;

   unsigned int record (0), dataset (0);
   std::size_t length (0);

   auto setRecord = [&record](auto& ctx) { record = x3::_attr (ctx); };
   auto setDataset = [&dataset](auto& ctx) { dataset = x3::_attr (ctx); };
   auto setLength = [&length](auto& ctx) {             // Extended datasets are not supported
      length = x3::_attr (ctx);
      x3::_pass (ctx) = !(length & 0x8000); };
   auto value = [&](auto& ctx) {
      TRACE8 ("ParseJPEG::parseIPTC (const std::string&, Properties&) - " << record << ':'
              << dataset << " = " << x3::_attr (ctx));
      if (record != 2)
         return;
      switch (dataset) {
      case IPTC_TITLE: result.strTitle = x3::_attr (ctx); break;
      case IPTC_AUTHOR: result.strAuthor = x3::_attr (ctx); break;
      case IPTC_COMMENT: result.strComment = x3::_attr (ctx); break;
      } };

   auto dataSet = x3::lit ('\x1c') >> x3::byte_[setRecord] >> x3::byte_[setDataset]
      >> x3::big_word[setLength] >> bytes (length)[value];

   SpiritParser::Iterator act (data.data ());
   x3::parse (act, data.data () + data.size (), *dataSet);
}
