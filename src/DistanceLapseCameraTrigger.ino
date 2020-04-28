/*
  Distance Lapse Camera Trigger for Bicycles
  Created April 2020 (during corona virius pandemic)

  Counts the number of wheel rotations on a bicycle and triggers a camera shutter release every Nth time
  the "N" number of rotations to trigger the shutter release can be set in the code or chosen via hardware jumpers

  The circuit:
  - LED attached from pin 13 to ground
  - Reed Switch attached to pin 10 from +5V


  Explain the rest of the circuit here:
  - 390 ohm resistor attached to pin 5 from the annode of an LED who's cathode is attached to ground

*/
// ************************************************
//        HARDWARE DECLARATIONS
// ************************************************
// #define __ARDUINO_UNO__
#define __FEATHER_NRF52832__
// #define __FEATHER_ESP8266__

// use this to turn on the serial monitor
// will post the temperature readings from the thermistor to serial monitor
// #define __SERIAL_MONITOR__

// ************************************************
//        LIBRARY INCLUDES
// ************************************************
#if defined(__FEATHER_NRF52832__)
  #include <bluefruit.h>
  BLEDis bledis;
  BLEHidAdafruit blehid;
#elif defined(__FEATHER_ESP8266__)
  #include <ESP8266WiFi.h>
#endif

// INFO ON THE BUTTON LIBRARY HERE: https://github.com/JChristensen/JC_Button
#include <JC_Button.h>
#include "DL_LEDController.h"


// ************************************************
//        PIN AND HARDWARE DEFINITIONS
// ************************************************
#if defined(__ARDUINO_UNO__)
  #define WHEEL_TRIGGER_PIN 2
  #define SHUTTER_TRIGGER_PIN 11
  #define BLE_RESET_PIN 10
  #define LED_PIN 9
  #define RESISTANCE_OFFSET 0
  #define SERIAL_BAUDRATE 9600
#elif defined(__FEATHER_NRF52832__)
  #define WHEEL_TRIGGER_PIN 12
  #define SHUTTER_TRIGGER_PIN 14
  #define BLE_RESET_PIN 13
  #define LED_PIN 11
  // #define LED_PIN LED_RED // onboard LED
  #define RESISTANCE_OFFSET 0
  #define SERIAL_BAUDRATE 115200
#elif defined(__FEATHER_ESP8266__)
  #define WHEEL_TRIGGER_PIN 14
  #define SHUTTER_TRIGGER_PIN 12
  #define BLE_RESET_PIN 13
  #define LED_PIN 15
  #define RESISTANCE_OFFSET 10000
  #define SERIAL_BAUDRATE 9600
#endif



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
//        BUTTON LIBRARY VARIABLES
// ************************************************
#define PULLUP true    // these aren't required in the most recent version of JC_Button because they are defaults when using a button that is set up to be pulled low
#define INVERT true    // these aren't required in the most recent version of JC_Button because they are defaults when using a button that is set up to be pulled low
#define DEBOUNCE_MS   40       // sets the debounce time in ms. Should be between 20-50 ms depending on the button
#define LONG_PRESS 1000    //We define a "long press" to be 1000 milliseconds.

Button wheelTriggerButton(WHEEL_TRIGGER_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)
Button shutterTriggerButton(SHUTTER_TRIGGER_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)
Button bluetoothResetButton(BLE_RESET_PIN, DEBOUNCE_MS);    //Declare the button (this also initializes the pin it's attached to)


// ************************************************
//        BUTTON STATE LOGIC VARIABLES
// ************************************************
enum states_t {NORMAL, LONGPRESS, CANCEL_LONGPRESS};
states_t SHUTTER_TRIGGER_STATE = NORMAL;      // current state machine state
states_t BLE_RESET_STATE = NORMAL;      // current state machine state

unsigned long longPressTimeoutStartTime = 0;
unsigned long longPressTimeoutInterval = 4000;


// ************************************************
//        WHEEL TRIGGER LOGIC VARIABLES
// ************************************************
int wheelRotationCounter = 0;   // counter for the number of button presses
int numRotationsToTrigger = 4;  // the number of rotations it takes to trigger the camera
bool wheelTriggerActive = true;


// ************************************************
//        LED PIN AND CONTROL CLASS VARIABLES
// ************************************************

DL_LEDController ledController(LED_PIN);


// ************************************************
//        THERMISTOR VARIABLES
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
//        JUMPER PIN VARIABLES
// ************************************************
const int selectPin1 = 16; // Jumper for 2 rotations per trigger
const int selectPin2 = 2; // Jumper for 3 rotations per trigger
const int selectPin3 = 5; // Jumper for 4 rotations per trigger
const int selectPin4 = 4; // Jumper for 5 rotations per trigger



// ************************************************
//        SETUP FUNCTION 
//        SETUP FUNCTION 
//        SETUP FUNCTION 
// ************************************************
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

#if defined(__SERIAL_MONITOR__)
  Serial.begin(SERIAL_BAUDRATE);       // use the serial port at the baudrate defined by the hardware
  #if defined(__FEATHER_NRF52832__)
    while ( !Serial ) delay(10);   // for nrf52840 with native usb
  #endif
#endif

  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("Bluefruit52");

  // Configure and start DIS (Device Information Service)
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather 52");
  bledis.begin();

  /* Start BLE HID
   * Note: Apple requires BLE devices to have a min connection interval >= 20m
   * (The smaller the connection interval the faster we can send data).
   * However, for HID and MIDI device Apple will accept a min connection
   * interval as low as 11.25 ms. Therefore BLEHidAdafruit::begin() will try to
   * set the min and max connection interval to 11.25 ms and 15 ms respectively
   * for the best performance.
   */
  blehid.begin();

  /* Set connection interval (min, max) to your perferred value.
   * Note: It is already set by BLEHidAdafruit::begin() to 11.25ms - 15ms
   * min = 9*1.25=11.25 ms, max = 12*1.25= 15 ms
   */
  /* Bluefruit.Periph.setConnInterval(9, 12); */

  // Set up and start advertising
  startAdv();




}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);

  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.Advertising.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

// ************************************************
//        MAIN LOOP 
//        MAIN LOOP 
//        MAIN LOOP 
// ************************************************
void loop() {

  //Read the buttons
  wheelTriggerButton.read();   
  shutterTriggerButton.read();
  bluetoothResetButton.read();

  /*----------------------------------------------------------------------*
  / wheelTriggerButton Logic:
  /   If the wheel completed a rotation and wheelTriggerActive is "true", 
  /   update the LED classes state
  /-----------------------------------------------------------------------*/
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

  // run the logic for our buttons
  updateShutterTriggerButtonLogic();
  updateBluetoothResetButtonLogic();

  // Update the LED based on the logic set by the wheel trigger and buttons
  ledController.Update();

}


/*----------------------------------------------------------------------*
/ shutterTriggerButton Logic:
/   If the button was released, flash the led as an indicator
/   If the button was long pressed:
/       disconnect/connect the wheel trigger
/       set the LED into a pulsing or off state
/       invert the on/off logic for the LED when this button is short-pressed again
/-----------------------------------------------------------------------*/
void updateShutterTriggerButtonLogic(){
  switch (SHUTTER_TRIGGER_STATE){
    // this state watches for short and long presses, triggers the LED for
    // short presses, and moves to the LONGPRESS state for long presses.
    // for short presses it sets the logic based on if the wheel trigger is active
    case NORMAL:
      if (shutterTriggerButton.wasReleased()){
        ledController.turnOnOff(wheelTriggerActive);
        sendShutterReleaseCommand();
      }
      else if (shutterTriggerButton.pressedFor(LONG_PRESS)){
        longPressTimeoutStartTime = millis();
        SHUTTER_TRIGGER_STATE = LONGPRESS;
      }
      break;

    // this is a transition state where we are waiting for the user to release the button.
    // If they take too long to release the button, we move to the CANCEL_LONGPRESS state
    // If they release the button within the longPressTimeoutInterval, we do a few things:
    //    Set the LED to its pulsing or non pulsing state (depending on if wheelTriggerActive)
    //    Enable or disable the wheelTriggerActive boolean
    //    move back to the NORMAL state
    case LONGPRESS:
      if(millis() - longPressTimeoutStartTime > longPressTimeoutInterval){
        SHUTTER_TRIGGER_STATE = CANCEL_LONGPRESS;
      }
      else{
        if (shutterTriggerButton.wasReleased()){
          ledController.pulse(wheelTriggerActive);

          wheelTriggerActive = !wheelTriggerActive;

          SHUTTER_TRIGGER_STATE = NORMAL;
        }
      }
      break;

    // this is a transition state where we just wait for the user to release the button
    // before moving back to the NORMAL state. Once the button is released we need to 
    // set the led to be pulsing and invert the logic for subsequent button presses
    case CANCEL_LONGPRESS:
      if (shutterTriggerButton.wasReleased()){
          SHUTTER_TRIGGER_STATE = NORMAL;
      }
      break;      
  }
}


/*----------------------------------------------------------------------*
/ bluetoothResetButton Logic:
/   If the button was released, flash the led as an indicator
/   If the button was long pressed:
/       disconnect/connect the wheel trigger
/       set the LED into a pulsing or off state
/       invert the on/off logic for the LED when this button is short-pressed again
/-----------------------------------------------------------------------*/
void updateBluetoothResetButtonLogic(){

  switch (BLE_RESET_STATE){
    // this state watches for short and long presses, triggers the LED for
    // short presses, and moves to the LONGPRESS state for long presses.
    // for short presses it sets the logic based on if the wheel trigger is active
    case NORMAL:
      if (bluetoothResetButton.wasReleased()){
        readAndReactToTemperature();
      }
      else if (bluetoothResetButton.pressedFor(LONG_PRESS)){
        longPressTimeoutStartTime = millis();
        BLE_RESET_STATE = LONGPRESS;
      }
      break;

    // this is a transition state where we are waiting for the user to release the button.
    // If they take too long to release the button, we move to the CANCEL_LONGPRESS state
    // If they release the button within the longPressTimeoutInterval, we 
    // reset the bluetooth credentials and flash the LED as 
    case LONGPRESS:
      if(millis() - longPressTimeoutStartTime > longPressTimeoutInterval){
        BLE_RESET_STATE = CANCEL_LONGPRESS;
      }
      else{
        if (bluetoothResetButton.wasReleased()){
          resetBluetoothCredentials();
          BLE_RESET_STATE = NORMAL;
        }
      }
      break;

    // this is a transition state where we just wait for the user to release the button
    // before moving back to the NORMAL state. 
    case CANCEL_LONGPRESS:
      if (bluetoothResetButton.wasReleased()){
          BLE_RESET_STATE = NORMAL;
      }
      break;      
  }
}


void resetBluetoothCredentials(){

  // digitalWrite(LED_PIN, LOW);
  analogWrite(ledPin, 0);
  delay(100);

  for(uint8_t i = 0; i < 15; i++){
    // digitalWrite(LED_PIN, HIGH);
    analogWrite(ledPin, 255);
    delay(50);
    // digitalWrite(LED_PIN, LOW);
    analogWrite(ledPin, 0);
    delay(50);
  }
}




void sendShutterReleaseCommand(){
  // Make sure we are connected and bonded/paired
  for (uint16_t conn_hdl=0; conn_hdl < BLE_MAX_CONNECTION; conn_hdl++)
  {
    BLEConnection* connection = Bluefruit.Connection(conn_hdl);

    if ( connection && connection->connected() && connection->paired() )
    {
      // Turn on red LED when we start sending data
      // digitalWrite(LED_RED, 1);
      analogWrite(LED_RED, 255);

      // Send the 'volume down' key press to the peer
      // Check tinyusb/src/class/hid/hid.h for a list of valid consumer usage codes
      blehid.consumerKeyPress(conn_hdl, HID_USAGE_CONSUMER_VOLUME_DECREMENT);

      // Delay a bit between reports
      delay(10);

      // Send key release
      blehid.consumerKeyRelease(conn_hdl);

      // Turn off the red LED
      // digitalWrite(LED_RED, 0);
      analogWrite(LED_RED, 0);
    }
  }
}









void readAndReactToTemperature(){

  float reading;
 
  reading = analogRead(THERMISTORPIN);
 
 #if defined(__SERIAL_MONITOR__) 
  Serial.print("Analog reading "); 
  Serial.println(reading);
 #endif

  // convert the value to resistance
  reading = (1023 / reading)  - 1;     // (1023/ADC - 1) 
  reading = SERIESRESISTOR / reading + RESISTANCE_OFFSET;  // 100K / (1023/ADC - 1)
  // Serial.print("Thermistor resistance "); 
  // Serial.println(reading);

  float steinhart;
  steinhart = reading / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  
#if defined(__SERIAL_MONITOR__)  
  Serial.print("Thermistor resistance "); 
  Serial.println(reading);

  Serial.print("Temperature "); 
  Serial.print(steinhart);
  Serial.println(" *C");

  Serial.print("Temperature "); 
  Serial.print(steinhart * 1.8 + 32);
  Serial.println(" *F");
#endif

}





















