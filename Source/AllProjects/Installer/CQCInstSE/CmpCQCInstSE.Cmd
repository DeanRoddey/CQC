@ECHO OFF
SETLOCAL
REM ---------------------------------------------------------------
REM  This is a simple command file for building this program for testing
REM  purposes. Generally it is built as part of the installer image
REM  building process (via the MakeInstImage.cmd file.) We have a simple
REM  place holder source data file just for testing here, and build it
REM  into the program. Normally that file is generated on the fly  and
REM  combined with the main cpp file into a temp directory and built
REM  in the installer image generation process.
REM ---------------------------------------------------------------


SET CQC_VERSION=%CID_VERSIONUL%

REM
REM We have to replace some tokens in the RC file with actual values and the
REM resource compiler is too stupid to do it, so we just use a powershell
REM command and do a couple passes.
REM
Powershell -Command "(gc CQCInstSE.RC) -replace 'CQC_VERSION', '%CQC_CIDLIBVER%' | Out-File -encoding ASCII CQCInstSE1.RC"
Powershell -Command "(gc CQCInstSE1.RC) -replace 'CQC_FULL_VERSION', '%CQC_VERSION%' | Out-File -encoding ASCII CQCInstSE_Final.RC"

REM Set up flags for the various programs basd on build mode
If "%CID_BUILDMODE%"=="Dev" GOTO OPTS_DEBUG

SET INSTSEOPTS=/O2 /DNDEBUG /MT
SET INSTSELINKOPTS=
SET INSTSERCOPTS=
GOTO DO_IT

:OPTS_DEBUG
Set INSTSEOPTS=/Od /D_DEBUG /Zi /MTd
SET INSTSELINKOPTS=/debug
SET INSTSERCOPTS=/dCQC_DEBUG


REM And now do the compile
:DO_IT
RC /r %INSTSERCOPTS% CQCInstSE_Final.RC
cl /EHsc "/Fe..\CQCInstSE_%CQC_VERSION%.exe" /DCQC_VERSION=%CQC_CIDLIBVER% /DCQC_FULL_VERSION=%CQC_VERSION% /D_CONSOLE %INSTSEOPTS% CQCInstSE.cpp /link CQCInstSE_Final.Res %INSTSELINKOPTS%
