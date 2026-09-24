#ifndef WRITER_H
#define WRITER_H

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

#include <memory>
#include <string>

#include <YGP/TableWriter.h>

namespace YGP {
struct File;
}
struct Properties;

/**Baseclass of output classes
 */
class Writer : public YGP::TableWriter {
public:
  Writer(const std::string &format, const std::string &New,
         unsigned long age = 0, const char *startRow = "",
         const char *endRow = "", const char *sepColumn = " ",
         const char *startTab = "", const char *endTab = "",
         const char *sepTab = " ", const char *rowStartHdr = nullptr,
         const char *rowEndHdr = nullptr, const char *sepHdrCol = nullptr,
         const char *defColumns = nullptr);
  ~Writer() override = default;

  std::string getSubstitute(char ctrl, bool extend = false) const override;

  void printFile(std::ostream &out, const YGP::File &file,
                 const Properties &prop);
  void printSeparator(std::ostream &out, const YGP::File &file,
                      const std::string &data, const std::string &title) const;

  virtual void printMessage(std::ostream &out, const YGP::File &file,
                            const std::string &msg) const = 0;
  void printHeaderTail(std::ostream &out) const override;

  virtual std::string changeSpecialChars(const std::string &value) const;
  virtual std::string changeSpecialFileChars(const std::string &value) const;

protected:
  bool isNew(const YGP::File &file) const { return file.time() > limit; }

  const std::string strNew;
  long limit;

  static std::string convertToHumanString(unsigned long value);

  const YGP::File *file_{nullptr};
  const Properties *prop_{nullptr};

private:
  Writer(const Writer &) = delete;
  Writer &operator=(const Writer &) = delete;
};

/**Class to write fileinfo in text format
 */
class TextWriter : public Writer {
public:
  TextWriter(const std::string &format, const std::string &strNew,
             unsigned long age = 0);
  ~TextWriter() override = default;

  void printMessage(std::ostream &out, const YGP::File &file,
                    const std::string &msg) const override;

  /// Creates a text writer
  /// \param format: Format how to display entries
  static std::unique_ptr<Writer> create(const std::string &format,
                                        const std::string &strNew,
                                        unsigned long age = 0) {
    return std::make_unique<TextWriter>(format, strNew, age);
  }

private:
  TextWriter(const TextWriter &) = delete;
  TextWriter &operator=(const TextWriter &) = delete;
};

/**Class to write fileinfo in text format
 */
class QuotedTextWriter : public Writer {
public:
  QuotedTextWriter(const std::string &format, const std::string &strNew,
                   unsigned long age = 0);
  ~QuotedTextWriter() override = default;

  void printMessage(std::ostream &out, const YGP::File &file,
                    const std::string &msg) const override;

  std::string changeSpecialChars(const std::string &value) const override;

  /// Creates a text writer
  /// \param format: Format how to display entries
  static std::unique_ptr<Writer> create(const std::string &format,
                                        const std::string &strNew,
                                        unsigned long age = 0) {
    return std::make_unique<QuotedTextWriter>(format, strNew, age);
  }

private:
  QuotedTextWriter(const QuotedTextWriter &) = delete;
  QuotedTextWriter &operator=(const QuotedTextWriter &) = delete;
};

/**Class to write fileinfo in HTML format
 */
class HTMLWriter : public Writer {
public:
  HTMLWriter(const std::string &format, const std::string &strNew,
             unsigned long age = 0);
  ~HTMLWriter() override = default;

  void printMessage(std::ostream &out, const YGP::File &file,
                    const std::string &msg) const override;

  std::string changeSpecialChars(const std::string &value) const override;
  std::string changeSpecialFileChars(const std::string &value) const override;

  /// Creates an HTML writer
  /// \param format: Format how to display entries
  static std::unique_ptr<Writer> create(const std::string &format,
                                        const std::string &strNew,
                                        unsigned long age = 0) {
    return std::make_unique<HTMLWriter>(format, strNew, age);
  }

private:
  HTMLWriter(const HTMLWriter &) = delete;
  HTMLWriter &operator=(const HTMLWriter &) = delete;
};

/**Class to write fileinfo in XML format
 */
class XMLWriter : public HTMLWriter {
public:
  XMLWriter(const std::string &format, const std::string &strNew,
            unsigned long age = 0)
      : HTMLWriter(format, strNew, age) {}
  ~XMLWriter() override = default;

  void printMessage(std::ostream &out, const YGP::File &file,
                    const std::string &msg) const override;

  /// Creates an XML writer
  /// \param format: Format how to display entries
  static std::unique_ptr<Writer> create(const std::string &format,
                                        const std::string &strNew,
                                        unsigned long age = 0) {
    return std::make_unique<XMLWriter>(format, strNew, age);
  }

private:
  XMLWriter(const XMLWriter &) = delete;
  XMLWriter &operator=(const XMLWriter &) = delete;
};

/**Class to write fileinfo in LaTeX format
 */
class LaTeXWriter : public Writer {
public:
  LaTeXWriter(const std::string &format, const std::string &strNew,
              unsigned long age = 0);
  ~LaTeXWriter() override = default;

  void printMessage(std::ostream &out, const YGP::File &file,
                    const std::string &msg) const override;
  void printHeaderLead(std::ostream &out) const override;

  std::string changeSpecialChars(const std::string &value) const override;

  /// Creates a LaTeX writer
  /// \param format: Format how to display entries
  static std::unique_ptr<Writer> create(const std::string &format,
                                        const std::string &strNew,
                                        unsigned long age = 0) {
    return std::make_unique<LaTeXWriter>(format, strNew, age);
  }

private:
  LaTeXWriter(const LaTeXWriter &) = delete;
  LaTeXWriter &operator=(const LaTeXWriter &) = delete;
};

#endif
