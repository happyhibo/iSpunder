// NTP Handling

#include <NtpClientLib.h>

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event


void processSyncEvent(NTPSyncEvent_t ntpEvent) {
	if (ntpEvent) {
		Serial.print(F("Time Sync error: "));
		if (ntpEvent == noResponse)
			Serial.println(F("NTP server not reachable"));
		else if (ntpEvent == invalidAddress)
			Serial.println(F("Invalid NTP server address"));
	}
	else {
		Serial.print(F("Got NTP time: "));
		Serial.println(NTP.getTimeDateString(NTP.getLastNTPSync()));
	}
}


void HandleNTP() {

	if (syncEventTriggered) {
		processSyncEvent(ntpEvent);
		syncEventTriggered = false;
	}
	else
	{
		return;
	}
}


void StartNTP() {

	NTP.begin("pool.ntp.org", timeZone, true, minutesTimeZone);
	NTP.setInterval(63);

	NTP.onNTPSyncEvent([](NTPSyncEvent_t event) {
		ntpEvent = event;
		syncEventTriggered = true;
	});
}