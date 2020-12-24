@ECHO OFF
SETLOCAL

REM Set the user name and password for the CQCVoice account in the environment
SET APPCMD=%CQC_RESDIR%\CQCVoice.exe /User=%CQCVOICE_USER% /Password=%CQCVOICE_PASSWORD% /SpeakReplies

IF "%1"=="debug" GOTO DO_DEBUG
start "CQC Voice" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%

:DONE
