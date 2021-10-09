@ECHO OFF
SETLOCAL
@ECHO.
@ECHO.
ECHO Web RIVA Compiler
@ECHO.

REM We output ot directories relative to the data directory
SET TARPATH=%CQC_DATADIR%

REM Change to the project directory
SET OURROOTDIR=%CQC_SRCTREE%\Source\AllProjects\Web\WebRIVA
CD %OURROOTDIR%

REM
REM Any options that are not input/output directory specific are set in the
REM tsconfig file. So we just need to know where to spit out the files to.
REM
CALL tsc.cmd --outDir %TARPATH%\HTMLRoot\CQSL\WebRIVA\Src

IF NOT EXIST %TARPATH%\HTMLRoot\CQSL\WebRIVA\ (
    MD %TARPATH%\HTMLRoot\CQSL\WebRIVA\
)

IF NOT EXIST %TARPATH%\HTMLRoot\CQSL\WebRIVA\Src\ (
    MD %TARPATH%\HTMLRoot\CQSL\WebRIVA\Src\
)

IF NOT EXIST %TARPATH%\HTMLRoot\CQSL\WebRIVA\images\ (
    MD %TARPATH%\HTMLRoot\CQSL\WebRIVA\images\
)

IF NOT EXIST %TARPATH%\HTMLRoot\CQSL\WebRIVA\scripts\ (
    MD %TARPATH%\HTMLRoot\CQSL\WebRIVA\scripts\
)

IF NOT EXIST %TARPATH%\HTMLRoot\CQSL\WebRIVA\styles\ (
    MD %TARPATH%\HTMLRoot\CQSL\WebRIVA\styles\
)

REM Copy any other files that need to be copied
XCOPY /D /Y .\src\Websocket.js %TARPATH%\HTMLRoot\CQSL\WebRIVA\Src\
XCOPY /D /Y .\images\* %TARPATH%\HTMLRoot\CQSL\WebRIVA\images\
XCOPY /D /Y .\scripts\* %TARPATH%\HTMLRoot\CQSL\WebRIVA\scripts\
XCOPY /D /Y .\styles\* %TARPATH%\HTMLRoot\CQSL\WebRIVA\styles\
XCOPY /D /Y .\index.html %TARPATH%\HTMLRoot\CQSL\WebRIVA\
XCOPY /D /Y .\manifest.json %TARPATH%\HTMLRoot\CQSL\WebRIVA\

REM Only done if in debug mode
IF "%CID_BUILDMODE%"=="Dev" (
    XCOPY /D /Y .\src\*.ts %TARPATH%\HTMLRoot\CQSL\WebRIVA\Src\
)
