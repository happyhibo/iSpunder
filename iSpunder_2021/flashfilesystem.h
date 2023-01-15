// flashfilesystem.h

#ifndef _FLASHFILESYSTEM_h
#define _FLASHFILESYSTEM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "debuging.h"
#include <FS.h>
#include <ArduinoJson.h>

//Prototypen
bool readConfigFile(int config);
bool writeConfigFile(int config);
bool deleteConfigFile(int config);
void formatSpiffs();

#endif

