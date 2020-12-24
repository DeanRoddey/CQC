@ECHO OFF
SETLOCAL
SET APPCMD=%CQC_RESDIR%\CQCGWSrv.exe /GlobalLogMode=Failed /DataDir=%CQC_DATADIR%

IF "%1"=="debug" GOTO DO_DEBUG
start "Gateway Server" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%

:DONE


REM


