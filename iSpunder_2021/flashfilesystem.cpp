// 
// 
// 

#include "flashfilesystem.h"


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
			Serial.println();

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
				if (json.containsKey("SUDID"))
					My_SudID = (int)json["SUDID"];
			}
			if (config == KARBOCONF) {
				if (json.containsKey("SKARBO"))
					sollcarbo = (double)json["SKARBO"];
				if (json.containsKey("MINTERVAL"))
					MessInterval = (ulong)json["MINTERVAL"];
				if (json.containsKey("KARBOHYST"))
					carbohyst = (double)json["KARBOHYST"];
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
	yield();
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
		json["SUDNAME"] = My_SudName; //Name = # + SudID
		json["SUDID"] = My_SudID;
	}
	if (config == KARBOCONF) {
		json["SKARBO"] = sollcarbo;
		json["MINTERVAL"] = MessInterval;
		json["KARBOHYST"] = carbohyst;
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
	yield();
	SerialOut(F("Config file was successfully saved"));
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
		yield();
		SerialOut("Config file was successfully deleted");
		return true;
	}
}

void formatSpiffs() {
	SPIFFS.end();
	SPIFFS.begin();
	SerialOut(F("Formating SPIFFS: "), false);
	SerialOut(SPIFFS.format());
	SPIFFS.end();
}

