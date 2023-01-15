@ECHO OFF
REM Wrapper Script for vMicro Web Debugger
TITLE vMicro Web Debug Starter
color 18
REM Set the Below Variables to suit your Setup:-
SET SERIALPORT=COM3
SET IPADDRESS=127.0.0.1
SET BAUDRATE=115200
SET HTMLPORT=8212
SET WEBSOCKETPORT=8213
REM Set these as needed to true, e.g. Nano 33 BLE needs both true (case sensitive)
SET USEDTR=false
SET USERTS=false
REM The below are optional if accessing socket from a remote source (e.g. via DDNS)
SET REMOTEWEBSOCKETADDRESS=
SET REMOTEWEBSOCKETPORT=
SET TEENSYTOOLSFOLDER=
REM Kick off the Webserver
start "vMicro Webserver" /D "%~dp0" "MicroWebserver.exe" %SERIALPORT% %BAUDRATE% %IPADDRESS% %HTMLPORT% %WEBSOCKETPORT%  %USEDTR% %USERTS% "%REMOTEWEBSOCKETADDRESS%" "%REMOTEWEBSOCKETPORT%" "%TEENSYTOOLSFOLDER%"
