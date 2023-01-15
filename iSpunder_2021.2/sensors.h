// sensors.h

#ifndef _SENSORS_h
#define _SENSORS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "config.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include "debugSerialOut.h"


//Prototypen
void OWSensorInit();
void OWSensorRequest();
float OWSensorGetTemp();
float getPressure();
double calcCarbo(const float & druck, const float & temp);
double calcPressure(const float & carbo,const float & temp);


#endif


