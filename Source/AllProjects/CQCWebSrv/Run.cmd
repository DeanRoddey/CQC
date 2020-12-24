@ECHO OFF
SETLOCAL
SET APPCMD=%CQC_RESDIR%\CQCWebSrv.exe /DataDir=%CQC_DATADIR% /HTTPPort=80

IF "%1"=="debug" GOTO DO_DEBUG
start "Web Server" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%

:DONE

