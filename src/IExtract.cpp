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

#undef  ENABLE_THREADS
#define ENABLE_THREADS

#include <gzo-cfg.h>

#include <assert.h>
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

#include <Trace_.h>
#include <XStream.h>
#include <DirSrch.h>
#include <XDirSrch.h>
#include <PathSrch.h>
#include <IVIOAppl.h>

#include "Writer.h"
#include "ParseJPG.h"
#include "ParseHTML.h"
#include "ParseWord.h"
#include "Properties.h"

#if SYSTEM == UNIX
#  include <unistd.h>
#elif SYSTEM == WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#  define strcasecmp stricmp
#  define sleep      Sleep
#endif


// Class to run Extract-Application
class Application : public IVIOApplication {
 public:
   Application (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo), options (0), outputStyle (TEXT)
      , ageOfNewFiles (30 * 24 * 60 * 60), pTextForNewFiles (NULL)
#ifdef ENABLE_THREADS
      , listFiles (), mxListFiles (), aThreads (0), mxThreads (), mxOutput ()
#endif
      , showOptions (0), writer (NULL) {
#ifdef ENABLE_THREADS
      aThreads.reserve (1);
#endif
  }
  ~Application () { }

 protected:
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

   void handleFiles (const char* pFile) const;
   void processFile (const File& file) const;
#ifdef ENABLE_THREADS
   void* processThread (void*);
#endif

   void processJPG (Xistream& hFile, Properties& result) const throw (std::string);
   void processHTML (Xistream& hFile, Properties& result) const throw (std::string);
   void processOffice (Xistream& hFile, Properties& result) const
      throw (std::string);

   enum { RECURSIVE = 0x1, SHOW_ALL = 0x2, SHOW_ERRORS = 0x4 };
   unsigned int showOptions;
   unsigned long ageOfNewFiles;
   const char* pTextForNewFiles;
   unsigned int options;

   Writer* writer;

#ifdef ENABLE_THREADS
   vector<Thread*> aThreads;
   Mutex           mxThreads;

   Mutex           mxOutput;

   Mutex        mxListFiles;
   queue<File>  listFiles;
#endif

   string filelist;

   enum { TEXT = 0, HTML } outputStyle;

   static const longOptions lo[];
};


const Application::FILEHANDLERS Application::handlers[] = {
   { "jpg", &Application::processJPG },
   { "jpeg", &Application::processJPG },
   { "htm", &Application::processHTML },
   { "html", &Application::processHTML },
   { "shtm", &Application::processHTML },
   { "shtml", &Application::processHTML },
   { "doc", &Application::processOffice },
   { "xls", &Application::processOffice },
   { "ppt", &Application::processOffice } };


const IVIOApplication::longOptions Application::lo[] = {
   { "help", 'h' },
   { "recursive", 'r' },
   { "threads", 't' },
   { "include", 'i' },
   { "exclude", 'x' },
   { "show-errors", 'e' },
   { "show-path", 'p' },
   { "all", 'a' },
   { "new", 'n' },
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
                "  -o, --output=STYLE .... Sets the output-style (text or HTML)\n"
                "  -e, --show-errors ..... Puts error messages (additionally) into output\n"
                "  -p, --show-path ....... Print path for files in output\n"
                "  -a, --all ............. Show all files (including unknown types) in output\n"
#ifdef ENABLE_THREADS
                "  -t, --threads=NR ...... Number of threads for examining files (default: 0)\n"
#endif
                "  -n, --new=TIME:TEXT ... Show TEXT for files younger than TIME days (def: 30)\n"
                "  -i, --include=NODES ... Node of files to inspect\n"
                "  -x, --exclude=NODES ... Node of files to not inspect\n"
                "  -V, --version ......... Output version information and exit\n"
                "  -h, -?, --help ........ Displays this help and exit\n"
                "  File(s) ... File to analyze (the last part can contain wildcards)\n\n"
                "TIME (in option -n) may be omited or may have an multiplier suffix: m for 30.\n\n"
                "NODES is a list of files; seperated with the path-separator of the operating\n"
                "      system (':' for UNICES, ';' for Windows)\n\n"
                "Currently supported files are: HTML, JPEG, WinWord, Excel & Powerpoint\n";
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
              && (outputStyle = TEXT, strcmp (pType, "text"))))
         cerr << PACKAGE "-warning: Style of output " << pType << " is not "
                 "valid! Using text\n";
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

   case 'p': showOptions |= Writer::SHOW_PATH; break;

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
         pTextForNewFiles = pEnd + 1;
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

   typedef Writer* (*CREATEWRITER) (unsigned int, unsigned long, const char*);
   static struct {
      unsigned int opt;
      CREATEWRITER fnc;
   } t[] = { { TEXT, (CREATEWRITER)&TextWriter::create },
             { HTML, (CREATEWRITER)&HTMLWriter::create } };

   for (unsigned int i (0); i < (sizeof (t) / sizeof (t[0])); ++i)
      if (outputStyle == t[i].opt) {
         writer = t[i].fnc (showOptions, ageOfNewFiles, pTextForNewFiles);
      }
   assert (writer);

   writer->printStart (cout);

   std::string file;
   for (unsigned int j (0); j < argc; ++j) {
      file = argv[j];
      if (DirectorySearch::isValid (argv[j])) {
         file += File::DIRSEPARATOR;
         file += "*";
      }
      handleFiles (file.c_str ());
   }

   // Wait for threads to terminte
   while (true) {
      LOCKTHREADS
      if (aThreads.empty ()) {
         UNLOCKTHREADS
         break;
      }
      if(aThreads[0])
         Thread::waitForThread (*aThreads[0]);
      else
         aThreads.erase (aThreads.begin ());
      UNLOCKTHREADS
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
#ifdef ENABLE_THREADS
      LOCKFILES
      while (listFiles.size () > 100) {
         UNLOCKFILES
         sleep (0);
         LOCKFILES
      }
      ((Application*)this)->listFiles.push (*file);
      UNLOCKFILES
      LOCKTHREADS
      bool newThread (aThreads.size () < aThreads.capacity ());
      if (newThread)
         ((Application*)this)->aThreads.push_back (
            OThread<Application>::create2 ((Application*)this,
                                             &Application::processThread, NULL));
      UNLOCKTHREADS
#else
      processFile (*file);
#endif
      file = ds.next ();
   } // end-while

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
   TRACE1 ("Application::processThread (void*) - " << ((Thread*)pThread)->getID ());
   File file;

   while (true) {
      LOCKFILES
      if (listFiles.size ()) {
         file = listFiles.front ();
         TRACE1 ("Application::processThread (void*) - File " << file.name ()
                  << "; Remaining: " << listFiles.size ());
         listFiles.pop ();
         UNLOCKFILES
         processFile (file);
      }
      else {
         UNLOCKFILES
         break;
      }
   } // end-while

   TRACE9 ("Thread ends!");
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
/*--------------------------------------------------------------------------*/
void Application::processFile (const File& file) const {
   TRACE1 ("Application::processFile (const File&) const - " << file.name ());

   HANDLER fnc = getFileTypeHandler (strrchr (file.name (), '.'));
   if (fnc) {
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
   else
      if (options & SHOW_ALL) {
         LOCKOUTPUT
         writer->printMessage (cout, file,
                               (options & SHOW_ERRORS) ? "Unknown file-type" : "");
         UNLOCKOUTPUT
      }
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
//Purpose   : Entrypoint of application
//Parameters: argc: Anzahl der Parameter
//            argv: Array mit Zeigern auf Parameter
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
int main (int argc, const char* argv[]) {
   Application appl (argc, argv);
   return appl.run ();
}
