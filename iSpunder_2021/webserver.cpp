// 
// 
// 

#include "webserver.h"



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

	for (uint8_t i = 0; i < server.headers(); i++) {
		SerialOut("HeaderName: " + server.headerName(i) + " Header: " + server.header(i));
	}
	for (uint8_t i = 0; i < server.args(); i++) {
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
		//My_SudID = server.arg("SudName").toInt();
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
	yield();
}

void handle_Wifi() {
	SerialOut(F("Enter handle_Wifi"));

	for (uint8_t i = 0; i < server.headers(); i++) {
		SerialOut("HeaderName: " + server.headerName(i) + " Header: " + server.header(i));
	}
	for (uint8_t i = 0; i < server.args(); i++) {
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
	yield();
}

void handle_WebRoot() {
	SerialOut(F("Enter handle_WebRoot"));

	for (uint8_t i = 0; i < server.headers(); i++) {
		SerialOut("HeaderName: " + server.headerName(i) + " Header: " + server.header(i));
	}
	for (uint8_t i = 0; i < server.args(); i++) {
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
	yield();
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
	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}
