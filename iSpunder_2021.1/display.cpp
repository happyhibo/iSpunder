// 
// 
// 

#include "display.h"

int oldMenuepos = 0;

SSD1306Brzo display(ADDRESS, SDA, SCL);

void DisplayInit() {
	SerialOut(F("Init OLED-Screen..."));
	display.init();
	display.flipScreenVertically();
	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	String Ausrichtung = "m";
	String Zeile1 = "iSpunder";
	String Zeile2 = "V " + String(PROG_VERSION);
	String Zeile3 = "by HappyHibo";
	String Zeile4 = "";
	DisplayInfo(Ausrichtung, Zeile1, Zeile2, Zeile3, Zeile4);
}


void DisplayInfo(String &Ausrichtung, String &Zeile1, String &Zeile2, String &Zeile3, String &Zeile4) {

	display.setTextAlignment(TEXT_ALIGN_LEFT);
	display.setFont(ArialMT_Plain_16);
	display.clear();
	int DisplayWidth = 128;
	int DisplayHight = 64;
	int FontHight = 16;
	int _Abstand;
	int _Zeile1;
	int _Spalte1, _Spalte2, _Spalte3, _Spalte4;
	int Zeilen;

	if (Zeile1 != "") Zeilen = 1;
	if (Zeile2 != "") Zeilen = 2;
	if (Zeile3 != "") Zeilen = 3;
	if (Zeile4 != "") Zeilen = 4;

	if (Zeilen == 1) {
		_Zeile1 = 32 - FontHight / 2;
		_Abstand = 0;
	}
	else if (Zeilen == 2)
	{
		_Abstand = 21;
		_Zeile1 = _Abstand - FontHight / 2;
	}
	else if (Zeilen == 3)
	{
		_Abstand = 16;
		_Zeile1 = _Abstand - FontHight / 2;
	}
	else if (Zeilen == 4)
	{
		_Abstand = 13;
		_Zeile1 = _Abstand - FontHight / 2;
	}

	int _Zeile2 = _Zeile1 + _Abstand;
	int _Zeile3 = _Zeile2 + _Abstand;
	int _Zeile4 = _Zeile3 + _Abstand;

	if (Ausrichtung == "m") {
		_Spalte1 = (DisplayWidth - display.getStringWidth(Zeile1)) / 2;
		_Spalte2 = (DisplayWidth - display.getStringWidth(Zeile2)) / 2;
		_Spalte3 = (DisplayWidth - display.getStringWidth(Zeile3)) / 2;
		_Spalte4 = (DisplayWidth - display.getStringWidth(Zeile3)) / 2;
	}
	else if (Ausrichtung = "l")
	{
		_Spalte1 = 5;
		_Spalte2 = 5;
		_Spalte3 = 5;
		_Spalte4 = 5;
	}
	else if (Ausrichtung = "r")
	{
		_Spalte1 = (DisplayWidth - display.getStringWidth(Zeile1)) - 5;
		_Spalte2 = (DisplayWidth - display.getStringWidth(Zeile2)) - 5;
		_Spalte3 = (DisplayWidth - display.getStringWidth(Zeile3)) - 5;
		_Spalte4 = (DisplayWidth - display.getStringWidth(Zeile3)) - 5;
	}

	display.drawRect(0, 0, 128, 64);
	display.drawString(_Spalte1, _Zeile1, Zeile1);
	if (Zeilen >= 2) display.drawString(_Spalte2, _Zeile2, Zeile2);
	if (Zeilen >= 3) display.drawString(_Spalte3, _Zeile3, Zeile3);
	if (Zeilen == 4) display.drawString(_Spalte4, _Zeile4, Zeile4);
	display.display();
}


void DisplayDaten(float &temp, float &druck, float &carbo, float &sollcarbo) {

	int _ABST = 15;
	int _S1 = 0;
	int _S2 = 58;
	display.clear();
	//display.display();
	//if (!OLEDOnOff) return;
	int _Z1 = 0;
	display.drawString(_S1, _Z1, "Temp.: ");
	if (temp < 9.9 && temp >= 0.0) {
		display.drawString(_S2, _Z1, "0" + String(temp) + "  C");
	}
	else {
		display.drawString(_S2, _Z1, String(temp) + "  C");
	}
	display.drawCircle(102, 5, 2);
	int _Z2 = _Z1 + _ABST;
	display.drawString(_S1, _Z2, "Druck :");
	if (druck < 9.9 && druck >= 0.0) {
		display.drawString(_S2, _Z2, "0" + String(druck) + " bar");
	}
	else {
		display.drawString(_S2, _Z2, String(druck) + " bar");
	}
	int _Z3 = _Z2 + _ABST;
	display.drawString(_S1, _Z3, "Karbo :");
	if (carbo < 9.9 && carbo >= 0.0) {
		display.drawString(_S2, _Z3, "0" + String(carbo) + " g/l");
	}
	else
	{
		display.drawString(_S2, _Z3, String(carbo) + " g/l");
	}
	int _Z4 = _Z3 + _ABST;
	display.drawString(_S1, _Z4, "SoKar : ");
	if (sollcarbo < 9.9 && sollcarbo >= 0.0) {
		display.drawString(_S2, _Z4, "0" + String(sollcarbo) + " g/l");
	}
	else
	{
		display.drawString(_S2, _Z4, String(sollcarbo) + " g/l");
	}
	display.display();
}

void DisplayMenue(int &Menuepos) {

	int _Abstand = 15;
	int _S2 = 17;
	int _Z1 = 2;
	int _Z2 = _Z1 + _Abstand;
	int _Z3 = _Z2 + _Abstand;

	display.setTextAlignment(TEXT_ALIGN_LEFT);	
	display.setFont(ArialMT_Plain_16);
	//display.clear();
	if (Menuepos != oldMenuepos) {
		display.clear();
	}
	display.drawRect(0, 0, 128, 64);
	display.drawString(20, 1, "--- Menue ---");
	display.drawLine(0, 18, 128, 18);
	display.drawString(2, _Z3, ">");
	display.drawString(117, _Z3, "<");
	switch (Menuepos) {
	case 1:
		display.drawString(_S2, _Z3, "Soll-Karbo");
		display.display();
		break;
	case 2:
		display.drawString(_S2, _Z3, "K-Hysterese");
		display.display();
		break;
	case 3:
		display.drawString(_S2, _Z3, "MessInterval");
		display.display();
		break;
	case 4:
		display.drawString(_S2, _Z3, "Wifi STA");
		display.display();
		break;
	case 5:
		display.drawString(_S2, _Z3, "Wifi AP");
		display.display();
		break;
	case 6:
		display.drawString(_S2, _Z3, "Sud ID");
		display.display();
		break;
	case 7:
		display.drawString(_S2, _Z3, "Set Spund.ID");
		display.display();
		break;
	case 8:
		display.drawString(_S2, _Z3, "IP Address");
		display.display();
		break;
	case 9:
		display.drawString(_S2, _Z3, "Save Config");
		display.display();
		break;
	case 10:
		display.drawString(_S2, _Z3, "Format SPIFF");
		display.display();
		break;
	case 11:
		display.drawString(_S2, _Z3, "Exit");
		display.display();
		break;
	}
	oldMenuepos = Menuepos;
}
