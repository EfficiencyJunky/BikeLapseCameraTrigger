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
#define WHEEL_TRIGGER_PIN 2
#define SHUTTER_TRIGGER_PIN 11
#define BLE_RESET_PIN 10
#define PULLUP true    // these aren't required in the most recent version of JC_Button because they are defaults when using a button that is set up to be pulled low
#define INVERT true    // these aren't required in the most recent version of JC_Button because they are defaults when using a button that is set up to be pulled low
#define DEBOUNCE_MS   40       // sets the debounce time in ms. Should be between 20-50 ms depending on the button
#define LONG_PRESS 1000    //We define a "long press" to be 1000 milliseconds.

Button wheelTriggerButton(WHEEL_TRIGGER_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)
Button shutterTriggerButton(SHUTTER_TRIGGER_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)
Button bluetoothResetButton(BLE_RESET_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)




// ************************************************
//        THERMISTOR DECLARATIONS
// ************************************************
// Thermistor Example #3 from the Adafruit Learning System guide on Thermistors 
// https://learn.adafruit.com/thermistor/overview by Limor Fried, Adafruit Industries
// MIT License - please keep attribution and consider buying parts from Adafruit

// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C -- set this to the value in your thermistor's datasheet
#define THERMISTORNOMINAL 100000      
// temp. for nominal resistance (almost always 25 C (or 77 F) ) -- To convert Celsius to Farenheight, Multiply by 1.8 (or 9/5) and add 32.
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor which can be any value but to make things easy just make it as close to the 
// nominal value of the thermistor. Make sure to measure the ACTUAL value of your resistor and put it's actual value here
#define SERIESRESISTOR 100000
// #define SERIESRESISTOR 93600  
// What pin to connect the sensor to
#define THERMISTORPIN A0 
 
//const int thermistorPin = A0;  // the pin that the thermistor reading is taken from
int thermistorValue = 0;  // variable to store the value coming from the sensor
bool displayThermistor = false;



// ************************************************
//        LED PIN DECLARATION
// ************************************************
#define LED_PIN 9


// ************************************************
//        JUMPER PIN DECLARATION
// ************************************************
const int selectPin1 = 16; // Jumper for 2 rotations per trigger
const int selectPin2 = 2; // Jumper for 3 rotations per trigger
const int selectPin3 = 5; // Jumper for 4 rotations per trigger
const int selectPin4 = 4; // Jumper for 5 rotations per trigger


// ************************************************
//        MUTABLE VARIABLES
// ************************************************
int wheelRotationCounter = 0;   // counter for the number of button presses
int numRotationsToTrigger = 4;  // the number of rotations it takes to trigger the camera
bool wheelTriggerActive = true;



// ENUM TO BE USED IN "TransistorTrigger" Class
enum {INACTIVE, LED_ON, LED_OFF, LED_ON_OFF, LED_OFF_ON, LED_PULSE};

// ******************************************************************    
//                 LED Controller CLASS
//    This class manages the state of the led based
//    on the input from the buttons
//    It uses software timers to ensure that it doesn't 
//    block the main thread
// ******************************************************************
class LEDController
{

  private:    
    const uint8_t ledPin;
    uint8_t STATE = LED_OFF;

    unsigned long ledOnOffStartTime = 0;   // the time when the LED was turned on for the LED_ON_OFF or LED_OFF_ON cycle
    unsigned long ledOnOffInterval = 500;   // milliseconds the led will turn on then off (or off then on)
    
    unsigned long lastPulseUpdateTime = 0;   // milliseconds the pulse will update
    unsigned long pulseUpdateInterval = 20;   // milliseconds the pulse will update
    
    uint8_t brightness = 10;    // how bright the LED is
    uint8_t fadeAmount = 5;    // how many points to fade the LED by
    uint8_t lowBrightness = 10;
    uint8_t highBrightness = 150;

  public: 
  // ******************************************************************    
  //                         CONSTRUCTOR                     
  // ******************************************************************
    LEDController(const uint8_t _ledPin) : ledPin(_ledPin){}


  // ******************************************************************    
  //                         UPDATE METHOD
  // ******************************************************************  
  void Update() {

    switch (STATE) {
      // inactive state so we aren't constantly writing to the LED pin
      case INACTIVE:
        break;
      
      // turn the LED ON
      case LED_ON:
        digitalWrite(ledPin, HIGH);
        STATE = INACTIVE;
        break;
        
      // turn the LED OFF
      case LED_OFF:
        digitalWrite(ledPin, LOW);
        STATE = INACTIVE;
        break;
      
      // turn the LED ON and then OFF after "ledOnOffInterval" milliseconds
      case LED_ON_OFF:
        digitalWrite(ledPin, HIGH);
        if( (millis() - ledOnOffStartTime) > ledOnOffInterval){
          STATE = LED_OFF;
        }
        break;
        
      // turn the LED OFF and then ON after "ledOnOffInterval" milliseconds
      case LED_OFF_ON: 
        digitalWrite(ledPin, LOW);
        if( (millis() - ledOnOffStartTime) > ledOnOffInterval){
          STATE = LED_ON;
        }
        break;

      // slowly pulse the led brightness
      case LED_PULSE:
        if( (millis() - lastPulseUpdateTime) > pulseUpdateInterval){
          analogWrite(ledPin, brightness);

          // change the brightness for next time through the loop:
          brightness = brightness + fadeAmount;

          // reverse the direction of the fading at the ends of the fade:
          if (brightness <= lowBrightness || brightness >= highBrightness) {
            fadeAmount = -fadeAmount;
          }

          lastPulseUpdateTime = millis();
        }
        break;

//      default:
//        break;

    }
    
  }


  void turnOn() {
    STATE = LED_ON;
  }

  void turnOff() {
    STATE = LED_OFF;
  }

  void turnOnOff(bool inverted = false) {
    if(!inverted) {
      STATE = LED_ON_OFF;
    }
    else {
      STATE = LED_OFF_ON;
    }

    ledOnOffStartTime = millis();
    
  }

  void pulse() {
    STATE = LED_PULSE;
  }




};




LEDController ledController(LED_PIN);



void setup() {

  // initialize the LED as an output:
  pinMode(LED_PIN, OUTPUT);

  wheelTriggerButton.begin();
  shutterTriggerButton.begin();
  bluetoothResetButton.begin();

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

}

void loop() {

  wheelTriggerButton.read();   //Read the button
  shutterTriggerButton.read();   //Read the button
  bluetoothResetButton.read();   //Read the button

  //If the button was released, change the LED state
  if (wheelTriggerButton.wasPressed() && wheelTriggerActive) {
    wheelRotationCounter++;
    // turns on the LED every four wheel rotationsby checking the modulo of the
    // wheelRotationCounter. the modulo function gives you the remainder of the
    // division of two numbers:
    if (wheelRotationCounter % numRotationsToTrigger == 0)
    {
      ledController.turnOn();
    }
    else
    {
      ledController.turnOff();
    }
  }

  if(shutterTriggerButton.wasPressed() ) {
    ledController.turnOnOff();
  }

  if(bluetoothResetButton.wasPressed() ) {
    ledController.pulse();
  }


  // update the state of the LED
  ledController.Update();

}


  

void getAndDisplayTemperature(){

  float reading;
 
  reading = analogRead(THERMISTORPIN);
 
  Serial.print("Analog reading "); 
  Serial.println(reading);
 
  // convert the value to resistance
  reading = (1023 / reading)  - 1;     // (1023/ADC - 1) 
  reading = SERIESRESISTOR / reading;  // 10K / (1023/ADC - 1)
  Serial.print("Thermistor resistance "); 
  Serial.println(reading);

  float steinhart;
  steinhart = reading / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  
  Serial.print("Temperature "); 
  Serial.print(steinhart);
  Serial.println(" *C");

  Serial.print("Temperature "); 
  Serial.print(steinhart * 1.8 + 32);
  Serial.println(" *F");

}





















