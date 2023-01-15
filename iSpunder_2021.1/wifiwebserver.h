// wifiwebserver.h

#ifndef _WIFIWEBSERVER_h
#define _WIFIWEBSERVER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "config.h"
#include "filesystem.h"
//#include <MySQL_Generic_WiFi.h>
#include <ESP8266WebServer.h>
//#include <WiFiClient.h>
//#include <ESP8266WiFi.h>
#include "debugSerialOut.h"
//#include <IPAddress.h>


//Prototypen
void rootPage();
void handleWifi();
void handleNotFound();
void handleWebserver();
void startWebserver();
bool startWifi();
//void sendDataToMySQL(float temp, float druck, float carbo, float sollcarbo, String iSpunderName, String SudName, bool setResetFlag);

#endif

