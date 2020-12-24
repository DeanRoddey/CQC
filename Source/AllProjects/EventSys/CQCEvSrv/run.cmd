@ECHO OFF
SETLOCAL
SET APPCMD=%CQC_RESDIR%\CQCEventSrv.exe /DataDir=%CQC_DATADIR%

IF "%1"=="debug" GOTO DO_DEBUG
start "CQC Events Server" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%

:DONE

