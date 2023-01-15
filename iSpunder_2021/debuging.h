// debuging.h

#ifndef _DEBUGING_h
#define _DEBUGING_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

template <typename T>
void SerialOut(const T aValue, bool newLine = true)
{
	if (!isDebugEnabled())
		return;
	Serial.print(aValue);
	if (newLine)
		Serial.print("\n");
}

bool isDebugEnabled()
{
#ifdef DEBUG
	return true;
#endif // DEBUG
	return false;
}

void SerialOut() { SerialOut(""); }


#endif

