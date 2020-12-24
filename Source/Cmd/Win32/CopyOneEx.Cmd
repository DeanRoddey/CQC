REM
REM  This file takes three parms:
REM
REM     1. The result path to copy from
REM     2. The root name of an executable or library
REM     3. The sub dir under the top level release target to copy to.
REM
REM It will look for any of the files that make up such an executable
REM package and copy any that exist over. This can be called for CIDLib
REM or CQC files, so we
REM

SETLOCAL
SET FOUNDANY=No
SET FOUNDLIBEXE=No

REM If there's a DLL, then copy it
IF NOT EXIST %1\%2*.dll GOTO NO_DLL
COPY %1\%2*.dll %TARGET%\%3\ > Nul
SET FOUNDANY=Yes
SET FOUNDLIBEXE=Yes
:NO_DLL

REM If there's an Exe, then copy it
IF NOT EXIST %1\%2*.exe GOTO NO_EXE
COPY %1\%2*.exe %TARGET%\%3\ > Nul
SET FOUNDANY=Yes
SET FOUNDLIBEXE=Yes
:NO_EXE

REM If there's a resource file, copy it
IF NOT EXIST %1\%2*.CIDRes GOTO NO_RES
COPY %1\%2*.CIDRes %TARGET%\%3\ > Nul
SET FOUNDANY=Yes
:NO_RES

REM If there's a message file, copy it
IF NOT EXIST %1\%2*.CIDMsg GOTO NO_MSGS
COPY %1\%2*.CIDMsg %TARGET%\%3\ > Nul
SET FOUNDANY=Yes
:NO_MSGS

IF "%FOUNDANY%"=="Yes" GOTO FOUND_SOME
ECHO !!!No files were found for Exe/Lib '%2' in path '%1'
:FOUND_SOME

IF "%FOUNDLIBEXE%"=="Yes" GOTO FOUND_LIBEXE
ECHO !!!No Lib or Exe files were found for '%2' in path '%1'
:FOUND_LIBEXE

