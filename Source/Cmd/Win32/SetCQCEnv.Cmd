@ECHO OFF
REM ----------------------------------------------------------------------
REM  This file is invoked generally from a command processor short cut
REM  that is set up to open a window for CQC development. The parameters
REM  it expects are as below.
REM
REM  It obviously assumes the presence of a built CIDLib source tree, and
REM  sets up the appropriate paths and such to insure CIDLib stuff is
REM  available.
REM
REM Usage:
REM   CQCDev drive srctree mode cidlibtree platform nsaddr
REM
REM     drive       - The drive where the development tree is, with colon!
REM     srctree     - The root of the source tree with leading slash, so
REM                   something like \Develop\CQC_Dev\ or the like.
REM     mode        - 'Dev' or 'Prod' (development or production)
REM     cidlibtree  - The CIDLib tree we are working against. It assumes
REM                   the source tree, not a distribution tree, so this
REM                   is the full path, without drive or trailing slash. It
REM                   is assumed to be on the same drive as the drive parameter.
REM     platform    - Currently this will be WIN32_WIN7
REM     nsaddr      - The location of the name server, in the standard
REM                   form 'host:port'. If its on the local host on its
REM                   well known port, then you can leave it blank but
REM                   you have to provide a " " placeholder in the command
REM                   line so indicate an empty parameter.
REM ----------------------------------------------------------------------

REM Set the target version to build and the CIDLib version we are building against
SET CID_VERSION=6.0.6
SET CID_VERSIONUL=%CID_VERSION:.=_%

REM For now, the underlying CIDLib is always the same M/m version, though this may change
SET CQC_CIDLIBVER=6_0
SET CQC_CIDLIBVERFULL=%CID_VERSIONUL%

REM Set our command window title
TITLE CQC [%3, %CID_VERSION%]

REM Build up some paths of ours, for convenience below
SET CID_DRIVE=%1
SET CQC_SRCTREE=%1%2
SET CIDLIB_SRCDIR=%1%4
SET CQC_ALL=%CQC_SRCTREE%\Source\AllProjects
SET CQC_RESDIR=%CQC_SRCTREE%\Output\%CID_VERSIONUL%\%3Result
SET CIDLIB_RESDIR=%1%4\Output\%CQC_CIDLIBVERFULL%\%3Result
SET CQSWEB=%1\CQSLWeb

REM This is needed before visual C++ set and by the installer image builder
SET CID_DEPENDS=%CIDLIB_SRCDIR%\Source\Depends

REM
REM  Set up the VC++ environment in prod or debug mode. The file has to
REM  be in the CID_Dev area, so we use the path we just built above.
REM
CALL %CIDLIB_SRCDIR%\Source\Cmd\Win32\VisualC++.Cmd %3

REM Set up the required CIDBuild environment variables
SET CID_SRCTREE=%CQC_SRCTREE%
SET CID_BUILDMODE=%3
SET CID_RESDIR=%CQC_SRCTREE%\Output\%CID_VERSIONUL%\%3Result
SET CID_PLATFORM=%5

REM
REM We let the programs set these, so clear them out.
REM
SET CID_LOCALLOG=

REM And set up the CIDLib paths, and our product specific environment
Path=%Path%;%CIDLIB_RESDIR%;%CQC_RESDIR%;%CQC_SRCTREE%\Source\Cmd\Win32;

REM Set the name server address. Can be empty string, which just undefines it
SET CID_NSADDR=%6

REM And set the data directory root for CQCServer and CQCClient
SET CQC_DATADIR=%1%2\CQCData

REM Set up symbol paths
SET _NT_ALT_SYMBOL_PATH=%CIDLIB_RESDIR%;%CQC_RESDIR%;%_NT_ALT_SYMBOL_PATH%;

REM
REM This is used to indicate to the code that we are in a development
REM setup, not an installed system. In an installed system, the data server
REM will point at the CQCData directory for everything. This will make it
REM point into the source directory for some stuff such as CML content,
REM protocol files, etc... For clients it will make them point to the
REM CQCData directory for everything, where some stuff it normally uses
REM C:\ProgramData.
REM
SET CQC_INDEVENV=1


REM
REM  These are just for testing some stuff that pulls in some special
REM  environment variables. Some tests will depend on these variables
REM  being set.
REM
SET CQC_EnvRTV07=TestVal7
SET CQC_EnvRTV08=TestVal8
SET CQC_EnvRTV09=TestVal9

REM
REM These are for testing the auto-generation system, which can optionally use a
REM environment variable to set the default room.
REM
SET CQC_EnvRTV01=Living Room


REM
REM For local development we can set a server account in environment variables
REM so it doesn't have to be set in the registry (which requires doing an install.)
REM We set it to the account that will be automatically created by the security
REM server in debug mode if there isn't one already there in the repo.
REM
SET CQC_SRVNAME=SecServer
SET CQC_SRVPASS=Welcome
