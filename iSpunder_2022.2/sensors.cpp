// 
// 
// 

#include "sensors.h"

OneWire oneWire(OW_BUS);
DallasTemperature OW(&oneWire);
DeviceAddress OWDeviceAddress;

bool isRequested = false;


void OWSensorInit()
{
	// workaround for DS not enough power to boot
	pinMode(OW_BUS, OUTPUT);
	digitalWrite(OW_BUS, LOW);
	delay(100);
	OW.begin();
	OW.setWaitForConversion(false);
	OW.getAddress(OWDeviceAddress, 0);
	OW.setResolution(OWDeviceAddress, OWRESOLUTION);
	SerialOut(F("Debug: Init DS18B20..."));
	//OWSensorRequest();
}

void OWSensorRequest()
{
	//if (isRequested) return;

	OW.requestTemperatures();
	//isRequested = true;
	SerialOut(F("Debug: Requesting DS18B20 temperature..."));
}

float OWSensorGetTemp() {
	
	//if (!isRequested) return;

	float _SensorTemp;
	SerialOut(F("Debug: Reading DS18B20 temperature..."));
	//OW.requestTemperatures();
	_SensorTemp = OW.getTempCByIndex(0);
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
	//isRequested = false;
	_SensorTemp = roundf(_SensorTemp * 10.0f) / 10.0f;
	SerialOut(F("Temp: "), false);
	SerialOut(_SensorTemp);
	return _SensorTemp;
}

float getPressure()
{
	float _druck;
	uint anain = analogRead(A0);
	_druck = ((anain / 204.8 - 0.5) / 0.8) - 0.05;
	SerialOut(F("Debug: AnalogIn: "), false);
	SerialOut(anain, false);
	SerialOut(F("  Druck: "), false);
	SerialOut(_druck);

	//  0 Bar = 0,5 V
	//	5 Bar = 4,5 V
	//	Daraus ergibt sich die Geradengleichung y(in V) = x(in Bar) * 0,8V/Bar + 0,5V
	//	Da ich y messe, die Gleichung nach x umstellen und es ergibt sich
	//	x = (y - 0,5) / 0,8 (in Bar)
	//	Da ich für 5V den Wert 1023 als reading erhalte, habe ich noch den Teilungsfaktor 1024/5 = 204,8 einfügen müssen.
	//  x = (y / 204,8 - 0,5) / 0,8 (in Bar)
	//  Quelle: https://forum.fhem.de/index.php/topic,51725.0.html
	return _druck;
}

double calcCarbo(const float &druck, const float &temp) {

	//double _carbo = ((double)druck + 1.013) * (pow(2.71828182845904, (-10.73797 + (2617.25 / (temp + 273.15))))) * 10;
	double _carbo = (1.013 + (double)druck) * exp(-10.73797 + (2617.25 / (temp + 273.15))) * 10;
	
	//SerialOut(F("Karbo: "), false);
	//SerialOut(_carbo);

	// Quelle: http://braukaiser.com/wiki/index.php/Carbonation_Tables
	// Cbeer = (Phead+1.013)*(2.71828182845904^(-10.73797+(2617.25/(Tbeer+273.15))))*10
	// Cbeer - carbonation of the beer in g/l
	// Phead - head pressure in bar
	// Tbeer - temperature of the beer in C
	// e = eulscher Zahl = 2.71828182845904
	//return _carbo;
	//return (1.013 + (double)druck) * exp(-10.73797 + (2617.25 / (temp + 273.15))) * 10;
	return round(_carbo * 10) / 10;
}


double calcPressure(const float &carbo, const float &temp) {
	//*der Spunddruckrechner basiert auf folgender Formel:
	//Spunddruck[bar] = CO2[g / l] / 10.e(-10.73797 + (2617, 25 / Temperatur[°C] + 273.15)) - 1.013 Quelle: Braukaiser
	
	return carbo / (exp(-10.73797 + (2617.25 / (temp + 273.15))) * 10) - 1.013;
}
