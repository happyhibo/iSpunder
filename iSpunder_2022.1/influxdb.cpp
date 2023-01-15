// 
// 
// 

#include "influxdb.h"

InfluxDBClient idb(INFLUXDB_URL, INFLUXDB_DB);

// Data point
#ifndef DEBUG
Point sensor("iSpunder");
#else
Point sensor("iSpunderTest");
#endif // !Debug



void influxdb_init() {

	SerialOut(F("Debug: Init InfluxDB..."));
	idb.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB, INFLUXDB_USER, INFLUXDB_PW);
	
}

void influxdb_setTags(String &SpunderName, String &SudName) {

	SerialOut(F("Debug: Add InfluxDB Tags..."));
	sensor.clearTags();
	sensor.addTag("iSpunderName", SpunderName);
	sensor.addTag("SudNr", SudName);

}

void influxdb_write(float &temp, float &druck, float &carbo, float &sollcarbo, float &solldruck){

	SerialOut(F("Debug: influxdb_write..."));
	if (temp == 85.0 || temp == (-127.0))
	{
		SerialOut(F("return T85 or T-127"));
		return;
	}

	sensor.clearFields();
	sensor.addField("Temp", temp);
	sensor.addField("Druck", druck);
	sensor.addField("Karbo", carbo);
	sensor.addField("SollKarbo", sollcarbo);
	sensor.addField("SollDruck", solldruck);

	String output = idb.pointToLineProtocol(sensor);
	SerialOut(F("Debug: Write to Influxdb: "),false);
	SerialOut(output);
	if (!idb.writePoint(sensor)) {
		SerialOut(F("Debug: InfluxDB write failed: "), false);
		SerialOut(idb.getLastErrorMessage());
	}
	SerialOut(F("Debug: influxdb_write return..."));

}


void influxdb_write_TestData() {

	float temp = random(10.0, 25.0);
	float druck = random(0.0, 5.0);
	float sollcarbo = 4.5;
	float carbo = ((double)druck + 1.013) * (pow(2.71828182845904, (-10.73797 + (2617.25 / (temp + 273.15))))) * 10;

	sensor.clearFields();
	sensor.addField("Temp", temp);
	sensor.addField("Druck", druck);
	sensor.addField("Karbo", carbo);
	sensor.addField("SollKarbo", sollcarbo);

	String output = idb.pointToLineProtocol(sensor);
	SerialOut(F("Write to Influxdb: "), false);
	SerialOut(output);
	if (!idb.writePoint(sensor)) {
		SerialOut(F("InfluxDB write failed: "), false);
		SerialOut(idb.getLastErrorMessage());
	}

}