/*
  DL_LED_Controller.h     - Class that provides control of an LED indicator for TK's Distance Lapse Camera Trigger
                                s
  Created by Turner E. Kirk, April 20, 2020.
  Released into the public domain.
*/

#ifndef BL_LEDControl_h
#define BL_LEDControl_h

// ******************************************************************
//            Includes and Defines
// ******************************************************************
#include <Arduino.h>

// ******************************************************************    
//                 LED Controller CLASS
//    This class manages the state of the led based
//    on the input from the buttons
//    It uses software timers to ensure that it doesn't 
//    block the main thread
// ******************************************************************
class BL_LEDControl
{

  private:
    // ENUM TO KEEP TRACK OF VARIOUS STATES
    enum {INACTIVE, LED_ON, LED_OFF, LED_ON_OFF, LED_OFF_PULSE, LED_PULSE};
    
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
    BL_LEDControl(const uint8_t _ledPin) : ledPin(_ledPin){}

  // ******************************************************************    
  //                         PUBLIC METHODS
  // ******************************************************************  
  void Update();
  void turnOn();
  void turnOff();
  void turnOnOff(bool normalState);
  void pulse(bool normalState);

};


#endif