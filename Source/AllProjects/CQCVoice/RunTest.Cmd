@ECHO OFF
SETLOCAL
SET APPCMD=%CQC_RESDIR%\CQCVoice.exe /Grammar=CQCVoice.grxml /BTree=CQCVoice_BTree.xml

IF "%1"=="debug" GOTO DO_DEBUG
start "CQC Voice" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%

:DONE

