@ECHO OFF

REM
REM Create a non-quoted version of the notes parameter or things get weird.
REM
SET RELNOTES=%2
SET RELNOTES=%RELNOTES:"=%

IF "%1"=="" GOTO BAD_PARMS
IF "%RELNOTES%"=="" GOTO BAD_PARMS

ECHO[
ECHO ----------------
ECHO      Target: %1
ECHO       Notes: "%RELNOTES%
ECHO ----------------
ECHO[

SETLOCAL
SET TARGET=%1

REM Locally alias these for future flexibility
SET CQCDATA=%CQC_DATADIR%
SET CIDLIBRESDIR=%CIDLIB_RESDIR%
SET CQCRESDIR=%CQC_RESDIR%
SET CQCSRCDIR=%CQC_SRCTREE%

REM ---------------------------------------------------------------------
REM If the target already exists, don't do anything
REM ---------------------------------------------------------------------
IF EXIST "%TARGET%" GOTO TARGET_EXISTS


REM ---------------------------------------------------------------------
REM We need three sub-dirs under the target. One for the install image, one
REM for the extractor files, and one for the self-extractor.
REM ---------------------------------------------------------------------
SET TARGET_IMG=%TARGET%\InstImage
SET TARGET_EXTR=%TARGET%\Extractor
SET TARGET_SELFEXTR=%TARGET%\SelfExtractor


REM ---------------------------------------------------------------------
REM Make sure they all exist
REM ---------------------------------------------------------------------
md %TARGET% > Nul
md %TARGET_IMG% > Nul
md %TARGET_EXTR% > Nul
md %TARGET_SELFEXTR% > Nul



REM ---------------------------------------------------------------------
REM Copy over the installer program itself, the release notes, and any other
REM stuff that goes into the root directory of the image.
REM ---------------------------------------------------------------------
COPY %CQCRESDIR%\CQCInst.exe* %TARGET_IMG%\ > Nul
COPY %CQCRESDIR%\CQCInst_*.CIDMsg %TARGET_IMG%\ > Nul
COPY %CQCRESDIR%\CQCInst.CIDRes %TARGET_IMG%\ > Nul
COPY %CQCSRCDIR%\Source\Docs\ReleaseNotes.htm %TARGET_IMG%\ > Nul



REM ---------------------------------------------------------------------
REM  Make the install image and extractor directories
REM ---------------------------------------------------------------------
ECHO Making install images
md %TARGET_IMG%\Image > Nul
md %TARGET_IMG%\Image\AppIcons > Nul
md %TARGET_IMG%\Image\AppImages > Nul
md %TARGET_IMG%\Image\CIDLib > Nul
md %TARGET_IMG%\Image\Client > Nul
md %TARGET_IMG%\Image\CQC > Nul
md %TARGET_IMG%\Image\CQC\LogoImages > Nul
md %TARGET_IMG%\Image\Help > Nul
md %TARGET_IMG%\Image\GWServer > Nul
md %TARGET_IMG%\Image\HTML > Nul
md %TARGET_IMG%\Image\HTML\WebRIVA > Nul
md %TARGET_IMG%\Image\HTML\WebRIVAExt > Nul
md %TARGET_IMG%\Image\Images > Nul
md %TARGET_IMG%\Image\Interfaces > Nul
md %TARGET_IMG%\Image\CQCVoice > Nul
md %TARGET_IMG%\Image\Local\DevClasses > Nul
md %TARGET_IMG%\Image\Local\ZWInfo > Nul
md %TARGET_IMG%\Image\Macros\System > Nul
md %TARGET_IMG%\Image\Manifests > Nul
md %TARGET_IMG%\Image\Master > Nul
md %TARGET_IMG%\Image\Protocols > Nul
md %TARGET_IMG%\Image\Server > Nul
md %TARGET_IMG%\Image\Utils > Nul
md %TARGET_IMG%\Image\WebServer > Nul
md %TARGET_IMG%\Image\ZWU3Info> Nul


REM ---------------------------------------------------------------------
REM  Copy over the CIDLib support files needed by CQC.
REM ---------------------------------------------------------------------
ECHO Copying CIDLib support files
CALL CopyOneEx %CIDLIBRESDIR% CIDAI InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDAppSh InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDAudStream InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDComm InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDCrypto InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDCtrls InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDDAE InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDDBase InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDEncode InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDGraphDev InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDImage InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDImgFact InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDJPEG InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDKernel InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDLib InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDMacroDbg InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDMacroEng InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDMath InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDMData InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDMetaExtr InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDNet InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDObjStore InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDOrb InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDOrbUC InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDPack InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDPNG InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDRegX InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDSChan InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDSock InstImage\Image\CIDLib\
CALL CopyOneEx %CIDLIBRESDIR% CIDSpreco InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDUPnP InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDWebBrowser InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDWUtils InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDXML InstImage\Image\CIDLib
CALL CopyOneEx %CIDLIBRESDIR% CIDZLib InstImage\Image\CIDLib


COPY %CIDLIBRESDIR%\AppImages\*.PNG %TARGET_IMG%\Image\AppImages\ > Nul
COPY %CIDLIBRESDIR%\CIDCommonText_en.CIDMsg %TARGET_IMG%\Image\CIDLib\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the CQC support files required. These are are installed in any type
REM  of CQC installation.
REM
REM  NOTE:  The generic drivers are used by the driver test harnesses, not
REM         just by CQCServer, so we treat it as a support file and as a
REM         driver, so we include them in the CQC image, and below in the
REM         loadable drivers.
REM
REM         The MQTTSh facility is used by the MQTT driver but also by the driver's
REM         config file checker program.
REM ---------------------------------------------------------------------
ECHO Copying CQC support files
CALL CopyOneEx %CQCRESDIR% CQCAct InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCDriver InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCEvCl InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCEvMonEng InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCGKit InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCGWCl InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCIGKit InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCIntfEd  InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCIntfEng  InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCIntfWEng  InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCIR InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCKit InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCMedia InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCMEng InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCMQTT InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCPollEng InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCRemBrws InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCRPortClient InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCSrvFW InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCSysCfg InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCTreeBrws InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCWebRIVA InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQCWebSrvC InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% CQLogicSh InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% GC100Ser InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% JAPwrSer InstImage\Image\CQC

CALL CopyOneEx %CQCRESDIR% CQCGenDrvS InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% GenProtoS InstImage\Image\CQC
CALL CopyOneEx %CQCRESDIR% MQTTSh InstImage\Image\CQC

COPY %CQCRESDIR%\AppImages\*.PNG %TARGET_IMG%\Image\AppImages\ > Nul
COPY %CQCRESDIR%\LogoImages\*.PNG %TARGET_IMG%\Image\CQC\LogoImages\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the standard app icons from both code bases
REM ---------------------------------------------------------------------
ECHO Copying app icons
COPY %CIDLIBRESDIR%\AppIcons\*.ico %TARGET_IMG%\Image\AppIcons\ > Nul
COPY %CQCRESDIR%\AppIcons\*.ico %TARGET_IMG%\Image\AppIcons\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the C++ drivers. In some cases the server side is a CML based
REM  driver, but it still has a client side C++ driver. This command file
REM  will just copy what's there. Some also have a shared facility in
REM  addition to the main driver facility, so we pass in two parameters
REM  for those. The first can be suffixed with S or C for the server or
REM  client side driver. The second is taken as is, since it might not
REM  have a name directly related to the driver name.
REM ---------------------------------------------------------------------
ECHO Copying C++ driver files
CALL CopyOneDriver BarixAudPl
CALL CopyOneDriver CQCGenDrv
CALL CopyOneDriver CQCHostMon
CALL CopyOneDriver CQSLAudPl
CALL CopyOneDriver CQSLRepo
CALL CopyOneDriver CQSLLogic CQSLLogicSh
CALL CopyOneDriver CQSLSpeech
CALL CopyOneDriver DVDProfiler
CALL CopyOneDriver ElkM1
CALL CopyOneDriver ElkM1V2
CALL CopyOneDriver FileTagRepo
CALL CopyOneDriver ExtronVSW
CALL CopyOneDriver GenHTTPTrig
CALL CopyOneDriver GenLTrig
CALL CopyOneDriver GenLTCPOut
CALL CopyOneDriver GenTrig
CALL CopyOneDriver GC-100
CALL CopyOneDriver GC-IRE
CALL CopyOneDriver GC-IRL
CALL CopyOneDriver GenProto
CALL CopyOneDriver HAIOmni HAIOmniSh
CALL CopyOneDriver HAIOmniTCP HAIOmniTCPSh
CALL CopyOneDriver HAIOmniTCP2 HAIOmniTCP2Sh
CALL CopyOneDriver IntfVCtrl
CALL CopyOneDriver IRMan
CALL CopyOneDriver iTunesPlayerTM
CALL CopyOneDriver iTunesRendTM
CALL CopyOneDriver iTunesRepo
CALL CopyOneDriver iTunesRepo2
CALL CopyOneDriver iTunesRepoTM
CALL CopyOneDriver JRDiskRepo
CALL CopyOneDriver LutronHW
CALL CopyOneDriver LutronRA2
CALL CopyOneDriver MediaChMgr
CALL CopyOneDriver MQTT MQTTSh
CALL CopyOneDriver MyMoviesRepo
CALL CopyOneDriver QuadScan
CALL CopyOneDriver PCIIRO
CALL CopyOneDriver PlexRepo
CALL CopyOneDriver R2DI
CALL CopyOneDriver RedRat2
CALL CopyOneDriver SonosZPProp
CALL CopyOneDriver UPnPMediaRend
CALL CopyOneDriver USBUIRT
CALL CopyOneDriver VarDriver
CALL CopyOneDriver Wunderground
CALL CopyOneDriver ZWaveLevi ZWaveLeviSh
CALL CopyOneDriver ZWaveLevi2 ZWaveLevi2Sh
CALL CopyOneDriver ZWaveUSB ZWaveUSBSh
CALL CopyOneDriver ZWaveUSB3 ZWaveUSB3Sh

REM These don't follow the naming convention that CopyOneDriver needs
CALL CopyOneDriver2 IRClient


REM ---------------------------------------------------------------------
REM  Copy over the device class XML/DTD files
REM ---------------------------------------------------------------------
ECHO Copying device class files
COPY %CQCDATA%\Local\DevClasses\*.xml %TARGET_IMG%\Image\Local\DevClasses\ > Nul
COPY %CQCDATA%\Local\DevClasses\*.DTD %TARGET_IMG%\Image\Local\DevClasses\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the ZWave device info, XML, and DTD files. We have to do the old VRC0P
REM  one still for now, and the new one
REM ---------------------------------------------------------------------
ECHO Copying Z-Wave device class files

REM
REM For this one, we have to make sure the files are copied to the local directory
REM first, then run the zwdi compiler to build the index file.
cidbuild /target=ZWDevInfo
zwdicomp
COPY %CQCDATA%\Local\ZWInfo\*.ZWInfo %TARGET_IMG%\Image\Local\ZWInfo\ > Nul
COPY %CQCDATA%\Local\ZWInfo\*.xml %TARGET_IMG%\Image\Local\ZWInfo\ > Nul
COPY %CQCDATA%\Local\ZWInfo\*.DTD %TARGET_IMG%\Image\Local\ZWInfo\ > Nul

REM
REM  In this case, the zwdi compiler copies the files to the right place and builds
REM  the index file at the same time.
REM
zwu3dicomp
COPY %CQCDATA%\DataServer\ZWU3Info\*.zwdi %TARGET_IMG%\Image\ZWU3Info\ > Nul
COPY %CQCDATA%\DataServer\ZWU3Info\*.xml %TARGET_IMG%\Image\ZWU3Info\ > Nul
COPY %CQCDATA%\DataServer\ZWU3Info\*.DTD %TARGET_IMG%\Image\ZWU3Info\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the driver manifest files
REM ---------------------------------------------------------------------
ECHO Copying manifest files
COPY %CQCRESDIR%\Manifests.Out\*.Manifest %TARGET_IMG%\Image\Manifests\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the driver protocol files
REM ---------------------------------------------------------------------
ECHO Copying protocol files
COPY %CQCRESDIR%\Protocols.Out\*.CQCProto %TARGET_IMG%\Image\Protocols\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the CQC Master Server programs. Some are from CIDLib and
REM  some from CQC.
REM ---------------------------------------------------------------------
ECHO Copying master server programs
CALL CopyOneEx %CIDLIBRESDIR% CIDLogSrv InstImage\Image\Master
CALL CopyOneEx %CIDLIBRESDIR% CIDNameSrv InstImage\Image\Master
CALL CopyOneEx %CQCRESDIR% CQCDataSrv InstImage\Image\Master
CALL CopyOneEx %CQCRESDIR% CQCEventSrv InstImage\Image\Master
CALL CopyOneEx %CQCRESDIR% CQLogicSrv InstImage\Image\Master


REM ---------------------------------------------------------------------
REM  Copy over the CQC Server programs
REM ---------------------------------------------------------------------
ECHO Copying CQCServer programs
CALL CopyOneEx %CQCRESDIR% CQCServer InstImage\Image\Server


REM ---------------------------------------------------------------------
REM  Copy over the XML Gateway Server programs
REM ---------------------------------------------------------------------
ECHO Copying XML Gateway programs
CALL CopyOneEx %CQCRESDIR% CQCGWSrv InstImage\Image\GWServer
COPY %CQCRESDIR%\CQCGWSrv_*.CIDMsg %TARGET_IMG%\Image\GWServer\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the Web Server programs
REM ---------------------------------------------------------------------
ECHO Copying Web Server programs
CALL CopyOneEx %CQCRESDIR% CQCWebSrv InstImage\Image\WebServer

REM ---------------------------------------------------------------------
REM  Copy over the CQC client programs
REM ---------------------------------------------------------------------
ECHO Copying CQC client programs
CALL CopyOneEx %CQCRESDIR% MediaRepoMgr InstImage\Image\Client
CALL CopyOneEx %CQCRESDIR% CQCAdmin InstImage\Image\Client
CALL CopyOneEx %CQCRESDIR% CQCDrvDev InstImage\Image\Client
CALL CopyOneEx %CQCRESDIR% CQCIntfView InstImage\Image\Client
CALL CopyOneEx %CQCRESDIR% CQCVoice InstImage\Image\Client
CALL CopyOneEx %CQCRESDIR% CQCProtoDev InstImage\Image\Client
CALL CopyOneEx %CQCRESDIR% CQCTrayMon InstImage\Image\Client


REM ---------------------------------------------------------------------
REM  Copy over the CQCVoice related stuff
REM ---------------------------------------------------------------------
ECHO Copying CQCVoice support files
COPY %CQCDATA%\CQCVoice\* %TARGET_IMG%\Image\CQCVoice > Nul


REM ---------------------------------------------------------------------
REM  Copy over the CIDLib and CQC general utilities
REM ---------------------------------------------------------------------

ECHO Copying CIDLib general utils
CALL CopyOneEx %CIDLIBRESDIR% Crypto1 InstImage\Image\Utils
CALL CopyOneEx %CIDLIBRESDIR% CIDLogAdmin InstImage\Image\Utils
CALL CopyOneEx %CIDLIBRESDIR% CIDNSAdmin InstImage\Image\Utils
CALL CopyOneEx %CIDLIBRESDIR% CIDCfgSrv InstImage\Image\Utils
CALL CopyOneEx %CIDLIBRESDIR% CIDLogMon InstImage\Image\Client
CALL CopyOneEx %CIDLIBRESDIR% EnumMons InstImage\Image\Utils
CALL CopyOneEx %CIDLIBRESDIR% XMLDemo1 InstImage\Image\Utils

ECHO Copying CQC general utils
CALL CopyOneEx %CQCRESDIR% CheckMQTTCfg InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCAppShell InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCAppShellLib InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCBulkLoader InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCClService InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCEventDump InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCExtractDrvFlds InstImage\Image\Utils
REM CALL CopyOneEx %CQCRESDIR% CQCIdGen InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% GenIRRepo InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCNetTest InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCNSShell InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCRPortSrv InstImage\Image\Client
REM CALL CopyOneEx %CQCRESDIR% CQSLRepoImp InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCRescaleImgs InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCTerminal InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCUninst InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCWBHelper InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% CQCWCHelper InstImage\Image\Utils
CALL CopyOneEx %CQCRESDIR% GenDSReport InstImage\Image\Utils



REM ---------------------------------------------------------------------
REM  Copy over the pre-fab user interface images
REM ---------------------------------------------------------------------
ECHO Copying interface art
XCOPY %CQCDATA%\DataServer\Images\System\* %TARGET_IMG%\Image\Images\ /s > Nul


REM ---------------------------------------------------------------------
REM  Copy over the pre-fab user interfaces (which includes the themes)
REM ---------------------------------------------------------------------
ECHO Copying interfaces
XCOPY %CQCDATA%\DataServer\Interfaces\System\* %TARGET_IMG%\Image\Interfaces\ /s > Nul


REM ---------------------------------------------------------------------
REM  Copy over any third party support files.
REM
REM  We currently use the Scintilla library for our CML editor. We copy it to the CQC
REM  specific section for executables so it'll end up in the Bin directory and get found
REM  by dint of being in the same directory as the programs using it.
REM ---------------------------------------------------------------------
ECHO Copying Third Party Files
COPY %CID_DEPENDS%\SDKS\Scintilla\Scintilla.dll %TARGET_IMG%\Image\Client\ > Nul
COPY %CID_DEPENDS%\SDKS\Scintilla\SciLexer.dll %TARGET_IMG%\Image\Client\ > Nul


REM ---------------------------------------------------------------------
REM  Copy over the macros in the System section
REM ---------------------------------------------------------------------
ECHO Copying CML macros
XCOPY %CQCSRCDIR%\Source\AllProjects\Macros\System\*.mengc2 %TARGET_IMG%\Image\Macros\System\ /s > Nul


REM ---------------------------------------------------------------------
REM  We need the language runtime in the installer directory. It'll be copied from
REM  there by the installer to the CQC installation. The debug directory has more
REM  files since some stuff won't be on the target machine if the development system
REM  isn't intalled there.
REM ---------------------------------------------------------------------
ECHO Copying runtime libraries
IF "%CID_BUILDMODE%"=="Dev" GOTO DEVRT

XCOPY "%CID_DEPENDS%\VCRuntime\CRT\*" "%TARGET_IMG%\" > Nul
GOTO RTL_DONE

:DEVRT
XCOPY "%CID_DEPENDS%\VCRuntime\DebugCRT\*" "%TARGET_IMG%\" > Nul

:RTL_DONE


REM ---------------------------------------------------------------------
REM  Build the help content and then copy it over to its own section of the image.
REM ---------------------------------------------------------------------
ECHO Generating Help files
CQCDocComp
ECHO Copy Help files
XCOPY "%CQCDATA%\HTMLRoot\Web2\CQCDocs\*" "%TARGET_IMG%\Image\Help\" /s > Nul


REM ---------------------------------------------------------------------
REM Copy over the Web RIVA app code. Do a compile to make sure it is up to date. Delete
REM the map and ts files since those are not distributed. They may not be there if the
REM last compile after a clean was done in prod mode, since they aren't generated then.
REM
REM We also need to copy some support files that are used to enable web camera support
REM via various schemes.
REM ---------------------------------------------------------------------
ECHO Compiling WebRIVA client
CALL "%CQCSRCDIR%\Source\AllProjects\Web\WebRIVA\CmpWRCl.cmd"
ECHO Copying WebRIVA client
XCOPY %CQCDATA%\HTMLRoot\CQSL\WebRIVA "%TARGET_IMG%\Image\HTML\WebRIVA\" /s > Nul
del "%TARGET_IMG%\Image\HTML\WebRIVA\src\*.map" > Nul
del "%TARGET_IMG%\Image\HTML\WebRIVA\src\*.ts" > Nul

ECHO Copying WebRIVA sample files
XCOPY "%CQCDATA%\HTMLRoot\WebRIVAExt\Samples\*" "%TARGET_IMG%\Image\HTML\WebRIVAExt\" /s > Nul


REM ---------------------------------------------------------------------
REM Now we need to create the image for the extractor program. We have to:
REM
REM 1. Run our little Pack1 sample program to pack up the installer image we
REM    just created above, packed to the extractor directory.
REM 2. Copy the extractor EXE and it's files there.
REM ---------------------------------------------------------------------

ECHO[
ECHO[
ECHO Creating installer image CIDPack
ECHO[
Pack1 /pack %TARGET_IMG%\ %TARGET_EXTR%\CQCImage_%CID_VERSIONUL%.CIDPack  /Version=%CID_VERSION% "/Notes=%RELNOTES%"
CALL CopyOneEx %CQCRESDIR% CQCInstUnp Extractor
COPY %CIDLIBRESDIR%\CIDCommonText_en.CIDMsg %TARGET_EXTR% > Nul

REM ---------------------------------------------------------------------
REM  Now we need to create the self extractor. We have to:
REM
REM 1. Copy the main self extractor files to a local directory we created for it
REM 2. Compile the self-extractor, which will grab files as required out of the
REM    other directories.
REM ---------------------------------------------------------------------
COPY %CQC_SRCTREE%\Source\AllProjects\Installer\CQCInstSE\* %TARGET_SELFEXTR% > Nul
CD %TARGET_SELFEXTR%
CALL CmpCQCInstSE.Cmd

REM Normal exit
GOTO FINAL_EXIT


:BAD_PARMS
ECHO[
ECHO Incorrect Parameters
ECHO Usage: MakeInstallImage cidlibsrc cqcsrc targetdir
ECHO[
GOTO FINAL_EXIT

:TARGET_EXISTS
ECHO[
ECHO The target directory already exists, exiting!
ECHO[
GOTO FINAL_EXIT

:FINAL_EXIT
