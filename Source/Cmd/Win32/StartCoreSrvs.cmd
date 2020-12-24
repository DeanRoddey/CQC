@ECHO OFF
SETLOCAL
PUSHD .\

start /MIN "Name Server" CIDNameSrv /NoLease "/LocalLog=%CQC_DATADIR%\Logs\NS.Txt;UTF-8"
start /MIN "Log Server" CIDLogSrv "/LocalLog=%CQC_DATADIR%\Logs\LS.Txt;UTF-8" "/Outpath=%CQC_DATADIR%\Logs"
start /MIN "Cfg Server" CIDCfgSrv /VerboseLog "/RepoPath=%CQC_DATADIR%\Repository" /AltNSScope=/CQC/MSCfg
