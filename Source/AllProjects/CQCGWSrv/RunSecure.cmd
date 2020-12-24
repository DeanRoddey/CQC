@ECHO OFF
REM
REM To invoke the GW server with secure support. Pass the name of your certificate,
REM which is assumed to be in the "MStore:My,xxx" area. It will set the secure port to
REM 9998
REM
SETLOCAL

IF "%1"=="" GOTO NO_CERT

SET APPCMD=%CQC_RESDIR%\CQCGWSrv.exe /GlobalLogMode=Failed /DataDir=%CQC_DATADIR% "/CertInfo=MStore:My,%1" /SecPort=9998

IF "%2"=="debug" GOTO DO_DEBUG
start "Gateway Server" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%
GOTO DONE

:NO_CERT
ECHO You must provide a certificate name

:DONE
