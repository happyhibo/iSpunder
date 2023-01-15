// 
// 
// 

#include "filesystem.h"



void formatSpiffs() {
	SPIFFS.end();
	SPIFFS.begin();
	SerialOut(F("Formating SPIFFS: "), false);
	SerialOut(SPIFFS.format());
	SPIFFS.end();
}

void deleteAllFilesSpiffs() {
	formatSpiffs();
}

void startSpiffs() {
	
	if (!SPIFFS.begin()) {
		SerialOut(F("SPIFFS not ready"));
		if (!SPIFFS.exists(WIFICONF) && !SPIFFS.exists(DBCONF) && !SPIFFS.exists(KARBOCONF) && !SPIFFS.exists(SPUNDERCONF))
			{
				SerialOut(F("\nneed to format SPIFFS: "));
				formatSpiffs();
			}
	}
	else
	{
		SerialOut(F("SPIFFS ready..."));
	}

}

bool readWiFiConfigFile(String &WIFIUSR, String &WIFIPASS) {
	String configfile = WIFICONF;
	SerialOut(F("readConfigFile "), false);
	SerialOut(configfile);
	File f;
	SerialOut(F("mounting FS..."), false);
	//SPI.begin();
	if (SPIFFS.begin())
	{
		SerialOut(F(" mounted!"));
		if (SPIFFS.exists(configfile))
		{
			SerialOut(F("reading config file"));
			f = SPIFFS.open(configfile, "r");
			size_t size = f.size();
			SerialOut(F("Size config file: "), false);
			SerialOut(size);
			if (size > 1024) {
				SerialOut(F("Config file size is too large"));
				return false;
			}
			StaticJsonBuffer<256> jsonBuffer;
			JsonObject &json = jsonBuffer.parseObject(f);
			if (!json.success()) {
				SerialOut(F("JSON parseObject() failed"));
				return false;
			}
			json.printTo(Serial);
			Serial.println();

			if (json.containsKey("SSID"))
				WIFIUSR = (String)json["SSID"];
			if (json.containsKey("PSK"))
				WIFIPASS = (String)json["PSK"];
		}
		else {
			SerialOut(F("ERROR: failed to load json config"));
			f.close();
			SPIFFS.end();
			return false;
		}
		f.close();
		SPIFFS.end();
		//SPI.end();
	}
	SerialOut("Configfile loaded succesfully");
	return true;
}

bool writeWiFiConfigFile(String &WIFIUSR, String &WIFIPASS) {
	String configfile = WIFICONF;
	SerialOut(F("writeConfigFile "), false);
	SerialOut(configfile);
	SerialOut(F("Saving config file"));
	
	startSpiffs();
	SerialOut(F("mounting FS..."), false);
	if (SPIFFS.begin())
	{
		SerialOut(F(" mounted!"));
		StaticJsonBuffer<256> jsonBuffer;
		JsonObject &json = jsonBuffer.createObject();

		json["SSID"] = WIFIUSR;
		json["PSK"] = WIFIPASS;

		File f = SPIFFS.open(configfile, "w");
		if (!f) {
			SerialOut(F("Failed to open config file for writing"));
			SPIFFS.end();
			return false;
		}
		else {
			json.prettyPrintTo(Serial);
			json.printTo(f);
			size_t size = f.size();
			SerialOut(F("Size config file: "), false);
			SerialOut(size);
			f.close();
		}
	}
	SPIFFS.end();
	//SPI.end();
	SerialOut(F("Config file was successfully saved"));
	return true;
}




bool readSpunderConfigFile(int &SPUNDERNR, int &SUDNR) {
	String configfile = SPUNDERCONF;
	SerialOut(F("readConfigFile "), false);
	SerialOut(configfile);
	File f;
	SerialOut(F("mounting FS..."), false);
	//SPI.begin();
	if (SPIFFS.begin())
	{
		SerialOut(F(" mounted!"));
		if (SPIFFS.exists(configfile))
		{
			SerialOut(F("reading config file"));
			f = SPIFFS.open(configfile, "r");
			size_t size = f.size();
			SerialOut(F("Size config file: "), false);
			SerialOut(size);
			if (size > 1024) {
				SerialOut(F("Config file size is too large"));
				return false;
			}
			StaticJsonBuffer<256> jsonBuffer;
			JsonObject &json = jsonBuffer.parseObject(f);
			if (!json.success()) {
				SerialOut(F("JSON parseObject() failed"));
				return false;
			}
			json.printTo(Serial);
			Serial.println();

			if (json.containsKey("SPUNDERNR"))
				SPUNDERNR = (int)json["SPUNDERNR"];
			if (json.containsKey("SUDNR"))
				SUDNR = (int)json["SUDNR"];
		}
		else {
			SerialOut(F("ERROR: failed to load json config"));
			f.close();
			SPIFFS.end();
			return false;
		}
		f.close();
		SPIFFS.end();
		//SPI.end();
	}
	SerialOut("Configfile loaded succesfully");
	return true;
}

//
bool writeSpunderConfigFile(int &SPUNDERNR, int &SUDNR) {
	String configfile = SPUNDERCONF;
	SerialOut(F("writeConfigFile "), false);
	SerialOut(configfile);
	SerialOut(F("Saving config file"));

	startSpiffs();
	StaticJsonBuffer<256> jsonBuffer;
	JsonObject &json = jsonBuffer.createObject();

	json["SPUNDERNR"] = SPUNDERNR;
	json["SUDNR"] = SUDNR;

	File f = SPIFFS.open(configfile, "w");
	if (!f) {
		SerialOut(F("Failed to open config file for writing"));
		SPIFFS.end();
		return false;
	}
	else {
		json.prettyPrintTo(Serial);
		json.printTo(f);
		size_t size = f.size();
		SerialOut(F("Size config file: "), false);
		SerialOut(size);
		f.close();
	}
	SPIFFS.end();
	//SPI.end();
	SerialOut(F("Config file was successfully saved"));
	return true;
}






bool readKarboConfigFile(float &SKARBO, ulong &MINTERVAL, float &KARBOHYST, float &KOMPDRUCK) {
	String configfile = KARBOCONF;
	SerialOut(F("readConfigFile "), false);
	SerialOut(configfile);
	File f;
	SerialOut(F("mounting FS..."), false);

	if (SPIFFS.begin())
	{
		SerialOut(F(" mounted!"));
		if (SPIFFS.exists(configfile))
		{
			SerialOut(F("reading config file"));
			// this opens the config file in read-mode
			f = SPIFFS.open(configfile, "r");
			// we could open the file
			size_t size = f.size();
			SerialOut(F("Size config file: "), false);
			SerialOut(size);
			if (size > 1024) {
				SerialOut(F("Config file size is too large"));
				return false;
			}
			StaticJsonBuffer<256> jsonBuffer;	
			JsonObject &json = jsonBuffer.parseObject(f);
			if (!json.success()) {
				SerialOut(F("JSON parseObject() failed"));
				return false;
			}
			json.printTo(Serial);
			Serial.println();

			if (json.containsKey("SKARBO"))
				SKARBO = (float)json["SKARBO"];
			if (json.containsKey("MINTERVAL"))
				MINTERVAL = (ulong)json["MINTERVAL"];
			if (json.containsKey("KARBOHYST"))
				KARBOHYST = (float)json["KARBOHYST"];
			if (json.containsKey("KARBOHYST"))
				KOMPDRUCK = (float)json["KOMPDRUCK"];
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

bool writeKarboConfigFile(float &SKARBO, ulong &MINTERVAL, float &KARBOHYST, float &KOMPDRUCK) {
	String configfile = KARBOCONF;
	SerialOut(F("writeConfigFile "), false);
	SerialOut(configfile);
	SerialOut(F("Saving config file"));

	startSpiffs();
	StaticJsonBuffer<256> jsonBuffer;
	//DynamicJsonBuffer jsonBuffer;
	JsonObject &json = jsonBuffer.createObject();

	// JSONify local configuration parameters
	json["SKARBO"] = SKARBO;
	json["MINTERVAL"] = MINTERVAL;
	json["KARBOHYST"] = KARBOHYST;
	json["KOMPDRUCK"] = KOMPDRUCK;

	// Open file for writing
	File f = SPIFFS.open(configfile, "w");
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
	SerialOut(F("Config file was successfully saved"));
	return true;
}




bool deleteConfigFile(char *configfile) {
	SerialOut(F("deleteConfigFile "), false);
	SerialOut(configfile);
	if (!((configfile == WIFICONF) || (configfile == DBCONF) || (configfile == KARBOCONF) || (configfile == SPUNDERCONF))) {
		SerialOut(F("keyword not found... return deleteConfigFile false"));
		return false;
	}

	SerialOut(F("mounting FS..."), false);

	if (SPIFFS.begin())
	{
		SerialOut(F(" mounted!"));
		if (SPIFFS.exists(configfile))
		{
			SerialOut(F("delete config file"));
			SPIFFS.remove(configfile);
		}
		SPIFFS.end();
		SerialOut("Config file was successfully deleted");
		return true;
	}
}


