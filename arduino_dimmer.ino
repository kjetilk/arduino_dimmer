/*
 This is the code I use to control the dimmers in my house.
 
 The LEDs are driven by an Eltako LUD12, that takes a PWM signal with an amplitude of 10-24V, 
 and pulse width 0-90% to set the light level.
 It runs on a Mega where pins 2-11 except 4 and 10 (which is needed by an Ethernet controller (future project))
 are connected with some NPN transistors and some resistors, to the X2 connector on the LUD12.
 
 Based on the PWM.h example. Consider this code released under CC0.
 
 Kjetil Kjernsmo <kjetil@kjernsmo.net>
 */

#include <PWM.h>


char leds[] = {
  2,
  3,
  5,
  6,
  7,
  8,
  9,
  11
};

byte upinputs[] = { 
  33,
  37,
  41,
  45,
  32,
  36,
  40,
  44
};

byte downinputs[] = { 
  35,
  39,
  43,
  47,
  34,
  38,
  42,
  46
};

unsigned int brightness[8] = {100,100,100,100,100,100,100,100};    // how bright the LEDs are
int fadeAmount = 1;         // how many points to fade a LED by
int32_t frequency = 100;    // frequency (in Hz)
int maxlevel = 255 * 0.9;

void setup()
{
  Serial.begin(9600);
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe(); 

  //sets the frequency for the specified pins
  bool success = true;
  
  for( unsigned int i = 0; i < sizeof( leds ); ++i ) {
    success &= SetPinFrequencySafe(leds[i], frequency);
  }
  //if the pin frequency was set successfully, turn pin 13 on
  if(success) {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);    
  }
  
  // Initialize all inputs as pullups
  for( unsigned int i = 0; i < sizeof( upinputs ); ++i ) {
    pinMode(upinputs[i], INPUT_PULLUP);
  }  
  for( unsigned int i = 0; i < sizeof( downinputs ); ++i ) {
    pinMode(downinputs[i], INPUT_PULLUP);
  }  
}

void loop()
{
  for( unsigned int i = 0; i < sizeof( leds ); ++i ) {
    if( digitalRead( upinputs[ i ] ) == 0 ) { // Up button pressed
      if (brightness[i] < maxlevel - fadeAmount) {
         brightness[i] = brightness[i] + fadeAmount;
      } else {
         brightness[i] = maxlevel;
      }
    }  
    if( digitalRead( downinputs[ i ] ) == 0 ) { // Down button pressed
      if(brightness[i] >= fadeAmount) {      
        brightness[i] = brightness[i] - fadeAmount;
      } else {
        brightness[i] = 0;
      }
    }  
    
    // Write the state to the LUD
    pwmWrite(leds[i], brightness[i]);
  } 
  delay(20);
}

