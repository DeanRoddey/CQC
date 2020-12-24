@ECHO OFF
SETLOCAL

IF %1=="" GOTO NO_COMMENT

REM
REM Just calls the install image maker with the default parms. We have to
REM build a target file name based on the version info in the environment.
REM
SET INSTTARFILE=%CQCInstImages%\InstImage_%CID_VERSIONUL%

ECHO[
ECHO Making Image: %INSTTARFILE%
ECHO      Comment: %1
ECHO[

CALL MakeInstallImage.cmd %INSTTARFILE% %1
GOTO END

:NO_COMMENT
ECHO[
ECHO You must provide a release notes comments
ECHO[

:END
