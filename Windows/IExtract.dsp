# Microsoft Developer Studio Project File - Name="IExtract" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=IExtract - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE
!MESSAGE NMAKE /f "IExtract.mak".
!MESSAGE
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE
!MESSAGE NMAKE /f "IExtract.mak" CFG="IExtract - Win32 Debug"
!MESSAGE
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE
!MESSAGE "IExtract - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "IExtract - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IExtract - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\General\Common" /I "..\..\General\Windows" /D "NDEBUG" /D PACKAGE=\"IExtract\" /D VERSION="0.3" /D MICRO_VERSION="00" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D VERSION=\"0.3\" /D MICRO_VERSION=\"00\" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "IExtract - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\General\Common" /I "..\..\General\Windows" /D "_DEBUG" /D PACKAGE=\"IExtract\" /D VERSION="0.3" /D MICRO_VERSION="00" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D VERSION=\"0.3\" /D MICRO_VERSION=\"00\" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF

# Begin Target

# Name "IExtract - Win32 Release"
# Name "IExtract - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\General\Common\ADate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\ANumeric.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\ATime.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\ATStamp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\DirSrch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\File.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\FileRExp.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\IDirSrch.cpp
# End Source File
# Begin Source File

SOURCE=..\src\IExtract.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\INIFile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Entity.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\IVIOAppl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Parse.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ParseMP3.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ParseHTML.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ParseJPG.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ParseSOffice.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ParseWord.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ParsePDF.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\PathSrch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\StackTrc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Tokenize.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Writer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\XDirSrch.cpp
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\XStrBuf.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\General\Common\ADate.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\ANumeric.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\ATime.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\ATStamp.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\AttrVal.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\DirSrch.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\File.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\FileRExp.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\IDirSrch.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\INIFile.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Entity.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\IVIOAppl.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Parse.h
# End Source File
# Begin Source File

SOURCE=..\src\ParseMP3.h
# End Source File
# Begin Source File

SOURCE=..\src\ParseHTML.h
# End Source File
# Begin Source File

SOURCE=..\src\ParseJPG.h
# End Source File
# Begin Source File

SOURCE=..\src\ParseSOffice.h
# End Source File
# Begin Source File

SOURCE=..\src\ParseWord.h
# End Source File
# Begin Source File

SOURCE=..\src\ParsePDF.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\PathSrch.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\Tokenize.h
# End Source File
# Begin Source File

SOURCE=..\src\Writer.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\XDirSrch.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\XStrBuf.h
# End Source File
# Begin Source File

SOURCE=..\..\General\Common\XStream.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
