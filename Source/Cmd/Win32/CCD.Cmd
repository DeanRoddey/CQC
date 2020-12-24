@ECHO OFF
REM ---------------------------------------------------------------------------
REM  This file is just a simple way of quickly getting to a particular
REM  project directory. Its pretty simplistic because NT's command language is
REM  pretty brain dead.
REM
REM  It just takes a parameter that indicates the desired directory. Some are
REM  special ones, and all others are assumed to be project directories.
REM ---------------------------------------------------------------------------

%CID_DRIVE%

IF "%1"=="all" GOTO CD_ALLPROJECTS
IF "%1"=="cmd" GOTO CD_CMD
IF "%1"=="cmldrivers" GOTO CD_CMLDRIVERS
IF "%1"=="cqcdata" GOTO CD_CQCDATA
IF "%1"=="develop" GOTO CD_DEVELOP
IF "%1"=="docs" GOTO CD_DOCS
IF "%1"=="drivers" GOTO CD_DRIVERS
IF "%1"=="envsetup" GOTO CD_ENVSETUP
IF "%1"=="images" GOTO CD_IMAGES
IF "%1"=="instimage" GOTO CD_CURINSTIMAGE
IF "%1"=="instimages" GOTO CD_INSTIMAGES
IF "%1"=="macros" GOTO CD_MACROS
IF "%1"=="manifests" GOTO CD_MANIFESTS
IF "%1"=="mediarepomgr" GOTO CD_MEDIAREPOMGR
IF "%1"=="protocols" GOTO CD_PROTOCOLS
IF "%1"=="release" GOTO CD_CURRELEASE
IF "%1"=="releases" GOTO CD_RELEASES
IF "%1"=="root" GOTO CD_ROOT
IF "%1"=="top" GOTO CD_TOP

IF "%1"=="cppdrvtest" GOTO CD_CPPDRVTEST
IF "%1"=="cqcadmin" GOTO CD_CQCADMIN
IF "%1"=="cqcappshell" GOTO CD_CQCAPPSHELL
IF "%1"=="cqcclservice" GOTO CD_CQCCLSERVICE
IF "%1"=="cqcdatasrv" GOTO CD_CQCDATASRV
IF "%1"=="cqcdocs" GOTO CD_CQCDOCS
IF "%1"=="cqcdoccomp" GOTO CD_CQCDOCCOMP
IF "%1"=="cqcdrvdev" GOTO CD_CQCDRVDEV
IF "%1"=="cqcevmoneng" GOTO CD_CQCEVMONENG
IF "%1"=="cqcevcl" GOTO CD_CQCEVCL
IF "%1"=="cqceventsrv" GOTO CD_CQCEVENTSRV
IF "%1"=="cqcgkit" GOTO CD_CQCGKIT
IF "%1"=="cqcgwcl" GOTO CD_CQCGWCL
IF "%1"=="cqcgwsrv" GOTO CD_CQCGWSRV
IF "%1"=="cqcigkit" GOTO CD_CQCIGKIT
IF "%1"=="cqcintfed" GOTO CD_CQCINTFED
IF "%1"=="cqcintfeng" GOTO CD_CQCINTFENG
IF "%1"=="cqcintftheme" GOTO CD_CQCINTFTHEME
IF "%1"=="cqcintfweng" GOTO CD_CQCINTFWENG
IF "%1"=="cqcintfview" GOTO CD_CQCINTFVIEW
IF "%1"=="cqcinst" GOTO CD_CQCINST
IF "%1"=="cqcir" GOTO CD_CQCIR
IF "%1"=="cqckit" GOTO CD_CQCKIT
IF "%1"=="cqlogicsrv" GOTO CD_CQLOGICSRV
IF "%1"=="cqcmedia" GOTO CD_CQCMEDIA
IF "%1"=="cqcrembrws" GOTO CD_CQCREMBRWS
IF "%1"=="cqcserver" GOTO CD_CQCSERVER
IF "%1"=="cqcsrvfw" GOTO CD_CQCSRVFW
IF "%1"=="cqcsyscfg" GOTO CD_CQCSYSCFG
IF "%1"=="cqctreebrws" GOTO CD_CQCTREEBRWS
IF "%1"=="cqcvoice" GOTO CD_CQCVOICE
IF "%1"=="cqcwebsrv" GOTO CD_CQCWEBSRV
IF "%1"=="cqcwebsrvc" GOTO CD_CQCWEBSRVC
IF "%1"=="cqslrepo" GOTO CD_CQSLREPO

IF "%1"=="output" GOTO CD_OUTPUT
IF "%1"=="repo" GOTO CD_REPO
IF "%1"=="res" GOTO CD_RES
IF "%1"=="src" GOTO CD_SOURCE
IF "%1"=="tests2" GOTO CD_TESTS2
IF "%1"=="testmacros" GOTO CD_TESTMACROS
IF "%1"=="themes" GOTO CD_THEMES
IF "%1"=="tmptest" GOTO CD_TMPTEST
IF "%1"=="utilities" GOTO CD_UTILITIES
IF "%1"=="webres" GOTO CD_WEBRES
IF "%1"=="webriva" GOTO CD_WEBRIVA
IF "%1"=="webrivacmp" GOTO CD_WEBRIVACMP
IF "%1"=="zwaveusb3" GOTO CD_ZWAVEUSB3

ECHO Unknown target directory!
GOTO EXIT

:CD_ROOT
CD %CQC_SRCTREE%\
GOTO EXIT

:CD_TOP
CD %CQC_SRCTREE%\..
GOTO EXIT

:CD_ALLPROJECTS
CD %CQC_SRCTREE%\Source\AllProjects
GOTO EXIT

:CD_CMD
CD %CQC_SRCTREE%\Source\Cmd\Win32
GOTO EXIT

:CD_CMLDRIVERS
CD %CQC_SRCTREE%\Source\AllProjects\Macros\MEng\System\CQC\Drivers
GOTO EXIT

:CD_DEVELOP
CD %CQC_SRCTREE%\..
GOTO EXIT

:CD_DOCS
CD %CQC_SRCTREE%\Source\Docs
GOTO EXIT

:CD_DRIVERS
CD %CQC_SRCTREE%\Source\AllProjects\Drivers
GOTO EXIT

:CD_ENVSETUP
CD %CQC_SRCTREE%\..\EnvSetup\Win32
GOTO EXIT

:CD_IMAGES
CD %CQC_SRCTREE%\Source\Images
GOTO EXIT

:CD_CURINSTIMAGE
SET CCDTMPDIR=%CID_VERSION:.=_%
CD %CQCInstImages%\InstImage_%CCDTMPDIR%
SET CCDTMPDIR=
GOTO EXIT

:CD_INSTIMAGES
CD %CQCInstImages%\
GOTO EXIT

:CD_MACROS
CD %CQC_SRCTREE%\Source\AllProjects\Macros
GOTO EXIT

:CD_MANIFESTS
CD %CQC_SRCTREE%\Source\AllProjects\Drivers\Manifests
GOTO EXIT

:CD_MEDIAREPOMGR
CD %CQC_SRCTREE%\Source\AllProjects\Drivers\CQSLRepo\MediaRepoMgr
GOTO EXIT

:CD_PROTOCOLS
CD %CQC_SRCTREE%\Source\AllProjects\Drivers\Protocols
GOTO EXIT

:CD_CURRELEASE
CD \Develop\Releases\CQC_%CID_VERSIONUL%
SET CCDTMPDIR=
GOTO EXIT

:CD_RELEASES
CD \Develop\Releases
GOTO EXIT

:CD_REPO
CD %CQC_SRCTREE%\CQCData\Repository
GOTO EXIT

:CD_CQCDATA
CD %CQC_DATADIR%
GOTO EXIT



:CD_CPPDRVTEST
CD %CQC_SRCTREE%\Source\AllProjects\Utilities\CppDrvTest
GOTO EXIT

:CD_CQCADMIN
CD %CQC_SRCTREE%\Source\AllProjects\CQCAdmin
GOTO EXIT

:CD_CQCAPPSHELL
CD %CQC_SRCTREE%\Source\AllProjects\CQCAppShell
GOTO EXIT

:CD_CQCCLSERVICE
CD %CQC_SRCTREE%\Source\AllProjects\CQCClService
GOTO EXIT

:CD_CQCDATASRV
CD %CQC_SRCTREE%\Source\AllProjects\CQCDataSrv
GOTO EXIT

:CD_CQCDOCCOMP
CD %CQC_SRCTREE%\Source\AllProjects\Utilities\CQCDocComp
GOTO EXIT

:CD_CQCDOCS
CD %CQC_SRCTREE%\Source\Docs\CQCDocs
GOTO EXIT

:CD_CQCDRVDEV
CD %CQC_SRCTREE%\Source\AllProjects\CQCDrvDev
GOTO EXIT

:CD_CQCEVMONENG
CD %CQC_SRCTREE%\Source\AllProjects\EventSys\CQCEvMonEng
GOTO EXIT

:CD_CQCEVCL
CD %CQC_SRCTREE%\Source\AllProjects\EventSys\CQCEvCl
GOTO EXIT

:CD_CQCEVENTSRV
CD %CQC_SRCTREE%\Source\AllProjects\EventSys\CQCEvSrv
GOTO EXIT

:CD_CQCGKIT
CD %CQC_SRCTREE%\Source\AllProjects\CQCGKit
GOTO EXIT

:CD_CQCIGKIT
CD %CQC_SRCTREE%\Source\AllProjects\CQCIGKit
GOTO EXIT

:CD_CQCGWCL
CD %CQC_SRCTREE%\Source\AllProjects\CQCGWSrv\Client
GOTO EXIT

:CD_CQCGWSRV
CD %CQC_SRCTREE%\Source\AllProjects\CQCGWSrv
GOTO EXIT

:CD_CQCINTFED
CD %CQC_SRCTREE%\Source\AllProjects\CQCIntf\CQCIntfEd
GOTO EXIT

:CD_CQCINTFENG
CD %CQC_SRCTREE%\Source\AllProjects\CQCIntf\CQCIntfEng
GOTO EXIT

:CD_CQCSYSCFG
CD %CQC_SRCTREE%\Source\AllProjects\CQCSysCfg
GOTO EXIT

:CD_CQCINTFTHEME
CD %CQC_SRCTREE%\Source\AllProjects\CQCIntf\CQCIntfTheme
GOTO EXIT

:CD_CQCINTFWENG
CD %CQC_SRCTREE%\Source\AllProjects\CQCIntf\CQCIntfWEng
GOTO EXIT

:CD_CQCINTFVIEW
CD %CQC_SRCTREE%\Source\AllProjects\CQCIntf\CQCIntfView
GOTO EXIT

:CD_CQCINST
CD %CQC_SRCTREE%\Source\AllProjects\Installer\CQCInst
GOTO EXIT

:CD_CQLOGICSRV
CD %CQC_SRCTREE%\Source\AllProjects\CQLogicSrv\Server
GOTO EXIT

:CD_CQCREMBRWS
CD %CQC_SRCTREE%\Source\AllProjects\RemBrws\CQCRemBrws
GOTO EXIT

:CD_CQCSERVER
CD %CQC_SRCTREE%\Source\AllProjects\CQCServer
GOTO EXIT

:CD_CQCTRAYMON
CD %CQC_SRCTREE%\Source\AllProjects\CQCTrayMon
GOTO EXIT

:CD_CQCTREEBRWS
CD %CQC_SRCTREE%\Source\AllProjects\RemBrws\CQCTreeBrws
GOTO EXIT

:CD_CQCSRVFW
CD %CQC_SRCTREE%\Source\AllProjects\CQCSrvFw
GOTO EXIT

:CD_CQCIR
CD %CQC_SRCTREE%\Source\AllProjects\CoreTech\CQCIR
GOTO EXIT

:CD_CQCKIT
CD %CQC_SRCTREE%\Source\AllProjects\CQCKit
GOTO EXIT

:CD_CQCMEDIA
CD %CQC_SRCTREE%\Source\AllProjects\CQCMedia
GOTO EXIT

:CD_CQCVOICE
CD %CQC_SRCTREE%\Source\AllProjects\CQCVoice
GOTO EXIT

:CD_CQCWEBSRV
CD %CQC_SRCTREE%\Source\AllProjects\CQCWebSrv
GOTO EXIT

:CD_CQCWEBSRVC
CD %CQC_SRCTREE%\Source\AllProjects\CQCWebSrv\Client
GOTO EXIT

:CD_WEBRIVACMP
CD %CQC_SRCTREE%\Source\AllProjects\CQCWebSrv\Client\WebRIVACmp
GOTO EXIT

:CD_CQSLREPO
CD %CQC_SRCTREE%\Source\AllProjects\Drivers\CQSLRepo
GOTO EXIT

:CD_OUTPUT
CD %CQC_SRCTREE%\Output
GOTO EXIT

:CD_RES
CD %CQC_RESDIR%
GOTO EXIT

:CD_SOURCE
CD %CQC_SRCTREE%\Source
GOTO EXIT

:CD_TESTS2
CD %CQC_SRCTREE%\Source\AllProjects\Tests2
GOTO EXIT


:CD_TESTMACROS
CD %CQC_SRCTREE%\Source\AllProjects\CQCMacroEng\TestMacros
GOTO EXIT

:CD_THEMES
CD %CQC_SRCTREE%\Source\AllProjects\CQCIntf\Themes
GOTO EXIT

:CD_TMPTEST
CD %CQC_SRCTREE%\Source\AllProjects\Utilities\TmpTest
GOTO EXIT

:CD_UTILITIES
CD %CQC_SRCTREE%\Source\AllProjects\Utilities
GOTO EXIT

:CD_WEBRIVA
CD %CQC_SRCTREE%\Source\AllProjects\Web\WebRIVA
GOTO EXIT

:CD_WEBDEV
CD %CQC_SRCTREE%\Source\AllProjects\Web\
GOTO EXIT

:CD_WEBRES
CD %CQC_DATADIR%\HTMLRoot\CQSL
GOTO EXIT

:CD_ZWAVEUSB3
CD %CQC_SRCTREE%\Source\AllProjects\Drivers\ZWave\ZWaveUSB3


:EXIT
if "%2"=="" GOTO NO_SUB
CD %2
:NO_SUB

