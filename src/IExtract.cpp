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
#include <gzo-cfg.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#ifdef ENABLE_THREADS
#  include <queue>
#  include <vector>
#  include <algorithm>

#  include <Mutex.h>
#  include <Thread.h>
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

#include <Check.h>
#include <Trace_.h>
#include <XStream.h>
#include <DirSrch.h>
#include <XDirSrch.h>
#include <PathSrch.h>
#include <IVIOAppl.h>
#include <INIFile.h>

#include "Writer.h"
#include "Options.h"
#include "ParseMP3.h"
#include "ParsePDF.h"
#include "ParseJPG.h"
#include "ParseHTML.h"
#include "ParseWord.h"
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


static const char* const DEFAULT_FORMAT ("%n|-|%t|%a|%c|%d");
static const char* const DEFAULT_XML_FORMAT ("<tr><td>%n</td><td>%t</td>"
                                             "<td>%a</td><td>%c</td><td>%d</td></tr>");


// Class to run Extract-Application
class Application : public IVIOApplication {
 public:
   Application (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo), options (0), chgFlag (0), iniOpts ()
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
  }
  ~Application () { }

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

   typedef void (Application::*HANDLER) (Xistream& hFile, Properties& result) const;
   typedef struct {
      const char* pExt;
      HANDLER     pFnc; } FILEHANDLERS;
   HANDLER getFileTypeHandler (const char* pExt) const;
   static const FILEHANDLERS handlers[];

   static void convertFromUnicode (Properties& prop);

   void handleFiles (const char* pFile) const;
   void processFile (const File& file, HANDLER fnc) const;
#ifdef ENABLE_THREADS
   void* processThread (void*);
#endif

   void processMP3 (Xistream& hFile, Properties& result) const throw (std::string);
   void processPDF (Xistream& hFile, Properties& result) const throw (std::string);
   void processJPG (Xistream& hFile, Properties& result) const throw (std::string);
   void processHTML (Xistream& hFile, Properties& result) const throw (std::string);
   void processOffice (Xistream& hFile, Properties& result) const
      throw (std::string);
   void processStarOffice (Xistream& hFile, Properties& result) const
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
   vector<Thread*> aThreads;
   Mutex           mxThreads;
   Mutex           mxOutput;

   typedef struct FileFunction : public File {
      HANDLER fnc;
      FileFunction () : File (), fnc (NULL) { }
      FileFunction (const struct File& file) : File (file) { }
      FileFunction (const struct FileFunction& ffnc) : File (ffnc)
         , fnc (ffnc.fnc) { }

      const struct FileFunction& operator= (const struct FileFunction& ffnc) {
         if (this != &ffnc) {
            File::operator= (ffnc);
            fnc = ffnc.fnc;
         }
         return *this; }
   } FILEFNC;
   Mutex           mxListFiles;
   queue<FILEFNC>  listFiles;
#endif
};


// TODO: If this table gets bigger change it to std::map!
const Application::FILEHANDLERS Application::handlers[] = {
   { "doc", &Application::processOffice },
   { "htm", &Application::processHTML },
   { "html", &Application::processHTML },
   { "jpeg", &Application::processJPG },
   { "jpg", &Application::processJPG },
   { "mp3", &Application::processMP3 },
   { "pdf", &Application::processPDF },
   { "php", &Application::processHTML },
   { "ppt", &Application::processOffice },
   { "sda", &Application::processStarOffice },
   { "sdc", &Application::processStarOffice },
   { "sdd", &Application::processStarOffice },
   { "sdw", &Application::processStarOffice },
   { "sht", &Application::processHTML },
   { "shtm", &Application::processHTML },
   { "shtml", &Application::processHTML },
   { "xls", &Application::processOffice } };


const IVIOApplication::longOptions Application::lo[] = {
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


/*--------------------------------------------------------------------------*/
//Purpose   : Displays the help
/*--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------*/
//Purpose   : Checks the validity of the passed option
//Parameters: option: Actual option
//Returns   : bool: Status; false: Invalid option/option-value
//Require   : option not '\0´'
/*--------------------------------------------------------------------------*/
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
         filelist += option;
         filelist += files;
         filelist += PathSearch::PATHSEPARATOR;
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

/*--------------------------------------------------------------------------*/
//Purpose   : Performs the job of the applications
//Parameters: argc: Number of parameters (without options)
//            argv: Array with pointer to arguments
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
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
      if (DirectorySearch::isValid (argv[j])) {
         if (file[file.size () - 1] != File::DIRSEPARATOR)
            file += File::DIRSEPARATOR;
         file += "*";
      }
      handleFiles (file.c_str ());
   }

   writer->printEnd (std::cout);
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Expands the filespecification and processes every file
//Parameters: pFile: Filespecification; may contain wildcards
//Requires  : pFile not NULL
/*--------------------------------------------------------------------------*/
void Application::handleFiles (const char* pFile) const {
   Check3 (pFile);
   TRACE5 ("Application::handleFiles (const char*) const - " << pFile);

   ExtDirectorySearch ds (pFile);
   std::string node;
   PathSearch list (filelist);
   while (!(node = list.getNextNode ()).empty ()) {
      bool include (node[0] == 'i');
      node.replace (0, 1, 0, '\0');
      include ? ds.addFilesToInclude (node) : ds.addFilesToExclude (node);
   } // end-while

   const File* file = ds.find (IDirectorySearch::FILE_NORMAL);
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
                  OThread<Application>::create2 ((Application*)this,
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
      Thread::waitForThread (id);
   }
#endif

   // Now handle subdirectories (if specified)
   if (options & RECURSIVE) {
      std::string files (ds.getFileSpec ());      // Use same filespecification

      file = ds.find (ds.getDirectory () + "*", DirectorySearch::FILE_DIRECTORY);
      while (file) {
         if (!IDirectorySearch::isSpecial (file->name ())) {
            LOCKOUTPUT
            writer->printSeparator (std::cout, *file, iniOpts.separate, iniOpts.title);
            UNLOCKOUTPUT
            std::string strFile (file->path ());
            strFile += file->name ();
            strFile += File::DIRSEPARATOR;
            strFile += files;
            handleFiles (strFile.c_str ());
         }
         file = ds.next ();
      }
   }
}

#ifdef ENABLE_THREADS
/*--------------------------------------------------------------------------*/
//Purpose   : Threadfunction to process files (as long as the filelist is full)
/*--------------------------------------------------------------------------*/
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

   ParseObject::freeBuffer ();

   LOCKTHREADS
   Check3 (find (aThreads.begin (), aThreads.end (), pThread));
   aThreads.erase (find (aThreads.begin (), aThreads.end (), pThread));
   UNLOCKTHREADS
   return NULL;
}
#endif

/*--------------------------------------------------------------------------*/
//Purpose   : Processes a single file with a known handler
//Parameters: pFile: File to processs
//            fnc: Handling function
/*--------------------------------------------------------------------------*/
void Application::processFile (const File& file, HANDLER fnc) const {
   TRACE1 ("Application::processFile (const File&) const - " << file.name ());

   std::string strFile (file.path ());
   strFile += file.name ();

   Xifstream ifile;
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
         (this->*fnc) ((Xistream&)ifile, prop);
         convertFromUnicode (prop);
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

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract the properties of a HTML-document
//Parameters: hFile: File to processs
//            result: Result of parsing
/*--------------------------------------------------------------------------*/
void Application::processHTML (Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseHTML obj;
   obj.parse (hFile, result);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract the properties of a PDF document
//Parameters: hFile: File to processs
//            result: Result of parsing
/*--------------------------------------------------------------------------*/
void Application::processPDF (Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParsePDF::parse (hFile, result);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract the properties of a MP3 file
//Parameters: hFile: File to processs
//            result: Result of parsing
/*--------------------------------------------------------------------------*/
void Application::processMP3 (Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseMP3::parse (hFile, result);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract the properties of a StarOffice document
//Parameters: hFile: File to processs
//            result: Result of parsing
/*--------------------------------------------------------------------------*/
void Application::processStarOffice (Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseStarOffice obj;
   obj.parse (hFile, result);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract the properties of a MS-office document
//Parameters: hFile: File to processs
//            result: Result of parsing
/*--------------------------------------------------------------------------*/
void Application::processOffice (Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseWord obj;
   obj.parse (hFile, result);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract the properties of a JPEG image
//Parameters: hFile: File to processs
//            result: Result of parsing
/*--------------------------------------------------------------------------*/
void Application::processJPG (Xistream& hFile, Properties& result) const
   throw (std::string) {
   ParseJPEG obj;
   obj.parse (hFile, result);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Returns a handling function to a filetype
//Parameters: pExt: Pointer to file extensions
//Returns   : HANDLER: Method to handle this filetype; NULL in case of error
/*--------------------------------------------------------------------------*/
Application::HANDLER Application::getFileTypeHandler (const char* pExt) const {
   if (pExt && *pExt++) {
      unsigned int i (0);
      for (; i < (sizeof (handlers) / sizeof (handlers[0])); ++i)
         if (!strcasecmp (handlers[i].pExt, pExt))
            return handlers[i].pFnc;
   }

   return NULL;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Converts Unicode characters to normal strings
//Parameters: prop: Properties to convert
/*--------------------------------------------------------------------------*/
void Application::convertFromUnicode (Properties& prop) {
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

/*--------------------------------------------------------------------------*/
//Purpose   : Reads the options of the INI-file
//Parameters: pFile: Pointer to filename
//Requieres : pFile not NULL
/*--------------------------------------------------------------------------*/
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


/*--------------------------------------------------------------------------*/
//Purpose   : Entrypoint of application
//Parameters: argc: Anzahl der Parameter
//            argv: Array mit Zeigern auf Parameter
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
int main (int argc, const char* argv[]) {
   IVIOApplication::initI18n (PACKAGE, LOCALEDIR),

   Application::initI18n ();
   Application appl (argc, argv);
   return appl.run ();
}
