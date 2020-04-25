/*
  Distance Lapse Camera Trigger for Bicycles
  Created April 2020 (during corona virius pandemic)

  Counts the number of wheel rotations on a bicycle and triggers a camera shutter release every Nth time
  the "N" number of rotations to trigger the shutter release can be set in the code or chosen via hardware jumpers

  The circuit:
  - LED attached from pin 13 to ground
  - Reed Switch attached to pin 10 from +5V
  - 10K resistor attached to pin 10 from ground (pulldown resistor)


  Explain the rest of the circuit here:
  - 390 ohm resistor attached to pin 5 from the annode of an LED who's cathode is attached to ground

*/


// INFO ON THE BUTTON LIBRARY HERE: https://github.com/JChristensen/JC_Button
//#include <ESP8266WiFi.h>

#include <JC_Button.h>

/*
 * 
 * The constructor defines a button object.
 * Syntax
 * Button(pin, dbTime, puEnable, invert);
 * 
 * Required parameter
 * pin: Arduino pin number that the button is connected to (byte)
 * 
 * Optional parameters
 * dbTime (unsigned long): Debounce time in milliseconds. Defaults to 25ms if not given. 
 * puEnable (bool) ( Defaults to true if not given): true to enable the microcontroller's internal pull-up resistor, else false. 
 * invert (bool) (Defaults to true if not given): 
 *         true should be used when a pull-up resistor is employed or puEnable=true. true interprets a low level as pressed. 
 *         false should be used when a pull-down resistor is being used or puEnable=false. false interprets a high logic level to mean the button is pressed, 
 */
// ************************************************
//        BUTTON LIBRARY CLASS DEFINITIONS
// ************************************************
 
//#define WHEEL_TRIGGER_PIN 14
#define WHEEL_TRIGGER_PIN 3
#define SHUTTER_TRIGGER_PIN 4
#define BLE_RESET_PIN 11
#define PULLUP true    // these aren't required in the most recent version of JC_Button because they are defaults when using a button that is set up to be pulled low
#define INVERT true    // these aren't required in the most recent version of JC_Button because they are defaults when using a button that is set up to be pulled low
#define DEBOUNCE_MS   40       // sets the debounce time in ms. Should be between 20-50 ms depending on the button
#define LONG_PRESS 1000    //We define a "long press" to be 1000 milliseconds.

Button wheelTriggerButton(WHEEL_TRIGGER_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)
Button bluetoothResetButton(BLE_RESET_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)
Button shutterTriggerButton(SHUTTER_TRIGGER_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)



// ************************************************
//        OTHER DEFINITIONS
// ************************************************
// CONSTANT VARIABLE DECLARATIONS

// OUTPUT PINS
#define LED_PIN   13
//const int ledPin = 13;  // the pin that the LED is attached to

// INPUT PINS
// thermistor-1.ino Simple test program for a thermistor for Adafruit Learning System
// https://learn.adafruit.com/thermistor/using-a-thermistor by Limor Fried, Adafruit Industries
// MIT License - please keep attribution and consider buying parts from Adafruit
 
// the value of the 'other' resistor
#define SERIESRESISTOR 100000    
 
// What pin to connect the sensor to
#define THERMISTORPIN A0

//const int thermistorPin = A0;  // the pin that the thermistor reading is taken from
int thermistorValue = 0;  // variable to store the value coming from the sensor
bool displayThermistor = false;

const int selectPin1 = 16; // Jumper for 2 rotations per trigger
const int selectPin2 = 2; // Jumper for 3 rotations per trigger
const int selectPin3 = 5; // Jumper for 4 rotations per trigger
const int selectPin4 = 4; // Jumper for 5 rotations per trigger

// Variables will change:
int wheelRotationCounter = 0;   // counter for the number of button presses
int numRotationsToTrigger = 4;  // the number of rotations it takes to trigger the camera


// manual shutter trigger variables
bool shutterTriggerPressed = false;
unsigned long shutterTriggerStartTime = 0;   // milliseconds the trigger will last
unsigned long shutterTriggerBlinkLength = 500;   // milliseconds the trigger will last
unsigned long now_millis = 0;                //The current time from millis()





void setup() {

  // initialize the LED and Opto as an output:
  pinMode(LED_PIN, OUTPUT);

  wheelTriggerButton.begin();
  bluetoothResetButton.begin();
  shutterTriggerButton.begin();

  // pinMode(thermistorPin, INPUT); // this is modified only in the TransistorTrigger Class
  // sensorValue = analogRead(sensorPin);   
  
  // initialize the selectPins as inputs:
//  pinMode(selectPin1, INPUT_PULLUP);
//  pinMode(selectPin2, INPUT_PULLUP);
//  pinMode(selectPin3, INPUT_PULLUP);
//  pinMode(selectPin4, INPUT_PULLUP);


//  if(digitalRead(selectPin1) == LOW){
//    numRotationsToTrigger = 2;
//  }
//  else if(digitalRead(selectPin2) == LOW){
//    numRotationsToTrigger = 3;
//  }
//  else if(digitalRead(selectPin3) == LOW){
//    numRotationsToTrigger = 4;
//  }  
//  else if(digitalRead(selectPin4) == LOW){
//    numRotationsToTrigger = 5;
//  }
//  else{
//    numRotationsToTrigger = 4;
//  }

//  Serial.begin(9600);       // use the serial port
//delay(3000);
}

void loop() {

//  now_millis = millis();

  wheelTriggerButton.read();   //Read the button
  shutterTriggerButton.read();   //Read the button
  bluetoothResetButton.read();   //Read the button


  if(wheelTriggerButton.isPressed() || shutterTriggerButton.isPressed() || bluetoothResetButton.isPressed()){
    digitalWrite(LED_PIN, HIGH);
  }
  else{
    digitalWrite(LED_PIN, LOW);
  }  


}

























