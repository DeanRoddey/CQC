@ECHO OFF
SETLOCAL
SET APPCMD=%CQC_RESDIR%\CQCTrayMon.exe /Binding=Test /DataDir=%CQC_DATADIR% /AppCtrl

IF "%1"=="debug" GOTO DO_DEBUG
start "CQC Tray Mon" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%

:DONE


REM

