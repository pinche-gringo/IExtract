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
	-@erase "$(INTDIR)\IExtract.obj"
	-@erase "$(INTDIR)\ParseHTML.obj"
	-@erase "$(INTDIR)\ParseJPG.obj"
	-@erase "$(INTDIR)\ParseMP3.obj"
	-@erase "$(INTDIR)\ParseOOffice.obj"
	-@erase "$(INTDIR)\ParsePDF.obj"
	-@erase "$(INTDIR)\ParseRTF.obj"
	-@erase "$(INTDIR)\ParseSOffice.obj"
	-@erase "$(INTDIR)\ParseWord.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\Writer.obj"
	-@erase "$(OUTDIR)\IExtract.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "." /D "NDEBUG" /D PACKAGE=\"IExtract\" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\IExtract.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vcgenerals.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\IExtract.pdb" /machine:I386 /out:"$(OUTDIR)\IExtract.exe" 
LINK32_OBJS= \
	"$(INTDIR)\IExtract.obj" \
	"$(INTDIR)\ParseHTML.obj" \
	"$(INTDIR)\ParseJPG.obj" \
	"$(INTDIR)\ParseMP3.obj" \
	"$(INTDIR)\ParseOOffice.obj" \
	"$(INTDIR)\ParsePDF.obj" \
	"$(INTDIR)\ParseRTF.obj" \
	"$(INTDIR)\ParseSOffice.obj" \
	"$(INTDIR)\ParseWord.obj" \
	"$(INTDIR)\Writer.obj"

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
	-@erase "$(INTDIR)\IExtract.obj"
	-@erase "$(INTDIR)\ParseHTML.obj"
	-@erase "$(INTDIR)\ParseJPG.obj"
	-@erase "$(INTDIR)\ParseMP3.obj"
	-@erase "$(INTDIR)\ParseOOffice.obj"
	-@erase "$(INTDIR)\ParsePDF.obj"
	-@erase "$(INTDIR)\ParseRTF.obj"
	-@erase "$(INTDIR)\ParseSOffice.obj"
	-@erase "$(INTDIR)\ParseWord.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\Writer.obj"
	-@erase "$(OUTDIR)\IExtract.exe"
	-@erase "$(OUTDIR)\IExtract.ilk"
	-@erase "$(OUTDIR)\IExtract.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "." /D "_DEBUG" /D PACKAGE=\"IExtract\" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fp"$(INTDIR)\IExtract.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib vcgenerals.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\IExtract.pdb" /debug /machine:I386 /out:"$(OUTDIR)\IExtract.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\IExtract.obj" \
	"$(INTDIR)\ParseHTML.obj" \
	"$(INTDIR)\ParseJPG.obj" \
	"$(INTDIR)\ParseMP3.obj" \
	"$(INTDIR)\ParseOOffice.obj" \
	"$(INTDIR)\ParsePDF.obj" \
	"$(INTDIR)\ParseRTF.obj" \
	"$(INTDIR)\ParseSOffice.obj" \
	"$(INTDIR)\ParseWord.obj" \
	"$(INTDIR)\Writer.obj"

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
SOURCE=..\src\IExtract.cpp

"$(INTDIR)\IExtract.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseHTML.cpp

"$(INTDIR)\ParseHTML.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseJPG.cpp

"$(INTDIR)\ParseJPG.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseMP3.cpp

"$(INTDIR)\ParseMP3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseOOffice.cpp

"$(INTDIR)\ParseOOffice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParsePDF.cpp

"$(INTDIR)\ParsePDF.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseRTF.cpp

"$(INTDIR)\ParseRTF.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseSOffice.cpp

"$(INTDIR)\ParseSOffice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\ParseWord.cpp

"$(INTDIR)\ParseWord.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\Writer.cpp

"$(INTDIR)\Writer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

