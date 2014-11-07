# Microsoft Developer Studio Project File - Name="malcommon" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=malcommon - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "malcommon.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "malcommon.mak" CFG="malcommon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "malcommon - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "malcommon - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath "Desktop"
# PROP WCE_FormatVersion ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "malcommon - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MALCOMMON_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../client/common" /I "." /I "../../vendor/mozilla" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "WIN32" /D "_MBCS" /D "XP_PC" /D "STANDALONE_REGISTRY" /D MAL_INTERFACE_VERSION=1 /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x60000000" /dll /map /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if not exist $(AGINSTALLDIR)\bin mkdir $(AGINSTALLDIR)\bin	copy Release\malcommon.dll $(AGINSTALLDIR)\bin	copy Release\malcommon.lib $(AGINSTALLDIR)\bin	if exist $(AGINSTALLDIR)\bin\. copy release\*.map $(AGINSTALLDIR)\bin\.
# End Special Build Tool

!ELSEIF  "$(CFG)" == "malcommon - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MALCOMMON_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../client/common" /I "." /I "../../vendor/mozilla" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "WIN32" /D "_MBCS" /D "XP_PC" /D "STANDALONE_REGISTRY" /D MAL_INTERFACE_VERSION=1 /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x60000000" /dll /map /debug /machine:I386 /out:"Debug/malcommond.dll" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=if not exist $(AGINSTALLDIR)\bin mkdir $(AGINSTALLDIR)\bin	copy Debug\malcommond.dll $(AGINSTALLDIR)\bin	copy Debug\malcommond.lib $(AGINSTALLDIR)\bin	if exist $(AGINSTALLDIR)\bin\. copy debug\*.map $(AGINSTALLDIR)\bin\.
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "malcommon - Win32 Release"
# Name "malcommon - Win32 Debug"
# Begin Group "Mozilla"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\vendor\mozilla\NSReg.h
# End Source File
# Begin Source File

SOURCE=..\..\vendor\mozilla\reg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vendor\mozilla\reg.h
# End Source File
# Begin Source File

SOURCE=..\..\vendor\mozilla\vr_stubs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\vendor\mozilla\vr_stubs.h
# End Source File
# Begin Source File

SOURCE=..\..\vendor\mozilla\winfile.h
# End Source File
# Begin Source File

SOURCE=..\..\vendor\mozilla\xp_file.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AGArray.c
# End Source File
# Begin Source File

SOURCE=.\AGArray.h
# End Source File
# Begin Source File

SOURCE=.\AGBase64.c
# End Source File
# Begin Source File

SOURCE=.\AGBase64.h
# End Source File
# Begin Source File

SOURCE=.\AGBufferedNet.c
# End Source File
# Begin Source File

SOURCE=.\AGBufferedNet.h
# End Source File
# Begin Source File

SOURCE=.\AGBufferReader.c
# End Source File
# Begin Source File

SOURCE=.\AGBufferReader.h
# End Source File
# Begin Source File

SOURCE=.\AGBufferWriter.c
# End Source File
# Begin Source File

SOURCE=.\AGBufferWriter.h
# End Source File
# Begin Source File

SOURCE=..\client\common\AGClientProcessor.c
# End Source File
# Begin Source File

SOURCE=..\client\common\AGClientProcessor.h
# End Source File
# Begin Source File

SOURCE=.\AGCollection.c
# End Source File
# Begin Source File

SOURCE=.\AGCollection.h
# End Source File
# Begin Source File

SOURCE=..\client\common\AGCommandProcessor.c
# End Source File
# Begin Source File

SOURCE=..\client\common\AGCommandProcessor.h
# End Source File
# Begin Source File

SOURCE=.\AGDBConfig.c
# End Source File
# Begin Source File

SOURCE=.\AGDBConfig.h
# End Source File
# Begin Source File

SOURCE=.\AGDesktopInfoPalm.c
# End Source File
# Begin Source File

SOURCE=.\AGDesktopInfoPalm.h
# End Source File
# Begin Source File

SOURCE=.\AGDesktopInfoWinCE.c
# End Source File
# Begin Source File

SOURCE=.\AGDesktopInfoWinCE.h
# End Source File
# Begin Source File

SOURCE=.\AGDeviceInfo.c
# End Source File
# Begin Source File

SOURCE=.\AGDeviceInfo.h
# End Source File
# Begin Source File

SOURCE=.\AGDigest.c
# End Source File
# Begin Source File

SOURCE=.\AGDigest.h
# End Source File
# Begin Source File

SOURCE=.\AGHashTable.c
# End Source File
# Begin Source File

SOURCE=.\AGHashTable.h
# End Source File
# Begin Source File

SOURCE=.\AGLocationConfig.c
# End Source File
# Begin Source File

SOURCE=.\AGLocationConfig.h
# End Source File
# Begin Source File

SOURCE=.\AGMD5.c
# End Source File
# Begin Source File

SOURCE=.\AGMD5.h
# End Source File
# Begin Source File

SOURCE=.\AGMsg.c
# End Source File
# Begin Source File

SOURCE=.\AGMsg.h
# End Source File
# Begin Source File

SOURCE=.\AGNet.c
# End Source File
# Begin Source File

SOURCE=.\AGNet.h
# End Source File
# Begin Source File

SOURCE=.\AGNetPilot.c
# End Source File
# Begin Source File

SOURCE=.\AGPalmProtocol.c
# End Source File
# Begin Source File

SOURCE=.\AGPalmProtocol.h
# End Source File
# Begin Source File

SOURCE=..\client\common\AGPasswordPrompt.c
# End Source File
# Begin Source File

SOURCE=..\client\common\AGPasswordPrompt.h
# End Source File
# Begin Source File

SOURCE=.\AGProtectedMem.c
# End Source File
# Begin Source File

SOURCE=.\AGProtectedMem.h
# End Source File
# Begin Source File

SOURCE=.\AGProtocol.c
# End Source File
# Begin Source File

SOURCE=.\AGProtocol.h
# End Source File
# Begin Source File

SOURCE=.\AGProxy.c
# End Source File
# Begin Source File

SOURCE=.\AGProxy.h
# End Source File
# Begin Source File

SOURCE=..\client\common\AGProxyDebug.c
# End Source File
# Begin Source File

SOURCE=..\client\common\AGProxyDebug.h
# End Source File
# Begin Source File

SOURCE=.\AGProxySearch.cpp
# End Source File
# Begin Source File

SOURCE=.\AGProxySearch.h
# End Source File
# Begin Source File

SOURCE=.\AGReader.c
# End Source File
# Begin Source File

SOURCE=.\AGReader.h
# End Source File
# Begin Source File

SOURCE=.\AGRecord.c
# End Source File
# Begin Source File

SOURCE=.\AGRecord.h
# End Source File
# Begin Source File

SOURCE=.\AGResourceManager.h
# End Source File
# Begin Source File

SOURCE=.\AGResourceManagerWin.c
# End Source File
# Begin Source File

SOURCE=.\AGServerConfig.c
# End Source File
# Begin Source File

SOURCE=.\AGServerConfig.h
# End Source File
# Begin Source File

SOURCE=..\client\common\AGShlapi.c
# End Source File
# Begin Source File

SOURCE=..\client\common\AGShlapi.h
# End Source File
# Begin Source File

SOURCE=..\client\common\AGSyncCommon.c
# End Source File
# Begin Source File

SOURCE=..\client\common\AGSyncCommon.h
# End Source File
# Begin Source File

SOURCE=.\AGSynchronize.c
# End Source File
# Begin Source File

SOURCE=.\AGSynchronize.h
# End Source File
# Begin Source File

SOURCE=..\client\common\AGSyncProcessor.c
# End Source File
# Begin Source File

SOURCE=..\client\common\AGSyncProcessor.h
# End Source File
# Begin Source File

SOURCE=.\AGTypes.h
# End Source File
# Begin Source File

SOURCE=.\AGUserConfig.c
# End Source File
# Begin Source File

SOURCE=.\AGUserConfig.h
# End Source File
# Begin Source File

SOURCE=.\AGUtil.h
# End Source File
# Begin Source File

SOURCE=.\AGUtilPalmOS.c
# End Source File
# Begin Source File

SOURCE=.\AGUtilPalmOS.h
# End Source File
# Begin Source File

SOURCE=.\AGUtilUnix.c
# End Source File
# Begin Source File

SOURCE=.\AGUtilUnix.h
# End Source File
# Begin Source File

SOURCE=.\AGUtilWin.c
# End Source File
# Begin Source File

SOURCE=.\AGUtilWin.h
# End Source File
# Begin Source File

SOURCE=.\AGWriter.c
# End Source File
# Begin Source File

SOURCE=.\AGWriter.h
# End Source File
# Begin Source File

SOURCE=.\MALCommonWin.rc
# End Source File
# Begin Source File

SOURCE=.\md5.c
# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# End Target
# End Project
