/***************************************************************************
  This is a datalogging flight computer software for experimental use only.

  Designed specifically to work with: (Teensy 4.1 MCU, BME388 Barometer,
  BNO055 IMU, Adafruit SD flash memory)

  Written by Weymen Koo Chen Huan
 ***************************************************************************/

#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <Servo.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"

#define SEALEVELPRESSURE_HPA (1013.25) // Pressure at sea level is set at 1013.25Pa

Adafruit_BMP3XX bmp;
File SDflash;
Servo servo;

const int chipSelect = 10;
const int buttonPin = 32;
const int R = 20;
const int G = 22;
const int B = 23;
bool error = false;
int apogeeCount = 0;
int mode = 0;
double alt;
double lastAlt;
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

/***************************************************************************/
void setup() {
  // Define whether a pin is input or output
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(buttonPin, INPUT);

  // Assign pin 33 for servo and set it at initial position of 90deg
  servo.attach(33);
  servo.write(90);

  // Initialize serial monitor for debugging at baud rate 115200, not mission critical
  Serial.begin(115200);

  // Initialize all sensor and SD card, report error if one of the fails
  if (!bmp.begin_I2C()) {
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    error = true;
  }
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    //error = true;
  }
  if (SD.exists("Data.txt")) {
    if (SD.remove("Data.txt") == true) {
      Serial.println("removed data");
    }
  }
  SDflash = SD.open("Data.txt", FILE_WRITE);
  if (!SDflash) {
    Serial.println("Error opening Data.txt");
    //error = true;
  }

  //If error occurs, then RGB light will turn red and program stops, else RGB flashes green 5x
  if (error == true) {
    RGB_color(0, 255, 255);
    while (1);
  }
  else {
    for (int j = 0; j < 5; j++) {
      RGB_color(255, 0, 255);
      delay(100);
      RGB_color(0, 0, 0);
      delay(200);
    }
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

}

/***************************************************************************/
void loop() {
  // If button is pressed, flight computer switches to flight/datalogging mode, indicated by green light
  Switch(digitalRead(buttonPin));

  // Logs data and checks for altitude change every 10 ms
  while (mode == 1) {
    //If at any point bme388 fails, report status, deploy parachute immediately
    if (! bmp.performReading()) {
      failsafe();
    }
    datalog();
    apogeeCheck(); //Calls function called apogeeCheck
    RGB_color(0, 0, 0);
    delay(50);
    RGB_color(255, 255, 255);
  }
}

/***************************************************************************/
//Must get 5 reading lower than the last reading in a row to be considered apogee, then deploy parachute by actuating servo
void apogeeCheck() {
  alt = bmp.readAltitude(SEALEVELPRESSURE_HPA);   //Set current alt as sea level
  if (alt - lastAlt < 0) {
    apogeeCount += 1; // Counter adds 1 everytime current alt is lower than previous alt
    alt = lastAlt; //Set current alt to become last alt
  } else {
    apogeeCount = 0; //Reset to 0 if altitude increases instead of decreases
  }
  if (apogeeCount >= 3) {
    for (int j = 0; j < 3; j++) {
      servo.write(90); //Servo actuates from 90 to 180 to release parachute
      delay(500);
      servo.write(180);
      delay(500);
    }
    Serial.println("Parachute deployed!");
  }
}

/***************************************************************************/
//Function to control RGB light
void RGB_color(int red_light_value, int green_light_value, int blue_light_value) {
  analogWrite(R, red_light_value);
  analogWrite(G, green_light_value);
  analogWrite(B, blue_light_value);
}

/***************************************************************************/
// Fail safe actions
void failsafe() {
  Serial.println("Fail safe initiated!");
  SDflash.close();
  RGB_color(0, 255, 255);
  servo.write(180);
  /* Need to also copies data from flash to SD, close file afterwards */
  while (1);
}

/***************************************************************************/
// Datalogging
void datalog() {
  Serial.print(bmp.pressure / 100.0);
  Serial.print("Pa ");
  Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.print("m ");
  Serial.println();
  SDflash.print(bmp.pressure / 100.0);
  SDflash.print("Pa ");
  SDflash.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
  SDflash.print("m ");
  SDflash.println();
  Switch(digitalRead(buttonPin));
  if (mode == 2) {
    SDflash.close();
    RGB_color(0, 0, 0);
    while (1);
  }
  /* Need to also prints data to SD card */
}

/***************************************************************************/
int Switch(int reading) {
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        mode++;
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  return mode;
}
