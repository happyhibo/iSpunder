// display.h

#ifndef _DISPLAY_h
#define _DISPLAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <SSD1306Brzo.h>
#include <ESPRotary.h>
#include <Button2.h>
#include <brzo_i2c.h>
#include "debugSerialOut.h"
#include "config.h"

#define ADDRESS 0x3C

//Prototypen
void DisplayInit();
void DisplayDualSud(bool& DUALSUD);
void DisplayInfo(String &Ausrichtung, String &Zeile1, String &Zeile2, String &Zeile3, String &Zeile4);
void DisplayInfo(String &Ausrichtung, String &Zeile1, String &Zeile2, String &Zeile3, String &Zeile4);
void DisplayDaten(float &temp, float &druck, float &carbo, float &sollcarbo);
void DisplayMenue(int &Menuepos);
void DisplayMenue(int& Menuepos, String& MenueTxt);

#endif

