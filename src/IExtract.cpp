//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : Extract
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 10.08.2002
//COPYRIGHT   : Anticopyright (A) 2002, 2003

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


#include <IExtract-cfg.h>
#include <ygp-cfg.h>

#include <cctype>
#include <cstdlib>
#include <cstring>

#include <map>
#include <string>

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

#include <YGP/Check.h>
#include <YGP/Trace.h>
#include <YGP/XStream.h>
#include <YGP/DirSrch.h>
#include <YGP/XDirSrch.h>
#include <YGP/PathSrch.h>
#include <YGP/IVIOAppl.h>
#include <YGP/INIFile.h>

#include "Writer.h"
#include "Options.h"
#include "ParseMP3.h"
#include "ParsePDF.h"
#include "ParseJPG.h"
#include "ParseRTF.h"
#include "ParseHTML.h"
#include "ParseWord.h"
#include "ParseOOffice.h"
#include "ParseSOffice.h"
#include "Properties.h"

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
      version += (_("Compiled on %1 at %2\n\nAuthor: Markus Schwab; email: g17m0@lycos.com\nDistributed under the terms of the GNU General Public License"));
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

   typedef void (Application::*HANDLER) (YGP::Xistream& hFile, Properties& result) const;
   HANDLER getFileTypeHandler (const char* pExt) const;

   typedef std::map<const std::string, HANDLER> handlerMap;
   typedef std::pair<const std::string, HANDLER> handlerValue;
   handlerMap handlers;

   static void convertFromWideChar (Properties& prop);

   void handleFiles (const char* pFile) const;
   void processFile (const YGP::File& file, HANDLER fnc) const;
#ifdef ENABLE_THREADS
   void* processThread (void*);
#endif

   void processMP3 (YGP::Xistream& hFile, Properties& result) const throw (std::string);
   void processPDF (YGP::Xistream& hFile, Properties& result) const throw (std::string);
   void processJPG (YGP::Xistream& hFile, Properties& result) const throw (std::string);
   void processHTML (YGP::Xistream& hFile, Properties& result) const throw (std::string);
   void processRTF (YGP::Xistream& hFile, Properties& result) const throw (std::string);
   void processOffice (YGP::Xistream& hFile, Properties& result) const
      throw (std::string);
   void processOpenOffice (YGP::Xistream& hFile, Properties& result) const
      throw (std::string);
   void processStarOffice (YGP::Xistream& hFile, Properties& result) const
      throw (std::string);

   enum { RECURSIVE = 0x1, SHOW_ALL = 0x2, SHOW_ERRORS = 0x4 };

   unsigned int options;

   unsigned int chgFlag;
   Options iniOpts;

   Writer* writer;
   std::string filelist;

   enum { TEXT = 0, HTML, LATEX, XML } outputStyle;

   static const longOptions lo[];

#ifdef ENABLE_THREADS
   std::vector<YGP::Thread*> aThreads;
   YGP::Mutex                mxThreads;
   YGP::Mutex                mxOutput;

   typedef struct FileFunction : public YGP::File {
      HANDLER fnc;
      FileFunction () : YGP::File (), fnc (NULL) { }
      FileFunction (const YGP::File& file) : YGP::File (file) { }
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
   { "all", 'a' },
   { "new", 'n' },
   { "ini-file", 'f' },
   { "version", 'V' },
   { "output", 'o' },
   { NULL, '\0' } };



//----------------------------------------------------------------------------
/// Constructor
/// \Param argc: Number of parameters to the program
/// \param argv: Array holding (pointer to) arguments
//----------------------------------------------------------------------------
Application::Application (const int argc, const char* argv[]) 
    : YGP::IVIOApplication (argc, argv, lo), options (0), chgFlag (0), iniOpts ()
    , writer (NULL), outputStyle (TEXT)
#ifdef ENABLE_THREADS
    , aThreads (0)
#endif
{
   iniOpts.format = DEFAULT_FORMAT;
   iniOpts.ageOfNewFiles = 30 * 24 * 60 * 60;
#ifdef ENABLE_THREADS
   aThreads.reserve (1);
#endif

   handlers.insert (handlers.end (), handlerValue ("doc", &Application::processOffice));
   handlers.insert (handlers.end (), handlerValue ("htm", &Application::processHTML));
   handlers.insert (handlers.end (), handlerValue ("html", &Application::processHTML));
   handlers.insert (handlers.end (), handlerValue ("jpeg", &Application::processJPG));
   handlers.insert (handlers.end (), handlerValue ("jpg", &Application::processJPG));
   handlers.insert (handlers.end (), handlerValue ("mp3", &Application::processMP3));
   handlers.insert (handlers.end (), handlerValue ("pdf", &Application::processPDF));
   handlers.insert (handlers.end (), handlerValue ("php", &Application::processHTML));
   handlers.insert (handlers.end (), handlerValue ("ppt", &Application::processOffice));
   handlers.insert (handlers.end (), handlerValue ("rtf", &Application::processRTF));
   handlers.insert (handlers.end (), handlerValue ("sda", &Application::processStarOffice));
   handlers.insert (handlers.end (), handlerValue ("sdc", &Application::processStarOffice));
   handlers.insert (handlers.end (), handlerValue ("sdd", &Application::processStarOffice));
   handlers.insert (handlers.end (), handlerValue ("sdw", &Application::processStarOffice));
   handlers.insert (handlers.end (), handlerValue ("sht", &Application::processHTML));
   handlers.insert (handlers.end (), handlerValue ("shtm", &Application::processHTML));
   handlers.insert (handlers.end (), handlerValue ("shtml", &Application::processHTML));
   handlers.insert (handlers.end (), handlerValue ("sxc", &Application::processOpenOffice));
   handlers.insert (handlers.end (), handlerValue ("sxd", &Application::processOpenOffice));
   handlers.insert (handlers.end (), handlerValue ("sxi", &Application::processOpenOffice));
   handlers.insert (handlers.end (), handlerValue ("sxm", &Application::processOpenOffice));
   handlers.insert (handlers.end (), handlerValue ("sxw", &Application::processOpenOffice));
   handlers.insert (handlers.end (), handlerValue ("xls", &Application::processOffice));
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
   std::cout << _("Extracts a description out of files (depending on the file-type)\n\nUsage:")
      << " " PACKAGE " " << _("[OPTIONS] <File(s)>")
      << "\n\n  -r, --recursive ....... " << _("Recurse into subdirectories")
      << "\n  -o, --output=STYLE .... " << _("Sets the output-style (text, HTML XML or LaTeX)")
      << "\n  -F, --format=FORMAT ... " << _("Format of output; default: ") << DEFAULT_FORMAT
   << "\n                          " << _("and for XML:") << DEFAULT_XML_FORMAT
   << "\n  -T, --title=TITLE ..... " << _("Title of output")
      << "\n  -s, --separate=TEXT ... " << _("Separate subdirectories with TEXT (default: empty);\n                          implies recursion into subdirectories (--recursive)")
      << "\n  -e, --show-errors ..... " << _("Puts error messages (additionally) into the output")
      << "\n  -a, --all ............. " << _("Show all files (including unknown types) in output")
#ifdef ENABLE_THREADS
      << "\n  -t, --threads=NR ...... " << _("Number of threads for examining files (default: 1)")
#endif
      << "\n  -n, --new=DAYS:TEXT ... " << _("Show TEXT for files younger than DAYS days (def: 30)")
      << "\n  -i, --include=LIST .... " << _("Files to inspect")
      << "\n  -x, --exclude=LIST .... " << _("Files not to inspect")
      << "\n  -f, --ini-file=FILE ... " << _("Read further options from specified file")
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
      << _("       %n is substituted with the name of the file\n")
      << _("       %N is substituted with path and name of the file\n")
      << _("       %p is substituted with the path of the file\n")
      << _("       %P is substituted with the path of the file in UNIX style (with /)\n")
      << _("       %s is substituted with the size of the file\n")
      << _("       %S is substituted with the size of the file (human readable)\n")
      << _("       %t is substituted with the title\n")
      << _("       %U is substituted with path and name of the file in UNIX style (with /)\n")
      << _("       %(LETTERS) is substituted with first of the above substitutions\n")
      << _("          producing a non-empty string (e.g. %(tn) is the titel if not \n")
      << _("          empty or else the filename.)\n\n")
       // xgettext:no-c-format
      << _("       In every other constellation the '%' is removed!\n\n")
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
      << ("   [Output]\n"
      "   Format=<a href=\")%N\" title=\"%c\">%n</a>|%t|%a|%D\n"
      "   Title=File|Title|Author|Date\n"
      "   TextForNewFiles=<img src=../images/new.gif>\n"
      "   MaxAgeForNewFiles=15\n"
      "   DirSeparatorText=%eListing of %n%s\n"
      "   Style=HTML\n\n")
      << _("Currently supported files are:")
      << ("\n  - HTML (*.html, *.htm, *.shtml, *.shtm, *.sht, *.php)\n"
      "  - JPEG (*.jpeg, *.jpg)\n"
      "  - MP3 (*.mp3)\n"
      "  - PDF (*.pdf)\n"
      "  - StarOffice (Write (*.sdw), Calc (*.sdc), Impress (*.sdd) & Draw (*.sda))\n"
      "  - Microsoft Office (WinWord (*.doc), Excel (*.xls) & Powerpoint (*.ppt))\n");
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
         error.replace (error.find ("%1"), 2, 1, 's');
         std::cerr << PACKAGE << error;
         break; } }
      // Don't add a break in OK case, as -s implies -r!

   case 'r': options |= RECURSIVE; break;

   case 'o': {
      const char* pType = getOptionValue ();
      if (!pType
          || ((outputStyle = HTML, strcmp (pType, "HTML"))
              && (outputStyle = TEXT, strcmp (pType, "text"))
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
         error.replace (error.find ("%1"), 2, 1, 'F');
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'T': {
      const char* pTitle = getOptionValue ();
      if (pTitle)
         iniOpts.title = pTitle;
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, 'T');
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'n': {
      const char* pNew = getOptionValue ();
      char* pEnd = NULL;
      unsigned int time (0);
      if (!pNew) {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, 'n');
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
         YGP::PathSearch list (filelist);
         while (!(node = list.getNextNode ()).empty ()) {
            filelist += option;
            filelist += node;
            filelist += YGP::PathSearch::PATHSEPARATOR;
         }
      }
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, 'x');
         std::cerr << PACKAGE << error;
      }
      break; }

   case 'a': options |= SHOW_ALL; break;

   case 'f': {
      const char* pFile = getOptionValue ();
      if (pFile)
         readINIFile (pFile);
      else {
         std::string error (_("-warning: Option `%1' needs an argument! Ignoring option!\n"));
         error.replace (error.find ("%1"), 2, 1, 'f');
         std::cerr << PACKAGE << error;
      }
      break; }

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

   if ((outputStyle == XML) && !(chgFlag & 1)) {
      iniOpts.format = DEFAULT_XML_FORMAT;
   }

   Check3 (iniOpts.format.size ());

   typedef Writer* (*CREATEWRITER) (const std::string&, const std::string&,
                                    unsigned long);
   static struct {
      int opt;
      CREATEWRITER fnc;
   } t[] = { { TEXT, (CREATEWRITER)&TextWriter::create },
             { HTML, (CREATEWRITER)&HTMLWriter::create },
             { LATEX, (CREATEWRITER)&LaTeXWriter::create },
             { XML, (CREATEWRITER)&XMLWriter::create } };

   for (unsigned int i (0); i < (sizeof (t) / sizeof (t[0])); ++i)
      if (outputStyle == t[i].opt)
         writer = t[i].fnc (iniOpts.format, iniOpts.newText, iniOpts.ageOfNewFiles);
   Check3 (writer);

   writer->printStart (std::cout, iniOpts.title);

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

   writer->printEnd (std::cout);
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

   YGP::ExtDirectorySearch ds (pFile);
   std::string node;
   YGP::PathSearch list (filelist);
   while (!(node = list.getNextNode ()).empty ()) {
      bool include (node[0] == 'i');
      node.replace (0, 1, 0, '\0');
      include ? ds.addFilesToInclude (node) : ds.addFilesToExclude (node);
   } // end-while

   const YGP::File* file = ds.find (YGP::IDirectorySearch::FILE_NORMAL);
   while (file) {
      HANDLER fnc = getFileTypeHandler (strrchr (file->name (), '.'));
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
            catch (std::string& err) {
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
   // Wait for threads to terminte
   while (true) {
      LOCKTHREADS
      if (aThreads.empty ()) {
         UNLOCKTHREADS
         break;
      }
      TRACE9 ("Application::handleFiles (const char*) - Wait for thread "
              << aThreads[0]->getID ());
      unsigned long id (aThreads[0]->getID ());
      UNLOCKTHREADS
      YGP::Thread::waitForThread (id);
   }
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
         Check3 (file.fnc == getFileTypeHandler (strrchr (file.name (), '.')));
         processFile (file, file.fnc);
      }
      else {
         UNLOCKFILES
         break;
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
         (this->*fnc) ((YGP::Xistream&)ifile, prop);
         convertFromWideChar (prop);
         LOCKOUTPUT
         writer->printFile (std::cout, file, prop);
         UNLOCKOUTPUT
      }
      catch (std::string& err) {
         LOCKOUTPUT;
         std::cerr << PACKAGE << _("-error: ") << err.c_str () << '\n';
         err = ((options & SHOW_ERRORS)
                ? std::string (_("Error while processing: ")) + err
                : "");
         writer->printMessage (std::cout, file, err);
         UNLOCKOUTPUT
      } // end-catch
   } // end-else file could be opened
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a HTML-document
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processHTML (YGP::Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseHTML obj;
   obj.parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a PDF document
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processPDF (YGP::Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParsePDF::parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a MP3 file
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processMP3 (YGP::Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseMP3::parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a StarOffice document
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processStarOffice (YGP::Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseStarOffice obj;
   obj.parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a OpenOffice document
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processOpenOffice (YGP::Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseOpenOffice obj;
   obj.parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a RTF-document
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processRTF (YGP::Xistream& hFile, Properties& result) const
   throw (std::string) {
   TRACE9 ("Parsing RTF");
   ParseRTF obj;
   obj.parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a MS-office document
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processOffice (YGP::Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseWord obj;
   obj.parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Tries to extract the properties of a JPEG image
/// \param hFile: File to processs
/// \param result: Result of parsing
//-----------------------------------------------------------------------------
void Application::processJPG (YGP::Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseJPEG obj;
   obj.parse (hFile, result);
}

//-----------------------------------------------------------------------------
/// Returns a handling function to a filetype
/// \param pExt: Pointer to file extensions
/// \returns \c HANDLER: Method to handle this filetype; NULL in case of error
//-----------------------------------------------------------------------------
Application::HANDLER Application::getFileTypeHandler (const char* pExt) const {
   if (pExt && *pExt++) {
      handlerMap::const_iterator i (handlers.find (pExt));
      if (i != handlers.end ())
         return i->second;
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
   try {
      INIFILE (pFile);
      INIOBJ (iniOpts, Output);

      INIFILE_READ ();
   }
   catch (std::string& err) {
      TRACE1 (err);
   }

   if (iniOpts.style.size ()) {
      if (iniOpts.style == "HTML")
         outputStyle = HTML;
      else if (iniOpts.style == "LaTeX")
         outputStyle = LATEX;
      else if (iniOpts.style == "XML")
         outputStyle = XML;
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
}


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
