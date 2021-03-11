/*
 Name:		iSpunder_V2.ino
 Created:	28.01.2018 18:22:46
 Author:	hh

 Pinbelegung

   WEMOS | OLED  
 --------+--------
   D1    | SCL
   D2    | SDA

         | ENC
 --------+--------
   D6    | CLK - Pullup 10k
   D7    | SW  - Pullup 10k
   D8    | DT  - Pullup 10k (Muss beim booten auf LOW liegen, sonst kein Start)

		 | TIP120  
 --------+--------
   D5    | R1k - Base 

		 | Drucksensor
 --------+--------
   A0    | R200k - Daten-Gelb

		 | DS18B20
 --------+--------
   D4	 | PIN2 - Pullup R4k7 (Muss beim booten auf High oder unbeschaltet sein)


*/


#include "config.h"

bool BtnPress = false;
bool DSrequested = false;
bool checkCarbo = false;

int number = 0;
int oldnumber = 0;
int encpos = 0;
int oldencpos = 0;
int menue = 0;

double carbo = 0.0;
double carbohyst = 0.25;
double sollcarbo;

ulong MessInterval = 60000;
ulong DSreqTime = MessInterval * 2;
int buttonState;

String incomingChar = "";
String My_ssid = "";
String My_psk = "";
String My_MySqlSrv = "";
String My_MySqlPort = "";
String My_iSpunderName = "";
String My_SudName = "";

unsigned long buttonPressTimeStamp;

float temp;
float druck;
float anain;

DeviceAddress OWDeviceAddress;
DynamicJsonBuffer jsonBuffer;
JsonVariant jsonVariant;


bool isDebugEnabled()
{
#ifdef DEBUG
	return true;
#endif // DEBUG
	return false;
}

template <typename T>
void SerialOut(const T aValue, bool newLine = true)
{
	if (!isDebugEnabled())
		return;
	Serial.print(aValue);
	if (newLine)
		Serial.print("\n");
}

void SerialOut() { SerialOut(""); }

void DisplayInfo(String Ausrichtung = "m", String Zeile1 = "", String Zeile2 = "", String Zeile3 = "", String Zeile4 = "") {

	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	int DisplayWidth = 128;
	int DisplayHight = 64;
	int FontHight = 16;
	int _Abstand;
	int _Zeile1;
	int _Spalte1, _Spalte2, _Spalte3, _Spalte4;
	int Zeilen;

	if (Zeile1 != "") Zeilen = 1;
	if (Zeile2 != "") Zeilen = 2;
	if (Zeile3 != "") Zeilen = 3;
	if (Zeile4 != "") Zeilen = 4;

	if (Zeilen == 1) {
		_Zeile1 = 32 - FontHight / 2;
		_Abstand = 0;
	}
	else if (Zeilen == 2)
	{
		_Abstand = 21;
		_Zeile1 = _Abstand - FontHight / 2;
	}
	else if (Zeilen == 3)
	{
		_Abstand = 16;
		_Zeile1 = _Abstand - FontHight / 2;
	}
	else if (Zeilen == 4)
	{
		_Abstand = 13;
		_Zeile1 = _Abstand - FontHight / 2;
	}

	int _Zeile2 = _Zeile1 + _Abstand;
	int _Zeile3 = _Zeile2 + _Abstand;
	int _Zeile4 = _Zeile3 + _Abstand;

	if (Ausrichtung == "m") {
		_Spalte1 = (DisplayWidth - display.getStringWidth(Zeile1)) / 2;
		_Spalte2 = (DisplayWidth - display.getStringWidth(Zeile2)) / 2;
		_Spalte3 = (DisplayWidth - display.getStringWidth(Zeile3)) / 2;
		_Spalte4 = (DisplayWidth - display.getStringWidth(Zeile3)) / 2;
	}
	else if (Ausrichtung = "l")
	{
		_Spalte1 = 5;
		_Spalte2 = 5;
		_Spalte3 = 5;
		_Spalte4 = 5;
	}
	else if (Ausrichtung = "r")
	{
		_Spalte1 = (DisplayWidth - display.getStringWidth(Zeile1)) - 5;
		_Spalte2 = (DisplayWidth - display.getStringWidth(Zeile2)) - 5;
		_Spalte3 = (DisplayWidth - display.getStringWidth(Zeile3)) - 5;
		_Spalte4 = (DisplayWidth - display.getStringWidth(Zeile3)) - 5;
	}

	display.drawRect(0, 0, 128, 64);
	display.drawString(_Spalte1, _Zeile1, Zeile1);
	if (Zeilen >= 2) display.drawString(_Spalte2, _Zeile2, Zeile2);
	if (Zeilen >= 3) display.drawString(_Spalte3, _Zeile3, Zeile3);
	if (Zeilen == 4) display.drawString(_Spalte4, _Zeile4, Zeile4);
	display.display();
}

void initRelais() {
	pinMode(RELAISPIN, OUTPUT);
	digitalWrite(RELAISPIN, LOW);
	SerialOut(F("init RelaisPin"));
}

void initDSSensor()
{
	// workaround for DS not enough power to boot
	pinMode(OW_BUS, OUTPUT);
	digitalWrite(OW_BUS, LOW);
	delay(100);
	OW.begin();
	OW.setWaitForConversion(false);
	OW.getAddress(OWDeviceAddress, 0);
	OW.setResolution(OWDeviceAddress, OWRESOLUTION);
	SerialOut(F("Init DS18B20..."));
	reqDSTemp();
}

void reqDSTemp()
{
	if (DSrequested) return;

	OW.requestTemperatures();
	DSreqTime = millis();
	DSrequested = true;
	SerialOut(F("Requesting DS18B20 temperature..."));
}

void getDSTemp() {
	if (!DSrequested) return;
	SerialOut(F("Reading DS18B20 temperature..."));
	//OW.requestTemperatures();
	temp = OW.getTempCByIndex(0);
	/*if (temp == 85.0 || temp == (-127.0)) {
	SerialOut(F("ERROR: OW DISCONNECTED"));
	pinMode(ONE_WIRE_BUS, OUTPUT);
	digitalWrite(ONE_WIRE_BUS, LOW);
	delay(100);
	oneWire.reset();
	initDSSensor();
	delay(OWINTERVAL + 100);
	getDSTemp();
	}*/
	DSrequested = false;
	temp = roundf(temp * 10.0f) / 10.0f;
	SerialOut(F("Temp: "), false);
	SerialOut(temp);
}

void getPressure()
{
	anain = analogRead(A0);
	druck = ((anain / 204.8 - 0.5) / 0.8) - 0.05;
	SerialOut(F("AnalogIn: "), false);
	SerialOut(anain);
	SerialOut(F("Druck: "), false);
	SerialOut(druck);

	//  0 Bar = 0,5 V
	//	5 Bar = 4,5 V
	//	Daraus ergibt sich die Geradengleichung y(in V) = x(in Bar) * 0,8V/Bar + 0,5V
	//	Da ich y messe, die Gleichung nach x umstellen und es ergibt sich
	//	x = (y - 0,5) / 0,8 (in Bar)
	//	Da ich für 5V den Wert 1023 als reading erhalte, habe ich noch den Teilungsfaktor 1024/5 = 204,8 einfügen müssen.
	//  x = (y / 204,8 - 0,5) / 0,8 (in Bar)
	//  Quelle: https://forum.fhem.de/index.php/topic,51725.0.html
}

void calcCarbo() {

	carbo = ((double)druck + 1.013) * (pow(2.71828182845904, (-10.73797 + (2617.25 / (temp + 273.15))))) * 10;
	SerialOut(F("Karbo: "), false);
	SerialOut(carbo);
}

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
}

void handle_Btn_pressed(Button2& btn) {
	SerialOut(F("pressed"));
	BtnPress = true;
}
	
void handle_Btn_tap(Button2& btn) {
	SerialOut(F("tap"));
	SerialOut(F("Debug: Was pressed for "), false);
	SerialOut(btn.wasPressedFor(), false);
	SerialOut(F(" ms"));
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
		checkCarbo = true;
		break;
	case TRIPLE_CLICK:
		SerialOut(F("TRIPLE_CLICK= "), false);
		SerialOut(TRIPLE_CLICK);
		init_WiFi_AP();
		break;
	case LONG_CLICK:
		SerialOut(F("LONG_CLICK= "), false);
		SerialOut(LONG_CLICK);
		SerialOut(F("Reset..."));
		checkCarbo = false;
		BtnPress = false;
		connectWifi();
		break;
	}
	SerialOut(F("Debug: ClickType = "), false);
	SerialOut(btn.getClickType());
	SerialOut(F("Debug: Was pressed for "), false);
	SerialOut(btn.wasPressedFor(),false);
	SerialOut(F(" ms"));
}

void changeMessInterval() {
	double _Minuten;
	SerialOut(F("Insert changeMessInterval"));
	SerialOut(BtnPress);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	_Minuten = MessInterval / 60000;
	DisplayInfo("m", "M-Interval", String(_Minuten), "Min.");
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
		if (encpos != oldencpos) {
			DisplayInfo("m", "M-Interval", String(_Minuten), "Min.");
		}
		display.display();
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	MessInterval = _Minuten * 60000;
	SerialOut(F("MessInterval "),false);
	SerialOut(MessInterval);
	BtnPress = false;
	SerialOut(F("Exit changeMessInterval"));
	//display.clearDisplay();
	menue = 0;

}


void changeCarbo() {
	SerialOut(F("Insert ChangeCarbo"));
	SerialOut(BtnPress);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	display.drawString(10, 2, "Karbonisierung");
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		if (encpos > oldencpos) {
			sollcarbo += 0.1;
			SerialOut(F("sollcarbo ++ "), false);
			SerialOut(sollcarbo);
		}
		else if (encpos < oldencpos) {
			sollcarbo -= 0.1;
			if (sollcarbo <= 0) sollcarbo = 0;
			SerialOut(F("sollcarbo -- "), false);
			SerialOut(sollcarbo);
		}
		if (encpos != oldencpos) display.clear();
		display.drawString(10, 2, "Karbonisierung");
		display.drawRect(0, 0, 128, 64);
		if (sollcarbo < 9.9) {
			display.drawString(35, 37, "0" + String(sollcarbo) + " g/l");
		}
		else {
			display.drawString(35, 37, String(sollcarbo) + " g/l");
		}
		display.display();
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Exit ChangeCarbo"));
	//display.clearDisplay();
	menue = 0;
}

void handle_CarboValve() {
	SerialOut(F("handle_CarboValve"));

	if ((temp > -5) && (carbo > sollcarbo + carbohyst)) {
		SerialOut(F("Open Valve"));
		digitalWrite(RELAISPIN, HIGH);
		checkCarbo = true;
	}
	else
	{
		SerialOut(F("Close Valve"));
		digitalWrite(RELAISPIN, LOW);
	}

	SerialOut(F("Exit handle_CarboValve"));
}

void handle_CarboPressure() {
	SerialOut(F("handle_CarboPressure"));

	if (!checkCarbo) {
		SerialOut(F("Return handle_CarboPressure"));
		return;
	}
	do
	{
		BTNA.loop();
		getPressure();
		calcCarbo();
		DisplayDaten();
		if (carbo <= sollcarbo || BtnPress)
		{
			checkCarbo = false;
		}
		SerialOut(F("checkCarbo "), false);
		SerialOut(checkCarbo);
		delay(100);
		yield();
	} while (checkCarbo);
	BtnPress = false;
	SerialOut(F("Close Valve"));
	digitalWrite(RELAISPIN, LOW);

	SerialOut(F("Exit handle_CarboPressure"));
}

void DisplayDaten() {

	int _ABST = 15;
	int _S1 = 0;
	int _S2 = 58;
	display.clear();
	//display.display();
	//if (!OLEDOnOff) return;
	int _Z1 = 0;
	display.drawString(_S1, _Z1, "Temp.: ");
	if (temp < 9.9 && temp >= 0.0) {
		display.drawString(_S2, _Z1, "0" + String(temp) + "  C");
	}
	else {
		display.drawString(_S2, _Z1, String(temp) + "  C");
	}
	display.drawCircle(102, 5, 2);
	int _Z2 = _Z1 + _ABST;
	display.drawString(_S1, _Z2, "Druck :");
	if (druck < 9.9 && druck >= 0.0) {
		display.drawString(_S2, _Z2, "0" + String(druck) + " bar");
	}
	else {
		display.drawString(_S2, _Z2, String(druck) + " bar");
	}
	int _Z3 = _Z2 + _ABST;
	display.drawString(_S1, _Z3, "Karbo :");
	if (carbo < 9.9 && carbo >= 0.0) {
		display.drawString(_S2, _Z3, "0" + String(carbo) + " g/l");
	}
	else
	{
		display.drawString(_S2, _Z3, String(carbo) + " g/l");
	}
	int _Z4 = _Z3 + _ABST;
	display.drawString(_S1, _Z4, "SoKar : ");
	if (sollcarbo < 9.9 && sollcarbo >= 0.0) {
		display.drawString(_S2, _Z4, "0" + String(sollcarbo) + " g/l");
	}
	else
	{
		display.drawString(_S2, _Z4, String(sollcarbo) + " g/l");
	}
	display.display();

}


void DisplayMenue() {
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	int _MenuePos = 1;
	int _Abstand = 15;
	int _S1 = 2;
	int _S2 = 17;
	int _Z1 = 2;
	int _Z2 = _Z1 + _Abstand;
	int _Z3 = _Z2 + _Abstand;
	int _Z4 = _Z3 + _Abstand;
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		if (encpos > oldencpos) {
			_MenuePos += 1;
			if (_MenuePos > 8) _MenuePos = 1;
		}
		else if (encpos < oldencpos) {
			_MenuePos -= 1;
			if (_MenuePos < 1) _MenuePos = 8;
		}
		if (encpos != oldencpos) {
			display.clear();
		}
		display.drawRect(0, 0, 128, 64);
		display.drawString(20, 1, "--- Menue ---");
		display.drawLine(0, 18, 128, 18);
		display.drawString(2, _Z3, ">");
		display.drawString(117, _Z3, "<");
		//display.drawRect(2, _Z3, 124, 17);
		switch (_MenuePos) {
			case 1:
				//display.drawString(_S2, _Z2, "Exit");
				display.drawString(_S2, _Z3, "Soll-Karbo");
				//display.drawString(_S2, _Z4, "Wifi STA");
				display.display();
				break;
			case 2:
				//display.drawString(_S2, _Z2, "Exit");
				display.drawString(_S2, _Z3, "MessInterval");
				//display.drawString(_S2, _Z4, "Wifi STA");
				display.display();
				break;
			case 3:
				//display.drawString(_S2, _Z2, "Soll-Karbo");
				display.drawString(_S2, _Z3, "Wifi STA");
				//display.drawString(_S2, _Z4, "Wifi AP");
				display.display();
				break;
			case 4:
				//display.drawString(_S2, _Z2, "Wifi STA");
				display.drawString(_S2, _Z3, "Wifi AP");
				//display.drawString(_S2, _Z4, "Save Config");
				display.display();
				break;
			case 5:
				//display.drawString(_S2, _Z2, "Wifi AP");
				display.drawString(_S2, _Z3, "Save Config");
				//display.drawString(_S2, _Z4, "IP Address");
				display.display();
				break;
			case 6:
				//display.drawString(_S2, _Z2, "Save Config");
				display.drawString(_S2, _Z3, "IP Address");
				//display.drawString(_S2, _Z4, "Exit");
				display.display();
				break;
			case 7:
				//display.drawString(_S2, _Z2, "IP Address");
				display.drawString(_S2, _Z3, "Format SPIFF");
				//display.drawString(_S2, _Z4, "Soll-Karbo");
				display.display();
				break;
			case 8:
				//display.drawString(_S2, _Z2, "IP Address");
				display.drawString(_S2, _Z3, "Exit");
				//display.drawString(_S2, _Z4, "Soll-Karbo");
				display.display();
				break;
		}
		menue = _MenuePos;
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	BtnPress = false;
}

void menueList() {
	String _configSave1 = "N";
	String _configSave2 = "N";
	String _configSave3 = "N";
	switch (menue) {
	case 1: //Soll-Karbo
		changeCarbo();
		break;
	case 2: //MessIntervall
		changeMessInterval();
		break;
	case 3: //WiFi STA
		connectWifi();
		break;
	case 4: //WiFi AP
		init_WiFi_AP();
		break;
	case 5: //Save Config
		if (My_psk != "" || My_ssid != "") {
			writeConfigFile(WIFICONF);
			_configSave1 = "Y";
		}
		if (My_MySqlSrv != "" || My_MySqlPort != "") {
			writeConfigFile(MYSQLCONF);
			_configSave2 = "Y";
		}
		writeConfigFile(KARBOCONF);
		_configSave3 = "Y";

		DisplayInfo("l", "Save Config", "Karbo = " + _configSave3, "Wifi = " + _configSave1, "MySql = " +_configSave2 );
		delay(3000);
		break;
	case 6: //IP Adresse
		BtnPress = false;
		do {
			BTNA.loop();
			DisplayInfo("m", "STA Mode", "IP " + WiFi.localIP().toString());
		} while (!BtnPress);
		BtnPress = false;
		break;
	case 7: //Format SPIFF
		SPIFFS.end();
		SPIFFS.begin();
		SerialOut(F("Formating SPIFFS: "), false);
		SerialOut(SPIFFS.format());
		SPIFFS.end();
		break;
	case 8: //Exit
		break;
	default:
		;
	}
	menue = 0;
}

bool connectWifi()
{
	SerialOut(F("Disconnect AP"));
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	SerialOut(F("Change Wifi Mode STA"));
	WiFi.mode(WIFI_STA);
	// Mount the filesystem
	//bool result = SPIFFS.begin();
	//SerialOut(F("SPIFFS opened: "), false);
	//SerialOut(result);
	if (!readConfigFile(WIFICONF)) {
		SerialOut(F("Failed to read wifi configuration file..."));
	}
	WiFi.begin(My_ssid.c_str(), My_psk.c_str());
	DisplayInfo("m", "Connect WiFi", "to", My_ssid);
	SerialOut(F("Connect Wifi credentials"));
	delay(100);
	SerialOut(F("...connecting..."));
	SerialOut(F("to "), false);
	SerialOut(My_ssid);
	SerialOut(F("with "), false);
	SerialOut(My_psk);
	//while (WiFi.status() != WL_CONNECTED) {
	//	delay(500);
	//	Serial.print(".");
	//}
	int connRes = WiFi.waitForConnectResult();
	SerialOut(F("Connection result is "), false);
	SerialOut(connRes);

	if (WiFi.status() == WL_CONNECT_FAILED) {
		SerialOut(F("Wrong Password..."));
		DisplayInfo("m", "Wrong PW");
	}
	if (WiFi.status() == WL_NO_SSID_AVAIL) {
		SerialOut(F("AP not found..."));
		DisplayInfo("m", "No AP found");
	}

	if (WiFi.status() != WL_CONNECTED) {
		SerialOut(F("failed to connect, change tp AP-Mode"));
		DisplayInfo("m", "Change", "to", "AP-Mode");
		init_WiFi_AP();
	}
	else {
		SerialOut(F("local ip: "), false);
		SerialOut(WiFi.localIP());
		DisplayInfo("m", "STA Mode", "IP " + WiFi.localIP().toString());
		if (!readConfigFile(MYSQLCONF)) {
			SerialOut(F("Failed to read mysql configuration file, using default values"));
		}
	}
	menue = 0;
}

void init_WiFi_AP() {
	SerialOut(F("WiFi Off"));
	WiFi.mode(WIFI_OFF);
	SerialOut(F("Change Wifi Mode AP-STA"));
	WiFi.mode(WIFI_AP_STA);
	delay(500);
	WiFi.begin();
	WiFi.softAP(ssidAP);

	WiFi.softAPConfig(
		IPAddress(atoi(wifi_ip[0]), atoi(wifi_ip[1]), atoi(wifi_ip[2]), atoi(wifi_ip[3])),
		IPAddress(atoi(wifi_gateway[0]), atoi(wifi_gateway[1]), atoi(wifi_gateway[2]), atoi(wifi_gateway[3])),
		IPAddress(atoi(wifi_subnet[0]), atoi(wifi_subnet[1]), atoi(wifi_subnet[2]), atoi(wifi_subnet[3]))
	);

	SerialOut(F("WiFi AP : "), false);
	SerialOut(ssidAP);

	SerialOut();
	SerialOut(F("WiFi AP Success"));
	IPAddress myIP = WiFi.softAPIP();
	SerialOut(F("AP IP address: "), false);
	SerialOut(myIP);
	server.stop();
	initWebserver();
	DisplayInfo("m", "AP Mode", "IP " + myIP.toString(), "/wifi");
	delay(5000);
}

void initWebserver() {

	// Start the server
	server.on("/", handle_WebRoot);
	server.on("/wifi", handle_Wifi);
	server.on("/mysql", handle_mysql);

	server.onNotFound(handle_WebNotFound);
	//here the list of headers to be recorded
	server.begin();
	SerialOut(F("WebServer started"));
}


void handle_mysql() {
	SerialOut(F("Enter handle_mysql"));

	for (uint8_t i = 0; i<server.headers(); i++) {
		SerialOut("HeaderName: " + server.headerName(i) + " Header: " + server.header(i));
	}
	for (uint8_t i = 0; i<server.args(); i++) {
		SerialOut("ArgName: " + server.argName(i) + " Arg: " + server.arg(i));
	}
	if (server.hasArg("MySqlSrv")) {
		My_MySqlSrv = server.arg("MySqlSrv").c_str();
		server.sendHeader("Location", "/mysql");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received My_MySqlSrv: "), false);
		SerialOut(My_MySqlSrv);

	}
	if (server.hasArg("MySqlPort")) {
		My_MySqlPort = server.arg("MySqlPort").c_str();
		server.sendHeader("Location", "/mysql");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received My_MySqlPort: "), false);
		SerialOut(My_MySqlPort);
	}
	if (server.hasArg("iSpunderName")) {
		My_iSpunderName = server.arg("iSpunderName").c_str();
		server.sendHeader("Location", "/mysql");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received My_iSpunderName: "), false);
		SerialOut(My_iSpunderName);
	}
	if (server.hasArg("SudName")) {
		My_SudName = server.arg("SudName").c_str();
		server.sendHeader("Location", "/mysql");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received My_SudName: "), false);
		SerialOut(My_SudName);
	}

	if (server.hasArg("LoadConfig")) {
		readConfigFile(MYSQLCONF);
		server.sendHeader("Location", "/mysql");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received LoadConfig: "), false);
	}

	if (server.hasArg("Speichern")) {
		SerialOut(F("Save Mysql-Config..."));
		writeConfigFile(MYSQLCONF);
		server.sendHeader("Location", "/mysql");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received BtnSpeichern"));
	}

	String content = "<html><head></head>";
	content += "<body><H2>iSpunder MySql Config</H2><br>";
	content += "iSpunder-Angaben: <br>";
	content += "<form action='/mysql' method='POST'>";
	content += "<label for='MySqlSrv'>MySql Server :</label>";
	content += "<input type='text' name='MySqlSrv' size='15' value=" + My_MySqlSrv + "><br>";
	content += "<label for='MySqlPort'>MySql Port :</label>";
	content += "<input type='text'' name='MySqlPort' size='15' value=" + My_MySqlPort + "><br>";
	content += "<label for='iSpunderName'>iSpunder Name :</label>";
	content += "<input type='text'' name='iSpunderName' size='15' value=" + My_iSpunderName + "><br>";
	content += "<label for='SudName'>Sud-Name :</label>";
	content += "<input type='text'' name='SudName' size='15' value=" + My_SudName + "><br>";
	content += "<input type='submit' name='Speichern' value='Speichern'></form><br>";
	content += "<form action='/mysql' method='post'>";
	content += "<button type='submit' name='LoadConfig' value='LoadConfig'>Konfig laden</button>";
	//content += "<button type='submit' name='SaveConfig' value='SaveConfig'>Config laden</button>";
	content += "</form><br>";
	content += "</body></html>";
	server.send(200, "text/html", content);
}

void handle_Wifi() {
	SerialOut(F("Enter handle_Wifi"));

	for (uint8_t i = 0; i<server.headers(); i++) {
		SerialOut("HeaderName: " + server.headerName(i) + " Header: " + server.header(i));
	}
	for (uint8_t i = 0; i<server.args(); i++) {
		SerialOut("ArgName: " + server.argName(i) + " Arg: " + server.arg(i));
	}
	if (server.hasArg("SSID")) {
		My_ssid = server.arg("SSID").c_str();
		server.sendHeader("Location", "/wifi");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received SSID: "), false);
		SerialOut(My_ssid);

	}
	if (server.hasArg("PSK")) {
		My_psk = server.arg("PSK").c_str();
		server.sendHeader("Location", "/wifi");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received PSK: "), false);
		SerialOut(My_psk);
	}
	if (server.hasArg("Speichern")) {
		if (My_psk != "" || My_ssid != "") {
			SerialOut(F("Save Config..."));
			writeConfigFile(WIFICONF);
		}
		server.sendHeader("Location", "/wifi");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
		SerialOut(F("Received BtnSpeichern"));

	}
	if (server.hasArg("AP_OFF")) {
		SerialOut(F("Wifi OFF"));
		WiFi.mode(WIFI_OFF);
		SerialOut(F("Change Wifi Mode STA"));
		WiFi.mode(WIFI_STA);
		connectWifi();
		//writeConfigFile();
		server.sendHeader("Location", "/");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
	}

	String content = "<html><head></head>";
	content += "<body><H2>iSpunder Wifi Config</H2><br>";
	content += "WLAN-Angaben: <br>";
	content += "<form action='/wifi' method='POST'>";
	content += "SSID: <input type='text' name='SSID' size='15' placeholder='ssid'><br>";
	content += "PSK : <input type='password' name='PSK' size='15' placeholder='psk'><br>";
	content += "<input type='submit' name='Speichern' value='Speichern'></form><br>";
	content += "<form action='' method='post'>";
	content += "<button type='submit' name='AP_OFF' value='AP_OFF'>Accesspoint OFF</button>";
	content += "</form><br>";
	content += "</body></html>";
	server.send(200, "text/html", content);
}

void handle_WebRoot() {
	SerialOut(F("Enter handle_WebRoot"));

	for (uint8_t i = 0; i<server.headers(); i++) {
		SerialOut("HeaderName: " + server.headerName(i) + " Header: " + server.header(i));
	}
	for (uint8_t i = 0; i<server.args(); i++) {
		SerialOut("ArgName: " + server.argName(i) + " Arg: " + server.arg(i));
	}

	if (server.hasArg("SollKarbo")) {
		sollcarbo = atof(server.arg("SollKarbo").c_str());
		server.sendHeader("Location", "/");
		server.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		server.send(301);
	}

	if (server.hasArg("RelaisOnOff")) {
		if (server.arg("RelaisOnOff") == "REL_ON") {
			digitalWrite(RELAISPIN, HIGH);
			SerialOut(F("RelaisPin High"));
		}
		else if (server.arg("RelaisOnOff") == "REL_OFF")
		{
			digitalWrite(RELAISPIN, LOW);
			SerialOut(F("RelaisPin Low"));

		}
	}
	String content = "<html><head><meta http-equiv='refresh' content='10' ></head>";
	content += "<body><H2>iSpunder Webpage</H2><br>";
	content += "<form action='/' method='POST'>";
	content += "Karbonisierung: <input type='text' name='SollKarbo' size='5' value=" + String(sollcarbo) + "> g/l<br>";
	content += "<input type='submit' name='SUBMIT' value='Submit'></form><br>";
	content += "<form action='' method='post'>";
	content += "<fieldset>";
	content += "<legend>Relais Status</legend>";
	content += "<input type='radio' name='RelaisOnOff' value='REL_ON'> ON";
	content += "<input type='radio' name='RelaisOnOff' value='REL_OFF'> OFF<br>";
	content += "</fieldset><input type='submit' value='Submit'></form><br>";
	content += "<br>Temperatur = ";
	content += String(temp);
	content += " C";
	content += "<br>";
	content += "Druck = ";
	content += String(druck);
	content += " Bar";
	content += "<br>";
	content += "Ist-Karbo = ";
	content += String(carbo);
	content += " g/l";
	content += "<br>";
	content += "Soll-Karbo = ";
	content += String(sollcarbo);
	content += " g/l";
	content += "<br>";
	content += "</body></html>";
	server.send(200, "text/html", content);
}

void handle_WebNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i = 0; i<server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}

bool readConfigFile(int config) {
	SerialOut(F("readConfigFile "), false);
	SerialOut(config);
	if (!((config == WIFICONF) || (config == MYSQLCONF) || (config == KARBOCONF))) {
		SerialOut(F("keyword not found... return readConfigFile"));
		return false;
	}
	File f;
	char *_configfile;
	if (config == WIFICONF) {
		_configfile = "/wificonf.json";
	}
	if (config == MYSQLCONF) {
		_configfile = "/mysqlconfig.json";
	}
	if (config == KARBOCONF) {
		_configfile = "/karboconfig.json";
	}

	SerialOut(F("mounting FS..."), false);

	if (SPIFFS.begin())
	{
		SerialOut(F(" mounted!"));
		if (SPIFFS.exists(_configfile))
		{
			SerialOut(F("reading config file"));
			// this opens the config file in read-mode
			f = SPIFFS.open(_configfile, "r");
			// we could open the file
			size_t size = f.size();
			SerialOut(F("Size config file: "), false);
			SerialOut(size);
			if (size > 1024) {
				SerialOut(F("Config file size is too large"));
				return false;
			}
			// Allocate a buffer to store contents of the file.
			//std::unique_ptr<char[]> buf(new char[size]);

			// Read and store file contents in buf
			//f.readBytes(buf.get(), size);
			// Closing file
			//f.close();
			// Using dynamic JSON buffer which is not the recommended memory model, but anyway
			// See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model
			//DynamicJsonBuffer jsonBuffer;
			StaticJsonBuffer<256> jsonBuffer;
			// Parse JSON string
			JsonObject &json = jsonBuffer.parseObject(f);
			// Test if parsing succeeds.
			if (!json.success()) {
				SerialOut(F("JSON parseObject() failed"));
				return false;
			}
			json.printTo(Serial);

			// Parse all config file parameters, override 
			// local config variables with parsed values
			if (config == WIFICONF) {
				if (json.containsKey("SSID"))
					My_ssid = (const char *)json["SSID"];
				if (json.containsKey("PSK"))
					My_psk = (const char *)json["PSK"];
			}
			if (config == MYSQLCONF) {
				if (json.containsKey("MYSQLSRV"))
					My_MySqlSrv = (const char *)json["MYSQLSRV"];
				if (json.containsKey("MYSQLPORT"))
					My_MySqlPort = (const char *)json["MYSQLPORT"];
				if (json.containsKey("SPUNDERNAME"))
					My_iSpunderName = (const char *)json["SPUNDERNAME"];
				if (json.containsKey("SUDNAME"))
					My_SudName = (const char *)json["SUDNAME"];
			}
			if (config == KARBOCONF) {
				if (json.containsKey("SKARBO"))
					sollcarbo = (double)json["SKARBO"];
				if (json.containsKey("MINTERVAL"))
					MessInterval = (ulong)json["MINTERVAL"];
			}
		}
		else {
			SerialOut(F("ERROR: failed to load json config"));
			f.close();
			SPIFFS.end();
			return false;
		}
		f.close();
		SPIFFS.end();
	}
	
	SerialOut("Configfile loaded succesfully");
	return true;
}

bool writeConfigFile(int config) {
	SerialOut(F("writeConfigFile "), false);
	SerialOut(config);
	if (!((config == WIFICONF) || (config == MYSQLCONF) || (config == KARBOCONF))) {
		SerialOut(F("keyword not found... return writeConfigFile"));
		return false;
	}
	char *_configfile;
	if (config == WIFICONF) {
		_configfile = "/wificonf.json";
	}
	if (config == MYSQLCONF) {
		_configfile = "/mysqlconfig.json";
	}
	if (config == KARBOCONF) {
		_configfile = "/karboconfig.json";
	}

	SerialOut(F("Saving config file"));

	if (!SPIFFS.begin() 
			|| (!SPIFFS.exists("/wificonfig.json") && !SPIFFS.exists("/mysqlconfig.json") && !SPIFFS.exists("/karboconfig.json")) 
			|| (!SPIFFS.open("/wificonfig.json", "w") && !SPIFFS.open("/mysqlconfig.json", "w") && !SPIFFS.open("/karboconfig.json", "w")))
	{
		SerialOut(F("\nneed to format SPIFFS: "), false);
		SPIFFS.end();
		SPIFFS.begin();
		SerialOut(SPIFFS.format());
	}
	StaticJsonBuffer<256> jsonBuffer;
	//DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	// JSONify local configuration parameters
	if (config == WIFICONF) {
		json["SSID"] = My_ssid;
		json["PSK"] = My_psk;
	}
	if (config == MYSQLCONF) {
		json["MYSQLSRV"] = My_MySqlSrv;
		json["MYSQLPORT"] = My_MySqlPort;
		json["SPUNDERNAME"] = My_iSpunderName;
		json["SUDNAME"] = My_SudName;
	}
	if (config == KARBOCONF) {
		json["SKARBO"] = sollcarbo;
		json["MINTERVAL"] = MessInterval;
	}
	// Open file for writing
	File f = SPIFFS.open(_configfile, "w");
	if (!f) {
		SerialOut(F("Failed to open config file for writing"));
		SPIFFS.end();
		return false;
	}
	else {
		json.prettyPrintTo(Serial);
		// Write data to file and close it
		json.printTo(f);
		size_t size = f.size();
		SerialOut(F("Size config file: "), false);
		SerialOut(size);
		f.close();	
	}
	SPIFFS.end();
	SerialOut("Config file was successfully saved");
	return true;
}

bool deleteConfigFile(int config) {
	SerialOut(F("deleteConfigFile "), false);
	SerialOut(config);
	if (!((config == WIFICONF) || (config == MYSQLCONF) || (config == KARBOCONF))) {
		SerialOut(F("keyword not found... return deleteConfigFile false"));
		return false;
	}
	char *_configfile;
	if (config == WIFICONF) {
		_configfile = "/wificonfig.json";
	}
	if (config == MYSQLCONF) {
		_configfile = "/mysqlconfig.json";
	}
	if (config == KARBOCONF) {
		_configfile = "/karboconfig.json";
	}

	SerialOut(F("mounting FS..."), false);

	if (SPIFFS.begin())
	{
		SerialOut(F(" mounted!"));
		if (SPIFFS.exists(_configfile))
		{
			SerialOut(F("delete config file"));
			SPIFFS.remove(_configfile); 
		}
		SPIFFS.end();
		SerialOut("Config file was successfully deleted");
		return true;
	}
}

void sendData() {

	SerialOut(F("Send data to mysql..."));
	if (My_MySqlSrv == "" || druck < 0 || temp < 0) {
		SerialOut(F("No Mysql-Server - return"));
		return;
	}

	WiFiClient client;
	jsonVariant = jsonBuffer.createObject();

	jsonVariant["id"] = ESP.getChipId();
	jsonVariant["name"] = My_iSpunderName;
	jsonVariant["temperatur"] = temp;
	jsonVariant["druck"] = druck;
	jsonVariant["karbo"] = carbo;
	jsonVariant["sollkarbo"] = sollcarbo;
	jsonVariant["sudname"] = My_SudName;
	//jsonVariant["relais"] = schaltrelais;

	jsonVariant.printTo(Serial);

	if (client.connect(My_MySqlSrv.c_str(), My_MySqlPort.toInt()))
	{
		SerialOut(F("Sender: TCP stream"));
		jsonVariant.printTo(client);
		client.println();
	}
	else
	{
		SerialOut(F("\nERROR Sender: couldnt connect"));
	}

	int timeout = 0;
	while (!client.available() && timeout < 2000)
	{
		timeout++;
		delay(1);
	}
	while (client.available())
	{
		char c = client.read();
		Serial.write(c);
	}
	client.stop();
	delay(100); // allow gracefull session close
	SerialOut(F("Exit SendData..."));
}

void reqData() {
	reqDSTemp();
	getDSTemp();
	getPressure();
	calcCarbo();
	handle_CarboValve();
	DisplayDaten();
	sendData();
}



// the setup function runs once when you press reset or power the board
void setup() {
	Serial.begin(115200);
	SerialOut(F("Setup begin..."));
	SerialOut(F("Init OLED-Screen..."));
	display.init();
	display.flipScreenVertically();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	DisplayInfo("m", "iSPUNDER", "V " + String(PROG_VERSION), "by HappyHibo");
	SerialOut(F("Init Harware..."));
	initEncoder();
	initDSSensor();
	initRelais();
	delay(2000);
	SerialOut(F("Read KarboConfig..."));
	if (!readConfigFile(KARBOCONF)) {
		SerialOut(F("Failed to read sollkarbo configuration file, using default values"));
		sollcarbo = 5.50;
		MessInterval = 60000;
	}

	SerialOut(F("Connect Wifi..."));
	connectWifi();

	if (WiFi.status() == WL_CONNECTED)
	{
		SerialOut(F("Wifi Connect..."));
		SerialOut(F("Starte Webserver"));
		initWebserver();

	}
	else
	{
		SerialOut(F("No Wifi Connect..."));
		//SerialOut(F("Goto AP-Mode..."));
		//init_WiFi_AP();

		// test if ssid exists
		if (WiFi.SSID() == "" &&
			My_ssid != "" && My_psk != "")
		{

		}
	}

	delay(5000);
	reqData();
	SerialOut(F("Exit Setup..."));
}

// the loop function runs over and over again until power down or reset
void loop() {

	BTNA.loop();
	server.handleClient();
	
	if (millis() - DSreqTime >= MessInterval) {
		reqData();
	}

	//readBtnEncoder();
	//readEncoder();
	
	if (BtnPress) {
		BtnPress = false;
		//SerialOut(F("*LOOP* ButtonPress state FALSE "), false);
		//SerialOut(BtnPress);
		DisplayMenue();
		menueList();
		reqData();
	}
		
	if (checkCarbo) {
		handle_CarboPressure();
	}
	//ESPR.loop();
#ifdef DEBUG
	//Abfrage serielle Eingabe
	while (Serial.available()) {
		incomingChar = Serial.readString();// read the incoming data as string
		SerialOut(F("*LOOP* Incoming Serial: "), false);
		SerialOut(incomingChar);
		if (incomingChar == "R1") {
			digitalWrite(RELAISPIN, HIGH);
			SerialOut(F("Relais On"));
		}
		else if (incomingChar == "R0")
		{
			digitalWrite(RELAISPIN, LOW);
			SerialOut(F("Relais Off"));
		}
		if (incomingChar == "sendmysql") sendData();

		if (incomingChar == "delWifi") deleteConfigFile(WIFICONF);
		if (incomingChar == "delMySql") deleteConfigFile(MYSQLCONF);
		if (incomingChar == "delKarbo") deleteConfigFile(KARBOCONF);
		if (incomingChar == "Sudhaus") My_ssid = incomingChar;
		if (incomingChar == "***********") My_psk = incomingChar;
	}
#endif // DEBUG

	yield();
}
