@ECHO OFF
SETLOCAL
SET APPCMD=%CQC_RESDIR%\CQCDrvDev.exe /SysMacros /Pack

IF "%1"=="debug" GOTO DO_DEBUG
start %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%

:DONE

