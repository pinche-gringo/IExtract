//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : Extract
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 10.08.2002
//COPYRIGHT   : Copyright (C) 2002 - 2007

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#ifdef _MSC_VER
#pragma warning(disable:4786) // disable warning about truncating debug info
#endif

#include <IExtract-cfg.h>
#include <ygp-cfg.h>

#include <cctype>
#include <cstdlib>
#include <cstring>

#include <map>
#include <string>

#include <YGP/Check.h>
#include <YGP/Trace.h>

#ifdef ENABLE_PLUGINS
#  include <YGP/Module.h>

const char* PLUGIN_PROCESS ("processFile");
const char* PLUGIN_CHECKTYPE ("getFileType");
#endif

#ifdef ENABLE_THREADS
#  include <queue>
#  include <vector>
#  include <algorithm>

#  include <YGP/Mutex.h>
#  include <YGP/Thread.h>
#  define LOCKFILES     { ((Application*)this)->mxListFiles.lock (); }
#  define UNLOCKFILES   { ((Application*)this)->mxListFiles.unlock (); }
#  define LOCKTHREADS   { ((Application*)this)->mxThreads.lock (); }
#  define UNLOCKTHREADS { ((Application*)this)->mxThreads.unlock (); }
#  define LOCKOUTPUT    { ((Application*)this)->mxOutput.lock (); }
#  define UNLOCKOUTPUT  { ((Application*)this)->mxOutput.unlock (); }
#else
#  define LOCKFILES
#  define UNLOCKFILES
#  define LOCKTHREADS
#  define UNLOCKTHREADS
#  define LOCKOUTPUT
#  define UNLOCKOUTPUT
#endif

#include <YGP/XStream.h>
#include <YGP/INIFile.h>
#include <YGP/DirSrch.h>
#include <YGP/XDirSrch.h>
#include <YGP/PathSrch.h>
#include <YGP/IVIOAppl.h>
#include <YGP/SortDirSrch.h>
#include <YGP/FileTypeChk.h>

#include "Writer.h"
#include "Options.h"
#include "Properties.h"

#ifdef SUPPORT_MP3
#  include "ParseMP3.h"
#endif
#ifdef SUPPORT_OGG
#  include "ParseOGG.h"
#endif
#ifdef SUPPORT_PDF
#  include "ParsePDF.h"
#endif
#ifdef SUPPORT_JPEG
#  include "ParseJPG.h"
#endif
#ifdef SUPPORT_PNG
#  include "ParsePNG.h"
#endif
#ifdef SUPPORT_GIF
#  include "ParseGIF.h"
#endif
#ifdef SUPPORT_RTF
#  include "ParseRTF.h"
#endif
#ifdef SUPPORT_HTML
#  include "ParseHTML.h"
#endif
#ifdef SUPPORT_MSOFFICE
#  include "ParseMSOffice.h"
#endif
#ifdef SUPPORT_ABIWORD
#  include "ParseAbiword.h"
#endif
#ifdef SUPPORT_OO
#  include "ParseOOffice.h"
#  include "ParseSOffice.h"
#endif

#if SYSTEM == UNIX
#  include <unistd.h>
#elif SYSTEM == WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#  define strcasecmp stricmp
#  define sleep      Sleep
#endif


static const char* const DEFAULT_FORMAT = "%n|-|%t|%a|%c|%d";
static const char* const DEFAULT_XML_FORMAT = "<tr><td>%n</td><td>%t</td>"
                                              "<td>%a</td><td>%c</td><td>%d</td></tr>";


// Class to run Extract-Application
class Application : public YGP::IVIOApplication {
 public:
   Application (const int argc, const char* argv[]);
   ~Application ();

 protected:
   virtual void readINIFile (const char* pFile);
   virtual bool handleOption (const char option);

   // Program-handling
   virtual bool        shallShowInfo () const { return false; }
   virtual int         perform (int argc, const char* argv[]);
   virtual const char* name () const { return PACKAGE; }
   virtual const char* description () const {
      static std::string version (PACKAGE " V" VERSION " - ");
      version += (_("Compiled on %1 at %2\n\nCopyright (C) 2002 - 2006 Markus Schwab; email: g17m0@lycos.com\nDistributed under the terms of the GNU General Public License"));
      version.replace (version.find ("%1"), 2, __DATE__);
      version.replace (version.find ("%2"), 2, __TIME__);
      return version.c_str (); }

   // Help-handling
   virtual void showHelp () const;

 private:
   // Prohobited manager functions
   Application ();
   Application (const Application&);
   const Application& operator= (const Application&);

   typedef void (*HANDLER) (YGP::Xistream& hFile, Properties& result);
   HANDLER getFileTypeHandler (const char* file) const;

   typedef std::map<unsigned int, HANDLER> handlerMap;
   typedef std::pair<unsigned int, HANDLER> handlerValue;
   handlerMap handlers;

   bool setMode (const std::string& mode);

   static void convertFromWideChar (Properties& prop);

   void handleFiles (const char* pFile) const;
   void processFile (const YGP::File& file, HANDLER fnc) const;
#ifdef ENABLE_THREADS
   void* processThread (void*);
#endif

#ifdef SUPPORT_MP3
   static void processMP3 (YGP::Xistream& hFile, Properties& result);
#endif
#ifdef SUPPORT_OGG
   static void processOGG (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_PDF
   static void processPDF (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_JPEG
   static void processJPG (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_PNG
   static void processPNG (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_GIF
   static void processGIF (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_HTML
   static void processHTML (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_RTF
   static void processRTF (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_MSOFFICE
   static void processMSOffice (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_OO
   static void processOpenOffice (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
   static void processStarOffice (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif
#ifdef SUPPORT_ABIWORD
   static void processAbiword (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError);
#endif

   enum { RECURSIVE = 0x1, SHOW_ALL = 0x2, SHOW_ERRORS = 0x4, TRUNC_EXTENSION = 0x8, TERMINATE = 0x10 };

   unsigned int options;

   unsigned int chgFlag;
   Options iniOpts;

   Writer* writer;
   std::string filelist;
   std::string append, prepend;

   enum { TEXT = 0, QUOTED, HTML, LATEX, XML } outputStyle;

   YGP::FileTypeChecker* ftchk;

   static const longOptions lo[];

#ifdef ENABLE_THREADS
   std::vector<YGP::Thread*> aThreads;
   YGP::Mutex                mxThreads;
   YGP::Mutex                mxOutput;

   typedef struct FileFunction : public YGP::File {
      HANDLER fnc;
      FileFunction () : YGP::File (), fnc (NULL) { }
      FileFunction (const YGP::File& file) : YGP::File (file), fnc (NULL) { }
      FileFunction (const struct FileFunction& ffnc) : YGP::File (ffnc)
         , fnc (ffnc.fnc) { }

      const struct FileFunction& operator= (const struct FileFunction& ffnc) {
         if (this != &ffnc) {
            YGP::File::operator= (ffnc);
            fnc = ffnc.fnc;
         }
         return *this; }
   } FILEFNC;
   YGP::Mutex           mxListFiles;
   std::queue<FILEFNC>  listFiles;
#endif

#ifdef ENABLE_PLUGINS
   std::map<std::string, std::string> dynHandlers;
   std::vector<YGP::Module*> modules;

   void setPlugins ();
#endif
};

const YGP::IVIOApplication::longOptions Application::lo[] = {
   { IVIOAPPL_HELP_OPTION },
   { "recursive", 'r' },
   { "format", 'F' },
   { "title", 'T' },
#ifdef ENABLE_THREADS
   { "threads", 't' },
#endif
   { "include", 'i' },
   { "exclude", 'x' },
   { "show-errors", 'e' },
   { "separate", 's' },
   { "add-unknown", 'u' },
   { "new", 'n' },
   { "ini-file", 'f' },
   { "version", 'V' },
   { "output", 'o' },
   { "append", 'a' },
   { "app-file", 'A' },
   { "prepend", 'p' },
   { "pre-file", 'P' },
   { "sort", 'S' },
   { "mode", 'M' },
   { NULL, '\0' } };



//----------------------------------------------------------------------------
/// Constructor
/// \Param argc: Number of parameters to the program
/// \param argv: Array holding (pointer to) arguments
//----------------------------------------------------------------------------
Application::Application (const int argc, const char* argv[])
   : YGP::IVIOApplication (argc, argv, lo), options (0), chgFlag (0), iniOpts (),
     writer (NULL), outputStyle (TEXT),
     ftchk (NULL)
#ifdef ENABLE_THREADS
    , aThreads (0)
#endif
{
   iniOpts.format = DEFAULT_FORMAT;
   iniOpts.ageOfNewFiles = 30 * 24 * 60 * 60;
#ifdef ENABLE_THREADS
   aThreads.reserve (1);
#endif

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
   handlers[YGP::FileTypeChecker::MSOFFICE] =  &Application::processMSOffice;
#endif
#ifdef SUPPORT_OGG
   handlers[YGP::FileTypeChecker::OGG] =  &Application::processOGG;
#endif
#ifdef SUPPORT_OO
   handlers[YGP::FileTypeChecker::OPENOFFICE] =  &Application::processOpenOffice;
   handlers[YGP::FileTypeChecker::STAROFFICE] =  &Application::processStarOffice;
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

//----------------------------------------------------------------------------
/// Destructor
//----------------------------------------------------------------------------
Application::~Application () {
}


//-----------------------------------------------------------------------------
/// Displays the help
//-----------------------------------------------------------------------------
void Application::showHelp () const {
   std::cout << _("Extracts descriptions/comments out of files (depending on the file-type)\n\nUsage:")
      << " " PACKAGE " " << _("[OPTIONS] <File(s)>")
      << "\n\n  -r, --recursive ....... " << _("Recurse into subdirectories")
      << "\n  -o, --output=STYLE .... " << _("Sets the output-style (default: text;\n\t\t\t  other Values are: quoted, HTML, XML or LaTeX)")
      << "\n  -F, --format=FORMAT ... " << _("Format of output; default: ") << DEFAULT_FORMAT
      << "\n  -T, --title=TITLE ..... " << _("Title of output")
      << "\n  -s, --separate=TEXT ... " << _("Separate subdirectories with TEXT (default: empty);\n\t\t\t  implies recursion into subdirectories (--recursive)")
      << "\n  -p, --prepend=HEAD .... " << _("Text to print before any output")
      << "\n  -P, --pre-file=FILE ... " << _("File to print before any output")
      << "\n  -a, --append=FOOT ..... " << _("Text to print at the end of the output")
      << "\n  -A, --app-file=FILE ... " << _("File to print at the end of the output")
      << "\n  -e, --show-errors ..... " << _("Puts error messages (additionally) into the output")
      << "\n  -u, --add-unknown ..... " << _("Show all files (including unknown types) in output")
#ifdef ENABLE_THREADS
      << "\n  -t, --threads=NR ...... " << _("Number of threads for examining files (default: 1)")
#endif
      << "\n  -n, --new=DAYS:TEXT ... " << _("Show TEXT for files younger than DAYS days (def: 30)")
      << "\n  -i, --include=LIST .... " << _("Files to inspect")
      << "\n  -x, --exclude=LIST .... " << _("Files not to inspect")
      << "\n  -f, --ini-file=FILE ... " << _("Read further options from specified file")
      << "\n  -S, --sort ..... ...... " << _("Sort found files alphabetically")
      << "\n  -M, --mode=[MODUS] .... " << _("Modus operandi to determine the file-type:\n"
	                                     "\t\t\t  Ext: From (last) extension (Default)\n"
	                                     "\t\t\t  EXT: From (last) extension (ignoring case)\n"
					     "\t\t\t  AllExt: From any extension (if unknown)\n"
					     "\t\t\t  AllEXT: From any extension (ignoring case)\n"
					     "\t\t\t  Content: From content of the file")
      << "\n  -V, --version ......... " << _("Output version information and exit")
      << "\n  -h, -?, --help ........ " << _("Displays this help and exit\n")
      << _("  File(s) ... Files to analyze (the last part can contain wildcards)\n\n")
      << _("DAYS (in option -n) may be omited or may have an multiplier suffix: m for 30.\n\n")
      << _("LIST is a list of files; seperated with the path-separator of the operating\n")
      << _("     system (':' for UNICES, ';' for Windows). E.g. *.html:*.doc\n\n")
      << _("FORMAT specifies how to print the entries;\n")
      << _("       The pipe symbol (|) separates columns\n")
      << _("       %a is substituted with the author\n")
      << _("       %c is substituted with the comment\n")
      << _("       %d is substituted with the modification time of the file\n")
      << _("       %D is substituted with the modification time of the file (day only)\n")
      << _("       %e is substituted with the extension of the file\n")
      << _("       %E is substituted with the name of the file without extension\n")
      << _("       %n is substituted with the name of the file\n")
      << _("       %N is substituted with path and name of the file\n")
      << _("       %p is substituted with the path of the file\n")
      << _("       %P is substituted with the path of the file in UNIX style (with /)\n")
      << _("       %s is substituted with the size of the file\n")
      << _("       %S is substituted with the size of the file (human readable)\n")
      << _("       %t is substituted with the title\n")
      << _("       %U is substituted with path and name of the file in UNIX style (with /)\n")
      << _("       %(LETTERS) is substituted with first of the above substitutions\n"
           "          producing a non-empty string (e.g. %(tn) is the titel if not \n"
           "          empty or else the filename.)\n")
/* xgettext:no-c-format */ << _("       %*LETTER changes the substitution slightly. For file names it causes a\n"
           "          conversion of special characters; for the others it suppresses them\n\n")
/* xgettext:no-c-format */ << _("       In every other constellation the '%' is removed!\n\n")
      << _("TITLE specifies the headers for the output; separated with (|); columns must\n")
      << _("      contain at least one character\n\n")
      << _("TEXT specifies the text to separate subdirectories; with the following\n")
      << _("     conversion strings:\n")
      << _("       %e prints the end-of-output for the specified output style\n")
      << _("       %n is substituted with the name of the directory\n")
      << _("       %N is substituted with the full path of the directory\n")
      << _("       %p is substituted with the path to the directory\n")
      << _("       %P is substituted with the path to the directory in UNIX style (with /)\n")
      << _("       %s prints the start-of-output for the specified output style\n")
      << _("       %U is substituted with the full path of the dir in UNIX style (with /)\n\n")
      << _("     As with FORMAT, the pipe symbol (|) separates columns.\n\n")
      << _("The format of the INI file is like this (entries can be missing):\n\n")
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
      << _("Currently supported files are:")
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
      "  - OpenOffice (Write (*.sxw), Calc (*.sxc), Impress (*.sxi), Math (*.sxm)"
          " &\n    Draw (*.sxd))\n"
      "  - OpenOffice 2 (Write (*.odt), Calc (*.ods), Impress (*.odp) & Draw (*.odg))\n"
      "  - StarOffice (Write (*.sdw), Calc (*.sdc), Impress (*.sdd) & Draw (*.sda))\n"
#endif
#ifdef SUPPORT_ABIWORD
      "  - Abiword (*.abw)\n"
#endif
#ifdef SUPPORT_RTF
      "  - RTF (*.rtf)\n"
#endif
#ifdef SUPPORT_MSOFFICE
      "  - Microsoft Office (Word (*.doc), Excel (*.xls) & Powerpoint (*.ppt))\n"
#endif
      ;
}

//-----------------------------------------------------------------------------
/// Checks the validity of the passed option
/// \param option: Actual option
/// \returns \c bool: Status; false: Invalid option/option-value Require :
///     option not '\0´'
//-----------------------------------------------------------------------------
bool Application::handleOption (const char option) {
   Check3 (option != '\0');

   switch (option) {
   case 's': {
      const char* pSep = getOptionValue ();
      if (pSep)
         iniOpts.separate = pSep;
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << error;
         break; } }
      // Don't add a break in OK case, as -s implies -r!

   case 'r': options |= RECURSIVE; break;

   case 'o': {
      const char* pType = getOptionValue ();
      if (!pType
          || ((outputStyle = HTML, strcmp (pType, "HTML"))
              && (outputStyle = TEXT, strcmp (pType, "text"))
              && (outputStyle = QUOTED, strcmp (pType, "quoted"))
              && (outputStyle = XML, strcmp (pType, "XML"))
              && (outputStyle = LATEX, strcmp (pType, "LaTeX")))) {
         outputStyle = TEXT;
         std::string error (_("-warning: Style of output `%1' is not valid! Using text\n"));
         error.replace (error.find ("%1"), 2, pType);
         std::cerr << PACKAGE << error;
      }
      break; }

#ifdef ENABLE_THREADS
   case 't': {
      const char* pThreads = getOptionValue ();
      unsigned int cThreads (0);
      char* pEnd = NULL;
      if (!pThreads
          || (!(cThreads = strtoul (pThreads, &pEnd, 10)))
          || (!pEnd || *pEnd)) {
         std::cerr << PACKAGE << _("-warning: Invalid number of threads!\n");
      }
      else
         aThreads.reserve (cThreads);
      break; }
#endif

   case 'e': options |= SHOW_ERRORS; break;

   case 'F': {
      const char* pFormat = getOptionValue ();
      if (pFormat) {
         iniOpts.format = pFormat;
         chgFlag |= 0x1;
      }
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'T': {
      const char* pTitle = getOptionValue ();
      if (pTitle)
         iniOpts.title = pTitle;
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'n': {
      const char* pNew = getOptionValue ();
      char* pEnd = NULL;
      unsigned int time (0);
      if (!pNew) {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << error;
      }
      else if ((time = strtoul (pNew, &pEnd, 10)),
               (!pEnd || ((*pEnd != ':') && (*pEnd != 'm')))) {
         std::string error (_("-warning: Argument for new files `%1' is not valid! Ignoring option `n'\n"));
         error.replace (error.find ("%1"), 2, pNew);
         std::cerr << PACKAGE << error;
      }
      else {
         if (*pEnd == 'm') {
            ++pEnd;
            time *= 30;
         }
         if (time)
            iniOpts.ageOfNewFiles = time * 24 * 60 * 60;
         iniOpts.newText = pEnd + 1;
      }
      break; }

   case 'x':
   case 'i':  {
      const char* files = getOptionValue ();
      if (files) {
         std::string node;
         YGP::PathSearch list (files);
         while ((node = list.getNextNode ()).size ()) {
            filelist += option;
            filelist += node;
            filelist += YGP::PathSearch::PATHSEPARATOR;
         }
      }
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'u': options |= SHOW_ALL; break;

   case 'f': {
      const char* pFile = getOptionValue ();
      if (pFile)
         readINIFile (pFile);
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'p':
   case 'a': {
      const char* text = getOptionValue ();
      if (text)
         ((option == 'p') ? prepend : append) += text;
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'P':
   case 'A': {
      const char* file = getOptionValue ();
      if (file) {
         std::ifstream input (file, std::ios::in | std::ios::binary);
         if (!input) {
            std::string error (_("-error: `%1' is not a (readable) file! Ignoring option `%2'!\n"));
            error.replace (error.find ("%1"), 2, file);
            error.replace (error.find ("%2"), 2, 1, option);
            std::cerr << PACKAGE << error;
         }
         else {
            std::string& target = (option == 'P') ? prepend : append;

            static const unsigned int bufLen (512);
            char buffer[bufLen];

            // Read as long as there is data/or an error occurs
            while (input.read (buffer, bufLen), input.gcount ())
               target.append (buffer, input.gcount ());
         }
      }
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, option);
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'S':
      iniOpts.sort = true;
      break;

   case 'M':
      if (checkOptionValue ()) {
	 const char* mode (getOptionValue ());
	 if (setMode (mode)) {
	    std::string error (_("-warning: Invalid mode `%1'! Ignoring option 'M'!\n"));
	    error.replace (error.find ("%1"), 2, mode);
	    std::cerr << PACKAGE << error;
	 }
      }
      break;

   case 'V': std::cout << description () << '\n'; exit (0);

   default: {
      std::string error (_("-warning: Ignoring invalid option `%1'\n"));
      error.replace (error.find ("%1"), 2, 1, option);
      std::cerr << PACKAGE << error;
   }
   }
   return true;
}

//-----------------------------------------------------------------------------
/// Sets the mode how to determine the file-type according to the passed value
/// \param mode: How to determine the file-type
///              - Ext: By (last) extension
///              - AllExt: By any extension (starting from the last)
///              - EXT: By (last) extension (ignoring case)
///              - AllEXT: By any extension (ignoring case)
///              - Content: By the content of the file
/// \returns bool: True, if the passed mode is invalid
//-----------------------------------------------------------------------------
bool Application::setMode (const std::string& mode) {
   YGP::FileTypeChecker* newFtchk (NULL);
   if (mode == "Ext") {
      newFtchk = new YGP::FileTypeCheckerByExtension;
      options &= ~TRUNC_EXTENSION;
   }
   else if (mode == "AllExt") {
      newFtchk = new YGP::FileTypeCheckerByExtension;
      options |= TRUNC_EXTENSION;
   }
   else if (mode == "EXT") {
      newFtchk = new YGP::FileTypeCheckerByCaseExt;
      options &= ~TRUNC_EXTENSION;
   }
   else if (mode == "AllEXT") {
      newFtchk = new YGP::FileTypeCheckerByCaseExt;
      options |= TRUNC_EXTENSION;
   }
   else if (mode == "Content") {
      newFtchk = new YGP::FileTypeCheckerByContent;
      options &= ~TRUNC_EXTENSION;
   }
   else
      return true;

   delete ftchk;
   ftchk = newFtchk;
   return false;
}

//-----------------------------------------------------------------------------
/// Performs the job of the applications
/// \param argc: Number of parameters (without options)
/// \param argv: Array with pointer to arguments
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int Application::perform (int argc, const char* argv[]) {
   if (!argc) {                        // Check if enough arguments are passed
      showHelp ();
      return -1;
   }

   if ((outputStyle == XML) && !(chgFlag & 1))
      iniOpts.format = DEFAULT_XML_FORMAT;
   Check3 (iniOpts.format.size ());

   if (!ftchk)
      ftchk = new YGP::FileTypeCheckerByExtension;
   Check3 (ftchk);

   typedef Writer* (*CREATEWRITER) (const std::string&, const std::string&,
                                    unsigned long);

   // This declaration must be in the same order as the outputStyle enum
   CREATEWRITER fnc[] = { (CREATEWRITER)&TextWriter::create,
                          (CREATEWRITER)&QuotedTextWriter::create,
                          (CREATEWRITER)&HTMLWriter::create,
                          (CREATEWRITER)&LaTeXWriter::create,
                          (CREATEWRITER)&XMLWriter::create };

   Check3 (outputStyle < (sizeof (fnc) / sizeof (fnc[0])));
   writer = fnc[outputStyle] (iniOpts.format, iniOpts.newText, iniOpts.ageOfNewFiles);
   Check3 (writer);

   if (prepend.size ())
      std::cout << prepend;

   writer->printStart (std::cout, iniOpts.title);

#ifdef ENABLE_PLUGINS
   setPlugins ();
#endif

   std::string file;
   for (int j (0); j < argc; ++j) {
      file = argv[j];
      if (YGP::DirectorySearch::isValid (argv[j])) {
         if (file[file.size () - 1] != YGP::File::DIRSEPARATOR)
            file += YGP::File::DIRSEPARATOR;
         file += "*";
      }
      handleFiles (file.c_str ());
   }

#ifdef ENABLE_THREADS
   ((Application*)this)->options |= TERMINATE;

   // Wait for threads to terminate
   LOCKTHREADS
   while (aThreads.size ()) {
      TRACE9 ("Application::handleFiles (const char*) - Wait for thread "
              << aThreads[0]->getID ());
      unsigned long id (aThreads[0]->getID ());
      UNLOCKTHREADS
      YGP::Thread::waitForThread (id);
      LOCKTHREADS
   }
   UNLOCKTHREADS
#endif

   writer->printEnd (std::cout);
   delete writer;

   if (append.size ())
      std::cout << append;

#ifdef ENABLE_PLUGINS
   for (std::vector<YGP::Module*>::const_iterator i (modules.begin ());
	i != modules.end (); ++i)
      delete (*i);
#endif
   return 0;
}

//-----------------------------------------------------------------------------
/// Expands the filespecification and processes every file
/// \param pFile: Filespecification; may contain wildcards
/// \pre pFile not NULL
//-----------------------------------------------------------------------------
void Application::handleFiles (const char* pFile) const {
   Check3 (pFile);
   TRACE5 ("Application::handleFiles (const char*) const - " << pFile);

   YGP::ExtDirectorySearch& ds
      (iniOpts.sort
       ? *new YGP::SortedDirSearch<YGP::ExtDirectorySearch> (pFile)
       : *new YGP::ExtDirectorySearch (pFile));
   std::string node;
   YGP::PathSearch list (filelist);
   while (!(node = list.getNextNode ()).empty ()) {
      bool include (node[0] == 'i');
      node.replace (0, 1, 0, '\0');
      include ? ds.addFilesToInclude (node) : ds.addFilesToExclude (node);
   } // end-while

   const YGP::File* file = ds.find (YGP::IDirectorySearch::FILE_NORMAL);
   std::string name;
   while (file) {
      // Determine file-type from extension (or second-to-last extension)
      name = file->path ();
      name += file->name ();
      HANDLER fnc (getFileTypeHandler (name.c_str ()));
      // U_nknown type; try second to-last extension (if option passed)
      if ((options & TRUNC_EXTENSION) && !fnc) {
	 do {
	    unsigned int pos (name.rfind ('.'));
	    if (pos == std::string::npos)
	       break;
	    else {
	       name.replace (name.begin () + pos, name.end (), 0, '\0');
	       fnc = getFileTypeHandler (name.c_str ());
	    }
	 } while (!fnc);
      }
      if (fnc) {
#ifdef ENABLE_THREADS
         LOCKFILES
         while (listFiles.size () > 100) {
            UNLOCKFILES
            sleep (0);
            LOCKFILES
         }
         ((Application*)this)->listFiles.push (*file);
         ((Application*)this)->listFiles.back ().fnc = fnc;
         UNLOCKFILES

         LOCKTHREADS
         if (aThreads.size () < aThreads.capacity ())
            try {
               ((Application*)this)->aThreads.push_back (
                  YGP::OThread<Application>::create2 ((Application*)this,
                                                      &Application::processThread, NULL));
            }
            catch (YGP::ExecError& err) {
               std::cerr << PACKAGE << _("-error: ") << err << '\n';
            }
         UNLOCKTHREADS
#else
         processFile (*file, fnc);
#endif
      } // endif handler found
      else
         if (options & SHOW_ALL) {
            LOCKOUTPUT
            writer->printMessage (std::cout, *file,
				  (options & SHOW_ERRORS) ? _("Unknown file-type") : "");
            UNLOCKOUTPUT
         }
      file = ds.next ();
   } // end-while

#ifdef ENABLE_THREADS
   // Wait for threads to handle all pending files
   LOCKFILES
   while (listFiles.size ()) {
      UNLOCKFILES
      sleep (0);
      LOCKFILES
   }
   UNLOCKFILES
#endif

   // Now handle subdirectories (if specified)
   if (options & RECURSIVE) {
      std::string files (ds.getFileSpec ());      // Use same filespecification

      file = ds.find (ds.getDirectory () + "*", YGP::DirectorySearch::FILE_DIRECTORY);
      while (file) {
         if (!YGP::IDirectorySearch::isSpecial (file->name ())) {
            LOCKOUTPUT
            writer->printSeparator (std::cout, *file, iniOpts.separate, iniOpts.title);
            UNLOCKOUTPUT
            std::string strFile (file->path ());
            strFile += file->name ();
            strFile += YGP::File::DIRSEPARATOR;
            strFile += files;
            handleFiles (strFile.c_str ());
         }
         file = ds.next ();
      }
   }

   delete &ds;
}

#ifdef ENABLE_THREADS
//-----------------------------------------------------------------------------
/// Threadfunction to process files (as long as the filelist is full)
//-----------------------------------------------------------------------------
void* Application::processThread (void* pThread) {
   Check3 (pThread);
   FILEFNC file;

   while (true) {
      LOCKFILES
      if (listFiles.size ()) {
         file = listFiles.front ();
         listFiles.pop ();
         UNLOCKFILES
         TRACE1 ("Application::processThread (void*) - File " << file.name ()
                  << "; Remaining: " << listFiles.size ());
         Check3 (file.fnc);
         Check3 (file.fnc == getFileTypeHandler (strrchr (file.name (), '.') + 1));
         processFile (file, file.fnc);
      }
      else {
         UNLOCKFILES
	 if (options & TERMINATE)
	    break;
	 else
	    sleep (1);
      }
   } // end-while

   YGP::ParseObject::freeBuffer ();

   LOCKTHREADS
   Check3 (find (aThreads.begin (), aThreads.end (), pThread) != aThreads.end ());
   aThreads.erase (find (aThreads.begin (), aThreads.end (), pThread));
   UNLOCKTHREADS
   return NULL;
}
#endif

//-----------------------------------------------------------------------------
/// Processes a single file with a known handler
/// \param pFile: File to processs
/// \param fnc: Handling function
//-----------------------------------------------------------------------------
void Application::processFile (const YGP::File& file, HANDLER fnc) const {
   TRACE1 ("Application::processFile (const YGP::File&) const - " << file.name ());

   std::string strFile (file.path ());
   strFile += file.name ();

   YGP::Xifstream ifile;
   ifile.open (strFile.c_str (), std::ios::in | std::ios::binary);
   if (!ifile) {
      LOCKOUTPUT;
      std::string error (_("-error: File `%1' can't be opened!\nReason: "));
      error.replace (error.find ("%1"), 2, strFile);
      std::cerr << PACKAGE << error;
      perror ("");
      UNLOCKOUTPUT
   }
   else {
      ifile.init ();

      try {
         Properties prop;
         fnc ((YGP::Xistream&)ifile, prop);
         convertFromWideChar (prop);
         LOCKOUTPUT
         writer->printFile (std::cout, file, prop);
         UNLOCKOUTPUT
      }
      catch (YGP::ParseError& err) {
         LOCKOUTPUT;
         std::cerr << PACKAGE << _("-error: ") << err.what () << '\n';
	 std::string msg;
	 if (options & SHOW_ERRORS) {
	    msg =  _("Error while processing: ");
	    msg += err.what ();
	 }
         writer->printMessage (std::cout, file, msg);
         UNLOCKOUTPUT
      } // end-catch
   } // end-else file could be opened
}

#ifdef SUPPORT_HTML
//-----------------------------------------------------------------------------
/// Tries to extract the properties of an HTML-document
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processHTML (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParseHTML ().parse (hFile, result);
}
#endif

#ifdef SUPPORT_PDF
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a PDF document
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processPDF (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParsePDF::parse (hFile, result);
}
#endif

#ifdef SUPPORT_MP3
//-----------------------------------------------------------------------------
/// Tries to extract the properties of an MP3 file
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processMP3 (YGP::Xistream& hFile, Properties& result) {
   ParseMP3::parse (hFile, result);
}
#endif

#ifdef SUPPORT_OGG
//-----------------------------------------------------------------------------
/// Tries to extract the properties out of an OGG file
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processOGG (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParseOGG::parse (hFile, result);
}
#endif

#ifdef SUPPORT_OO
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a StarOffice document
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processStarOffice (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParseStarOffice ().parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of an OpenOffice document
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processOpenOffice (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParseOpenOffice ().parse (hFile, result);
}
#endif

#ifdef SUPPORT_ABIWORD
//-----------------------------------------------------------------------------
/// Tries to extract the properties of an Abiword document
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processAbiword (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParseAbiword ().parse (hFile, result);
}
#endif

#ifdef SUPPORT_RTF
//-----------------------------------------------------------------------------
/// Tries to extract the properties of an RTF-document
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processRTF (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   TRACE9 ("Parsing RTF");
   ParseRTF ().parse (hFile, result);
}
#endif

#ifdef SUPPORT_MSOFFICE
//-----------------------------------------------------------------------------
/// Tries to extract the properties of an MS-office document
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processMSOffice (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParseMSOffice ().parse (hFile, result);
}
#endif

#ifdef SUPPORT_JPEG
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a JPEG image
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processJPG (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParseJPEG ().parse (hFile, result);
}
#endif

#ifdef SUPPORT_PNG
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a PNG image
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processPNG (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParsePNG (result).parse (hFile);
}
#endif

#ifdef SUPPORT_GIF
//-----------------------------------------------------------------------------
/// Tries to extract the properties of a GIF image
/// \param hFile: File to processs
/// \param result: Result of parsing
/// \throw YGP::ParseError: In case of an error
//-----------------------------------------------------------------------------
void Application::processGIF (YGP::Xistream& hFile, Properties& result) throw (YGP::ParseError) {
   ParseGIF (result).parse (hFile);
}
#endif

//-----------------------------------------------------------------------------
/// Returns a handling function to a filetype
/// \param file: Filename
/// \returns \c HANDLER: Method to handle this filetype; NULL in case of error
//-----------------------------------------------------------------------------
Application::HANDLER Application::getFileTypeHandler (const char* file) const {
   TRACE9 ("Application::getFileTypeHandler (const std::string&) - " << file);
   Check1 (ftchk);

   unsigned int type (ftchk->getType (file));
   if (type != YGP::FileTypeChecker::UNKNOWN) {
      handlerMap::const_iterator i (handlers.find (type));
      return (i != handlers.end ()) ? i->second : NULL;
   }
   return NULL;
}

//-----------------------------------------------------------------------------
/// Converts wide characters to normal strings
/// \param prop: Properties to convert
//-----------------------------------------------------------------------------
void Application::convertFromWideChar (Properties& prop) {
   static std::string Properties::* values[] = { &Properties::strTitle,
                                                 &Properties::strComment,
                                                 &Properties::strAuthor};

   for (unsigned int i (0); i < (sizeof (values) / sizeof (values[0])); ++i)
      if (iscntrl ((prop.*values[i])[1]) && (!((prop.*values[i]).size () & 1))) {
         for (unsigned int j (1); j < ((prop.*values[i]).size () >> 1); ++j)
            (prop.*values[i])[j] = (prop.*values[i])[j << 1];
         (prop.*values [i]).replace ((prop.*values [i]).size () >> 1,
                                     (prop.*values [i]).size (), 0, '\0');
      }
}

//-----------------------------------------------------------------------------
/// Reads the options of the INI-file
/// \param pFile: Pointer to filename
/// \param Requieres : pFile not NULL
//-----------------------------------------------------------------------------
void Application::readINIFile (const char* pFile) {
   TRACE5 ("Application::readINIFile (const char*) - " << pFile);
   Check3 (pFile);

   std::string Style;
   std::string mode;
   try {
      INIFILE (pFile);
      INIOBJ (iniOpts, Output);

      INISECTION (FileType);
      INIATTR2 (FileType, std::string, mode, Mode);

#ifdef ENABLE_PLUGINS
      INIMAP2 (Handler, std::string, dynHandlers);
#endif
      INIFILE_READ ();
   }
   catch (YGP::FileError&) { }
   catch (std::exception& error) {
      std::string err ("-warning: Error reading INI-file `%1'! %2\n");
      err.replace (err.find ("%1"), 2, pFile);
      err.replace (err.find ("%2"), 2, error.what ());
      std::cerr << name () << err;
   }

   if (iniOpts.style.size ()) {
      if (iniOpts.style == "HTML")
         outputStyle = HTML;
      else if (iniOpts.style == "LaTeX")
         outputStyle = LATEX;
      else if (iniOpts.style == "XML")
         outputStyle = XML;
      else if (iniOpts.style == "quoted")
         outputStyle = QUOTED;
      else {
         outputStyle = TEXT;
         if (iniOpts.style != "text") {
            std::string error (_("-warning: The INI-file `%1' contains an invalid entry for the output style (`%2')! Using text\n"));
            error.replace (error.find ("%1"), 2, pFile);
            error.replace (error.find ("%2"), 2, iniOpts.style);
            std::cerr << PACKAGE << error;
         }
      }
   }

   if (mode.size () && setMode (mode)) {
      std::string error (_("-warning: The INI-file `%1' contains an invalid mode to determine the file-type (`%2')! Using text\n"));
      error.replace (error.find ("%1"), 2, pFile);
      error.replace (error.find ("%2"), 2, iniOpts.style);
      std::cerr << PACKAGE << error;
   }
}

#ifdef ENABLE_PLUGINS
//-----------------------------------------------------------------------------
/// Adds the dynamic handlers (which are loaded as shared libraries) to the
/// default handlers (compiled-in).
///
/// Errors are reported to std::cerr
//-----------------------------------------------------------------------------
void Application::setPlugins () {
   TRACE1 ("Application::setPlugins (std::map<std::string, std::string>&): " << dynHandlers.size ());

   unsigned int offset (YGP::FileTypeChecker::LAST);
   for (std::map<std::string, std::string>::const_iterator i (dynHandlers.begin ());
	i != dynHandlers.end (); ++i) {
      try {
	 YGP::Module* mod (new YGP::Module (i->second.c_str ()));
	 modules.push_back (mod);
	 void* fnProcess (mod->getSymbol (PLUGIN_PROCESS));
	 if (!fnProcess) {
	    std::string error (_("Invalid module `%1'!\n"));
	    error.replace (error.find ("%1"), 2, i->second);
	    throw YGP::FileError (error);
	 }

	 // Add handling method
	 if (typeid (*ftchk) == typeid (YGP::FileTypeCheckerByContent)) {
	    void* fnCheckType (mod->getSymbol (PLUGIN_CHECKTYPE));
	    if (!fnCheckType) {
	       std::string error (_("Invalid module `%1'!\n"));
	       error.replace (error.find ("%1"), 2, i->second);
	       throw YGP::FileError (error);
	    }
	    ((YGP::FileTypeCheckerByContent*)ftchk)->addType (offset, (YGP::FileTypeCheckerByContent::MATCHFNC)fnCheckType);
	 }
	 else
	    ((YGP::FileTypeCheckerByExtension*)ftchk)->addType (i->first.c_str (), offset);
	 handlers[offset++] = (HANDLER)fnProcess;
      }
      catch (YGP::FileError& e) {
	 std::cerr << PACKAGE << _("-warning: ") << e.what () << '\n';
      }
   }
}
#endif


//-----------------------------------------------------------------------------
/// Entrypoint of application
/// \param argc: Anzahl der Parameter
/// \param argv: Array mit Zeigern auf Parameter
/// \returns \c int: Status
//-----------------------------------------------------------------------------
int main (int argc, const char* argv[]) {
   YGP::IVIOApplication::initI18n (PACKAGE, LOCALEDIR),

   Application::initI18n ();
   Application appl (argc, argv);
   return appl.run ();
}
