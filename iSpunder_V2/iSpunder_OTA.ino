
#include <ArduinoOTA.h>

bool ota_started = false;

void HandleOTA()
{
	StartOTA();
	ArduinoOTA.handle();
}

void StartOTA()
{
	if (ota_started) return;

	// Port defaults to 8266
	ArduinoOTA.setPort(8266);
	// Hostname defaults to esp8266-[ChipID]
	//if (ArduinoOTA.getHostname() && ArduinoOTA.getHostname().length())

	// No authentication by default
	//ArduinoOTA.setPassword((const char *)"123");
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
	ota_started = true;
	delay(500);

}


