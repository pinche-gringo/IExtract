# Microsoft Developer Studio Generated NMAKE File, Based on Extract.dsp
!IF "$(CFG)" == ""
CFG=Extract - Win32 Debug
!MESSAGE Keine Konfiguration angegeben. Extract - Win32 Debug wird als Standard verwendet.
!ENDIF

!IF "$(CFG)" != "Extract - Win32 Release" && "$(CFG)" != "Extract - Win32 Debug"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE
!MESSAGE NMAKE /f "Extract.mak" CFG="Extract - Win32 Debug"
!MESSAGE
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE
!MESSAGE "Extract - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "Extract - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

!IF  "$(CFG)" == "Extract - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Extract.exe"


CLEAN :
	-@erase "$(INTDIR)\ANumeric.obj"
	-@erase "$(INTDIR)\DirSrch.obj"
	-@erase "$(INTDIR)\Extract.obj"
	-@erase "$(INTDIR)\File.obj"
	-@erase "$(INTDIR)\FileRExp.obj"
	-@erase "$(INTDIR)\IDirSrch.obj"
	-@erase "$(INTDIR)\IVIOAppl.obj"
	-@erase "$(INTDIR)\Parse.obj"
	-@erase "$(INTDIR)\ParseHTML.obj"
	-@erase "$(INTDIR)\ParseWord.obj"
	-@erase "$(INTDIR)\StackTrc.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\XStrBuf.obj"
	-@erase "$(OUTDIR)\Extract.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "E:\Projects\General\Windows" /I "E:\Projects\General\Common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\Extract.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Extract.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\Extract.pdb" /machine:I386 /out:"$(OUTDIR)\Extract.exe"
LINK32_OBJS= \
	"$(INTDIR)\ANumeric.obj" \
	"$(INTDIR)\DirSrch.obj" \
	"$(INTDIR)\Extract.obj" \
	"$(INTDIR)\File.obj" \
	"$(INTDIR)\FileRExp.obj" \
	"$(INTDIR)\IDirSrch.obj" \
	"$(INTDIR)\IVIOAppl.obj" \
	"$(INTDIR)\Parse.obj" \
	"$(INTDIR)\ParseHTML.obj" \
	"$(INTDIR)\StackTrc.obj" \
	"$(INTDIR)\XStrBuf.obj" \
	"$(INTDIR)\ParseWord.obj"

"$(OUTDIR)\Extract.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Extract - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Extract.exe"


CLEAN :
	-@erase "$(INTDIR)\ANumeric.obj"
	-@erase "$(INTDIR)\DirSrch.obj"
	-@erase "$(INTDIR)\Extract.obj"
	-@erase "$(INTDIR)\File.obj"
	-@erase "$(INTDIR)\FileRExp.obj"
	-@erase "$(INTDIR)\IDirSrch.obj"
	-@erase "$(INTDIR)\IVIOAppl.obj"
	-@erase "$(INTDIR)\Parse.obj"
	-@erase "$(INTDIR)\ParseHTML.obj"
	-@erase "$(INTDIR)\ParseWord.obj"
	-@erase "$(INTDIR)\StackTrc.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\XStrBuf.obj"
	-@erase "$(OUTDIR)\Extract.exe"
	-@erase "$(OUTDIR)\Extract.ilk"
	-@erase "$(OUTDIR)\Extract.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "E:\Projects\General\Windows" /I "E:\Projects\General\Common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\Extract.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c /DTRACELEVEL=0 /DCheck=0

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Extract.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\Extract.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Extract.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\ANumeric.obj" \
	"$(INTDIR)\DirSrch.obj" \
	"$(INTDIR)\Extract.obj" \
	"$(INTDIR)\File.obj" \
	"$(INTDIR)\FileRExp.obj" \
	"$(INTDIR)\IDirSrch.obj" \
	"$(INTDIR)\IVIOAppl.obj" \
	"$(INTDIR)\Parse.obj" \
	"$(INTDIR)\ParseHTML.obj" \
	"$(INTDIR)\StackTrc.obj" \
	"$(INTDIR)\XStrBuf.obj" \
	"$(INTDIR)\ParseWord.obj"

"$(OUTDIR)\Extract.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Extract.dep")
!INCLUDE "Extract.dep"
!ELSE 
!MESSAGE Warning: cannot find "Extract.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Extract - Win32 Release" || "$(CFG)" == "Extract - Win32 Debug"
SOURCE=..\General\Common\ANumeric.cpp

"$(INTDIR)\ANumeric.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\General\Common\DirSrch.cpp

"$(INTDIR)\DirSrch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Extract.cpp

"$(INTDIR)\Extract.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\General\Common\File.cpp

"$(INTDIR)\File.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\General\Common\FileRExp.cpp

"$(INTDIR)\FileRExp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\General\Common\IDirSrch.cpp

"$(INTDIR)\IDirSrch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\General\Common\IVIOAppl.cpp

!IF  "$(CFG)" == "Extract - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /I "E:\Projects\General\Windows" /I "E:\Projects\General\Common" /I "E:\Projects\General" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\Extract.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\IVIOAppl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Extract - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "E:\Projects\General\Windows" /I "E:\Projects\General\Common" /I "E:\Projects\General" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\Extract.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\IVIOAppl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\General\Common\Parse.cpp

"$(INTDIR)\Parse.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ParseHTML.cpp

"$(INTDIR)\ParseHTML.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ParseWord.cpp

"$(INTDIR)\ParseWord.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\General\Common\StackTrc.cpp

"$(INTDIR)\StackTrc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\General\Common\XStrBuf.cpp

"$(INTDIR)\XStrBuf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

