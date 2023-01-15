// filesystem.h

#ifndef _FILESYSTEM_h
#define _FILESYSTEM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "config.h"
#include <FS.h>
//#include <SPI.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include "debugSerialOut.h"

//Prototypen
void formatSpiffs();
void deleteAllFilesSpiffs();
void startSpiffs();
bool readWiFiConfigFile(String &WIFIUSR, String &WIFIPASS);
bool readKarboConfigFile(float &SKARBO, ulong &MINTERVAL, float &KARBOHYST, float &KOMPDRUCK);

bool writeWiFiConfigFile(String &WIFIUSR, String &WIFIPASS);
bool readSpunderConfigFile(int& SPUNDERNR, int& SUDNR, int& SUDNR2, bool& DUALSUD);

bool writeSpunderConfigFile(int& SPUNDERNR, int& SUDNR, int& SUDNR2, bool& DUALSUD);

bool writeKarboConfigFile(float &SKARBO, ulong &MINTERVAL, float &KARBOHYST, float &KOMPDRUCK);

bool deleteConfigFile(char * configfile);

#endif

