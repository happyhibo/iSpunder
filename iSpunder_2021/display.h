// display.h

#ifndef _DISPLAY_h
#define _DISPLAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "config.h"
#include <SSD1306Brzo.h>
#include <brzo_i2c.h>
#include <Button2.h>
#include <ESPRotary.h>

extern SSD1306Brzo display(ADDRESS, SDA, SCL);

//Protoypen
void DisplayInfo(String Ausrichtung, String Zeile1, String Zeile2, String Zeile3, String Zeile4);
void DisplayDaten(float temp, float druck, float carbo, float s_carbo);
int DisplayMenue();

#endif

