// 
// 
// 

#include "wifiwebserver.h"
#include "index.h"


// Set WiFi credentials
String WIFI_SSID = "Sudhaus";
String WIFI_PASS = "LenggrieserSudhaus3a!";

//char ssid[] = "Sudhaus";        // your network SSID (name)
//char pass[] = "LenggrieserSudhaus3a";        // your network password

//char *WIFI_SSID = "Sudhaus";
//char *WIFI_PASS = "LenggrieserSudhaus3a!";


ESP8266WebServer webserver(80);


// Handle Root
void rootPage() {
	webserver.send(200, "text/plain", "It Works!! ");
}

void handleWifi() {
	String webSSID;
	String webPASS;
	for (uint8_t i = 0; i < webserver.headers(); i++) {
		SerialOut("HeaderName: " + webserver.headerName(i) + " Header: " + webserver.header(i));
	}
	for (uint8_t i = 0; i < webserver.args(); i++) {
		SerialOut("ArgName: " + webserver.argName(i) + " Arg: " + webserver.arg(i));
	}
	if (webserver.hasArg("tf_ssid")) {
		webSSID = webserver.arg("tf_ssid").c_str();
		webserver.sendHeader("Location", "/wifi");
		webserver.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		webserver.send(301);
		SerialOut(F("Received SSID: "), false);
		SerialOut(webSSID);

	}
	if (webserver.hasArg("tf_psk")) {
		webPASS = webserver.arg("tf_psk").c_str();
		webserver.sendHeader("Location", "/wifi");
		webserver.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		webserver.send(301);
		SerialOut(F("Received PSK: "), false);
		SerialOut(webPASS);
	}
	if (webserver.hasArg("Speichern")) {
		if (webPASS != "" || webSSID != "") {
			SerialOut(F("Save Config..."));
			WIFI_PASS = webPASS;
			WIFI_SSID = webSSID;
			writeWiFiConfigFile(WIFI_SSID, WIFI_PASS);
		}
		webserver.sendHeader("Location", "/wifi");
		webserver.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		webserver.send(301);
		SerialOut(F("Received BtnSpeichern"));
		

	}
	if (webserver.hasArg("AP_OFF")) {
		if (webserver.arg("AP_OFF") == "apoff") {
			SerialOut(F("Wifi OFF"));
			WiFi.mode(WIFI_OFF);
			SerialOut(F("Change Wifi Mode STA"));
			WiFi.mode(WIFI_STA);
			if (startWifi())
				startWebserver();
		}
		webserver.sendHeader("Location", "/");
		webserver.sendHeader("Cache-Control", "no-cache");
		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
		webserver.send(301);
	}

	String page = MAIN_page;
	webserver.send(200, "text/html", page);
	SerialOut();
	yield();
}

//void handleSpunder() {
//	int webSpunderNr;
//	int webSudNr;
//	for (uint8_t i = 0; i < webserver.headers(); i++) {
//		SerialOut("HeaderName: " + webserver.headerName(i) + " Header: " + webserver.header(i));
//	}
//	for (uint8_t i = 0; i < webserver.args(); i++) {
//		SerialOut("ArgName: " + webserver.argName(i) + " Arg: " + webserver.arg(i));
//	}
//	if (webserver.hasArg("tf_spundernr")) {
//		webSpunderNr = webserver.arg("tf_spundernr").toInt;
//		webserver.sendHeader("Location", "/spunder");
//		webserver.sendHeader("Cache-Control", "no-cache");
//		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
//		webserver.send(301);
//		SerialOut(F("Received SpunderNr: "), false);
//		SerialOut(webSpunderNr);
//
//	}
//	if (webserver.hasArg("tf_sudnr")) {
//		webSudNr = webserver.arg("tf_sudnr").toInt);
//		webserver.sendHeader("Location", "/spunder");
//		webserver.sendHeader("Cache-Control", "no-cache");
//		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
//		webserver.send(301);
//		SerialOut(F("Received SudNr: "), false);
//		SerialOut(webSudNr);
//	}
//	if (webserver.hasArg("Speichern")) {
//		int NewSpunderNr = 0;
//		int NewSudNr = 0;
//		if (webSpunderNr != 0) {
//			NewSpunderNr = webSpunderNr;
//		}
//		else
//		{
//			NewSpunderNr = 01;
//		}
//		if (webSudNr != 0) {
//			NewSudNr = webSudNr;
//		}
//		
//		if (NewSpunderNr != 0 && NewSudNr != 0){
//			SerialOut(F("Save Config..."));
//			writeSpunderConfigFile(NewSpunderNr, NewSudNr);
//		}
//		webserver.sendHeader("Location", "/spunder");
//		webserver.sendHeader("Cache-Control", "no-cache");
//		//server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
//		webserver.send(301);
//		SerialOut(F("Received BtnSpeichern"));
//
//
//	}
//	
//	String page = Spunder_page;
//	webserver.send(200, "text/html", page);
//	SerialOut();
//	yield();
//}

// Handle 404
void handleNotFound() {
	// Output a "404 not found" page. It includes the parameters which comes handy for test purposes.
	Serial.println(F("D015 handleNotFound()"));
	String message;
	message += F("404 - File Not Found\n\n"
		"URI: ");
	message += webserver.uri();
	message += F("\nMethod: ");
	message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
	message += F("\nArguments: ");
	message += webserver.args();
	message += F("\n");
	for (uint8_t i = 0; i < webserver.args(); i++)
	{
		message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
	}
	webserver.send(404, "text/plain", message);
}


void handleWebserver() {
	webserver.handleClient();
}

// init Webserver
void startWebserver() {
	SerialOut(F("Start Web Server"));	// Start Web Server
	webserver.on("/", rootPage);
	webserver.on("/wifi", handleWifi);
	//webserver.on("/spunder", handleSpunder);
	webserver.onNotFound(handleNotFound);
	webserver.begin();
	SerialOut(F("WebServer started"));
}

bool startWifi() {
	//Begin WiFi
	SerialOut(F("Start WiFi"));
	readWiFiConfigFile(WIFI_SSID, WIFI_PASS);

	SerialOut(F("Disconnect AP"));
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);
	SerialOut(F("Change Wifi Mode STA"));
	WiFi.mode(WIFI_STA);

	Serial.print(F("Verbinde mit : "));
	Serial.println(WIFI_SSID + " " + WIFI_PASS);
	WiFi.begin(WIFI_SSID, WIFI_PASS);

	int connRes = WiFi.waitForConnectResult();
	SerialOut(F("Connection result is "), false);
	SerialOut(connRes);

	if (WiFi.status() == WL_CONNECT_FAILED) {
		Serial.println(F("Wrong Password..."));
		return false;
	}
	if (WiFi.status() == WL_NO_SSID_AVAIL) {
		Serial.println(F("AP not found..."));
		return false;
	}

	if (WiFi.status() != WL_CONNECTED) {
		Serial.println(F("Failed to connect..."));
		return false;
	}
	else {
		// WiFi Connected
		Serial.println("Connected!");
		Serial.print("IP address: ");
		Serial.println(WiFi.localIP());
	}

	return true;
}


//void sendDataToMySQL(float temp, float druck, float carbo, float sollcarbo, String iSpunderName, String SudName, bool setResetFlag) {
//	SerialOut(F("Send data to mysql..."));
//#ifndef DEBUG
//	if (druck < 0 || temp < 0) {
//		SerialOut(F("No Send Data - return"));
//		return;
//	}
//#endif // !DEBUG
//	String sSQL;
//	cursor = new MySQL_Cursor(&conn);
//
//	//MySQL_Connection conn((Client *)&client);
//	SerialOut(F("Mysql Server Connecting..."), false);
//	if (conn.connect(server_addr, 3306, user, password))
//		SerialOut(F("OK!"));
//	else
//		SerialOut(F("FAILED!"));
//
//	if (setResetFlag) {
//		sSQL = "INSERT INTO iSpunder.Data (Timestamp, Name, SudName, ID, Temperatur, Druck, Karbo, SollKarbo, ResetFlag) ";
//		sSQL += "VALUES (now(), '" + iSpunderName + "', '" + SudName + "', " + String(ESP.getChipId()) + ", " + String(temp) + ", " + String(druck) + ", " + String(carbo) + ", " + String(sollcarbo) + ", 1 )";
//		setResetFlag = false;
//	}
//	else {
//		sSQL = "INSERT INTO iSpunder.Data (Timestamp, Name, SudName, ID, Temperatur, Druck, Karbo, SollKarbo) ";
//		sSQL += "VALUES (now(), '" + iSpunderName + "', '" + SudName + "', " + String(ESP.getChipId()) + ", " + String(temp) + ", " + String(druck) + ", " + String(carbo) + ", " + String(sollcarbo) + " )";
//	}
//	SerialOut(sSQL);
//	const char* query = sSQL.c_str();
//
//	if (conn.connected()) {
//		SerialOut(F("MySQL Connect OK."));
//		bool isOK = cursor->execute(query);
//		if (isOK) {
//			SerialOut(F("Cursor OK."));
//			//Serial.print(timeClient.getFormattedTime());
//			Serial.println(F(" send OK: "));
//
//		}
//		else
//		{
//			SerialOut(F("Cursor Failed."));
//			//Serial.print(timeClient.getFormattedTime());
//			Serial.println(F(" !!send failed!!: "));
//		}
//	}
//	else
//	{
//		Serial.print(F("DB Connect: "));
//		Serial.println(conn.connected());
//	}
//
//	delete cursor;
//}
