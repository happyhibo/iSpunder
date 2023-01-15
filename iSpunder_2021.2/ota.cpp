// 
// 
// 

#include "ota.h"


void handle_OTA()
{
	ArduinoOTA.handle();
}

void start_OTA(String hostname, int port)
{

	// Port defaults to 8266
	ArduinoOTA.setPort(port);

	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname(hostname.c_str());

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA.onStart([]() {
		Serial.println(F("OTA Start"));
	});
	ArduinoOTA.onEnd([]() {
		Serial.println(F("\nOTA End"));
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r\n", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
		else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
		else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
		else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
		else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
	});
	ArduinoOTA.begin();
}
