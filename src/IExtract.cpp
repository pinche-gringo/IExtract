//$Id$

//PROJECT     : Extract
//SUBSYSTEM   : Extract
//REFERENCES  :
//TODO        :
//BUGS        :
//REVISION    : $Revision$
//AUTHOR      : Markus Schwab
//CREATED     : 10.08.2002
//COPYRIGHT   : Anticopyright (A) 2002

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

#include <gzo-cfg.h>

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <mcheck.h>
static int x = (mtrace (), 0);

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


#include <Trace_.h>
#include <XStream.h>
#include <DirSrch.h>
#include <XDirSrch.h>
#include <PathSrch.h>
#include <IVIOAppl.h>
#include <INIFile.h>

#include "Writer.h"
#include "Options.h"
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


#define DEFAULT_FORMAT "%n|-|%t|%a|%c|%d"


// Class to run Extract-Application
class Application : public IVIOApplication {
 public:
   Application (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo), options (0), outputStyle (TEXT)
      , ageOfNewFiles (30 * 24 * 60 * 60), iniOpts ()
#ifdef ENABLE_THREADS
      , listFiles (), mxListFiles (), aThreads (0), mxThreads (), mxOutput ()
#endif
      , writer (NULL) {
      iniOpts.format = DEFAULT_FORMAT;
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
   virtual const char* description () const
      { return PACKAGE " V" VERSION "." MICRO_VERSION " - Compiled on " __DATE__ " - " __TIME__
               "\n\nAuthor: Markus Schwab; e-Mail: g17m0@lycos.com"
               "\nDistributed under the terms of the GNU General Public License"; }

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

   void processJPG (Xistream& hFile, Properties& result) const throw (std::string);
   void processHTML (Xistream& hFile, Properties& result) const throw (std::string);
   void processOffice (Xistream& hFile, Properties& result) const
      throw (std::string);
   void processStarOffice (Xistream& hFile, Properties& result) const
      throw (std::string);

   enum { RECURSIVE = 0x1, SHOW_ALL = 0x2, SHOW_ERRORS = 0x4 };

   unsigned long ageOfNewFiles;
   unsigned int options;

   Options iniOpts;

   Writer* writer;

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

   string filelist;

   enum { TEXT = 0, HTML, LATEX } outputStyle;

   static const longOptions lo[];
};


// TODO: If this table gets bigger change it to std::map!
const Application::FILEHANDLERS Application::handlers[] = {
   { "htm", &Application::processHTML },
   { "html", &Application::processHTML },
   { "shtm", &Application::processHTML },
   { "shtml", &Application::processHTML },
   { "sdw", &Application::processStarOffice },
   { "sdc", &Application::processStarOffice },
   { "sdd", &Application::processStarOffice },
   { "sda", &Application::processStarOffice },
   { "jpg", &Application::processJPG },
   { "jpeg", &Application::processJPG },
   { "doc", &Application::processOffice },
   { "xls", &Application::processOffice },
   { "ppt", &Application::processOffice } };


const IVIOApplication::longOptions Application::lo[] = {
   { IVIOAPPL_HELP_OPTION },
   { "recursive", 'r' },
   { "format", 'f' },
   { "title", 'T' },
   { "threads", 't' },
   { "include", 'i' },
   { "exclude", 'x' },
   { "show-errors", 'e' },
   { "all", 'a' },
   { "new", 'n' },
   { "ini-file", 'I' },
   { "version", 'V' },
   { "output", 'o' },
   { NULL, '\0' } };


/*--------------------------------------------------------------------------*/
//Purpose   : Displays the help
/*--------------------------------------------------------------------------*/
void Application::showHelp () const {
   std::cout << "Extracts a description out of files (depending on the file-type)"
                "\n\nUsage: "
             << PACKAGE " [OPTIONS] <File(s)>\n\n"
                "  -r, --recursive ....... Recurse into subdirectories\n"
                "  -o, --output=STYLE .... Sets the output-style (text, HTML or LaTeX)\n"
                "  -f, --format=FORMAT ... Format of output (default: " DEFAULT_FORMAT "\n"
                "  -T, --title=TITLE ..... Title of output\n"
                "  -e, --show-errors ..... Puts error messages (additionally) into output\n"
                "  -a, --all ............. Show all files (including unknown types) in output\n"
#ifdef ENABLE_THREADS
                "  -t, --threads=NR ...... Number of threads for examining files (default: 1)\n"
#endif
                "  -n, --new=TIME:TEXT ... Show TEXT for files younger than TIME days (def: 30)\n"
                "  -i, --include=LIST .... Files to inspect\n"
                "  -x, --exclude=LIST .... Files to not inspect\n"
                "  -I, --ini-file=FILE ... Read further options from specified file\n"
                "  -V, --version ......... Output version information and exit\n"
                "  -h, -?, --help ........ Displays this help and exit\n"
                "  File(s) ... Files to analyze (the last part can contain wildcards)\n\n"
                "TIME (in option -n) may be omited or may have an multiplier suffix: m for 30.\n\n"
                "LIST is a list of files; seperated with the path-separator of the operating\n"
                "     system (':' for UNICES, ';' for Windows). E.g. *.html"
             << PathSearch::PATHSEPARATOR << "*.doc\n\n"
                "FORMAT specifies how to print the entries;\n"
                "       %a is substituted with the author\n"
                "       %c is substituted with the comment\n"
                "       %d is substituted with the modification time of the file\n"
                "       %D is substituted with the modification time of the file (day only)\n"
                "       %n is substituted with the name of the file\n"
                "       %N is substituted with path and name of the file\n"
                "       %p is substituted with the path of the file\n"
                "       %t is substituted with the title\n"
                "       %(LETTERS) is substituted with first of the above substitutions\n"
                "          producing a non-empty string (e.g. %(nt) is the filename if not \n"
                "          empty or else the title.)\n\n"
                "TITLE specifies the headers for the output; seperateod with (|); columns must\n"
                "      contain at least one character\n\n"
                "The format of the INI file is like this (entries can be missing):\n\n"
                "   [Output]\n"
                "   Format=<a href=\"%N\" title=\"%c\">%n</a>|%t|%a|%D\n"
                "   Title=File|Title|Author|Date\n"
                "   TextForNewFiles=-n9:<img src=../images/new.gif>\n\n"
                "Currently supported files are:\n"
                "  - HTML (*.html, *.htm, *.shtml, *.shtm)\n"
                "  - JPEG (*.jpeg, *.jpg)\n"
                "  - StarOffice (Write (*.sdw), Calc (*.sdc), Impress (*.sdd) & Draw (*.sda))\n"
                "  - Microsoft Office (WinWord (*.doc), Excel (*.xls) & Powerpoint (*.ppt))\n";
}

/*--------------------------------------------------------------------------*/
//Purpose   : Checks the validity of the passed option
//Parameters: option: Actual option
//Returns   : bool: Status; false: Invalid option/option-value
//Require   : option not '\0´'
/*--------------------------------------------------------------------------*/
bool Application::handleOption (const char option) {
   assert (option != '\0');

   switch (option) {
   case 'r': options |= RECURSIVE; break;

   case 'o': {
      const char* pType = getOptionValue ();
      if (!pType
          || ((outputStyle = HTML, strcmp (pType, "HTML"))
              && (outputStyle = TEXT, strcmp (pType, "text"))
              && (outputStyle = LATEX, strcmp (pType, "LaTeX")))) {
         outputStyle = TEXT;
         cerr << PACKAGE "-warning: Style of output " << pType << " is not "
                 "valid! Using text\n";
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
         cerr << PACKAGE "-warning: Invalid number of threads!\n";
      }
      else
         aThreads.reserve (cThreads);
      break; }
#endif

   case 'e': options |= SHOW_ERRORS; break;

   case 'f': iniOpts.format = getOptionValue (); break;

   case 'T': iniOpts.title = getOptionValue (); break;

   case 'n': {
      const char* pNew = getOptionValue ();
      char* pEnd = NULL;
      unsigned int time (0);
      if (!pNew
          || ((time = strtoul (pNew, &pEnd, 10)),
              (!pEnd || ((*pEnd != ':') && (*pEnd != 'm')))))
         cerr << PACKAGE "-warning: Argument for new files " << pNew << " is not"
                 " valid! Ignoring option\n";
      else {
         if (*pEnd == 'm') {
            ++pEnd;
            time *= 30;
         }
         if (time)
            ageOfNewFiles = time * 24 * 60 * 60;
         iniOpts.newText = pEnd + 1;
      }

      break; }

   case 'x':
   case 'i':  {
      const char* files = getOptionValue ();
      filelist += option;
      filelist += files;
      filelist += PathSearch::PATHSEPARATOR;
      break; }

   case 'a': options |= SHOW_ALL; break;

   case 'I': {
      const char* pFile = getOptionValue ();
      if (pFile)
         readINIFile (pFile);
      else
         cerr << PACKAGE "-warning: No file specified! Ignoring option\n";
      break; }

   case 'V': std::cout << description () << '\n'; exit (0);

   default:
      std::cerr << PACKAGE "-warning: Ignoring invalid option '"
                << option << "'\n";
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

   assert (iniOpts.format.size ());

   typedef Writer* (*CREATEWRITER) (const char*, unsigned long, const char*);
   static struct {
      unsigned int opt;
      CREATEWRITER fnc;
   } t[] = { { TEXT, (CREATEWRITER)&TextWriter::create },
             { HTML, (CREATEWRITER)&HTMLWriter::create },
             { LATEX, (CREATEWRITER)&LaTeXWriter::create } };

   for (unsigned int i (0); i < (sizeof (t) / sizeof (t[0])); ++i)
      if (outputStyle == t[i].opt) {
         writer = t[i].fnc (iniOpts.format.c_str (), ageOfNewFiles,
                            iniOpts.newText.size () ? iniOpts.newText.c_str () : NULL);
      }
   assert (writer);

   writer->printStart (cout, iniOpts.title.size () ? iniOpts.title.c_str () : NULL);

   std::string file;
   for (unsigned int j (0); j < argc; ++j) {
      file = argv[j];
      if (DirectorySearch::isValid (argv[j])) {
         file += File::DIRSEPARATOR;
         file += "*";
      }
      handleFiles (file.c_str ());
   }

   writer->printEnd (cout);
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Expands the filespecification and processes every file
//Parameters: pFile: Filespecification; may contain wildcards
//Requires  : pFile not NULL
/*--------------------------------------------------------------------------*/
void Application::handleFiles (const char* pFile) const {
   assert (pFile);
   TRACE5 ("Application::handleFiles (const char*) const - " << pFile);

   XDirSrch ds (pFile);
   string node;
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
               cerr << PACKAGE "-error: " << err << '\n';
            }
         UNLOCKTHREADS
#else
         processFile (*file, fnc);
#endif
      } // endif handler found
      else
         if (options & SHOW_ALL) {
            LOCKOUTPUT
            writer->printMessage (cout, *file,
                                 (options & SHOW_ERRORS) ? "Unknown file-type" : "");
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
         if (IDirectorySearch::isSpecial (file->name ())) {
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
   assert (pThread);
   FILEFNC file;

   while (true) {
      LOCKFILES
      if (listFiles.size ()) {
         file = listFiles.front ();
         listFiles.pop ();
         UNLOCKFILES
         TRACE1 ("Application::processThread (void*) - File " << file.name ()
                  << "; Remaining: " << listFiles.size ());
         assert (file.fnc);
         assert (file.fnc == getFileTypeHandler (strrchr (file.name (), '.')));
         processFile (file, file.fnc);
      }
      else {
         UNLOCKFILES
         break;
      }
   } // end-while

   LOCKTHREADS
   assert (find (aThreads.begin (), aThreads.end (), pThread));
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
   ifile.open (strFile.c_str (), ios::in | ios::binary);
   if (!ifile) {
      LOCKOUTPUT
      std::cerr << PACKAGE "-error: File " << strFile.c_str ()
               << " can't be opened!\nReason: ";
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
         writer->printFile (cout, file, prop);
         UNLOCKOUTPUT
      }
      catch (std::string& err) {
         LOCKOUTPUT
         std::cerr << PACKAGE "-error: " << err.c_str () << '\n';
         writer->printMessage (cout, file,
                              (options & SHOW_ERRORS) ? "Error while processing" : "");
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
   static string Properties::* values[] = { &Properties::strTitle,
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
   assert (pFile);

   std::string Style;
   try {
      INIFILE (pFile);
      INIOBJ (iniOpts, Output);

      unsigned int rc (INIFILE_READ ());
   }
   catch (std::string& error) {
      std::cerr << PACKAGE "-error: Can't read INI-file '" << pFile
                << "'\nReason: " << error << '\n';
   }

   if (iniOpts.style.size ()) {
      if (iniOpts.style == "HTML")
         outputStyle = HTML;
      else if (iniOpts.style == "text")
         outputStyle = TEXT;
      else if (iniOpts.style == "LaTeX")
         outputStyle = LATEX;
      else
         cerr << PACKAGE "-warning: The INI-file '" << pFile << "' contains an "
                 "invalid entry for the output style ('" << iniOpts.style
              << "')! Using text\n";
   }
}


/*--------------------------------------------------------------------------*/
//Purpose   : Entrypoint of application
//Parameters: argc: Anzahl der Parameter
//            argv: Array mit Zeigern auf Parameter
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
int main (int argc, const char* argv[]) {
   Application::initI18n ();
   Application appl (argc, argv);
   return appl.run ();
}
