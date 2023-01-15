// webserver.h

#ifndef _WEBSERVER_h
#define _WEBSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <ESP8266WebServer.h>
#include <DNSServer.h>

extern ESP8266WebServer server(80);

//Prototypen
void initWebserver();
bool startWifi();
void handle_mysql();
void handle_Wifi();
void handle_WebRoot();
void handle_WebNotFound();

#endif


