@ECHO OFF
SETLOCAL
SET APPCMD=%CID_RESDIR%\CQCImgLoader.exe /Cmd=GenSysImages /InFile=%CQC_SRCTREE%\RawIntfImgs\IntfImages.xml /OutFile=%CQC_DATADIR%

IF "%1"=="debug" GOTO DEBUG
%APPCMD%
GOTO END

:DEBUG
devenv /debugexe %APPCMD%

:END

