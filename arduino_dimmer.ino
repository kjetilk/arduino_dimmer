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
  35, // Kitchen
  39, 
  43, // Dining
  47, // Living room
  34,
  38, 
  42, // WC 2.
  46  // WC 1.
};

byte lowlimit[] = {
  53,
  5,
  36,
  25,
  5,
  5,
  47,
  38
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
    delay(1000);    
    digitalWrite(13, LOW);
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
      digitalWrite(13, HIGH);
      if (brightness[i] < lowlimit[i]) {
        brightness[i] = lowlimit[i];
      }
      else if (brightness[i] < maxlevel - fadeAmount) {
         brightness[i] = brightness[i] + fadeAmount;
      } else {
         brightness[i] = maxlevel;
      }
      Serial.print(i);
      Serial.print(F("th circuit going up to "));
      Serial.println(brightness[i]);
    }  
    if( digitalRead( downinputs[ i ] ) == 0 ) { // Down button pressed
      digitalWrite(13, HIGH);
      if(brightness[i] >= fadeAmount + lowlimit[i]) {      
        brightness[i] = brightness[i] - fadeAmount;
      } else {
        brightness[i] = 0;
      }
      Serial.print(i);
      Serial.print(F("th circuit going down to "));
      Serial.println(brightness[i]);
    }  
    
    // Write the state to the LUD
    pwmWrite(leds[i], brightness[i]);
  } 
  delay(20);
  digitalWrite(13, LOW);
}

