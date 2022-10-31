#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <Servo.h>

Adafruit_BMP3XX bmp; // I2C
//WIRING: SCL -> A5
//        SDA -> A4

Servo myservo;  // create servo object to control a servo

float minHeight;
float maxHeight;
float currentPressure;

int state = 0;
const int R = 32;
const int B = 30;
const int G = 31;
int buttonPin = 29;
int servoPin = 28;

float launchedAltitude = 1; //what height does the rocket switch into Launch state
float apogeeDescentAltitude = 1; //How far must the rocket drop before it detects apogee

const int chipSelect = 10;
unsigned long currrenttime;
unsigned long previousTime;
boolean enterFunction = true;
int analogPin = 5;

//Start
void setup() {

  Serial.begin(9600); //Start up our serial line
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:

  ResetValues(); //Set our min/max altitude values to their respective min/max values

  //Define our pins
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(10, OUTPUT);

  RGB_color(0, 0, 0);

  Serial.println("Starting initialization");

  //Initialize our BMP280
  if (!bmp.begin_I2C()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  Serial.println(F("BMP388 Initialized"));

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");


  //Initialize our servo
  myservo.attach(servoPin);
  myservo.write(180);
}


//Resets the altitude values ready for another launch
void ResetValues() {
  minHeight = 100000;
  maxHeight = 0;
  currentPressure = 0;
}

void loop() {
  //Simple state machine
  //0 -> idle on launchpad
  //1 -> Rocket ascending
  //2 -> Rocket descending under parachute
  //3 -> Altitude readout requested
  switch (state) {
    case 0:
      IdleState();
      break;
    case 1:
      LaunchState();
      break;
    case 2:
      DescentState();
      break;
    case 3:
      AltitudeReadout();
      break;
    default:
      break;
  }

  currrenttime = micros();
  if (enterFunction == true) {
    int atraso = currrenttime - previousTime - 1000000;
    previousTime = currrenttime;
    Serial.println(atraso); // for debugging
    //-----------------------------------------------------
    // make a string for assembling the data to log:
    String dataString = "";

    // read three sensors and append to the string:
    dataString += String(bmp.readAltitude(currentPressure));
    dataString += " m ,";
    dataString += String(bmp.pressure);
    dataString += " hPa ,";
    dataString += String(bmp.temperature);
    dataString += " C ,";

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.csv", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.print(dataString);
      dataFile.print(",");
      dataFile.println(atraso);
      dataFile.close();
      // print to the serial port too:
      Serial.println(dataString);
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
    //---------------------------------------------------
  }

  // The DELAY time is adjusted in the constant below >>
  if (currrenttime - previousTime < 999988) { // 1 million microsencods= 1 second delay
    /* I have actually used 0.999990 seconds, in a trial to compensate the time that
       this IF function takes to be executed. this is really a point that
       need improvement in my code */
    enterFunction = false;
  }
  else {
    enterFunction = true;
  }
}

//We're waiting on the launchpad - State 0
void IdleState() {
  RGB_color(255, 0, 255); //light is on solid, indicating we're ready to launch
  //When button is tapped, set out minimum height
  if (digitalRead(buttonPin) == HIGH) {
    currentPressure = bmp.readPressure() / 100;
    minHeight = bmp.readAltitude(currentPressure);
    Serial.print("Min value set: ");
    Serial.print(minHeight);
    Serial.println("m");

    //flash our readout light to indicate minimum height has been set
    RGB_color(0, 0, 0);
    delay(100);
    RGB_color(255, 0, 255);
  }

  //We're in Launch state when the rocket is Xm higher than its lowest point
  if (minHeight > -1000) {
    if (bmp.readAltitude(currentPressure) - minHeight > launchedAltitude) {
      state = 1;
      Serial.println("Launch detected!");
    }
  }
}

//Rocket is going up - State 1
void LaunchState() {
  //Turn readout light off when we're in the air
  RGB_color(0, 0, 0);

  //determine our height, and log it as the max height we've reached when appropriate
  float height = bmp.readAltitude(currentPressure) - minHeight;
  if (height > maxHeight) {
    maxHeight = height;
  }

  //Open the parachute if we've dropped Xm below detected apogee
  if (maxHeight - height > apogeeDescentAltitude) {
    state = 2;
    //Deploy parachute
    Serial.println("Parachute deployed");
    myservo.write(0);
    delay(1000);
    myservo.write(180);
    delay(1000);
  }

}

//We're falling, hopefully under parachute - State 2
void DescentState() {
  RGB_color(0, 0, 0);

  //Set to readout state if the reset button is pressed
  if (digitalRead(buttonPin) == HIGH) {
    state = 3;
    Serial.println("Readout state detected");
  }
}

//Tell us the altitude, using the readout light - State 3
void AltitudeReadout() {
  //Loop through height reading 3 times
  for (int l = 0; l < 3; l++) {
    //A 1.5 second on/off informs that a readout is about to be prepared
    RGB_color(255, 0, 255);
    delay(1500);
    RGB_color(0, 0, 0);
    delay(1500);

    //Get hundreds, tens and ones values separately
    int hundreds = floor(maxHeight / 100);
    int tens = floor(((int)maxHeight % 100) / 10);
    int ones = (int)maxHeight % 10;


    //One flash - hundreds unit
    for (int i = 0; i < 1; i++) {
      RGB_color(255, 0, 255);
      delay(75);
      RGB_color(0, 0, 0);
      delay(75);
    }
    delay(500);

    //One flash per Hundreds value
    for (int i = 0; i < hundreds; i++) {
      RGB_color(255, 0, 255);
      delay(400);
      RGB_color(0, 0, 0);
      delay(400);
    }
    delay(1000);

    //Two flashes - tens unit
    for (int i = 0; i < 2; i++) {
      RGB_color(255, 0, 255);
      delay(75);
      RGB_color(0, 0, 0);
      delay(75);
    }
    delay(500);

    //One flash per Tens value
    for (int i = 0; i < tens; i++) {
      RGB_color(255, 0, 255);
      delay(400);
      RGB_color(0, 0, 0);
      delay(400);
    }
    delay(1000);


    //Three flashes - ones unit
    for (int i = 0; i < 3; i++) {
      RGB_color(255, 0, 255);
      delay(75);
      RGB_color(0, 0, 0);
      delay(75);
    }
    delay(500);

    //One flash per Ones value
    for (int i = 0; i < ones; i++) {
      RGB_color(255, 0, 255);
      delay(400);
      RGB_color(0, 0, 0);
      delay(400);
    }
    delay(500);

    //quick series of flashes to show end
    for (int i = 0; i < 5; i++) {
      RGB_color(255, 0, 255);
      delay(75);
      RGB_color(0, 0, 0);
      delay(75);
    }
  }
  //Reset to Idle state
  ResetValues();
  state = 0;
}

//Function to control RGB light
void RGB_color(int red_light_value, int green_light_value, int blue_light_value) {
  digitalWrite(R, 255 - red_light_value);
  digitalWrite(G, 255 - green_light_value);
  digitalWrite(B, 255 - blue_light_value);
}
