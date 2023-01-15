#pragma once

#ifndef CONFIG_H
#define CONFIG_H

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#ifndef DEBUG
#define DEBUG 1 // uncomment this line to enable serial diagnostic messages
#endif


#include <ESP8266WiFi.h>

#include <MySQL_Cursor.h>
#include <MySQL_Connection.h>

#include <DallasTemperature.h>
#include <OneWire.h>

#include <Button2.h>
#include <ESPRotary.h>

#define PROG_VERSION "21.1.0"
#define RELAISPIN D5
#define ENCPINSW D7
#define ENCPINDT D8
#define ENCPINCLK D6
#define ADDRESS 0x3C
#define OW_BUS D4
#define OWRESOLUTION 12 // 12bit resolution == 750ms update rate
#define WIFICONF 1
#define MYSQLCONF 2
#define KARBOCONF 3


/*=== WiFiAccessPoint ===*/
const char* ssidAP = "iSpunder"; // Change your name access point
const char* ssidPass = ""; // Change your password access point
const char* wifi_ip[4] = { "192", "168", "5", "1" }; // Change your ip local network
const char* wifi_subnet[4] = { "255", "255", "255", "0" }; // Change your subnet local network
const char* wifi_gateway[4] = { "192", "168", "5", "1" }; // Change your gateway local network
IPAddress server_addr(192, 168, 1, 50);  // IP of the MySQL *server* here
char user[] = "iSpunder"; // MySQL user login username
char password[] = "iSpunder"; // MySQL user login password

extern ESPRotary ESPR = ESPRotary(ENCPINDT, ENCPINCLK);
extern Button2 BTNA = Button2(ENCPINSW);
extern OneWire oneWire(OW_BUS);
extern DallasTemperature OW(&oneWire);

int encpos = 0;
int oldencpos = 0;
bool BtnPress = false;

bool DSrequested = false;
bool checkCarbo = false;
bool setResetFlag = false;

int number = 0;
int oldnumber = 0;
int menue = 0;
int My_SudID;

double carbo = 0.0;
double carbohyst = 0.25;
double sollcarbo;

ulong MessInterval = 60000;
ulong DSreqTime = MessInterval * 2;
int buttonState;

String incomingChar = "";
String My_MySqlSrv = "";
String My_MySqlPort = "";
String My_ssid = "";
String My_psk = "";

#ifdef DEBUG
String My_iSpunderName = "Test";
String My_SudName = "#9999";
#else
String My_iSpunderName = "";
String My_SudName = "";
#endif // DEBUG

unsigned long buttonPressTimeStamp;





#endif // !CONFIG_H

