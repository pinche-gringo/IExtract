# Microsoft Developer Studio Generated NMAKE File, Based on IExtract.dsp
!IF "$(CFG)" == ""
CFG=IExtract - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. IExtract - Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "IExtract - Win32 Release" && "$(CFG)" != "IExtract - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "IExtract.mak" CFG="IExtract - Win32 Debug"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "IExtract - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "IExtract - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "IExtract - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\IExtract.exe"


CLEAN :
	-@erase "$(INTDIR)\ADate.obj"
	-@erase "$(INTDIR)\ANumeric.obj"
	-@erase "$(INTDIR)\ATime.obj"
	-@erase "$(INTDIR)\ATStamp.obj"
	-@erase "$(INTDIR)\DirSrch.obj"
	-@erase "$(INTDIR)\File.obj"
	-@erase "$(INTDIR)\FileRExp.obj"
	-@erase "$(INTDIR)\IDirSrch.obj"
	-@erase "$(INTDIR)\IExtract.obj"
	-@erase "$(INTDIR)\INIFile.obj"
	-@erase "$(INTDIR)\IVIOAppl.obj"
	-@erase "$(INTDIR)\Parse.obj"
	-@erase "$(INTDIR)\ParseHTML.obj"
	-@erase "$(INTDIR)\ParseJPG.obj"
	-@erase "$(INTDIR)\ParseSOffice.obj"
	-@erase "$(INTDIR)\ParsePDF.obj"
	-@erase "$(INTDIR)\ParseWord.obj"
	-@erase "$(INTDIR)\PathSrch.obj"
	-@erase "$(INTDIR)\StackTrc.obj"
	-@erase "$(INTDIR)\Thread.obj"
	-@erase "$(INTDIR)\Tokenize.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\Writer.obj"
	-@erase "$(INTDIR)\XDirSrch.obj"
	-@erase "$(INTDIR)\XStrBuf.obj"
	-@erase "$(OUTDIR)\IExtract.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\General\Common" /I "..\..\General\Windows" /D "NDEBUG" /D PACKAGE=\"IExtract\" /D VERSION="0.2" /D MICRO_VERSION="01" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D VERSION=\"0.2\" /D MICRO_VERSION=\"01\" /Fp"$(INTDIR)\IExtract.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\IExtract.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\IExtract.pdb" /machine:I386 /out:"$(OUTDIR)\IExtract.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ADate.obj" \
	"$(INTDIR)\ANumeric.obj" \
	"$(INTDIR)\ATime.obj" \
	"$(INTDIR)\ATStamp.obj" \
	"$(INTDIR)\DirSrch.obj" \
	"$(INTDIR)\File.obj" \
	"$(INTDIR)\FileRExp.obj" \
	"$(INTDIR)\IDirSrch.obj" \
	"$(INTDIR)\IExtract.obj" \
	"$(INTDIR)\INIFile.obj" \
	"$(INTDIR)\IVIOAppl.obj" \
	"$(INTDIR)\Parse.obj" \
	"$(INTDIR)\ParseHTML.obj" \
	"$(INTDIR)\ParseJPG.obj" \
	"$(INTDIR)\ParseWord.obj" \
	"$(INTDIR)\PathSrch.obj" \
	"$(INTDIR)\StackTrc.obj" \
	"$(INTDIR)\Thread.obj" \
	"$(INTDIR)\Tokenize.obj" \
	"$(INTDIR)\Writer.obj" \
	"$(INTDIR)\XDirSrch.obj" \
	"$(INTDIR)\XStrBuf.obj" \
	"$(INTDIR)\ParseSOffice.obj" \
	"$(INTDIR)\ParsePDF.obj"

"$(OUTDIR)\IExtract.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IExtract - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\IExtract.exe"


CLEAN :
	-@erase "$(INTDIR)\ADate.obj"
	-@erase "$(INTDIR)\ANumeric.obj"
	-@erase "$(INTDIR)\ATime.obj"
	-@erase "$(INTDIR)\ATStamp.obj"
	-@erase "$(INTDIR)\DirSrch.obj"
	-@erase "$(INTDIR)\File.obj"
	-@erase "$(INTDIR)\FileRExp.obj"
	-@erase "$(INTDIR)\IDirSrch.obj"
	-@erase "$(INTDIR)\IExtract.obj"
	-@erase "$(INTDIR)\INIFile.obj"
	-@erase "$(INTDIR)\IVIOAppl.obj"
	-@erase "$(INTDIR)\Parse.obj"
	-@erase "$(INTDIR)\ParseHTML.obj"
	-@erase "$(INTDIR)\ParseJPG.obj"
	-@erase "$(INTDIR)\ParseSOffice.obj"
	-@erase "$(INTDIR)\ParsePDF.obj"
	-@erase "$(INTDIR)\ParseWord.obj"
	-@erase "$(INTDIR)\PathSrch.obj"
	-@erase "$(INTDIR)\StackTrc.obj"
	-@erase "$(INTDIR)\Thread.obj"
	-@erase "$(INTDIR)\Tokenize.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\Writer.obj"
	-@erase "$(INTDIR)\XDirSrch.obj"
	-@erase "$(INTDIR)\XStrBuf.obj"
	-@erase "$(OUTDIR)\IExtract.exe"
	-@erase "$(OUTDIR)\IExtract.ilk"
	-@erase "$(OUTDIR)\IExtract.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\General\Common" /I "..\..\General\Windows" /D "_DEBUG" /D PACKAGE=\"IExtract\" /D VERSION="0.2" /D MICRO_VERSION="01" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D VERSION=\"0.2\" /D MICRO_VERSION=\"01\" /Fp"$(INTDIR)\IExtract.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\IExtract.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\IExtract.pdb" /debug /machine:I386 /out:"$(OUTDIR)\IExtract.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\ADate.obj" \
	"$(INTDIR)\ANumeric.obj" \
	"$(INTDIR)\ATime.obj" \
	"$(INTDIR)\ATStamp.obj" \
	"$(INTDIR)\DirSrch.obj" \
	"$(INTDIR)\File.obj" \
	"$(INTDIR)\FileRExp.obj" \
	"$(INTDIR)\IDirSrch.obj" \
	"$(INTDIR)\IExtract.obj" \
	"$(INTDIR)\INIFile.obj" \
	"$(INTDIR)\IVIOAppl.obj" \
	"$(INTDIR)\Parse.obj" \
	"$(INTDIR)\ParseHTML.obj" \
	"$(INTDIR)\ParseJPG.obj" \
	"$(INTDIR)\ParseWord.obj" \
	"$(INTDIR)\PathSrch.obj" \
	"$(INTDIR)\StackTrc.obj" \
	"$(INTDIR)\Thread.obj" \
	"$(INTDIR)\Tokenize.obj" \
	"$(INTDIR)\Writer.obj" \
	"$(INTDIR)\XDirSrch.obj" \
	"$(INTDIR)\XStrBuf.obj" \
	"$(INTDIR)\ParseSOffice.obj" \
	"$(INTDIR)\ParsePDF.obj"

"$(OUTDIR)\IExtract.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("IExtract.dep")
!INCLUDE "IExtract.dep"
!ELSE 
!MESSAGE Warning: cannot find "IExtract.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "IExtract - Win32 Release" || "$(CFG)" == "IExtract - Win32 Debug"
SOURCE=..\..\General\Common\ADate.cpp

"$(INTDIR)\ADate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\ANumeric.cpp

"$(INTDIR)\ANumeric.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\ATime.cpp

"$(INTDIR)\ATime.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\ATStamp.cpp

"$(INTDIR)\ATStamp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\DirSrch.cpp

"$(INTDIR)\DirSrch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\File.cpp

"$(INTDIR)\File.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\FileRExp.cpp

"$(INTDIR)\FileRExp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\IDirSrch.cpp

"$(INTDIR)\IDirSrch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\IExtract.cpp

"$(INTDIR)\IExtract.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\INIFile.cpp

"$(INTDIR)\INIFile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\IVIOAppl.cpp

"$(INTDIR)\IVIOAppl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\Parse.cpp

"$(INTDIR)\Parse.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseHTML.cpp

"$(INTDIR)\ParseHTML.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseJPG.cpp

"$(INTDIR)\ParseJPG.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseSOffice.cpp

"$(INTDIR)\ParseSOffice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseWord.cpp

"$(INTDIR)\ParseWord.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParsePDF.cpp

"$(INTDIR)\ParsePDF.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\PathSrch.cpp

"$(INTDIR)\PathSrch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\StackTrc.cpp

"$(INTDIR)\StackTrc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\Thread.cpp

"$(INTDIR)\Thread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\Tokenize.cpp

"$(INTDIR)\Tokenize.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\Writer.cpp

"$(INTDIR)\Writer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\XDirSrch.cpp

"$(INTDIR)\XDirSrch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\General\Common\XStrBuf.cpp

"$(INTDIR)\XStrBuf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

