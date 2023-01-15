// debugSerialOut.h

#ifndef _DEBUGSERIALOUT_h
#define _DEBUGSERIALOUT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#ifndef DEBUG
	#define DEBUG 1 // uncomment this line to enable serial diagnostic messages
#endif

template <typename T>
void SerialOut(const T aValue, bool newLine = true)
{
#ifdef DEBUG
	Serial.print(aValue);
	if (newLine)
		Serial.print("\n");
#endif // DEBUG	
}

void SerialOut();

#endif


