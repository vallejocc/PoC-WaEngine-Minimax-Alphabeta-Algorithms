# Microsoft Developer Studio Project File - Name="waenginepru" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=waenginepru - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "waenginepru.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "waenginepru.mak" CFG="waenginepru - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "waenginepru - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "waenginepru - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "waenginepru - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\inc\pub" /I "..\inc\priv" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "TREE_DEBUG_ENABLE" /YX /FD /c
# ADD BASE RSC /l 0xc0a /d "NDEBUG"
# ADD RSC /l 0xc0a /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "waenginepru - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\inc\pub" /I "..\inc\priv" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "TREE_DEBUG_ENABLE" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0xc0a /d "_DEBUG"
# ADD RSC /l 0xc0a /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "waenginepru - Win32 Release"
# Name "waenginepru - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\a.c
# End Source File
# Begin Source File

SOURCE=..\src\md5.c
# End Source File
# Begin Source File

SOURCE=..\src\resource.h
# End Source File
# Begin Source File

SOURCE=..\src\TreeControl.rsrc.rc
# End Source File
# Begin Source File

SOURCE=..\src\waalphabeta.c
# End Source File
# Begin Source File

SOURCE=..\src\wadebug.c
# End Source File
# Begin Source File

SOURCE=..\src\wadepthfirst.c
# End Source File
# Begin Source File

SOURCE=..\src\waenginepriv.c
# End Source File
# Begin Source File

SOURCE=..\src\wahash.c
# End Source File
# Begin Source File

SOURCE=..\src\warandom.c
# End Source File
# End Group
# Begin Group "inc"

# PROP Default_Filter ""
# Begin Group "priv"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\inc\priv\global.h
# End Source File
# Begin Source File

SOURCE=..\inc\priv\md5.h
# End Source File
# Begin Source File

SOURCE=..\inc\priv\waalphabeta.h
# End Source File
# Begin Source File

SOURCE=..\inc\priv\wadepthfirst.h
# End Source File
# Begin Source File

SOURCE=..\inc\priv\waenginepriv.h
# End Source File
# Begin Source File

SOURCE=..\inc\priv\wamacros.h
# End Source File
# End Group
# Begin Group "pub"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\inc\pub\wadebug.h
# End Source File
# Begin Source File

SOURCE=..\inc\pub\waengine.h
# End Source File
# Begin Source File

SOURCE=..\inc\pub\waerror.h
# End Source File
# Begin Source File

SOURCE=..\inc\pub\wahash.h
# End Source File
# Begin Source File

SOURCE=..\inc\pub\wamem.h
# End Source File
# Begin Source File

SOURCE=..\inc\pub\warandom.h
# End Source File
# Begin Source File

SOURCE=..\inc\pub\wasnapshots.h
# End Source File
# Begin Source File

SOURCE=..\inc\pub\watypes.h
# End Source File
# End Group
# End Group
# End Target
# End Project
