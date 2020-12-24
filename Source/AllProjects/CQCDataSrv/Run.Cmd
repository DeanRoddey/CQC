@ECHO OFF
SETLOCAL
SET APPCMD=%CQC_RESDIR%\CQCDataSrv.exe /DataDir=%CQC_DATADIR%

IF "%1"=="debug" GOTO DO_DEBUG
start "CQC Data Server" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%

:DONE

