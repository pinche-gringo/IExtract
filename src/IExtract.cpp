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

#define PACKAGE       "Extract"
#define VERSION       "0.1"
#define MICRO_VERSION "00"

#include <gzo-cfg.h>

#include <assert.h>
#include <string.h>

#include <string>

#include <XStream.h>
#include <DirSrch.h>
#include <IVIOAppl.h>

#include "ParseHTML.h"
#include "ParseWord.h"

#if SYSTEM == WINDOWS
#  define strcmp stricmp
#endif

// Class to run Extract-Application
class Application : public IVIOApplication {
 public:
   Application (const int argc, const char* argv[])
      : IVIOApplication (argc, argv, lo), verbose (false) { }
  ~Application () { }

 protected:
   virtual bool handleOption (const char option);

   // Program-handling
   virtual int         perform (int argc, const char* argv[]);
   virtual const char* name () const { return PACKAGE; }
   virtual const char* description () const
      { return verbose ?
                VERSION "." MICRO_VERSION " - Compiled on " __DATE__ " - " __TIME__
                "\nAuthor: Markus Schwab; e-Mail: g17m0@lycos.com"
                "\nDistributed under the terms of the GNU General Public License"
              : VERSION; }

   // Help-handling
   virtual void showHelp () const;

 private:
   // Prohobited manager functions
   Application ();
   Application (const Application&);
   const Application& operator= (const Application&);

   void handleFiles (const char* pFile) const;
   void showFile (const char* pFile, const char* pDesc) const;

   char* processHTML (Xistream& hFile) const throw (std::string);
   char* processOffice (Xistream& hFile) const throw (std::string);

   bool verbose;

   typedef char* (Application::*HANDLER) (Xistream& hFile) const throw (std::string);
   typedef struct {
      const char* pExt;
      HANDLER     pFnc; } FILEHANDLERS;

   static const FILEHANDLERS handlers[];

   static const longOptions lo[];
};


const Application::FILEHANDLERS Application::handlers[] = {
   { "htm", processHTML },
   { "html", processHTML },
   { "shtm", processHTML },
   { "shtml", processHTML },
   { "doc", processOffice },
   { "xls", processOffice },
   { "ppt", processOffice} };


const IVIOApplication::longOptions Application::lo[] = {
   { "help", 'h' },
   { "verbose", 'v' },
   { "version", 'V' },
   { NULL, '\0' } };


/*--------------------------------------------------------------------------*/
//Purpose   : Displays the help
/*--------------------------------------------------------------------------*/
void Application::showHelp () const {
   std::cout << "Extracts (depending on the file-type) a description out of files"
                "\n\nUsage: "
             << PACKAGE " <File(s)>\n\n"
                "  File(s) ... File to analyze\n";
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
   case 'v': verbose = true; break;

   case 'V': verbose = true; std::cout << description (); exit (0);

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

   for (unsigned int i (0); i < argc; ++i) {
      if (verbose) {
         std::cout << "Handling file(s) " << argv[i] << '\n'; std::cout.flush (); }
      handleFiles (argv[i]);
   }
   return 0;
}

/*--------------------------------------------------------------------------*/
//Purpose   : Performs the job of the applications
//Parameters: argc: Number of parameters (without options)
//            argv: Array with pointer to arguments
//Returns   : int: Status
/*--------------------------------------------------------------------------*/
void Application::handleFiles (const char* pFile) const {
   assert (pFile);

   DirectorySearch ds (pFile);
   const File* file = ds.find (IDirectorySearch::FILE_NORMAL
                               | IDirectorySearch::FILE_DIRECTORY);
   while (file) {
      if (verbose) {
         std::cout << "Handling file " << file->name () << '\n'; std::cout.flush (); }

      std::string strFile (file->path ());
      strFile += file->name ();

      if (file->isDirectory ()) {
         if (strcmp (file->name (), ".") && strcmp (file->name (), "..")) {
            strFile += File::DIRSEPARATOR;
            strFile += "*";
            handleFiles (strFile.c_str ());
         }
      }
      else {
         const char* pExt = strrchr (file->name (), '.');
         if (pExt++) {
            Xifstream ifile;
            ifile.open (strFile.c_str (), ios::in | ios::binary);
            if (!ifile) {
               std::cerr << PACKAGE "-error: File " << strFile.c_str ()
                         << " can't be opened!\nReason: ";
               perror ("");
            }
            else {
               ifile.init ();

               unsigned int i (0);
               for (; i < (sizeof (handlers) / sizeof (handlers[0])); ++i) {
                  char* pDescription = NULL;
                  if (!strcmp (handlers[i].pExt, pExt)) {
                     try {
                        pDescription = (this->*(handlers[i].pFnc)) ((Xistream&)ifile);
                        showFile (strFile.c_str (), pDescription);
                     }
                     catch (std::string& err) {
                        std::cerr << PACKAGE "-error: " << err.c_str ();
                        showFile (strFile.c_str (), "Error while processing");
                     }
                     if (pDescription) free (pDescription);
                     break;
                  }
               }
               if (i == (sizeof (handlers) / sizeof (handlers[0])))
                  showFile (strFile.c_str (), "Unknown file-type");
            }
         }
         else
            showFile (strFile.c_str (), "");
      } // end-else file

      file = ds.next ();
   } // end-while
}

/*--------------------------------------------------------------------------*/
//Purpose   : Displays a filename with its description
//Parameters: pFile: Name of file
//            pDesc: Description of file; can be NULL
//Requires  : pFile not NULL
/*--------------------------------------------------------------------------*/
void Application::showFile (const char* pFile, const char* pDesc) const {
   assert (pFile);

   std::cout << pFile << " - " << (pDesc ? pDesc : "") << '\n';
   std::cout.flush ();
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract to title of a HTML-document
//Parameters: hFile: File to processs
//Returns   : char*: Description; NULL in case of error
/*--------------------------------------------------------------------------*/
char* Application::processHTML (Xistream& hFile) const {
   if (verbose)
      std::cout << "Processing HTML-file\n";

   ParseHTML obj;
   return strdup (obj.parse (hFile));
}

/*--------------------------------------------------------------------------*/
//Purpose   : Tries to extract to title of a MS-office document
//Parameters: hFile: File to processs
//Returns   : char*: Description; NULL in case of error
/*--------------------------------------------------------------------------*/
char* Application::processOffice (Xistream& hFile) const {
   if (verbose)
      std::cout << "Processing MS Office document";

   ParseWord obj;
   return strdup (obj.parse (hFile));
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
