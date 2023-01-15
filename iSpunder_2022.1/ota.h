// ota.h

#ifndef _OTA_h
#define _OTA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <ArduinoOTA.h>
#include "debugSerialOut.h"

void handle_OTA();
void start_OTA(String hostname, int port);

#endif


