@ECHO OFF
SETLOCAL
IF "%1"=="debug" GOTO DO_DEBUG

REM Nothing special, just run it
start "%CID_RESDIR%\MediaRepoMgr.exe"
GOTO DONE

REM They want to debug, so either open an existin session, or create a new one
:DO_DEBUG
IF NOT EXIST "%CID_RESDIR%\MediaRepoMgr.sln" GOTO NEWSESS
devenv "%CID_RESDIR%\MediaRepoMgr.sln"
GOTO DONE

:NEWSESS
REM No existing session, so start a new one
devenv /debugexe "%CID_RESDIR%\MediaRepoMgr.exe"
GOTO DONE

:DONE


