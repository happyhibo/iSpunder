/*
 Name:		iSpunder_2021.ino
 Created:	11.03.2021 11:34:21
 Author:	hh
*/

/*
	-V21.1.0
		neu Überarbeitet
	-V21.1.1
		InfulxDB integriert
	-V21.2.0
		OTA hinzu
	-V21.2.1
		3bar Überdruck Notentlüftung
		Spundung start/stop
	-V21.2.2
		Druckkompensation
	-V22.1.1
		Auto-Druck-Kalibrierung im Menü
	-V22.1.2
		neue Encoder (keyes entfernt) mit 10K Pullup und neuer Lib 1.5.0
	-V22.1.5
		Ventiltest, Versionanzeige
		an ArduinoJSON 6.x angepasst, Bugs entfernt
	-V22.2.0
		Projektkopie
		DualSud
	-V22.2.1
		Bildschirmschoner

*/




//includes

#include <OneWire.h>
#include "ota.h"
#include "filesystem.h"
#include "influxdb.h"
#include "sensors.h"
#include "display.h"
#include "wifiwebserver.h"
#include "config.h"
#include "debugSerialOut.h"
#include "iSpunder_2022.2.h"
#include <math.h>

bool BtnPress = false;
bool CarboPressureCheck = false;
bool setResetFlag = false;
bool maxPressure = false;
bool isActive;
bool EncIsChange = false;
bool EncUp = false;
bool EncDn = false;
bool DualSud = false;

int encpos = 0;
int oldencpos = 0;
int menue = 0;
int number = 0;
int oldnumber = 0;

float fTemp, fDruck, fCarbo, fSollDruck;
float fSollCarbo = 4.5;
float fHystCarbo = 0.25;
float fMaxDruck = 3.0;
float fKompDruck = 0.0;

ulong MessInterval = 300000;
ulong reqTime = MessInterval * 2;

String iSpunderName = "iSpunder00";
String SudName = "#001";
String SudName2 = "#000";
int iSpunderNr = 0;
int SudNr = 1;
int SudNr2 = 0;


//ESPRotary ESPR = ESPRotary(ENCPINDT, ENCPINCLK);
ESPRotary ESPR;
Button2 BTNA;



void initEncoder()
{
	SerialOut(F("Debug: Init Encoder..."));
	ESPR.begin(ENCPINDT, ENCPINCLK, CLICKS_PER_STEP);
	ESPR.setChangedHandler(handle_Enc_change);
	ESPR.setLeftRotationHandler(handle_Enc_down);
	ESPR.setRightRotationHandler(handle_Enc_up);

	SerialOut(F("Debug: Init EncButton..."));
	BTNA.begin(BUTTON_PIN);
	BTNA.setDebounceTime(75);
	//BTNA.setChangedHandler(handle_Btn);
	BTNA.setTapHandler(handle_Btn_tap);
	BTNA.setPressedHandler(handle_Btn_pressed);
	//BTNA.setClickHandler(handle_Btn);
	//BTNA.setLongClickHandler(handle_Btn);
	//BTNA.setDoubleClickHandler(handle_Btn);
	//BTNA.setTripleClickHandler(handle_Btn);
	delay(10);
	yield();
}

void initRelais() {
	pinMode(RELAISPIN, OUTPUT);
	digitalWrite(RELAISPIN, LOW);
	SerialOut(F("Debug: init RelaisPin"));
}

void handle_Enc_down(ESPRotary& enc) {
	SerialOut(F("Debug: handle_Enc_down --"));
	EncDn = true;
}

void handle_Enc_up(ESPRotary& enc) {
	SerialOut(F("Debug: handle_Enc_up ++"));
	EncUp = true;	
}

void handle_Enc_change(ESPRotary& enc) {
	EncIsChange = true;
}

//void handle_Enc(ESPRotary& enc) {
//	//SerialOut(enc.getPosition());
//	//SerialOut(ESPR.getDirection());
//	if (enc.getDirection() == RE_RIGHT) number++;
//	if (enc.getDirection() == RE_LEFT) number--;
//	if (number != oldnumber)
//	{
//		if (number > oldnumber)   // < > Zeichen ändern = Encoderdrehrichtung ändern
//		{
//			//if (number - oldnumber == 2) {
//				encpos++;
//				SerialOut(F("Debug: Drehen ++ "), false);
//				SerialOut(encpos);
//				oldnumber = number;
//			//}
//		}
//		else
//		{
//			//if (number - oldnumber == -2) {
//				encpos--;
//				SerialOut(F("Debug: Drehen -- "), false);
//				SerialOut(encpos);
//				oldnumber = number;
//			//}
//		}
//	}
	//SerialOut(F("Debug: Nummer= "), false);
	//SerialOut(number);
	//yield();
//}

void handle_Btn_pressed(Button2& btn) {
	SerialOut(F("Debug: Btn_pressed"));
	BtnPress = true;
}

void handle_Btn_tap(Button2& btn) {
	SerialOut(F("Btn_tap"));
	SerialOut(F("Debug: Was pressed for "), false);
	SerialOut(btn.wasPressedFor(), false);
	SerialOut(F(" ms"));
}

//void handle_Btn(Button2& btn) {
//	SerialOut(F("Debug: Btn-Handler"));
//	switch (btn.getClickType()) {
//	case SINGLE_CLICK:
//		BtnPress = true;
//		SerialOut(F("Debug: SINGLE_CLICK= "), false);
//		SerialOut(SINGLE_CLICK);
//		break;
//	case DOUBLE_CLICK:
//		SerialOut(F("Debug: DOUBLE_CLICK= "), false);
//		SerialOut(DOUBLE_CLICK);
//		//checkCarbo = true;
//		break;
//	case TRIPLE_CLICK:
//		SerialOut(F("Debug: TRIPLE_CLICK= "), false);
//		SerialOut(TRIPLE_CLICK);
//		//init_WiFi_AP();
//		break;
//	case LONG_CLICK:
//		SerialOut(F("Debug: LONG_CLICK= "), false);
//		SerialOut(LONG_CLICK);
//		SerialOut(F("Debug: Reset..."));
//		//checkCarbo = false;
//		BtnPress = false;
//		//connectWifi();
//		break;
//	}
//	SerialOut(F("Debug: ClickType = "), false);
//	SerialOut(btn.getClickType());
//	SerialOut(F("Debug: Was pressed for "), false);
//	SerialOut(btn.wasPressedFor(), false);
//	SerialOut(F(" ms"));
//	yield();
//}

void changeCarbo() {
	SerialOut(F("Debug: Insert ChangeCarbo"));
	SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "Soll-Karbo";
	String Zeile2 = String(fSollCarbo) + "g/l";
	String Zeile3 = "";
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	//float _fPreCarbo = fSollCarbo;
	//fEncIncrementZ = 0.0;
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		//if (encpos > oldencpos) {
		//if (ESPR.getDirection() == RE_RIGHT){
		//	fSollCarbo += 0.1;
		//	SerialOut(F("sollcarbo ++ "), false);
		//	SerialOut(fSollCarbo);
		//}
		////else if (encpos < oldencpos) {
		//else if (ESPR.getDirection() == RE_LEFT){
		//	fSollCarbo -= 0.1;
		//	if (fSollCarbo <= 0) fSollCarbo = 0;
		//	SerialOut(F("sollcarbo -- "), false);
		//	SerialOut(fSollCarbo);
		//}
		if (EncUp)
		{
			fSollCarbo += 0.1;
			EncUp = false;
		}
		if (EncDn)
		{
			fSollCarbo -= 0.1;
			if (fSollCarbo <= 0) fSollCarbo = 0;
			EncDn = false;
		}
		
		//if (_fPreCarbo != fSollCarbo)
		if (EncIsChange)
		{
			Zeile2 = String(fSollCarbo) + "g/l";
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
			EncIsChange = false;
			SerialOut(F("Debug: SollKarbo "), false);
			SerialOut(fSollCarbo);
		}
		//_fPreCarbo = fSollCarbo;
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Debug: Exit ChangeCarbo"));
	fSollDruck = calcPressure(fSollCarbo, fTemp);
	saveLastKarboConfig();
}

void changeHystCarbo() {
	SerialOut(F("Debug: Insert ChangeHystCarbo"));
	SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "K-Hysterese";
	String Zeile2 = String(fHystCarbo);
	String Zeile3 = "";
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		//if (encpos > oldencpos) {
		//	fHystCarbo += 0.05;
		//	SerialOut(F("Debug: HystCarbo ++ "), false);
		//	SerialOut(fHystCarbo);
		//}
		//else if (encpos < oldencpos) {
		//	fHystCarbo -= 0.05;
		//	if (fHystCarbo <= 0) fHystCarbo = 0;
		//	SerialOut(F("Debug: HystCarbo -- "), false);
		//	SerialOut(fHystCarbo);
		//}
		//if (encpos != oldencpos)
		//{
		//	Zeile2 = String(fHystCarbo);
		//	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		//}
		//oldencpos = encpos;

		if (EncUp)
		{
			fHystCarbo += 0.05;
			EncUp = false;
		}
		
		if (EncDn)
		{
			fHystCarbo -= 0.05;
			if (fHystCarbo <= 0) fHystCarbo = 0;
			EncDn = false;
		}

		if (EncIsChange)
		{
			Zeile2 = String(fHystCarbo);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
			EncIsChange = false;
			SerialOut(F("Debug: fHystCarbo "), false);
			SerialOut(fHystCarbo);
		}
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Debug: Exit ChangeHystCarbo"));
	saveLastKarboConfig();
}


void changeMessInterval() {
	double _Minuten;
	SerialOut(F("Debug: Insert changeMessInterval"));
	SerialOut(BtnPress);
	_Minuten = MessInterval / 60000;
	String Ausrichtung = "m";
	String Zeile1 = "M-Interval";
	String Zeile2 = String(_Minuten);
	String Zeile3 = "Min.";
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		//if (encpos > oldencpos) {
		//	_Minuten += 0.5;
		//}
		//else if (encpos < oldencpos) {
		//	_Minuten -= 0.5;
		//	if (_Minuten <= 0) _Minuten = 0;
		//}
		//if (encpos != oldencpos)
		//{
		//	Zeile2 = String(_Minuten);
		//	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		//}
		//oldencpos = encpos;

		if (EncUp)
		{
			_Minuten += 0.5;
			EncUp = false;
		}

		if (EncDn)
		{
			_Minuten -= 0.5;
			if (_Minuten <= 0) _Minuten = 0;
			EncDn = false;
		}

		if (EncIsChange)
		{
			Zeile2 = String(_Minuten);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
			EncIsChange = false;
			SerialOut(F("Debug: M-Interval "), false);
			SerialOut(_Minuten);
		}
		yield();
	} while (!BtnPress);
	MessInterval = _Minuten * 60000;
	SerialOut(F("Debug: MessInterval "), false);
	SerialOut(MessInterval);
	BtnPress = false;
	SerialOut(F("Debug: Exit changeMessInterval"));
	saveLastKarboConfig();
}

void changeDualSud() {
	SerialOut(F("Debug: Insert DualSud"));
	SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "Dual-Sud";
	String Zeile2 = "ist";
	String Zeile3 = "";
	if (DualSud)
	{
		Zeile3 = "an";
	}
	else {
		Zeile3 = "aus";
	}
	
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		if (EncUp)
		{
			DualSud = true;
			EncUp = false;
		}
		if (EncDn)
		{
			DualSud = false;
			EncDn = false;
		}

		if (EncIsChange)
		{
			if (DualSud)
			{
				Zeile3 = "an";
			}
			else {
				Zeile3 = "aus";
			}
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
			EncIsChange = false;
			SerialOut(F("Debug: DualSud ist "), false);
			SerialOut(Zeile3);
		}
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Debug: Exit DualSud"));
	saveLastSpunderConfig();
}

void saveLastKarboConfig() {

	writeKarboConfigFile(fSollCarbo, MessInterval, fHystCarbo, fKompDruck);
	//requestData();
}


void changeSudID(int& SudID) {

	SerialOut(F("Debug: Insert changeSudID"));
	SerialOut(BtnPress);
	int _Sudid = SudID;
	
	String Ausrichtung = "m";
	String Zeile1 = "Sud-Nr";
	if (DualSud)
	{
		Zeile1 = "Sud-Nr2";
	}
	String Zeile2 = "# " + String(_Sudid);
	String Zeile3 = "";
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop

		if (EncUp)
		{
			_Sudid += 1;
			EncUp = false;
		}

		if (EncDn)
		{
			_Sudid -= 1;
			if (_Sudid <= 0) _Sudid = 0;
			EncDn = false;
		}

		if (EncIsChange)
		{
			Zeile2 = "# " + String(_Sudid);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
			EncIsChange = false;
			SerialOut(F("Debug: Sud-ID #"), false);
			SerialOut(_Sudid);
		}

		yield();
	} while (!BtnPress);

	SudID = _Sudid;

	SerialOut(F("Debug: Sud-ID #"), false);
	SerialOut(SudID);
	BtnPress = false;
	SerialOut(F("Debug: Exit changeSudID"));
	//saveLastSpunderConfig();
}

void changeSpunderID() {

	SerialOut(F("Debug: Insert changeSpunderID"));
	SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "iSpunder-Nr";
	String Zeile2 = String(iSpunderNr);
	String Zeile3 = "";
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		//if (encpos > oldencpos) {
		//	iSpunderNr += 1;
		//	if (iSpunderNr >= 99) iSpunderNr = 99;
		//}
		//else if (encpos < oldencpos) {
		//	iSpunderNr -= 1;
		//	if (iSpunderNr <= 0) iSpunderNr = 0;
		//}
		//if (encpos != oldencpos) {
		//	Zeile2 = String(iSpunderNr);
		//	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		//}
		//oldencpos = encpos;

		if (EncUp)
		{
			iSpunderNr += 1;
			if (iSpunderNr > 99) iSpunderNr = 0;
			EncUp = false;
		}

		if (EncDn)
		{
			iSpunderNr -= 1;
			if (iSpunderNr <= 0) iSpunderNr = 99;
			EncDn = false;
		}

		if (EncIsChange)
		{
			Zeile2 = String(iSpunderNr);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
			EncIsChange = false;
			SerialOut(F("Debug: iSpunder-Nr "), false);
			SerialOut(iSpunderNr);
		}
		yield();
	} while (!BtnPress);

	SerialOut(F("Debug: iSpunder-ID "), false);
	SerialOut(iSpunderNr);
	BtnPress = false;
	SerialOut(F("Debug: Exit changeSpunderID"));
	saveLastSpunderConfig();
}

void changeKompensationDruck() {
	SerialOut(F("Debug: changeKompensationDruck"));
	SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "Komp.-Druck";
	String Zeile2 = String(fKompDruck) + " bar";
	String Zeile3 = "";
	String Zeile4 = "+- 0.01";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		//if (encpos > oldencpos) {
		//	fKompDruck += 0.01;
		//	SerialOut(F("Debug: fKompDruck ++ "), false);
		//	SerialOut(fKompDruck);
		//}
		//else if (encpos < oldencpos) {
		//	fKompDruck -= 0.01;
		//	SerialOut(F("Debug: fKompDruck -- "), false);
		//	SerialOut(fKompDruck);
		//}
		//if (encpos != oldencpos)
		//{
		//	Zeile2 = String(fKompDruck);
		//	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		//}
		//oldencpos = encpos;

		if (EncUp)
		{
			fKompDruck += 0.01;
			EncUp = false;
		}

		if (EncDn)
		{
			fKompDruck -= 0.01;
			EncDn = false;
		}

		if (EncIsChange)
		{
			Zeile2 = String(fKompDruck);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
			EncIsChange = false;
			SerialOut(F("Debug: Komp.-Druck "), false);
			SerialOut(fKompDruck);
		}
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Debug: Exit changeKompensationDruck"));
	saveLastKarboConfig();
}

void setDruckKompensation() {
	SerialOut(F("Debug: setDruckKompensation"));
	//SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "automatische";
	String Zeile2 = "Druck-";
	String Zeile3 = "kompensation";
	String Zeile4 = "Start";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	delay(250);
	SerialOut(F("Debug: Open Valve"));
	digitalWrite(RELAISPIN, HIGH);
	float _fDruck;
	do {
		_fDruck = getPressure();
		fKompDruck = _fDruck * -1;
		
		Ausrichtung = "m";
		Zeile1 = "Kompensation";
		Zeile2 = String(fKompDruck) + " bar";
		Zeile3 = "Druck";
		Zeile4 = String(_fDruck + fKompDruck) + " bar";
		DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		yield();
	} while (_fDruck + fKompDruck != 0.0);

	Ausrichtung = "m";
	Zeile1 = "automatische";
	Zeile2 = "Druck-";
	Zeile3 = "kompensation";
	Zeile4 = "Ende";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);

	SerialOut(F("Debug: Close Valve"));
	digitalWrite(RELAISPIN, LOW);
	saveLastKarboConfig();
}

void TestMagnetVentil() {
	SerialOut(F("Debug: TestMagnetVentil"));
	//SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "Test";
	String Zeile2 = "Magnet-";
	String Zeile3 = "Ventil";
	String Zeile4 = "Start";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	delay(250);
	SerialOut(F("Debug: Open Valve"));
	digitalWrite(RELAISPIN, HIGH);
	do {
		BTNA.loop();
		Ausrichtung = "m";
		Zeile1 = "Magnet-";
		Zeile2 = "Ventil";
		Zeile3 = "";
		Zeile4 = "OFFEN";
		DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		yield();
	} while (!BtnPress);

	digitalWrite(RELAISPIN, LOW);
	do {
		BTNA.loop();
		Ausrichtung = "m";
		Zeile1 = "Magnet-";
		Zeile2 = "Ventil";
		Zeile3 = "";
		Zeile4 = "ZU";
		DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		yield();
	} while (!BtnPress);

	Ausrichtung = "m";
	Zeile1 = "Test";
	Zeile2 = "Magnet-";
	Zeile3 = "Ventil";
	Zeile4 = "Ende";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	SerialOut(F("Debug: Exit TestMagnetVentil"));

}


void saveLastSpunderConfig() {

	setSpunderName();
	setSudName();
	influxdb_setTags(iSpunderName, SudName);
	writeSpunderConfigFile(iSpunderNr, SudNr, SudNr2, DualSud);
	requestData();

}


void handle_CarboValve() {
	SerialOut(F("Debug: handle_CarboValve"));

	if ((fTemp > -5) && (fCarbo > fSollCarbo + fHystCarbo)) {
		SerialOut(F("Debug: Open Valve"));
		digitalWrite(RELAISPIN, HIGH);
		CarboPressureCheck = true;
	}
	else
	{
		SerialOut(F("Debug: Close Valve"));
		digitalWrite(RELAISPIN, LOW);
	}

	SerialOut(F("Debug: Exit handle_CarboValve"));
	yield();
}


void handle_CarboPressure() {
	SerialOut(F("Debug: handle_CarboPressure"));

	if (!CarboPressureCheck) {
		SerialOut(F("Debug: Return handle_CarboPressure"));
		return;
	}

	do
	{
		BTNA.loop();

		fDruck = getPressure() + fKompDruck;
		fCarbo = calcCarbo(fDruck, fTemp);
		DisplayDaten(fTemp, fDruck, fCarbo, fSollCarbo);
		if (fCarbo <= fSollCarbo - fHystCarbo)
		{
			CarboPressureCheck = false;
		}
		SerialOut(F("Debug: checkCarbo "), false);
		SerialOut(CarboPressureCheck);
		delay(100);
		yield();
	} while (CarboPressureCheck);
	SerialOut(F("Debug: Close Valve"));
	digitalWrite(RELAISPIN, LOW);
	requestData();
	SerialOut(F("Debug: Exit handle_CarboPressure"));
}

void handle_MaxPressure()
{
	SerialOut(F("Debug: handle_MaxPressure"));
	bool maxpress = true;
	do
	{
		fDruck = getPressure() + fKompDruck;
		if (fDruck >= fMaxDruck)
		{
			SerialOut(F("Debug: Open Valve"));
			digitalWrite(RELAISPIN, HIGH);
		}
		else if (fDruck < fMaxDruck)
		{
			SerialOut(F("Debug: Close Valve"));
			digitalWrite(RELAISPIN, LOW);
			maxpress = false;
		}

		yield();
	} while (maxpress);
	maxpress = false;
	requestData();
	SerialOut(F("Debug: Exit handle_MaxPressure"));
}


void MenueEncoder() {
	SerialOut(F("Debug: MenueEncoder"));
	//--------Max 11 Zeichen-"12345678901"
	String _MenueTitel[18] = { "Soll-Karbo", "K-Hysterese", "MessInterv.", "Wifi STA", "Wifi AP", "DualSud", "Sud ID", 
		"Set Spun.ID", "IP Address", "Save Config", "Format SPIF", "STOP Spng", "START Spng", "Druck-Komp.", "Auto-Komp.", "Test Ventil", "Version", "Exit" };
	int _MenuePos = 0;
	int _MaxPos = 17;
	DisplayMenue(_MenuePos, _MenueTitel[_MenuePos]);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop

		if (EncUp)
		{
			_MenuePos += 1;
			if (_MenuePos > _MaxPos) _MenuePos = 0;
			EncUp = false;
		}

		if (EncDn)
		{
			_MenuePos -= 1;
			if (_MenuePos < 0) _MenuePos = _MaxPos;
			EncDn = false;
		}

		if (EncIsChange)
		{
			DisplayMenue(_MenuePos, _MenueTitel[_MenuePos]);
			EncIsChange = false;
		}
		yield();
	} while (!BtnPress);
	BtnPress = false;
	MenueAktion(_MenuePos);
	SerialOut(F("Debug: Exit MenueEncoder"));
}

void MenueAktion(int MP) {
	SerialOut(F("Debug: MenueAktion"));
	//String _configSave1 = "N";
	//String _configSave2 = "N";
	//String _configSave3 = "N";
	String Ausrichtung = "m";
	String Zeile1;
	String Zeile2;
	String Zeile3;
	String Zeile4;

	switch (MP) {
	case 0: //Soll-Karbo
		changeCarbo();
		break;
	case 1: //Karbo-Hysertese
		changeHystCarbo();
		break;
	case 2: //MessIntervall
		changeMessInterval();
		break;
	case 3: //WiFi STA
		if (startWifi())
			startWebserver();
		break;
	case 4: //WiFi AP
		//init_WiFi_AP();
		break;
	case 5: //Dualsud 
		changeDualSud();
		break;
	case 6: //Sud ID
		changeSudID(SudNr);
		if (DualSud)
		{
			changeSudID(SudNr2);
		}
		saveLastSpunderConfig();
		break;
	case 7: //Spunder ID
		changeSpunderID();
		break;
	case 8: //IP Adresse
		BtnPress = false;
		do {
			BTNA.loop();
			Ausrichtung = "m";
			Zeile1 = "STA Mode";
			Zeile2 = "IP " + WiFi.localIP().toString();
			Zeile3 = "";
			Zeile4 = "";
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		} while (!BtnPress);
		BtnPress = false;
		break;
	case 9: //Save Config
		
		//delay(3000);
		break;
	case 10: //Format SPIFF
		formatSpiffs();
		break;
	case 11: //Spundung Stoppen
		isActive = false;
		break;
	case 12: //Spundung Starten
		isActive = true;
		Ausrichtung = "m";
		//Nur 11Zeichen "12345678901"
		Zeile1 = "iSpunder";
		Zeile2 = "Spundung";
		Zeile3 = "wird";
		Zeile4 = "gestartet!";
		DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		delay(250);
		requestData();
		break;
	case 13: //Druckkorrektur
		changeKompensationDruck();
		break;
	case 14: //Druckkompensation
		setDruckKompensation();
		break;
	case 15: //Veniltest
		TestMagnetVentil();
		delay(250);
		break;
	case 16: //Version
		DisplayInit();
		delay(1500);
		break;
	case 17: //Exit
		break;
	case 18: //Exit
		break;
	default:
		;
	}
	DisplayDaten(fTemp, fDruck, fCarbo, fSollCarbo);
	SerialOut(F("Debug: Exit MenueAktion"));
}


void requestData() {
	SerialOut(F("Debug: requestData"));
	OWSensorRequest();
	reqTime = millis();
	if (isActive == false)
	{
		SerialOut(F("Debug: isActive false requestData"));
		return;
	}
	fTemp = OWSensorGetTemp();
	fDruck = getPressure() + fKompDruck;
	if (fDruck > fMaxDruck)
	{
		maxPressure = true;
	}
	fCarbo = calcCarbo(fDruck, fTemp);
	fSollDruck = calcPressure(fSollCarbo, fTemp);
	handle_CarboValve();
	DisplayDaten(fTemp, fDruck, fCarbo, fSollCarbo);
	//influxdb_write_TestData();
	influxdb_setTags(iSpunderName, SudName);
	influxdb_write(fTemp, fDruck, fCarbo, fSollCarbo, fSollDruck);
	if (DualSud)
	{
		influxdb_setTags(iSpunderName, SudName2);
		influxdb_write(fTemp, fDruck, fCarbo, fSollCarbo, fSollDruck);
	}

	//setResetFlag = false;
	SerialOut(F("Debug: Exit requestData"));
}

void setSudName() {

	if (SudNr <= 9)
	{
		SudName = "#00" + (String)SudNr;
	}
	else if (SudNr >=10 && SudNr <= 99)
	{
		SudName = "#0" + (String)SudNr;
	}
	else
	{
		SudName = "#" + (String)SudNr;
	}

	if (DualSud)
	{
		if (SudNr2 <= 9)
		{
			SudName2 = "#00" + (String)SudNr2;
		}
		else if (SudNr2 >= 10 && SudNr2 <= 99)
		{
			SudName2 = "#0" + (String)SudNr2;
		}
		else
		{
			SudName2 = "#" + (String)SudNr2;
		}
	}
	
}

void setSpunderName() {

	if (iSpunderNr <= 9)
	{
		iSpunderName = "iSpunder0" + (String)iSpunderNr;
	}
	else
	{
		iSpunderName = "iSpunder" + (String)iSpunderNr;
	}
}


void setup() {
	Serial.begin(115200);
	Serial.println();
	SerialOut(F("Debug: Setup begin..."));
	DisplayInit();
	Serial.println("iSPUNDER V" + String(PROG_VERSION) + " by HappyHibo");
	SerialOut(F("Debug: Init Hardware..."));
	initEncoder();
	initRelais();
	OWSensorInit();

	//leztzteSpunderConfig lesen
	readSpunderConfigFile(iSpunderNr, SudNr, SudNr2, DualSud);
	readKarboConfigFile(fSollCarbo, MessInterval, fHystCarbo, fKompDruck);
	fSollCarbo = round(fSollCarbo * 10) / 10;
	float _fCarbo = calcCarbo(0.0, 20.0); //Standard Wert für 0Bar und 20°C
	if (fSollCarbo < _fCarbo) // unlogische gespeicherte Werte anfangen
	{
		fSollCarbo = _fCarbo;
	}
	setSudName();
	setSpunderName();

	if (startWifi()) {
		startWebserver();
		influxdb_init();
		influxdb_setTags(iSpunderName, SudName);
	}

	start_OTA(iSpunderName, OTAPORT);
	OWSensorRequest();
	isActive = true;
	DisplayDualSud(DualSud);
	SerialOut(F("Debug: Setup ended..."));
	requestData();
}



void loop() {
	handle_OTA();
	handleWebserver();
	BTNA.loop();

	if (millis() - reqTime >= MessInterval) {
		requestData();
	}

	if (BtnPress)
	{
		SerialOut(F("Debug: BtnPress True"));
		BtnPress = false;
		MenueEncoder();
	}

	if (CarboPressureCheck)
	{
		SerialOut(F("Debug: CarboPressureCheck True"));
		handle_CarboPressure();
	}

	if (maxPressure)
	{
		handle_MaxPressure();
	}

	if (isActive == false)
	{
		SerialOut(F("Debug: isActive False"));
		String Ausrichtung = "m";
		//Nur 11Zeichen "12345678901"
		String Zeile1 = "iSpunder";
		String Zeile2 = "Spundung";
		String Zeile3 = "ist";
		String Zeile4 = "gestoppt!";
		DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	}

#ifdef DEBUG
	//Abfrage serielle Eingabe
	while (Serial.available()) {
		String incomingChar = Serial.readString();// read the incoming data as string
		SerialOut(F("*LOOP* Incoming Serial: "), false);
		SerialOut(incomingChar);
		
		String firstChar = incomingChar.substring(0, 1);
		SerialOut(F("1st: "), false);
		SerialOut(firstChar);
		String secondChars = incomingChar.substring(1);
		SerialOut(F("2nd: "), false);
		SerialOut(secondChars);

		bool sendData = false;
		//SerialInput zum Werte setzten im DEBUG
		/* Möglich Eingaben:
		T25 - setzen Temperatur
		D6.5 - setzen Druck
		K4.5 - setzen Karbo
		*/
		if (firstChar == "T") {
			fTemp = secondChars.toFloat();
			sendData = true;
		}
		if (firstChar == "D") {
			fDruck = secondChars.toFloat();
			sendData = true;
		}
		if (firstChar == "K") {
			fSollCarbo = secondChars.toFloat();
			sendData = true;
		}

		if (sendData)
		{
			fCarbo = calcCarbo(fDruck, fTemp);
			fSollDruck = calcPressure(fSollCarbo, fTemp);
			influxdb_write(fTemp, fDruck, fCarbo, fSollCarbo, fSollDruck);
			sendData = false;
		}
		// Button BtnPress simulation
		if (firstChar == "B") {
			BtnPress = true;
		}
		// Button isActive simulation
		if (firstChar == "A") {
			isActive = false;
		}
	
	}
#endif // DEBUG

}
