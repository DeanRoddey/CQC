@ECHO OFF
ECHO Cleaning Room Cfg Client output
SETLOCAL
SET OutDir=%CQC_DATADIR%\HTMLRoot\CQSL\WebRIVA

DEL %OutDir%\*.ts /s /q
DEL %OutDir%\*.js /s /q
DEL %OutDir%\*.js.map /s /q

IF NOT EXIST "%OutDir%\index.html" GOTO NO_INDEX
DEL %OutDir%\index.html
:NO_INDEX

DEL %OutDir%\images\* /q
DEL %OutDir%\scripts\* /q
DEL %OutDir%\styles\* /q
