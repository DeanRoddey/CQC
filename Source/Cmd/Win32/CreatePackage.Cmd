@ECHO OFF
SETLOCAL
SET APPCMD=CQCImgLoader.exe /Cmd=Package /InFile=%CQC_SRCTREE%\RawIntfImgs\Packs\ElkImages /OutFile=%CQC_RESDIR%\ImgPacks\ElkImages\ElkM1

IF "%1"=="debug" GOTO DEBUG
%APPCMD%
GOTO END

:DEBUG
devenv /debugexe %APPCMD%

:END

