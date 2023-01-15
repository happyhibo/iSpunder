/*
 Name:		iSpunder_2021.ino
 Created:	08.03.2021 16:54:04
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

/*
	- V21.1.0 08.03.21
		neue Überarbeitung
*/

#include "flashfilesystem.h"
#include "webserver.h"
#include "display.h"
#include "debuging.h"
#include "config.h"

float temp;
float druck;
float anain;

DeviceAddress OWDeviceAddress;
DynamicJsonBuffer jsonBuffer;
JsonVariant jsonVariant;

WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

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
	yield();
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

	// Quelle: http://braukaiser.com/wiki/index.php/Carbonation_Tables
	// Cbeer = (Phead+1.013)*(2.71828182845904^(-10.73797+(2617.25/(Tbeer+273.15))))*10
	// Cbeer - carbonation of the beer in g/l
	// Phead - head pressure in bar
	// Tbeer - temperature of the beer in C
	// e = eulscher Zahl = 2.71828182845904
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
	yield();
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
	yield();
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
	SerialOut(btn.wasPressedFor(), false);
	SerialOut(F(" ms"));
	yield();
}

void changeMessInterval() {
	double _Minuten;
	SerialOut(F("Insert changeMessInterval"));
	SerialOut(BtnPress);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	_Minuten = MessInterval / 60000;
	DisplayInfo("m", "M-Interval", String(_Minuten), "Min.","");
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
			DisplayInfo("m", "M-Interval", String(_Minuten), "Min.","");
		}
		display.display();
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	MessInterval = _Minuten * 60000;
	SerialOut(F("MessInterval "), false);
	SerialOut(MessInterval);
	BtnPress = false;
	SerialOut(F("Exit changeMessInterval"));
	//display.clearDisplay();
	menue = 0;

}

void changeSudID() {

	SerialOut(F("Insert changeSudID"));
	SerialOut(BtnPress);
	int _Sudid = My_SudID;

	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	DisplayInfo("m", "Sud-ID", "#" + String(_Sudid),"","");
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		if (encpos > oldencpos) {
			_Sudid += 1;
		}
		else if (encpos < oldencpos) {
			_Sudid -= 1;
			if (_Sudid <= 0) _Sudid = 0;
		}
		if (encpos != oldencpos) {
			DisplayInfo("m", "Sud-ID", "#" + String(_Sudid),"","");
		}
		display.display();
		oldencpos = encpos;
		yield();
	} while (!BtnPress);

	SerialOut(F("Sud-ID #"), false);
	SerialOut(_Sudid);
	BtnPress = false;
	SerialOut(F("Exit changeMessInterval"));
	//display.clearDisplay();
	menue = 0;
	My_SudID = _Sudid;
	My_SudName = String("#" + My_SudID);

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


void changeCarboHysteresis() {
	SerialOut(F("Insert changeCarboHysteresis"));
	SerialOut(BtnPress);
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	display.drawString(10, 2, "K-Hysterese");
	do {
		ESPR.loop(); //encoder loop
		BTNA.loop(); //button loop
		if (encpos > oldencpos) {
			carbohyst += 0.05;
			SerialOut(F("carbohyst ++ "), false);
			SerialOut(carbohyst);
		}
		else if (encpos < oldencpos) {
			carbohyst -= 0.05;
			//if (carbohyst <= 0) carbohyst = 0;
			SerialOut(F("carbohyst -- "), false);
			SerialOut(carbohyst);
		}
		if (encpos != oldencpos) display.clear();
		display.drawString(10, 2, "K-Hysterese");
		display.drawRect(0, 0, 128, 64);
		display.drawString(35, 37, String(carbohyst));
		/*if (carbohyst < 9.9) {
			display.drawString(35, 37, "0" + String(carbohyst));
		}
		else {
			display.drawString(35, 37, String(carbohyst));
		}*/
		display.display();
		oldencpos = encpos;
		yield();
	} while (!BtnPress);
	BtnPress = false;
	SerialOut(F("Exit changeCarboHysteresis"));
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
	yield();
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
		DisplayDaten(temp, druck, carbo, sollcarbo);
		if (carbo <= sollcarbo - carbohyst || BtnPress)
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

void sendDataMySQL() {
	SerialOut(F("Send data to mysql..."));
#ifndef DEBUG
	if (druck < 0 || temp < 0) {
		SerialOut(F("No Send Data - return"));
		return;
	}
#endif // !DEBUG
	String sSQL;
	cursor = new MySQL_Cursor(&conn);

	//MySQL_Connection conn((Client *)&client);
	SerialOut(F("Mysql Server Connecting..."), false);
	if (conn.connect(server_addr, 3306, user, password))
		SerialOut(F("OK!"));
	else
		SerialOut(F("FAILED!"));

	if (setResetFlag) {
		sSQL = "INSERT INTO iSpunder.Data (Timestamp, Name, SudName, ID, Temperatur, Druck, Karbo, SollKarbo, ResetFlag) ";
		sSQL += "VALUES (now(), '" + My_iSpunderName + "', '" + My_SudName + "', " + String(ESP.getChipId()) + ", " + String(temp) + ", " + String(druck) + ", " + String(carbo) + ", " + String(sollcarbo) + ", 1 )";
		setResetFlag = false;
	}
	else {
		sSQL = "INSERT INTO iSpunder.Data (Timestamp, Name, SudName, ID, Temperatur, Druck, Karbo, SollKarbo) ";
		sSQL += "VALUES (now(), '" + My_iSpunderName + "', '" + My_SudName + "', " + String(ESP.getChipId()) + ", " + String(temp) + ", " + String(druck) + ", " + String(carbo) + ", " + String(sollcarbo) + " )";
	}
	SerialOut(sSQL);
	const char* query = sSQL.c_str();

	if (conn.connected()) {
		SerialOut(F("MySQL Connect OK."));
		bool isOK = cursor->execute(query);
		if (isOK) {
			SerialOut(F("Cursor OK."));
			//Serial.print(timeClient.getFormattedTime());
			Serial.println(F(" send OK: "));

		}
		else
		{
			SerialOut(F("Cursor Failed."));
			//Serial.print(timeClient.getFormattedTime());
			Serial.println(F(" !!send failed!!: "));
		}
	}
	else
	{
		Serial.print(F("DB Connect: "));
		Serial.println(conn.connected());
	}

	//Serial.print(F("Uptime: "));
	//Serial.print(NTP.getUptimeString()); Serial.print(F(" since "));
	//Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());

	delete cursor;
}

void reqData() {
	yield();
	reqDSTemp();
	getDSTemp();
	getPressure();
	calcCarbo();
	handle_CarboValve();
	DisplayDaten(temp, druck, carbo, sollcarbo);
	sendDataMySQL();
}


void menueList() {
	menue = DisplayMenue();
	String _configSave1 = "N";
	String _configSave2 = "N";
	String _configSave3 = "N";
	switch (menue) {
	case 1: //Soll-Karbo
		changeCarbo();
		break;
	case 2: //Karbo-Hysetese
		changeCarboHysteresis();
		break;
	case 3: //MessIntervall
		changeMessInterval();
		break;
	case 4: //WiFi STA
		connectWifi();
		break;
	case 5: //WiFi AP
		init_WiFi_AP();
		break;
	case 6: //Sud ID
		changeSudID();
		break;
	case 7: //Set Reset in DB
		setResetFlag = true;
		reqData();
		break;
	case 8: //IP Adresse
		BtnPress = false;
		do {
			BTNA.loop();
			DisplayInfo("m", "STA Mode", "IP " + WiFi.localIP().toString(),"","");
			yield();
		} while (!BtnPress);
		BtnPress = false;
		break;
	case 9: //Save Config
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

		DisplayInfo("l", "Save Config", "Karbo = " + _configSave3, "Wifi = " + _configSave1, "MySql = " + _configSave2);
		delay(3000);
		break;
	case 10: //Format SPIFF
		formatSpiffs();
		break;
	case 11: //Exit
		break;
	default:
		;
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
	//initWebserver();
	DisplayInfo("m", "AP Mode", "IP " + myIP.toString(), "/wifi","");
	yield();
	delay(5000);
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
	DisplayInfo("m", "Connect WiFi", "to", My_ssid,"");
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
		DisplayInfo("m", "Wrong PW","","","");
	}
	if (WiFi.status() == WL_NO_SSID_AVAIL) {
		SerialOut(F("AP not found..."));
		DisplayInfo("m", "No AP found","","","");
	}

	if (WiFi.status() != WL_CONNECTED) {
		SerialOut(F("failed to connect, change tp AP-Mode"));
		DisplayInfo("m", "Change", "to", "AP-Mode","");
		init_WiFi_AP();
	}
	else {
		//SerialOut(F("local ip: "), false);
		//SerialOut(WiFi.localIP());
		Serial.print(F("local ip: "));
		Serial.println(WiFi.localIP());
		DisplayInfo("m", "STA Mode", "IP " + WiFi.localIP().toString(),"","");
		if (!readConfigFile(MYSQLCONF)) {
			SerialOut(F("Failed to read mysql configuration file, using default values"));
		}
	}
	menue = 0;
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
	DisplayInfo("m", "iSPUNDER", "V " + String(PROG_VERSION), "by HappyHibo","");
	Serial.println("iSPUNDER V" + String(PROG_VERSION) + " by HappyHibo");
	SerialOut(F("Init Harware..."));
	initEncoder();
	initDSSensor();
	initRelais();
	delay(2000);
	// SPIFFS formatieren
	//formatSpiffs();
	SerialOut(F("Read KarboConfig..."));
	if (!readConfigFile(KARBOCONF)) {
		SerialOut(F("Failed to read sollkarbo configuration file, using default values"));
		sollcarbo = 4.50;
		MessInterval = 300000;
	}

	SerialOut(F("Connect Wifi..."));
	connectWifi();

	if (WiFi.status() == WL_CONNECTED)
	{
		SerialOut(F("Wifi Connect..."));
		SerialOut(F("Starte Webserver"));
		//initWebserver();
	}
	else
	{
		SerialOut(F("No Wifi Connect..."));
		//SerialOut(F("Goto AP-Mode..."));
		//init_WiFi_AP();

		// test if ssid exists
		if (WiFi.SSID() == "" && My_ssid != "" && My_psk != "")
		{

		}
	}

	//StartOTA();
	//StartNTP();
	Serial.println();
	//Serial.print(timeClient.); Serial.print(F(" "));
	//Serial.println(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
	Serial.print(F("WiFi is "));
	Serial.print(WiFi.isConnected() ? "connected" : "not connected"); Serial.print(". IP: ");
	IPAddress myIP = WiFi.localIP();
	Serial.println(myIP);
	//timeClient.begin();
	//Serial.print(F("Uptime: "));
	//Serial.print(NTP.getUptimeString()); Serial.print(F(" since "));
	//Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());
	yield();
	delay(5000);
	reqData();
	SerialOut(F("Exit Setup..."));
}

// the loop function runs over and over again until power down or reset
void loop() {

	BTNA.loop();
	server.handleClient();
	//HandleOTA();
	//HandleNTP();
	//timeClient.update();

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
		if (incomingChar == "sendmysql") sendDataMySQL();
		if (incomingChar == "formatFS") formatSpiffs();
		//if (incomingChar == "delWifi") deleteConfigFile(WIFICONF);
		//if (incomingChar == "delMySql") deleteConfigFile(MYSQLCONF);
		//if (incomingChar == "delKarbo") deleteConfigFile(KARBOCONF);
		if (incomingChar == "Sudhaus") {
			My_ssid = incomingChar;
			My_psk = "LenggrieserSudhaus3a!";
		}
		if (incomingChar == "hh") {
			My_ssid = incomingChar;
			My_psk = "HappyHiboLenggries1968!";
		}

	}
#endif // DEBUG

	yield();
}
