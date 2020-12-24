@ECHO OFF

REM For driver testing we often want to bring down CQCServer and then
REM start it back up, so we have this separate command file to start it.

SETLOCAL
PUSHD .\
SET APPCMD=%CQC_RESDIR%\CQCServer.exe /VerboseLog /DataDir=%CQC_DATADIR%

IF "%1"=="debug" GOTO DO_DEBUG
start "CQC Server" %APPCMD%
GOTO DONE

REM If there's an existing solution, open that, else start a new one
:DO_DEBUG
IF NOT EXIST "%CQC_RESDIR%\CQCServer.sln" GOTO NEWSESS
devenv %CQC_RESDIR%\CQCServer.sln
GOTO DONE

:NEWSESS
devenv /debugexe %APPCMD%

:DONE
POPD

