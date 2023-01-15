// config.h

/*
Pinbelegung

WEMOS | OLED
----- + -----
   D1 | SCL
   D2 | SDA

      | ENC
----- + -----
   D6 | CLK - Pullup 10k
   D7 |  SW - Intern_Pullup
   D0 |  DT - Pullup 10k 

  Version1 | ENC
----- + -----
   D6 | CLK - Pullup 10k
   D7 |  SW - Pullup 10k
   D8 |  DT - Pullup 10k(Muss beim booten auf LOW liegen, sonst kein Start)

      | TIP120
----- + -----
   D5 | R1k - Base

      | Drucksensor
----- + -----
   A0 | R100k - Poti R100K - Daten - Gelb

      | DS18B20
----- + ------
   D4 | PIN2 - Pullup R4k7(Muss beim booten auf High oder unbeschaltet sein)


 Flashsize: 4MB (2MB SPIFFS, ~1MB OTA)

*/

#ifndef _CONFIG_h
#define _CONFIG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define Version1 1 //uncommet if new version

#define PROG_VERSION "22.1.5"
#define RELAISPIN D5
#define BUTTON_PIN D7

#ifdef Version1
#define ENCPINDT D8
#else
#define ENCPINDT D0
#endif // Version1

#define ENCPINCLK D6
#define OW_BUS D4
#define OWRESOLUTION 12 // 12bit resolution == 750ms update rate
#define WIFICONF "/wificonfig.json" //1
#define DBCONF "/dbconfig.json" //2
#define KARBOCONF "/karboconfig.json" //3
#define SPUNDERCONF "/spunderconfig.json"
#define OTAPORT 8266
#define CLICKS_PER_STEP 4

#endif

