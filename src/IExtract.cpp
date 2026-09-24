// PROJECT     : Extract
// SUBSYSTEM   : Extract
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 10.08.2002
// COPYRIGHT   : Copyright (C) 2002 - 2009, 2011, 2024, 2026

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

#ifdef _MSC_VER
#    pragma warning(disable : 4786) // disable warning about truncating debug info
#endif

#include <IExtract-cfg.h>
#include <ygp-cfg.h>

#include <cctype>
#include <cstdio>
#include <cstdlib>

#include <array>
#include <charconv>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <boost/tokenizer.hpp>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#ifdef ENABLE_PLUGINS
#    include <type_traits>
#    include <vector>

#    include <boost/dll/shared_library.hpp>

static constexpr const char* PLUGIN_PROCESS("processFile");
static constexpr const char* PLUGIN_CHECKTYPE("getFileType");
#endif

#ifdef ENABLE_THREADS
#    include <condition_variable>
#    include <queue>
#    include <thread>
#    include <vector>
#endif

#include <YGP/DirSrch.h>
#include <YGP/FileTypeChk.h>
#include <YGP/INIFile.h>
#include <YGP/IVIOAppl.h>
#include <YGP/Path.h>
#include <YGP/SortDirSrch.h>
#include <YGP/XDirSrch.h>

#include "Options.h"
#include "Properties.h"
#include "Writer.h"

#ifdef SUPPORT_MP3
#    include "ParseMP3.h"
#endif
#ifdef SUPPORT_OGG
#    include "ParseOGG.h"
#endif
#ifdef SUPPORT_PDF
#    include "ParsePDF.h"
#endif
#ifdef SUPPORT_JPEG
#    include "ParseJPG.h"
#endif
#ifdef SUPPORT_PNG
#    include "ParsePNG.h"
#endif
#ifdef SUPPORT_GIF
#    include "ParseGIF.h"
#endif
#ifdef SUPPORT_RTF
#    include "ParseRTF.h"
#endif
#ifdef SUPPORT_HTML
#    include "ParseHTML.h"
#endif
#ifdef SUPPORT_MSOFFICE
#    include "ParseMSOffice.h"
#    ifdef SUPPORT_OOXML
#        include "ParseOOXML.h"
#    endif
#endif
#ifdef SUPPORT_ABIWORD
#    include "ParseAbiword.h"
#endif
#ifdef SUPPORT_OO
#    include "ParseOOffice.h"
#    include "ParseSOffice.h"
#endif

static constexpr const char* DEFAULT_FORMAT = "%n|-|%t|%a|%c|%d";
static constexpr const char* DEFAULT_XML_FORMAT = "<tr><td>%n</td><td>%t</td>"
                                                  "<td>%a</td><td>%c</td><td>%d</td></tr>";

// Class to run Extract-Application
class Application : public YGP::IVIOApplication {
  public:
    Application(const int argc, const char* argv[]);
    ~Application() override = default;

  protected:
    void readINIFile(const char* pFile) override;
    bool handleOption(const char option) override;

    // Program-handling
    bool shallShowInfo() const override { return false; }
    int perform(int argc, const char* argv[]) override;
    const char* name() const override { return PACKAGE; }
    const char* description() const override {
        static const std::string version([] {
            std::string version(PACKAGE " V" VERSION " - ");
            version += (_("Compiled on %1 at %2\n\nCopyright (C) 2002 - 2009 Markus "
                          "Schwab; email: g17m0@users.sourceforge.net\nDistributed "
                          "under the terms of the GNU General Public License"));
            version.replace(version.find("%1"), 2, __DATE__);
            version.replace(version.find("%2"), 2, __TIME__);
            return version;
        }());
        return version.c_str();
    }

    // Help-handling
    void showHelp() const override;

  private:
    // Prohobited manager functions
    Application() = delete;
    Application(const Application&) = delete;
    const Application& operator=(const Application&) = delete;

    using HANDLER = void (*)(std::istream& hFile, Properties& result);
    HANDLER getFileTypeHandler(const char* file) const;

    std::map<unsigned int, HANDLER> handlers;

    bool setMode(const std::string& mode);

    static void convertFromWideChar(Properties& prop);

    void handleFiles(const char* pFile);
    void processFile(const YGP::File& file, HANDLER fnc) const;
#ifdef ENABLE_THREADS
    void processThread(std::stop_token stop);
#endif

#ifdef SUPPORT_MP3
    static void processMP3(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_OGG
    static void processOGG(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_PDF
    static void processPDF(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_JPEG
    static void processJPG(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_PNG
    static void processPNG(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_GIF
    static void processGIF(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_HTML
    static void processHTML(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_RTF
    static void processRTF(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_MSOFFICE
    static void processMSOffice(std::istream& hFile, Properties& result);
#    ifdef SUPPORT_OOXML
    static void processOOXML(std::istream& hFile, Properties& result);
#    endif
#endif
#ifdef SUPPORT_OO
    static void processOpenOffice(std::istream& hFile, Properties& result);
    static void processStarOffice(std::istream& hFile, Properties& result);
#endif
#ifdef SUPPORT_ABIWORD
    static void processAbiword(std::istream& hFile, Properties& result);
#endif

    void showSupportedTypes() const;

    enum { RECURSIVE = 0x1, SHOW_ALL = 0x2, SHOW_ERRORS = 0x4, TRUNC_EXTENSION = 0x8 };

    unsigned int options{0};

    unsigned int chgFlag{0};
    Options iniOpts;

    std::unique_ptr<Writer> writer;
    std::string filelist;
    std::string append, prepend;

    enum class OutputStyle { TEXT, QUOTED, HTML, LATEX, XML };
    OutputStyle outputStyle{OutputStyle::TEXT};
    static std::optional<OutputStyle> getOutputStyle(std::string_view style);

    std::unique_ptr<YGP::FileTypeChecker> ftchk;

    static const longOptions lo[];

    mutable std::mutex mxOutput;

#ifdef ENABLE_THREADS
    /// File to process (with its handler)
    struct FileFunction : public YGP::File {
        FileFunction(const YGP::File& file, HANDLER fnc) : YGP::File(file), fnc(fnc) {}

        HANDLER fnc;
    };

    static constexpr std::size_t MAX_PENDING_FILES = 100;

    unsigned int cThreads{1};
    std::vector<std::jthread> threads;
    std::mutex mxListFiles;
    std::condition_variable_any cvListFiles; ///< Signals changes of listFiles or cActive
    std::queue<FileFunction> listFiles;      ///< Files to process
    unsigned int cActive{0};                 ///< Number of files being processed
#endif

#ifdef ENABLE_PLUGINS
    std::map<std::string, std::string> dynHandlers;
    std::vector<boost::dll::shared_library> modules;

    void setPlugins();
#endif
};

const YGP::IVIOApplication::longOptions Application::lo[] = {
    {IVIOAPPL_HELP_OPTION}, {"recursive", 'r'}, {"format", 'F'},      {"title", 'T'},
#ifdef ENABLE_THREADS
    {"threads", 't'},
#endif
    {"include", 'i'},       {"exclude", 'x'},   {"show-errors", 'e'}, {"separate", 's'},   {"add-unknown", 'u'}, {"new", 'n'},
    {"ini-file", 'f'},      {"version", 'V'},   {"output", 'o'},      {"append", 'a'},     {"app-file", 'A'},    {"prepend", 'p'},
    {"pre-file", 'P'},      {"sort", 'S'},      {"mode", 'M'},        {"list-types", 'l'}, {nullptr, '\0'}};

using tokenizer = boost::tokenizer<boost::char_separator<char>>;

//----------------------------------------------------------------------------
/// Constructor
/// \Param argc Number of parameters to the program
/// \param argv Array holding (pointer to) arguments
//----------------------------------------------------------------------------
Application::Application(const int argc, const char* argv[]) : YGP::IVIOApplication(argc, argv, lo) {
    iniOpts.format = DEFAULT_FORMAT;
    iniOpts.ageOfNewFiles = 30 * 24 * 60 * 60;

#ifdef SUPPORT_ABIWORD
    handlers[YGP::FileTypeChecker::ABIWORD] = &Application::processAbiword;
#endif
#ifdef SUPPORT_GIF
    handlers[YGP::FileTypeChecker::GIF] = &Application::processGIF;
#endif
#ifdef SUPPORT_HTML
    handlers[YGP::FileTypeChecker::HTML] = &Application::processHTML;
#endif
#ifdef SUPPORT_JPEG
    handlers[YGP::FileTypeChecker::JPEG] = &Application::processJPG;
#endif
#ifdef SUPPORT_MP3
    handlers[YGP::FileTypeChecker::MP3] = &Application::processMP3;
#endif
#ifdef SUPPORT_MSOFFICE
    handlers[YGP::FileTypeChecker::MSOFFICE] = &Application::processMSOffice;
#    ifdef SUPPORT_OOXML
    handlers[YGP::FileTypeChecker::OOXML] = &Application::processOOXML;
#    endif
#endif
#ifdef SUPPORT_OGG
    handlers[YGP::FileTypeChecker::OGG] = &Application::processOGG;
#endif
#ifdef SUPPORT_OO
    handlers[YGP::FileTypeChecker::OPENOFFICE] = &Application::processOpenOffice;
    handlers[YGP::FileTypeChecker::STAROFFICE] = &Application::processStarOffice;
#endif
#ifdef SUPPORT_PDF
    handlers[YGP::FileTypeChecker::PDF] = &Application::processPDF;
#endif
#ifdef SUPPORT_PNG
    handlers[YGP::FileTypeChecker::PNG] = &Application::processPNG;
#endif
#ifdef SUPPORT_RTF
    handlers[YGP::FileTypeChecker::RTF] = &Application::processRTF;
#endif
}

//-----------------------------------------------------------------------------
/// Displays the help
//-----------------------------------------------------------------------------
void Application::showHelp() const {
    std::cout << _("Extracts descriptions/comments out of files (depending on the "
                   "file-type)\n\nUsage:")
              << " " PACKAGE " " << _("[OPTIONS] <File(s)>") << "\n\n  -r, --recursive ....... "
              << _("Recurse into subdirectories") << "\n  -o, --output=STYLE .... "
              << _("Sets the output-style (default: text;\n\t\t\t  other Values are: "
                   "quoted, HTML, XML or LaTeX)")
              << "\n  -F, --format=FORMAT ... " << _("Format of output; default: ") << DEFAULT_FORMAT
              << "\n  -T, --title=TITLE ..... " << _("Title of output") << "\n  -s, --separate=TEXT ... "
              << _("Separate subdirectories with TEXT (default: empty);\n\t\t\t  "
                   "implies recursion into subdirectories (--recursive)")
              << "\n  -p, --prepend=HEAD .... " << _("Text to print before any output") << "\n  -P, --pre-file=FILE ... "
              << _("File to print before any output") << "\n  -a, --append=FOOT ..... "
              << _("Text to print at the end of the output") << "\n  -A, --app-file=FILE ... "
              << _("File to print at the end of the output") << "\n  -e, --show-errors ..... "
              << _("Puts error messages (additionally) into the output") << "\n  -u, --add-unknown ..... "
              << _("Show all files (including unknown types) in output")
#ifdef ENABLE_THREADS
              << "\n  -t, --threads=NR ...... " << _("Number of threads for examining files (default: 1)")
#endif
              << "\n  -n, --new=DAYS:TEXT ... " << _("Show TEXT for files younger than DAYS days (def: 30)")
              << "\n  -i, --include=LIST .... " << _("Files to inspect") << "\n  -x, --exclude=LIST .... "
              << _("Files not to inspect") << "\n  -f, --ini-file=FILE ... " << _("Read further options from specified file")
              << "\n  -S, --sort ..... ...... " << _("Sort found files alphabetically") << "\n  -l, --list-types ...... "
              << _("List supported file types and exit") << "\n  -M, --mode=[MODUS] .... "
              << _("Modus operandi to determine the file-type:\n"
                   "\t\t\t  Ext: From (last) extension (Default)\n"
                   "\t\t\t  EXT: From (last) extension (ignoring case)\n"
                   "\t\t\t  AllExt: From any extension (if unknown)\n"
                   "\t\t\t  AllEXT: From any extension (ignoring case)\n"
                   "\t\t\t  Content: From content of the file")
              << "\n  -V, --version ......... " << _("Output version information and exit") << "\n  -h, -?, --help ........ "
              << _("Displays this help and exit\n")
              << _("  File(s) ... Files to analyse (the last part can contain "
                   "wildcards)\n\n")
              << _("DAYS (in option -n) may be omited or may have an multiplier "
                   "suffix: m for 30.\n\n")
              << _("LIST is a list of files; seperated with the path-separator of the "
                   "operating\n")
              << _("     system (':' for UNICES, ';' for Windows). E.g. "
                   "*.html:*.doc\n\n")
              << _("FORMAT specifies how to print the entries;\n") << _("       The pipe symbol (|) separates columns\n")
              << _("       %a is substituted with the author\n") << _("       %c is substituted with the comment\n")
              << _("       %d is substituted with the modification time of the file\n")
              << _("       %D is substituted with the modification time of the file "
                   "(day only)\n")
              << _("       %e is substituted with the extension of the file\n")
              << _("       %E is substituted with the name of the file without "
                   "extension\n")
              << _("       %n is substituted with the name of the file\n")
              << _("       %N is substituted with path and name of the file\n")
              << _("       %p is substituted with the path of the file\n")
              << _("       %P is substituted with the path of the file in UNIX style "
                   "(with /)\n")
              << _("       %s is substituted with the size of the file\n")
              << _("       %S is substituted with the size of the file (human "
                   "readable)\n")
              << _("       %t is substituted with the title\n")
              << _("       %U is substituted with path and name of the file in UNIX "
                   "style (with /)\n")
              << _("       %(LETTERS) is substituted with first of the above "
                   "substitutions\n"
                   "          producing a non-empty string (e.g. %(tn) is the titel if "
                   "not \n"
                   "          empty or else the filename.)\n")
              /* xgettext:no-c-format */
              << _("       %*LETTER changes the substitution slightly. For file names "
                   "it causes a\n"
                   "          conversion of special characters; for the others it "
                   "suppresses them\n\n")
              /* xgettext:no-c-format */
              << _("       In every other constellation the '%' is removed!\n\n")
              << _("TITLE specifies the headers for the output; separated with (|); "
                   "columns must\n")
              << _("      contain at least one character\n\n")
              << _("TEXT specifies the text to separate subdirectories; with the "
                   "following\n")
              << _("     conversion strings:\n")
              << _("       %e prints the end-of-output for the specified output "
                   "style\n")
              << _("       %n is substituted with the name of the directory\n")
              << _("       %N is substituted with the full path of the directory\n")
              << _("       %p is substituted with the path to the directory\n")
              << _("       %P is substituted with the path to the directory in UNIX "
                   "style (with /)\n")
              << _("       %s prints the start-of-output for the specified output "
                   "style\n")
              << _("       %U is substituted with the full path of the dir in UNIX "
                   "style (with /)\n\n")
              << _("     As with FORMAT, the pipe symbol (|) separates columns.\n\n")
              << _("The format of the INI file is like this (entries can be "
                   "missing):\n\n")
              << "   [Output]\n"
                 "   Format=<a href=\")%N\" title=\"%c\">%n</a>|%t|%a|%D\n"
                 "   Title=File|Title|Author|Date\n"
                 "   TextForNewFiles=<img src=\"../images/new.gif\">\n"
                 "   MaxAgeForNewFiles=15\n"
                 "   DirSeparatorText=%eListing of %n%s\n"
                 "   Style=HTML\n"
                 "   SortFiles=1\n\n"
                 "   [FileType]\n"
                 "   Mode=Content\n\n"
#ifdef ENABLE_PLUGINS
                 "   [Handler]\n"
                 "   <Extension1>=<Library1>\n"
                 "   <ExtensionN>=<LibraryN>\n\n"
#endif
        ;
    showSupportedTypes();
}

//-----------------------------------------------------------------------------
/// Checks the validity of the passed option
/// \param option Actual option
/// \returns \c bool Status; false: Invalid option/option-value Require :
///     option not '\0'
//-----------------------------------------------------------------------------
bool Application::handleOption(const char option) {
    Check3(option != '\0');

    switch (option) {
    case 's': {
        const char* pSep = getOptionValue();
        if (pSep)
            iniOpts.separate = pSep;
        else {
            std::string error(_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
            error.replace(error.find("%1"), 2, 1, option);
            std::cerr << PACKAGE << error;
            break;
        }
    }
        // Don't add a break in OK case, as -s implies -r!
        [[fallthrough]];

    case 'r':
        options |= RECURSIVE;
        break;

    case 'o': {
        const char* pType = getOptionValue();
        std::optional<OutputStyle> style(pType ? getOutputStyle(pType) : std::nullopt);
        outputStyle = style.value_or(OutputStyle::TEXT);
        if (!style) {
            std::string error(_("-warning: Style of output `%1' is not valid! Using text\n"));
            error.replace(error.find("%1"), 2, pType ? pType : "");
            std::cerr << PACKAGE << error;
        }
        break;
    }

#ifdef ENABLE_THREADS
    case 't': {
        const char* pThreads(getOptionValue());
        const std::string_view threads(pThreads ? pThreads : "");
        unsigned int count(0);
        auto [end, rc](std::from_chars(threads.data(), threads.data() + threads.size(), count));
        if ((rc != std::errc()) || (end != (threads.data() + threads.size())) || !count)
            std::cerr << PACKAGE << _("-warning: Invalid number of threads!\n");
        else
            cThreads = count;
        break;
    }
#endif

    case 'e':
        options |= SHOW_ERRORS;
        break;

    case 'F': {
        const char* pFormat = getOptionValue();
        if (pFormat) {
            iniOpts.format = pFormat;
            chgFlag |= 0x1;
        }
        else {
            std::string error(_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
            error.replace(error.find("%1"), 2, 1, option);
            std::cerr << PACKAGE << error;
        }
        break;
    }

    case 'T': {
        const char* pTitle = getOptionValue();
        if (pTitle)
            iniOpts.title = pTitle;
        else {
            std::string error(_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
            error.replace(error.find("%1"), 2, 1, option);
            std::cerr << PACKAGE << error;
        }
        break;
    }

    case 'n': {
        const char* pNew = getOptionValue();
        if (!pNew) {
            std::string error(_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
            error.replace(error.find("%1"), 2, 1, option);
            std::cerr << PACKAGE << error;
            break;
        }

        // Format: [DAYS[m]]:TEXT
        const std::string_view value(pNew);
        unsigned int time(0);
        const char* pos(std::from_chars(value.data(), value.data() + value.size(), time).ptr);
        if ((pos != (value.data() + value.size())) && (*pos == 'm')) {
            ++pos;
            time *= 30;
        }
        if ((pos == (value.data() + value.size())) || (*pos != ':')) {
            std::string error(_("-warning: Argument for new files `%1' is not valid! "
                                "Ignoring option `n'\n"));
            error.replace(error.find("%1"), 2, pNew);
            std::cerr << PACKAGE << error;
        }
        else {
            if (time)
                iniOpts.ageOfNewFiles = time * 24 * 60 * 60;
            iniOpts.newText = pos + 1;
        }
        break;
    }

    case 'x':
    case 'i': {
        const char* files = getOptionValue();
        if (files) {
            std::string lFiles(files);
            tokenizer list(lFiles, boost::char_separator<char>(YGP::Path::SEPARATOR_STR));

            for (const auto& file : list) {
                filelist += option;
                filelist += file;
                filelist += YGP::Path::SEPARATOR;
            }
        }
        else {
            std::string error(_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
            error.replace(error.find("%1"), 2, 1, option);
            std::cerr << PACKAGE << error;
        }
        break;
    }

    case 'u':
        options |= SHOW_ALL;
        break;

    case 'f': {
        const char* pFile = getOptionValue();
        if (pFile)
            readINIFile(pFile);
        else {
            std::string error(_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
            error.replace(error.find("%1"), 2, 1, option);
            std::cerr << PACKAGE << error;
        }
        break;
    }

    case 'p':
    case 'a': {
        const char* text = getOptionValue();
        if (text)
            ((option == 'p') ? prepend : append) += text;
        else {
            std::string error(_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
            error.replace(error.find("%1"), 2, 1, option);
            std::cerr << PACKAGE << error;
        }
        break;
    }

    case 'P':
    case 'A': {
        const char* file = getOptionValue();
        if (file) {
            std::ifstream input(file, std::ios::in | std::ios::binary);
            if (!input) {
                std::string error(_("-error: `%1' is not a (readable) file! Ignoring option `%2'!\n"));
                error.replace(error.find("%1"), 2, file);
                error.replace(error.find("%2"), 2, 1, option);
                std::cerr << PACKAGE << error;
            }
            else {
                std::string& target = (option == 'P') ? prepend : append;

                std::array<char, 512> buffer;

                // Read as long as there is data/or an error occurs
                while (input.read(buffer.data(), buffer.size()), input.gcount())
                    target.append(buffer.data(), input.gcount());
            }
        }
        else {
            std::string error(_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
            error.replace(error.find("%1"), 2, 1, option);
            std::cerr << PACKAGE << error;
        }
        break;
    }

    case 'S':
        iniOpts.sort = true;
        break;

    case 'M':
        if (checkOptionValue()) {
            const char* mode(getOptionValue());
            if (setMode(mode)) {
                std::string error(_("-warning: Invalid mode `%1'! Ignoring option 'M'!\n"));
                error.replace(error.find("%1"), 2, mode);
                std::cerr << PACKAGE << error;
            }
        }
        break;

    case 'l':
        showSupportedTypes();
        std::exit(0);

    case 'V':
        std::cout << description() << '\n';
        std::exit(0);

    default: {
        std::string error(_("-warning: Ignoring invalid option `%1'\n"));
        error.replace(error.find("%1"), 2, 1, option);
        std::cerr << PACKAGE << error;
    }
    }
    return true;
}

//-----------------------------------------------------------------------------
/// Sets the mode how to determine the file-type according to the passed value
/// \param mode How to determine the file-type
///             - Ext: By (last) extension
///             - AllExt: By any extension (starting from the last)
///             - EXT: By (last) extension (ignoring case)
///             - AllEXT: By any extension (ignoring case)
///             - Content: By the content of the file
/// \returns bool True, if the passed mode is invalid
//-----------------------------------------------------------------------------
bool Application::setMode(const std::string& mode) {
    if (mode == "Ext") {
        ftchk = std::make_unique<YGP::FileTypeCheckerByExtension>();
        options &= ~TRUNC_EXTENSION;
    }
    else if (mode == "AllExt") {
        ftchk = std::make_unique<YGP::FileTypeCheckerByExtension>();
        options |= TRUNC_EXTENSION;
    }
    else if (mode == "EXT") {
        ftchk = std::make_unique<YGP::FileTypeCheckerByCaseExt>();
        options &= ~TRUNC_EXTENSION;
    }
    else if (mode == "AllEXT") {
        ftchk = std::make_unique<YGP::FileTypeCheckerByCaseExt>();
        options |= TRUNC_EXTENSION;
    }
    else if (mode == "Content") {
        ftchk = std::make_unique<YGP::FileTypeCheckerByContent>();
        options &= ~TRUNC_EXTENSION;
    }
    else
        return true;
    return false;
}

//-----------------------------------------------------------------------------
/// Returns the output style for the passed name
/// \param style Name of the style
/// \returns std::optional<OutputStyle> The style, if the passed name is valid
//-----------------------------------------------------------------------------
std::optional<Application::OutputStyle> Application::getOutputStyle(std::string_view style) {
    static constexpr std::pair<std::string_view, OutputStyle> styles[] = {{"text", OutputStyle::TEXT},
                                                                          {"quoted", OutputStyle::QUOTED},
                                                                          {"HTML", OutputStyle::HTML},
                                                                          {"LaTeX", OutputStyle::LATEX},
                                                                          {"XML", OutputStyle::XML}};
    for (const auto& [name, value] : styles)
        if (name == style)
            return value;
    return std::nullopt;
}

//-----------------------------------------------------------------------------
/// Performs the job of the applications
/// \param argc Number of parameters (without options)
/// \param argv Array with pointer to arguments
/// \returns \c int Status
//-----------------------------------------------------------------------------
int Application::perform(int argc, const char* argv[]) {
    if (!argc) { // Check if enough arguments are passed
        showHelp();
        return -1;
    }

    if ((outputStyle == OutputStyle::XML) && !(chgFlag & 1))
        iniOpts.format = DEFAULT_XML_FORMAT;
    Check3(iniOpts.format.size());

    if (!ftchk)
        ftchk = std::make_unique<YGP::FileTypeCheckerByExtension>();
    Check3(ftchk);

    using CreateWriter = std::unique_ptr<Writer> (*)(const std::string&, const std::string&, unsigned long);

    // This declaration must be in the same order as the OutputStyle enum
    static constexpr CreateWriter fnc[] = {&TextWriter::create, &QuotedTextWriter::create, &HTMLWriter::create,
                                           &LaTeXWriter::create, &XMLWriter::create};
    static_assert(std::size(fnc) == (std::to_underlying(OutputStyle::XML) + 1));

    writer = fnc[std::to_underlying(outputStyle)](iniOpts.format, iniOpts.newText, iniOpts.ageOfNewFiles);
    Check3(writer);

    if (prepend.size())
        std::cout << prepend;

    writer->printStart(std::cout, iniOpts.title);

#ifdef ENABLE_PLUGINS
    setPlugins();
#endif

    std::string file;
    for (int j(0); j < argc; ++j) {
        file = argv[j];
        if (YGP::DirectorySearch::isValid(argv[j])) {
            if (file.back() != YGP::File::DIRSEPARATOR)
                file += YGP::File::DIRSEPARATOR;
            file += "*";
        }
        handleFiles(file.c_str());
    }

#ifdef ENABLE_THREADS
    // Terminate the threads (all files have been processed)
    threads.clear();
#endif

    writer->printEnd(std::cout);
    writer.reset();

    if (append.size())
        std::cout << append;
    return 0;
}

//-----------------------------------------------------------------------------
/// Expands the filespecification and processes every file
/// \param pFile Filespecification; may contain wildcards
/// \pre pFile not NULL
//-----------------------------------------------------------------------------
void Application::handleFiles(const char* pFile) {
    Check3(pFile);
    TRACE5("Application::handleFiles (const char*) - " << pFile);

    std::unique_ptr<YGP::ExtDirectorySearch> search;
    if (iniOpts.sort)
        search = std::make_unique<YGP::SortedDirSearch<YGP::ExtDirectorySearch>>(pFile);
    else
        search = std::make_unique<YGP::ExtDirectorySearch>(pFile);
    YGP::ExtDirectorySearch& ds(*search);

    tokenizer list(filelist, boost::char_separator<char>(YGP::Path::SEPARATOR_STR));

    for (const auto& entry : list)
        (entry.at(0) == 'i') ? ds.addFilesToInclude(entry.substr(1)) : ds.addFilesToExclude(entry.substr(1));

    const YGP::File* file = ds.find(YGP::IDirectorySearch::FILE_NORMAL);
    std::string name;
    while (file) {
        // Determine file-type from extension (or second-to-last extension)
        name = file->path();
        name += file->name();
        HANDLER fnc(getFileTypeHandler(name.c_str()));
        // Unknown type; try second to-last extension (if option passed)
        if ((options & TRUNC_EXTENSION) && !fnc) {
            do {
                std::size_t pos(name.rfind('.'));
                if (pos == std::string::npos)
                    break;
                else {
                    name.erase(pos);
                    fnc = getFileTypeHandler(name.c_str());
                }
            }
            while (!fnc);
        }
        if (fnc) {
#ifdef ENABLE_THREADS
            {
                std::unique_lock lock(mxListFiles);
                cvListFiles.wait(lock, [this] { return listFiles.size() < MAX_PENDING_FILES; });
                listFiles.emplace(*file, fnc);
            }
            cvListFiles.notify_all();

            if (threads.size() < cThreads)
                threads.emplace_back([this](std::stop_token stop) { processThread(stop); });
#else
            processFile(*file, fnc);
#endif
        } // endif handler found
        else if (options & SHOW_ALL) {
            std::scoped_lock lock(mxOutput);
            writer->printMessage(std::cout, *file, (options & SHOW_ERRORS) ? _("Unknown file-type") : "");
        }
        file = ds.next();
    } // end-while

#ifdef ENABLE_THREADS
    // Wait for threads to process all pending files (before handling
    // subdirectories)
    {
        std::unique_lock lock(mxListFiles);
        cvListFiles.wait(lock, [this] { return listFiles.empty() && !cActive; });
    }
#endif

    // Now handle subdirectories (if specified)
    if (options & RECURSIVE) {
        std::string files(ds.getFileSpec()); // Use same filespecification

        file = ds.find(ds.getDirectory() + "*", YGP::DirectorySearch::FILE_DIRECTORY);
        while (file) {
            if (!YGP::IDirectorySearch::isSpecial(file->name())) {
                {
                    std::scoped_lock lock(mxOutput);
                    writer->printSeparator(std::cout, *file, iniOpts.separate, iniOpts.title);
                }
                std::string strFile(file->path());
                strFile += file->name();
                strFile += YGP::File::DIRSEPARATOR;
                strFile += files;
                handleFiles(strFile.c_str());
            }
            file = ds.next();
        }
    }
}

#ifdef ENABLE_THREADS
//-----------------------------------------------------------------------------
/// Threadfunction to process files (until the thread is requested to stop)
/// \param stop Token signaling the thread to terminate
//-----------------------------------------------------------------------------
void Application::processThread(std::stop_token stop) {
    while (true) {
        std::unique_lock lock(mxListFiles);
        if (!cvListFiles.wait(lock, stop, [this] { return !listFiles.empty(); }))
            break; // Stop requested; no files left

        FileFunction file(std::move(listFiles.front()));
        listFiles.pop();
        ++cActive;
        TRACE1("Application::processThread (std::stop_token) - File " << file.name() << "; Remaining: " << listFiles.size());
        lock.unlock();
        cvListFiles.notify_all();

        Check3(file.fnc);
        processFile(file, file.fnc);

        lock.lock();
        --cActive;
        lock.unlock();
        cvListFiles.notify_all();
    } // end-while
}
#endif

//-----------------------------------------------------------------------------
/// Processes a single file with a known handler
/// \param pFile File to processs
/// \param fnc Handling function
//-----------------------------------------------------------------------------
void Application::processFile(const YGP::File& file, HANDLER fnc) const {
    TRACE1("Application::processFile (const YGP::File&) const - " << file.name());

    std::string strFile(file.path());
    strFile += file.name();

    std::ifstream ifile(strFile, std::ios::in | std::ios::binary);
    if (!ifile) {
        std::scoped_lock lock(mxOutput);
        std::string error(_("-error: File `%1' can't be opened!\nReason: "));
        error.replace(error.find("%1"), 2, strFile);
        std::cerr << PACKAGE << error;
        std::perror("");
    }
    else {
        try {
            Properties prop;
            fnc(ifile, prop);
            convertFromWideChar(prop);
            std::scoped_lock lock(mxOutput);
            writer->printFile(std::cout, file, prop);
        }
        catch (const YGP::ParseError& err) {
            std::scoped_lock lock(mxOutput);
            std::cerr << PACKAGE << _("-error: ") << err.what() << '\n';
            std::string msg;
            if (options & SHOW_ERRORS) {
                msg = _("Error while processing: ");
                msg += err.what();
            }
            writer->printMessage(std::cout, file, msg);
        } // end-catch
    } // end-else file could be opened
}

#ifdef SUPPORT_HTML
//-----------------------------------------------------------------------------
/// Tries to extract the properties of an HTML-document
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processHTML(std::istream& hFile, Properties& result) { ParseHTML().parse(hFile, result); }
#endif

#ifdef SUPPORT_PDF
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a PDF document
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processPDF(std::istream& hFile, Properties& result) { ParsePDF::parse(hFile, result); }
#endif

#ifdef SUPPORT_MP3
//-----------------------------------------------------------------------------
/// Tries to extract the properties of an MP3 file
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processMP3(std::istream& hFile, Properties& result) { ParseMP3::parse(hFile, result); }
#endif

#ifdef SUPPORT_OGG
//-----------------------------------------------------------------------------
/// Tries to extract the properties out of an OGG file
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processOGG(std::istream& hFile, Properties& result) { ParseOGG::parse(hFile, result); }
#endif

#ifdef SUPPORT_OO
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a StarOffice document
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processStarOffice(std::istream& hFile, Properties& result) { ParseStarOffice().parse(hFile, result); }

//-----------------------------------------------------------------------------
/// Tries to extract the properties of an OpenOffice document
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processOpenOffice(std::istream& hFile, Properties& result) { ParseOpenOffice().parse(hFile, result); }
#endif

#ifdef SUPPORT_ABIWORD
//-----------------------------------------------------------------------------
/// Tries to extract the properties of an Abiword document
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processAbiword(std::istream& hFile, Properties& result) { ParseAbiword().parse(hFile, result); }
#endif

#ifdef SUPPORT_RTF
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a RTF-document
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processRTF(std::istream& hFile, Properties& result) {
    TRACE9("Parsing RTF");
    ParseRTF().parse(hFile, result);
}
#endif

#ifdef SUPPORT_MSOFFICE
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a MS-office document
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processMSOffice(std::istream& hFile, Properties& result) { ParseMSOffice().parse(hFile, result); }

#    ifdef SUPPORT_OOXML
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a MS Office Open XML document
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processOOXML(std::istream& hFile, Properties& result) { ParseOOXML().parse(hFile, result); }
#    endif
#endif

#ifdef SUPPORT_JPEG
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a JPEG image
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processJPG(std::istream& hFile, Properties& result) { ParseJPEG().parse(hFile, result); }
#endif

#ifdef SUPPORT_PNG
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a PNG image
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processPNG(std::istream& hFile, Properties& result) { ParsePNG(result).parse(hFile); }
#endif

#ifdef SUPPORT_GIF
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a GIF image
/// \param hFile File to processs
/// \param result Result of parsing
//-----------------------------------------------------------------------------
void Application::processGIF(std::istream& hFile, Properties& result) { ParseGIF(result).parse(hFile); }
#endif

//-----------------------------------------------------------------------------
/// Returns a handling function to a filetype
/// \param file Filename
/// \returns \c HANDLER Method to handle this filetype; nullptr in case of error
//-----------------------------------------------------------------------------
Application::HANDLER Application::getFileTypeHandler(const char* file) const {
    TRACE9("Application::getFileTypeHandler (const std::string&) - " << file);
    Check1(ftchk);

    unsigned int type(ftchk->getType(file));
    if (type != YGP::FileTypeChecker::UNKNOWN)
        if (auto i(handlers.find(type)); i != handlers.end())
            return i->second;
    return nullptr;
}

//-----------------------------------------------------------------------------
/// Converts wide characters to normal strings
/// \param prop Properties to convert
//-----------------------------------------------------------------------------
void Application::convertFromWideChar(Properties& prop) {
    for (auto member : {&Properties::strTitle, &Properties::strComment, &Properties::strAuthor}) {
        std::string& value(prop.*member);
        if ((value.size() > 1) && std::iscntrl(static_cast<unsigned char>(value[1])) && !(value.size() & 1)) {
            for (std::size_t i(1); i < (value.size() >> 1); ++i)
                value[i] = value[i << 1];
            value.resize(value.size() >> 1);
        }
    }
}

//-----------------------------------------------------------------------------
/// Reads the options of the INI-file
/// \param pFile Pointer to filename
/// \pre pFile not NULL
//-----------------------------------------------------------------------------
void Application::readINIFile(const char* pFile) {
    TRACE5("Application::readINIFile (const char*) - " << pFile);
    Check3(pFile);

    std::string mode;
    try {
        INIFILE(pFile);
        INIOBJ(iniOpts, Output);

        INISECTION(FileType);
        INIATTR2(FileType, std::string, mode, Mode);

#ifdef ENABLE_PLUGINS
        INIMAP2(Handler, std::string, dynHandlers);
#endif
        INIFILE_READ();
    }
    catch (const YGP::FileError&) {
    }
    catch (const std::exception& error) {
        std::string err("-warning: Error reading INI-file `%1'! %2\n");
        err.replace(err.find("%1"), 2, pFile);
        err.replace(err.find("%2"), 2, error.what());
        std::cerr << name() << err;
    }

    if (iniOpts.style.size()) {
        std::optional<OutputStyle> style(getOutputStyle(iniOpts.style));
        outputStyle = style.value_or(OutputStyle::TEXT);
        if (!style) {
            std::string error(_("-warning: The INI-file `%1' contains an invalid "
                                "entry for the output style (`%2')! Using text\n"));
            error.replace(error.find("%1"), 2, pFile);
            error.replace(error.find("%2"), 2, iniOpts.style);
            std::cerr << PACKAGE << error;
        }
    }

    if (mode.size() && setMode(mode)) {
        std::string error(_("-warning: The INI-file `%1' contains an invalid mode "
                            "to determine the file-type (`%2')! Using text\n"));
        error.replace(error.find("%1"), 2, pFile);
        error.replace(error.find("%2"), 2, iniOpts.style);
        std::cerr << PACKAGE << error;
    }
}

//-----------------------------------------------------------------------------
/// Prints a list of supported files
//-----------------------------------------------------------------------------
void Application::showSupportedTypes() const {
    std::cout << _("Currently supported files are:")
              << "\n"
#ifdef SUPPORT_HTML
                 "  - HTML (*.html, *.htm, *.shtml, *.shtm, *.sht, *.php)\n"
#endif
#ifdef SUPPORT_JPEG
                 "  - JPEG (*.jpeg, *.jpg)\n"
#endif
#ifdef SUPPORT_PNG
                 "  - PNG (*.png)\n"
#endif
#ifdef SUPPORT_GIF
                 "  - GIF (*.gif)\n"
#endif
#ifdef SUPPORT_MP3
                 "  - MP3 (*.mp3)\n"
#endif
#ifdef SUPPORT_OGG
                 "  - OGG (*.ogg)\n"
#endif
#ifdef SUPPORT_PDF
                 "  - PDF (*.pdf)\n"
#endif
#ifdef SUPPORT_OO
                 "  - OpenOffice (Write (*.sxw), Calc (*.sxc), Impress (*.sxi), "
                 "Math (*.sxm)"
                 " &\n    Draw (*.sxd))\n"
                 "  - OpenOffice 2 (Write (*.odt), Calc (*.ods), Impress (*.odp) "
                 "& Draw (*.odg))\n"
                 "  - StarOffice (Write (*.sdw), Calc (*.sdc), Impress (*.sdd) & "
                 "Draw (*.sda))\n"
#endif
#ifdef SUPPORT_ABIWORD
                 "  - Abiword (*.abw)\n"
#endif
#ifdef SUPPORT_RTF
                 "  - RTF (*.rtf)\n"
#endif
#ifdef SUPPORT_MSOFFICE
                 "  - Microsoft Office (Word (*.doc), Excel (*.xls) & Powerpoint "
                 "(*.ppt))\n"
#    ifdef SUPPORT_OOXML
                 "  - Microsoft Office Open XML (*.docx, *.xlsx, *.pptx)\n"
#    endif
#endif
        ;
}

#ifdef ENABLE_PLUGINS
//-----------------------------------------------------------------------------
/// Adds the dynamic handlers (which are loaded as shared libraries) to the
/// default handlers (compiled-in).
///
/// Errors are reported to std::cerr
//-----------------------------------------------------------------------------
void Application::setPlugins() {
    TRACE1("Application::setPlugins (std::map<std::string, std::string>&): " << dynHandlers.size());

    using MatchFunction = std::remove_pointer_t<YGP::FileTypeCheckerByContent::MATCHFNC>;

    unsigned int offset(YGP::FileTypeChecker::LAST);
    for (const auto& [extension, library] : dynHandlers) {
        try {
            boost::dll::shared_library plugin(library);
            auto* byContent(dynamic_cast<YGP::FileTypeCheckerByContent*>(ftchk.get()));
            if (!plugin.has(PLUGIN_PROCESS) || (byContent && !plugin.has(PLUGIN_CHECKTYPE))) {
                std::string error(_("Invalid plug-in `%1'!\n"));
                error.replace(error.find("%1"), 2, library);
                throw YGP::FileError(error);
            }

            // Add handling method
            if (byContent)
                byContent->addType(offset, &plugin.get<MatchFunction>(PLUGIN_CHECKTYPE));
            else
                dynamic_cast<YGP::FileTypeCheckerByExtension&>(*ftchk).addType(extension.c_str(), offset);
            handlers[offset++] = &plugin.get<std::remove_pointer_t<HANDLER>>(PLUGIN_PROCESS);
            modules.push_back(std::move(plugin));
        }
        catch (const std::exception& e) {
            std::cerr << PACKAGE << _("-warning: ") << e.what() << '\n';
        }
    }
}
#endif

//-----------------------------------------------------------------------------
/// Entrypoint of application
/// \param argc Anzahl der Parameter
/// \param argv Array mit Zeigern auf Parameter
/// \returns \c int Status
//-----------------------------------------------------------------------------
int main(int argc, const char* argv[]) {
    YGP::IVIOApplication::initI18n(PACKAGE, LOCALEDIR);
    Application appl(argc, argv);
    return appl.run();
}
