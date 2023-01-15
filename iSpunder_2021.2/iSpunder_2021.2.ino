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


*/



#include <ESPRotary.h>
#include "ota.h"
#include "filesystem.h"
#include "influxdb.h"
#include "sensors.h"
#include "display.h"
#include "wifiwebserver.h"
#include "config.h"
#include "debugSerialOut.h"
#include "iSpunder_2021.2.h"

bool BtnPress = false;
bool CarboPressureCheck = false;
bool setResetFlag = false;
bool maxPressure = false;
bool isActive;

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
int iSpunderNr = 0;
int SudNr = 1;


ESPRotary ESPR = ESPRotary(ENCPINDT, ENCPINCLK);
Button2 BTNA = Button2(ENCPINSW);



void initEncoder()
{
	SerialOut(F("Init Encoder..."));
	//ESPR.setChangedHandler(rotateENC);
	ESPR.setLeftRotationHandler(handle_Enc);
	ESPR.setRightRotationHandler(handle_Enc);

	SerialOut(F("Init EncButton..."));
	BTNA.setDebounceTime(75);
	//BTNA.setChangedHandler(handle_changed);
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
	SerialOut(F("init RelaisPin"));
}


void handle_Enc(ESPRotary& enc) {
	//SerialOut(enc.getPosition());
	//SerialOut(ESPR.getDirection());
	if (enc.getDirection() == RE_RIGHT) number++;
	if (enc.getDirection() == RE_LEFT) number--;
	if (number != oldnumber)
	{
		if (number > oldnumber)   // < > Zeichen ändern = Encoderdrehrichtung ändern
		{
			if (number - oldnumber == 2) {
				encpos++;
				//SerialOut(F("Debug: Drehen ++ "), false);
				//SerialOut(encpos);
				oldnumber = number;
			}
		}
		else
		{
			if (number - oldnumber == -2) {
				encpos--;
				//SerialOut(F("Debug: Drehen -- "), false);
				//SerialOut(encpos);
				oldnumber = number;
			}
		}
	}
	//SerialOut(F("Debug: Nummer= "), false);
	//SerialOut(number);
	//yield();
}

void handle_Btn_pressed(Button2& btn) {
	SerialOut(F("Btn_pressed"));
	BtnPress = true;
}

void handle_Btn_tap(Button2& btn) {
	SerialOut(F("Btn_tap"));
	SerialOut(F("Debug: Was pressed for "), false);
	SerialOut(btn.wasPressedFor(), false);
	SerialOut(F(" ms"));
}

void handle_Btn(Button2& btn) {
	SerialOut(F("Btn-Handler"));
	switch (btn.getClickType()) {
	case SINGLE_CLICK:
		BtnPress = true;
		SerialOut(F("SINGLE_CLICK= "), false);
		SerialOut(SINGLE_CLICK);
		break;
	case DOUBLE_CLICK:
		SerialOut(F("DOUBLE_CLICK= "), false);
		SerialOut(DOUBLE_CLICK);
		//checkCarbo = true;
		break;
	case TRIPLE_CLICK:
		SerialOut(F("TRIPLE_CLICK= "), false);
		SerialOut(TRIPLE_CLICK);
		//init_WiFi_AP();
		break;
	case LONG_CLICK:
		SerialOut(F("LONG_CLICK= "), false);
		SerialOut(LONG_CLICK);
		SerialOut(F("Reset..."));
		//checkCarbo = false;
		BtnPress = false;
		//connectWifi();
		break;
	}
	SerialOut(F("Debug: ClickType = "), false);
	SerialOut(btn.getClickType());
	SerialOut(F("Debug: Was pressed for "), false);
	SerialOut(btn.wasPressedFor(), false);
	SerialOut(F(" ms"));
	yield();
}

void changeCarbo() {
	SerialOut(F("Insert ChangeCarbo"));
	SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "Soll-Karbo";
	String Zeile2 = String(fSollCarbo) + "g/l";
	String Zeile3 = "";
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		if (encpos > oldencpos) {
			fSollCarbo += 0.1;
			SerialOut(F("sollcarbo ++ "), false);
			SerialOut(fSollCarbo);
		}
		else if (encpos < oldencpos) {
			fSollCarbo -= 0.1;
			if (fSollCarbo <= 0) fSollCarbo = 0;
			SerialOut(F("sollcarbo -- "), false);
			SerialOut(fSollCarbo);
		}
		if (encpos != oldencpos)
		{
			Zeile2 = String(fSollCarbo) + "g/l";
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		}
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Exit ChangeCarbo"));
	fSollDruck = calcPressure(fSollCarbo, fTemp);
	saveLastKarboConfig();
}

void changeHystCarbo() {
	SerialOut(F("Insert ChangeHystCarbo"));
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
		if (encpos > oldencpos) {
			fHystCarbo += 0.05;
			SerialOut(F("HystCarbo ++ "), false);
			SerialOut(fHystCarbo);
		}
		else if (encpos < oldencpos) {
			fHystCarbo -= 0.05;
			if (fHystCarbo <= 0) fHystCarbo = 0;
			SerialOut(F("HystCarbo -- "), false);
			SerialOut(fHystCarbo);
		}
		if (encpos != oldencpos)
		{
			Zeile2 = String(fHystCarbo);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		}
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Exit ChangeHystCarbo"));
	saveLastKarboConfig();
}




void changeMessInterval() {
	double _Minuten;
	SerialOut(F("Insert changeMessInterval"));
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
		if (encpos > oldencpos) {
			_Minuten += 0.5;
		}
		else if (encpos < oldencpos) {
			_Minuten -= 0.5;
			if (_Minuten <= 0) _Minuten = 0;
		}
		if (encpos != oldencpos)
		{
			Zeile2 = String(_Minuten);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		}
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	MessInterval = _Minuten * 60000;
	SerialOut(F("MessInterval "), false);
	SerialOut(MessInterval);
	BtnPress = false;
	SerialOut(F("Exit changeMessInterval"));
	saveLastKarboConfig();
}

void saveLastKarboConfig() {

	writeKarboConfigFile(fSollCarbo, MessInterval, fHystCarbo, fKompDruck);
	//requestData();
}


void changeSudID() {

	SerialOut(F("Insert changeSudID"));
	SerialOut(BtnPress);
	//int _Sudid = SudNr;
	String Ausrichtung = "m";
	String Zeile1 = "Sud-Nr";
	String Zeile2 = "# " + String(SudNr);
	String Zeile3 = "";
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		if (encpos > oldencpos) {
			SudNr += 1;
		}
		else if (encpos < oldencpos) {
			SudNr -= 1;
			if (SudNr <= 0) SudNr = 0;
		}
		if (encpos != oldencpos) {
			Zeile2 = "# " + String(SudNr);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		}
		oldencpos = encpos;
		yield();
	} while (!BtnPress);

	SerialOut(F("Sud-ID #"), false);
	SerialOut(SudNr);
	BtnPress = false;
	SerialOut(F("Exit changeSudID"));
	saveLastSpunderConfig();
}

void changeSpunderID() {

	SerialOut(F("Insert changeSpunderID"));
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
		if (encpos > oldencpos) {
			iSpunderNr += 1;
			if (iSpunderNr >= 99) iSpunderNr = 99;
		}
		else if (encpos < oldencpos) {
			iSpunderNr -= 1;
			if (iSpunderNr <= 0) iSpunderNr = 0;
		}
		if (encpos != oldencpos) {
			Zeile2 = String(iSpunderNr);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		}
		oldencpos = encpos;
		yield();
	} while (!BtnPress);

	SerialOut(F("iSpunder-ID "), false);
	SerialOut(iSpunderNr);
	BtnPress = false;
	SerialOut(F("Exit changeSpunderID"));
	saveLastSpunderConfig();
}

void changeKompensationDruck() {
	SerialOut(F("changeKompensationDruck"));
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
		if (encpos > oldencpos) {
			fKompDruck += 0.01;
			SerialOut(F("fKompDruck ++ "), false);
			SerialOut(fKompDruck);
		}
		else if (encpos < oldencpos) {
			fKompDruck -= 0.01;
			SerialOut(F("fKompDruck -- "), false);
			SerialOut(fKompDruck);
		}
		if (encpos != oldencpos)
		{
			Zeile2 = String(fKompDruck);
			DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
		}
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Exit changeKompensationDruck"));
	saveLastKarboConfig();
}

void setDruckKompensation() {
	SerialOut(F("setDruckKompensation"));
	//SerialOut(BtnPress);
	String Ausrichtung = "m";
	String Zeile1 = "automatische";
	String Zeile2 = "Druck-";
	String Zeile3 = "kompensation";
	String Zeile4 = "Start";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
	SerialOut(F("Open Valve"));
	digitalWrite(RELAISPIN, HIGH);
	float _fDruck;
	do {
		_fDruck = getPressure();
		fKompDruck = _fDruck * -1;
		
		Ausrichtung = "m";
		Zeile1 = "Komp.-Druck";
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

	SerialOut(F("Close Valve"));
	digitalWrite(RELAISPIN, LOW);
	saveLastKarboConfig();
}

void saveLastSpunderConfig() {

	setSpunderName();
	setSudName();
	influxdb_setTags(iSpunderName, SudName);
	writeSpunderConfigFile(iSpunderNr, SudNr);
	requestData();

}


void handle_CarboValve() {
	SerialOut(F("handle_CarboValve"));

	if ((fTemp > -5) && (fCarbo > fSollCarbo + fHystCarbo)) {
		SerialOut(F("Open Valve"));
		digitalWrite(RELAISPIN, HIGH);
		CarboPressureCheck = true;
	}
	else
	{
		SerialOut(F("Close Valve"));
		digitalWrite(RELAISPIN, LOW);
	}

	SerialOut(F("Exit handle_CarboValve"));
	yield();
}


void handle_CarboPressure() {
	SerialOut(F("handle_CarboPressure"));

	if (!CarboPressureCheck) {
		SerialOut(F("Return handle_CarboPressure"));
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
		SerialOut(F("checkCarbo "), false);
		SerialOut(CarboPressureCheck);
		delay(100);
		yield();
	} while (CarboPressureCheck);
	SerialOut(F("Close Valve"));
	digitalWrite(RELAISPIN, LOW);
	requestData();
	SerialOut(F("Exit handle_CarboPressure"));
}

void handle_MaxPressure()
{
	SerialOut(F("handle_MaxPressure"));
	bool maxpress = true;
	do
	{
		fDruck = getPressure() + fKompDruck;
		if (fDruck >= fMaxDruck)
		{
			SerialOut(F("Open Valve"));
			digitalWrite(RELAISPIN, HIGH);
		}
		else if (fDruck < fMaxDruck)
		{
			SerialOut(F("Close Valve"));
			digitalWrite(RELAISPIN, LOW);
			maxpress = false;
		}

		yield();
	} while (maxpress);
	maxpress = false;
	requestData();
	SerialOut(F("Exit handle_MaxPressure"));
}


void MenueEncoder() {
	int _MenuePos = 1;
	DisplayMenue(_MenuePos);
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		if (encpos > oldencpos) {
			_MenuePos += 1;
			if (_MenuePos > 14) _MenuePos = 1;
		}
		else if (encpos < oldencpos) {
			_MenuePos -= 1;
			if (_MenuePos < 1) _MenuePos = 14;
		}
		if (encpos != oldencpos)
		{
			DisplayMenue(_MenuePos);
		}
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	BtnPress = false;
	MenueAktion(_MenuePos);
}

void MenueAktion(int MP) {
	//String _configSave1 = "N";
	//String _configSave2 = "N";
	//String _configSave3 = "N";
	String Ausrichtung = "m";
	String Zeile1;
	String Zeile2;
	String Zeile3;
	String Zeile4;

	switch (MP) {
	case 1: //Soll-Karbo
		changeCarbo();
		break;
	case 2: //Karbo-Hysertese
		changeHystCarbo();
		break;
	case 3: //MessIntervall
		changeMessInterval();
		break;
	case 4: //WiFi STA
		if (startWifi())
			startWebserver();
		break;
	case 5: //WiFi AP
		//init_WiFi_AP();
		break;
	case 6: //Sud ID
		changeSudID();
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
		//if (My_psk != "" || My_ssid != "") {
		//	writeConfigFile(WIFICONF);
		//	_configSave1 = "Y";
		//}
		//if (My_MySqlSrv != "" || My_MySqlPort != "") {
		//	writeConfigFile(MYSQLCONF);
		//	_configSave2 = "Y";
		//}
		//writeConfigFile(KARBOCONF);
		//_configSave3 = "Y";

		//DisplayInfo("l", "Save Config", "Karbo = " + _configSave3, "Wifi = " + _configSave1, "MySql = " + _configSave2);
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
	case 15: //Exit
		break;
	default:
		;
	}
	DisplayDaten(fTemp, fDruck, fCarbo, fSollCarbo);
}


void requestData() {
	
	OWSensorRequest();
	reqTime = millis();
	if (isActive == false)
	{
		SerialOut(F("isActive false requestData"));
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
	influxdb_write(fTemp, fDruck, fCarbo, fSollCarbo, fSollDruck);
	//setResetFlag = false;
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
	SerialOut(F("Setup begin..."));
	DisplayInit();
	Serial.println("iSPUNDER V" + String(PROG_VERSION) + " by HappyHibo");
	SerialOut(F("Init Hardware..."));
	initEncoder();
	initRelais();
	OWSensorInit();

	//leztzteSpunderConfig lesen
	readSpunderConfigFile(iSpunderNr, SudNr);
	readKarboConfigFile(fSollCarbo, MessInterval, fHystCarbo, fKompDruck);
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
	SerialOut(F("Setup ended..."));
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
		BtnPress = false;
		MenueEncoder();
	}

	if (CarboPressureCheck)
	{
		handle_CarboPressure();
	}

	if (maxPressure)
	{
		handle_MaxPressure();
	}

	if (isActive == false)
	{
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
		T25 - setzten Temperatur
		D6.5 . setzten Druck
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
	
	}
#endif // DEBUG

}
