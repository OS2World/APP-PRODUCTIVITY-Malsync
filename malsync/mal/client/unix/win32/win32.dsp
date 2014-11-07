# Microsoft Developer Studio Project File - Name="win32" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=win32 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "win32.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "win32.mak" CFG="win32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "win32 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "win32 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "win32 - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "./" /I "../common" /I "../../common" /I "../../../vendor/pilot/pilot-link.0.9.3/include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"Release/malsync.exe"

!ELSEIF  "$(CFG)" == "win32 - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../" /I "../../common" /I "../../../common" /I "../../../../vendor/pilot/pilot-link.0.9.3/include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib  kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/malsync.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "win32 - Win32 Release"
# Name "win32 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\getopt.c
# End Source File
# Begin Source File

SOURCE=..\MAL31DBConfig.c
# End Source File
# Begin Source File

SOURCE=..\MAL31ServerConfig.c
# End Source File
# Begin Source File

SOURCE=..\MAL31UserConfig.c
# End Source File
# Begin Source File

SOURCE=..\malsync.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\getopt.h
# End Source File
# Begin Source File

SOURCE=..\MAL31DBConfig.h
# End Source File
# Begin Source File

SOURCE=..\MAL31ServerConfig.h
# End Source File
# Begin Source File

SOURCE=..\MAL31UserConfig.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "clientcommon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\common\AGClientProcessor.c
# End Source File
# Begin Source File

SOURCE=..\..\common\AGClientProcessor.h
# End Source File
# Begin Source File

SOURCE=..\..\common\AGCommandProcessor.c
# End Source File
# Begin Source File

SOURCE=..\..\common\AGCommandProcessor.h
# End Source File
# Begin Source File

SOURCE=..\..\common\AGProxyDebug.c
# End Source File
# Begin Source File

SOURCE=..\..\common\AGProxyDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\common\AGSyncCommon.c
# End Source File
# Begin Source File

SOURCE=..\..\common\AGSyncCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\common\AGSyncProcessor.c
# End Source File
# Begin Source File

SOURCE=..\..\common\AGSyncProcessor.h
# End Source File
# End Group
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\common\AGArray.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGArray.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGBase64.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGBase64.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGBufferedNet.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGBufferedNet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGBufferReader.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGBufferReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGBufferWriter.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGBufferWriter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGCollection.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGCollection.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGDBConfig.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGDBConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGDeviceInfo.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGDeviceInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGDigest.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGDigest.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGHashTable.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGHashTable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGLocationConfig.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGLocationConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGMD5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGMD5.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGMsg.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGMsg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGNet.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGNet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGPalmProtocol.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGPalmProtocol.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGProtectedMem.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGProtectedMem.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGProtocol.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGProtocol.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGProxy.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGProxy.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGReader.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGReader.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGRecord.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGRecord.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGServerConfig.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGServerConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGSynchronize.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGSynchronize.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGUserConfig.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGUserConfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGUtilWin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGUtilWin.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGWriter.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\AGWriter.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\..\common\md5.h
# End Source File
# End Group
# Begin Group "pilot-link"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\appinfo.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\cmp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\dlp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\hinote.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\inet.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\inetserial.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\padp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\pi-file.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-hinote.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-macros.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-padp.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-serial.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-slp.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-sockaddr-win32.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-socket.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-source.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\include\pi-version.h"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\serial.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\slp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\socket.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\sync.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\syspkt.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\todo.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\utils.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\..\vendor\pilot\pilot-link.0.9.3\libsock\winserial.c"
# End Source File
# End Group
# End Target
# End Project
