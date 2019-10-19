// config.h

#ifndef _CONFIG_h
#define _CONFIG_h

#pragma once
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#ifndef DEBUG
	//#define DEBUG 1 // uncomment this line to enable serial diagnostic messages
#endif

#include <FS.h>
#include <ArduinoJson.h>
//#include <Bounce2.h>
#include <ESPRotary.h>
#include <Button2.h>
//#include <Wire.h>
#include <brzo_i2c.h>
#include <SSD1306Brzo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>


#define PROG_VERSION "1.0.1"
#define RELAISPIN D5
#define ENCPINSW D7
#define ENCPINDT D8
#define ENCPINCLK D6
#define ADDRESS 0x3C
#define OW_BUS D4
#define OWRESOLUTION 12 // 12bit resolution == 750ms update rate
//#define OWINTERVAL (800 / (1 << (12 - OWRESOLUTION))) 
//#define OWINTERVAL 30000
//#define CFGFILE "/config.json"
#define WIFICONF 1
#define MYSQLCONF 2
#define KARBOCONF 3


/*=== WiFiAccessPoint ===*/
const char* ssidAP = "iSpunder"; // Change your name access point
const char* ssidPass = ""; // Change your password access point
const char* wifi_ip[4] = { "192", "168", "5", "1" }; // Change your ip local network
const char* wifi_subnet[4] = { "255", "255", "255", "0" }; // Change your subnet local network
const char* wifi_gateway[4] = { "192", "168", "5", "1" }; // Change your gateway local network

// Initialize the OLED display using Wire library
//extern ESP_SSD1306 display(OLED_RESET); // FOR I2C
extern SSD1306Brzo display(ADDRESS, SDA, SCL);
//extern SSD1306 display(0x3C, sdaPin, sclPin);
extern ESPRotary ESPR = ESPRotary(ENCPINDT, ENCPINCLK);
extern Button2 BTNA = Button2(ENCPINSW);
//extern WiFiManager WifiMan;
//extern Bounce debouncerSW = Bounce();
extern OneWire oneWire(OW_BUS);
extern DallasTemperature OW(&oneWire);
extern ESP8266WebServer server(80);
//extern WiFiClient client;


#endif

