@ECHO OFF
SETLOCAL
PUSHD .\

REM Start all or subsets of the servers for development purposes, i.e. run them just as
REM command line programs. It always starts name, log, cfg, and data. Afer that you can
REM indicate specific other ones for testing purposes.
REM
REM If you give no parameter, it will start them all. 
REM
REM devices - Start up CQCServer in addition to the base ones. If needs to start any of
REM           the others it will also start CQCServer.
REM
REM Else it is one of: eventsrv, xmlgwsrv, logicsrv, websrv to start one of the other
REM servers.
REM 

start /MIN "Name Server" CIDNameSrv "/LocalLog=%CQC_DATADIR%\Logs\NS.Txt;UTF-8" /GlobalLogMode=Warn
start /MIN "Log Server" CIDLogSrv "/LocalLog=%CQC_DATADIR%\Logs\LS.Txt;UTF-8" "/Outpath=%CQC_DATADIR%\Logs"
start /MIN "Cfg Server" CIDCfgSrv  "/RepoPath=%CQC_DATADIR%\Repository" /AltNSScope=/CQC/MSCfg
start /MIN "Data Server" CQCDataSrv  "/DataDir=%CQC_DATADIR%"

REM If minimal servers only, then we are done
IF "%1"=="min" GOTO DONE

REM Else start the CQCServer at least. If they just want devices, we are done
start /MIN "CQC Server" CQCServer  "/DataDir=%CQC_DATADIR%"
IF "%1"=="devices" GOTO DONE

REM See if they want to just start one of the other servers
IF "%1"=="eventsrv" GOTO DO_EVENTS
IF "%1"=="xmlgwsrv" GOTO DO_GWSRV
IF "%1"=="logicsrv" GOTO DO_LOGIC
IF "%1"=="websrv" GOTO DO_WEB

REM
REM Nothing special so start them all. Client service won't work in production
REM mode since it is built to run as a service.
REM
IF "%CID_BUILDMODE%"=="Prod" GOTO NO_CLSERV
start /MIN "Client Service" CQCClService  "/DataDir=%CQC_DATADIR%"
:NO_CLSERV

start /MIN "XML GW Server" CQCGWSrv  "/DataDir=%CQC_DATADIR%"
start /MIN "Events Server" CQCEventSrv  "/DataDir=%CQC_DATADIR%"
start /MIN "Web Server" CQCWebSrv  "/DataDir=%CQC_DATADIR%" "/HTTPPort=80"
start /MIN "Logic Server" CQLogicSrv  "/DataDir=%CQC_DATADIR%"
GOTO DONE

REM They wanted just one of the other servers, so do it and exit
:DO_EVENTS
start /MIN "Events Server" CQCEventSrv  "/DataDir=%CQC_DATADIR%"
GOTO DONE

:DO_LOGIC
start /MIN "Logic Server" CQLogicSrv  "/DataDir=%CQC_DATADIR%"
GOTO DONE

:DO_WEB
start /MIN "Web Server" CQCWebSrv  "/DataDir=%CQC_DATADIR%" "/HTTPPort=80"
GOTO DONE

:DO_GWSRV
REM The GW server also indirectly can access the event server
start /MIN "Events Server" CQCEventSrv  "/DataDir=%CQC_DATADIR%"
start /MIN "XML GW Server" CQCGWSrv  "/DataDir=%CQC_DATADIR%"
GOTO DONE

:DONE
POPD

