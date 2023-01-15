// influxdb.h

#ifndef _INFLUXDB_h
#define _INFLUXDB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "debugSerialOut.h"
#include <InfluxDbClient.h>

#define INFLUXDB_URL "http://192.168.1.50:8086"
#define INFLUXDB_DB "iSpunder"
#define INFLUXDB_USER "iSpunder"
#define INFLUXDB_PW "iSpunder"

//Prototypen
void influxdb_init();
void influxdb_setTags(String &iSpunderName, String &SudName);
void influxdb_write(float &temp, float &druck, float &carbo, float &sollcarbo, float &solldruck);
void influxdb_write_TestData();

#endif

