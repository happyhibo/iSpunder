/*
 Name:		EnccoderTest.ino
 Created:	22.01.2022 11:05:18
 Author:	hh
*/

/////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////
//#define ENCPINSW D7
//#define ENCPINDT D8
//#define ENCPINCLK D6

#include <Button2.h>
#include <ESPRotary.h>

#define ROTARY_PIN1 D6
#define ROTARY_PIN2 D8
#define BUTTON_PIN  D7

#define CLICKS_PER_STEP 4   // this number depends on your rotary encoder
#define MIN_POS         -99
#define MAX_POS         99
#define START_POS       10
#define INCREMENT       1   // this number is the counter increment on each step

/////////////////////////////////////////////////////////////////

float fNum = 0.0;

ESPRotary r;
Button2 b;    // https://github.com/LennartHennigs/Button2

/////////////////////////////////////////////////////////////////

void setup() {
	Serial.begin(115200);
	delay(50);

	b.begin(BUTTON_PIN);
	b.setTapHandler(click);
	b.setLongClickHandler(resetPosition);

	r.begin(ROTARY_PIN1, ROTARY_PIN2, CLICKS_PER_STEP, MIN_POS, MAX_POS, START_POS, INCREMENT);
	r.setChangedHandler(rotate);
	r.setLeftRotationHandler(showDirection);
	r.setRightRotationHandler(showDirection);

	Serial.println("\n\nRanged Counter");
	Serial.println("You can only set values between " + String(MIN_POS) + " and " + String(MAX_POS) + ".");
	Serial.print("Increment: ");
	Serial.println(r.getIncrement());
	Serial.print("Current position: ");
	Serial.println(r.getPosition());
	Serial.println();
}

void loop() {
	r.loop();
	b.loop();
}

/////////////////////////////////////////////////////////////////

// on change
void rotate(ESPRotary& r) {
	Serial.println(r.getPosition());
}


// on left or right rotation
void showDirection(ESPRotary& r) {
	Serial.println(r.directionToString(r.getDirection()));
}

// single click
void click(Button2& btn) {
	Serial.println("Click!");
	r.setLeftRotationHandler(downDirection);
	r.setRightRotationHandler(upDirection);
}

// long click
void resetPosition(Button2& btn) {
	r.resetPosition();
	Serial.println("Reset!");
}

void downDirection(ESPRotary& r) {
	fNum -= 0.5;
	Serial.println(fNum);
}

void upDirection(ESPRotary& r) {
	fNum += 0.5;
	Serial.println(fNum);
}