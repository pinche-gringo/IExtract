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

#include <assert.h>
#include <string.h>

#include <string>

#include <XStream.h>
#include <DirSrch.h>
#include <IVIOAppl.h>

#include "Writer.h"
#include "ParseHTML.h"
#include "ParseWord.h"

#if SYSTEM == WINDOWS
#  define strcmp stricmp
#endif

// Class to run Extract-Application
class Application : public IVIOApplication {
 public:
   Application (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo), options (0), outputStyle (TEXT)
      , showOptions (0) { }
  ~Application () { }

 protected:
   virtual bool handleOption (const char option);

   // Program-handling
   virtual bool shallShowInfo () const { return false; }
   virtual int         perform (int argc, const char* argv[]);
   virtual const char* name () const { return PACKAGE; }
   virtual const char* description () const
      { return (options & VERBOSE) ?
                PACKAGE " V" VERSION "." MICRO_VERSION " - Compiled on " __DATE__ " - " __TIME__
                "\nAuthor: Markus Schwab; e-Mail: g17m0@lycos.com"
                "\nDistributed under the terms of the GNU General Public License"
              : PACKAGE " V" VERSION; }

   // Help-handling
   virtual void showHelp () const;

 private:
   // Prohobited manager functions
   Application ();
   Application (const Application&);
   const Application& operator= (const Application&);

   void handleFiles (Writer& writer, const char* pFile) const;

   std::string processHTML (Xistream& hFile) const throw (std::string);
   std::string processOffice (Xistream& hFile) const throw (std::string);

   enum { RECURSIVE = 0x1, VERBOSE = 0x2, SHOW_ALL = 0x4, SHOW_ERRORS = 0x8 };
   unsigned int showOptions;
   unsigned int options;

   enum { TEXT = 0, HTML } outputStyle;

   typedef std::string (Application::*HANDLER) (Xistream& hFile) const;
   typedef struct {
      const char* pExt;
      HANDLER     pFnc; } FILEHANDLERS;
   HANDLER getFileTypeHandler (const char* pExt) const;

   static const FILEHANDLERS handlers[];

   static const longOptions lo[];
};


const Application::FILEHANDLERS Application::handlers[] = {
   { "htm", &Application::processHTML },
   { "html", &Application::processHTML },
   { "shtm", &Application::processHTML },
   { "shtml", &Application::processHTML },
   { "doc", &Application::processOffice },
   { "xls", &Application::processOffice },
   { "ppt", &Application::processOffice} };


const IVIOApplication::longOptions Application::lo[] = {
   { "help", 'h' },
   { "recursive", 'r' },
   { "show-errors", 'e' },
   { "show-path", 'p' },
   { "all", 'a' },
   { "verbose", 'v' },
   { "version", 'V' },
   { "output", 'o' },
   { NULL, '\0' } };


/*--------------------------------------------------------------------------*/
//Purpose   : Displays the help
/*--------------------------------------------------------------------------*/
void Application::showHelp () const {
   std::cout << "Extracts (depending on the file-type) a description out of files"
                "\n\nUsage: "
             << PACKAGE " [OPTIONS] <File(s)>\n\n"
                "  -r, --recursive ...... Recurse into subdirectories\n"
                "  -o, --output=STYLE ... Sets the output-style (text or HTML)\n"
                "  -e, --show-errors .... Puts error messages (additionally) into output\n"
                "  -p, --show-path ...... Print path for files in output\n"
                "  -a, --all ............ Show all files (including unknown types) in output\n"
                "  -v, --verbose, ....... Displays the processed files (be verbose)\n"
                "  -V, --version ........ Output version information and exit\n"
                "  -h, -?, --help ....... Displays this help and exit\n"
                "  File(s) ... File to analyze\n\n"
                "Currently supported documents are: HTML, WinWord, Excel & Powerpoint\n";
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
   case 's': options |= RECURSIVE; break;

   case 'o': {
      const char* pType = getOptionValue ();
      if (!pType
          || ((outputStyle = HTML, strcmp (pType, "HTML"))
              && (outputStyle = TEXT, strcmp (pType, "text")))) {
         cerr << PACKAGE "-warning: Style of output " << pType << " is not"
                 "valid! Using text\n";
      }
      break; }

   case 'e': options |= SHOW_ERRORS; break;

   case 'p': showOptions |= Writer::SHOW_PATH; break;

   case 'v': options |= VERBOSE; break;

   case 'a': options |= SHOW_ALL; break;

   case 'V': options |= VERBOSE; std::cout << description () << '\n'; exit (0);

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

   typedef Writer* (*CREATEWRITER) (unsigned int);
   static struct {
      unsigned int opt;
      CREATEWRITER fnc;
   } t[] = { { TEXT, (CREATEWRITER)&TextWriter::create },
             { HTML, (CREATEWRITER)&HTMLWriter::create } };

   Writer* writer = NULL;
   for (unsigned int i (0); i < (sizeof (t) / sizeof (t[0])); ++i)
      if (outputStyle == t[i].opt) {
         writer = t[i].fnc (showOptions);
      }
   assert (writer);

   writer->printStart (cout);

   for (unsigned int j (0); j < argc; ++j)
      handleFiles (*writer, argv[j]);

   writer->printEnd (cout);
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Expands the filespecification and processes every file
//Parameters: writer: Class to use for output
//            pFile: Filespecification; may contain wildcards
//Requires  : pFile not NULL
/*--------------------------------------------------------------------------*/
void Application::handleFiles (Writer& writer, const char* pFile) const {
   assert (pFile);
   if (options & VERBOSE) {
      std::cout << "Handling file(s) " << pFile << '\n'; std::cout.flush (); }

   DirectorySearch ds (pFile);
   const File* file = ds.find ((options & RECURSIVE)
                               ? (IDirectorySearch::FILE_NORMAL
                                  | IDirectorySearch::FILE_DIRECTORY)
                               : IDirectorySearch::FILE_NORMAL);
   while (file) {
      if (options & VERBOSE) {
         std::cout << "Handling file " << file->name () << '\n'; std::cout.flush (); }

      std::string strFile (file->path ());
      strFile += file->name ();

      if (file->isDirectory ()) {
         if (strcmp (file->name (), ".") && strcmp (file->name (), "..")) {
            strFile += File::DIRSEPARATOR;
            strFile += "*";
            handleFiles (writer, strFile.c_str ());
         }
      }
      else {
         HANDLER fnc = getFileTypeHandler (strrchr (file->name (), '.'));
         if (fnc) {
            Xifstream ifile;
            ifile.open (strFile.c_str (), ios::in | ios::binary);
            if (!ifile) {
               std::cerr << PACKAGE "-error: File " << strFile.c_str ()
                         << " can't be opened!\nReason: ";
               perror ("");
            }
            else {
               ifile.init ();

               try {
                  writer.printFile (cout, *file, (this->*fnc) ((Xistream&)ifile));
               }
               catch (std::string& err) {
                  std::cerr << PACKAGE "-error: " << err.c_str ();
                  writer.printFile (cout, *file,
                                    ((options & SHOW_ERRORS)
                                     ? "Error while processing" : NULL));
               } // end-catch
            } // end-else file could be opened
         } // endif handler found
         else
            if (options & SHOW_ALL)
               writer.printFile (cout, *file,
                                 ((options & SHOW_ERRORS)
                                  ? "Unknown file-type" : NULL));
      }

      file = ds.next ();
   } // end-while
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract to title of a HTML-document
//Parameters: hFile: File to processs
//Returns   : std::string: Description
/*--------------------------------------------------------------------------*/
std::string Application::processHTML (Xistream& hFile) const throw (std::string) {
   if (options & VERBOSE)
      std::cout << "Processing HTML-file\n";

   ParseHTML obj;
   return obj.parse (hFile);
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract to title of a MS-office document
//Parameters: hFile: File to processs
//Returns   : std::string: Description
/*--------------------------------------------------------------------------*/
std::string Application::processOffice (Xistream& hFile) const throw (std::string) {
   if (options & VERBOSE)
      std::cout << "Processing MS Office document\n";

   ParseWord obj;
   return obj.parse (hFile);
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
         if (!strcmp (handlers[i].pExt, pExt))
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
