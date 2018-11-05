# Microsoft Developer Studio Project File - Name="UPXXGSM" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=UPXXGSM - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "UPXXGSM.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UPXXGSM.mak" CFG="UPXXGSM - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UPXXGSM - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "UPXXGSM - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "UPXXGSM - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "UPXXGSM - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "UPXXGSM - Win32 Release"
# Name "UPXXGSM - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Source\App\comm.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\GPRS.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\gsm.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\gsmcomm.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\IIC.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\main.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\mwork.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\timer.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\uart.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\ucs2.c
# End Source File
# Begin Source File

SOURCE=..\Source\App\work.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Source\App\comm.h
# End Source File
# Begin Source File

SOURCE=..\Source\App\define.h
# End Source File
# Begin Source File

SOURCE=..\Source\App\GPRS.h
# End Source File
# Begin Source File

SOURCE=..\Source\App\gsm.h
# End Source File
# Begin Source File

SOURCE=..\Source\App\iic.h
# End Source File
# Begin Source File

SOURCE=..\Source\App\mwork.h
# End Source File
# Begin Source File

SOURCE=..\Source\App\timer.h
# End Source File
# Begin Source File

SOURCE=..\Source\App\uart.h
# End Source File
# Begin Source File

SOURCE=..\Source\App\work.h
# End Source File
# End Group
# Begin Group "Comm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Source\Comm\Configuration.C
# End Source File
# Begin Source File

SOURCE=..\Source\Comm\Configuration.h
# End Source File
# Begin Source File

SOURCE=..\Source\Comm\ModbusM.C
# End Source File
# Begin Source File

SOURCE=..\Source\Comm\ModbusM.h
# End Source File
# Begin Source File

SOURCE=..\Source\Comm\wxcw.C
# End Source File
# Begin Source File

SOURCE=..\Source\Comm\wxcw.h
# End Source File
# End Group
# End Target
# End Project
