@ECHO OFF
REM
REM Pass the name of your certificate, assumed to be in the MStore:My, section of
REM the certificate store.
REM

IF "%1"=="" GOTO NO_CERT

SETLOCAL
SET APPCMD=%CQC_RESDIR%\CQCWebSrv.exe /DataDir=%CQC_DATADIR% /HTTPPort=80 /HTTPSPort=443 "/CertInfo=MStore:My,%1" /SecureHelp

IF "%2"=="debug" GOTO DO_DEBUG
start "Web Server" %APPCMD%
GOTO DONE

:DO_DEBUG
devenv /debugexe %APPCMD%
GOTO DONE

:NO_CERT
ECHO You must provide a certificate store indicator

:DONE

