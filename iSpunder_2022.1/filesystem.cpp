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
			StaticJsonDocument<256> sjdoc;
			//JsonObject &json = jsonBuffer.parseObject(f);
			DeserializationError error = deserializeJson(sjdoc, f);
			if (error) {
				SerialOut(F("JSON deserialize failed"),false);
				SerialOut(error.f_str());
				return false;
			}
			//json.printTo(Serial);
			//Serial.println();
			WIFIUSR = (String)sjdoc["SSID"];
			WIFIPASS = (String)sjdoc["PSK"];
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
		StaticJsonDocument<256> sjdoc;
		//JsonObject &json = sjdoc.createObject();

		sjdoc["SSID"] = WIFIUSR;
		sjdoc["PSK"] = WIFIPASS;

		File f = SPIFFS.open(configfile, "w");
		if (!f) {
			SerialOut(F("Failed to open config file for writing"));
			SPIFFS.end();
			return false;
		}
		else {
			//json.prettyPrintTo(Serial);
			//json.printTo(f);
			if (serializeJson(sjdoc, f) == 0) {
				SerialOut(F("Failed to write to file"));
			}
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
			StaticJsonDocument<256> sjdoc;
			//JsonObject &json = jsonBuffer.parseObject(f);
			DeserializationError error = deserializeJson(sjdoc, f);
			if (error) {
				SerialOut(F("JSON deserialize failed"), false);
				SerialOut(error.f_str());
				return false;
			}
			//json.printTo(Serial);
			//Serial.println();

			SPUNDERNR = (int)sjdoc["SPUNDERNR"];
			SUDNR = (int)sjdoc["SUDNR"];
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


bool writeSpunderConfigFile(int &SPUNDERNR, int &SUDNR) {
	String configfile = SPUNDERCONF;
	SerialOut(F("writeConfigFile "), false);
	SerialOut(configfile);
	SerialOut(F("Saving config file"));

	startSpiffs();
	StaticJsonDocument<256> sjdoc;
	//JsonObject &json = jsonBuffer.createObject();

	sjdoc["SPUNDERNR"] = SPUNDERNR;
	sjdoc["SUDNR"] = SUDNR;

	File f = SPIFFS.open(configfile, "w");
	if (!f) {
		SerialOut(F("Failed to open config file for writing"));
		SPIFFS.end();
		return false;
	}
	else {
		if (serializeJson(sjdoc, f) == 0) {
			SerialOut(F("Failed to write to file"));
		}
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
			StaticJsonDocument<256> sjdoc;
			//JsonObject &json = jsonBuffer.parseObject(f);
			DeserializationError error = deserializeJson(sjdoc, f);
			if (error) {
				SerialOut(F("JSON deserialize failed"), false);
				SerialOut(error.f_str());
				return false;
			}
			//json.printTo(Serial);
			Serial.println();
			SKARBO = (float)sjdoc["SKARBO"];
			MINTERVAL = (ulong)sjdoc["MINTERVAL"];
			KARBOHYST = (float)sjdoc["KARBOHYST"];
			KOMPDRUCK = (float)sjdoc["KOMPDRUCK"];
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
	StaticJsonDocument<256> sjdoc;
	//DynamicJsonBuffer jsonBuffer;
	//JsonObject &json = jsonBuffer.createObject();

	// JSONify local configuration parameters
	sjdoc["SKARBO"] = SKARBO;
	sjdoc["MINTERVAL"] = MINTERVAL;
	sjdoc["KARBOHYST"] = KARBOHYST;
	sjdoc["KOMPDRUCK"] = KOMPDRUCK;

	// Open file for writing
	File f = SPIFFS.open(configfile, "w");
	if (!f) {
		SerialOut(F("Failed to open config file for writing"));
		SPIFFS.end();
		return false;
	}
	else {
		//json.prettyPrintTo(Serial);
		// Write data to file and close it
		//json.printTo(f);
		if (serializeJson(sjdoc, f) == 0) {
			SerialOut(F("Failed to write to file"));
		}
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


