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


int leds[] = {
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
  22,
  24,
  26,
  28,
  30,
  32,
  34,
  36
};

byte downinputs[] = { 
  21,
  23,
  25,
  27,
  29,
  31,
  33,
  35
};

unsigned int brightness[8] = {0};    // how bright the LEDs are
int fadeAmount = 5;         // how many points to fade a LED by
int32_t frequency = 100;    // frequency (in Hz)
int maxlevel = 255 * 0.9;

void setup()
{
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

    // TODO: Deal with wrapparound?
  } 
}

