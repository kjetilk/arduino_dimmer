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

struct Dimmer {
    char name[8];
    char output;
    byte upinput;
    byte downinput;
    byte lowlimit;
};

// Initialize the system with background knowledge
struct Dimmer dimmers[] = {
  /* name       output  upinput  downinput  lowlimit */
  { "Kitchen",       2,      33,        35,       53 },
  { "Unassig",       3,      37,        39,        5 },
  { "Dining",        5,      41,        43,       55 },
  { "Living",        6,      45,        47,       25 },
  { "Master",        7,      32,        34,        5 },
  { "ULiving",       8,      36,        38,        5 },
  { "Bathrm2",       9,      40,        42,       47 },
  { "Bathrm1",      11,      44,        46,       38 }
};

#define NUMBER_OF_DIMMERS 8


unsigned int brightness[8] = {100,100,100,100,100,100,100,100};    // how bright the LEDs are
int fadeAmount = 1;         // how many points to fade a LED by
int32_t frequency = 100;    // frequency (in Hz)
int maxlevel = 255 * 0.9;
unsigned int upcounter[8] = {0,0,0,0,0,0,0,0}; // A proxy to see if the up button is kept pressed
unsigned int downcounter[8] = {0,0,0,0,0,0,0,0}; // A proxy to see if the down button is kept pressed


void increase(int i)
{
  digitalWrite(13, HIGH);
  if (brightness[i] < dimmers[i].lowlimit) {
    brightness[i] = dimmers[i].lowlimit;
  }
  else if (brightness[i] < maxlevel - fadeAmount) {
    brightness[i] = brightness[i] + fadeAmount;
  } else {
    brightness[i] = maxlevel;
  }
}

void decrease(int i)
{
  digitalWrite(13, HIGH);
  if(brightness[i] >= fadeAmount + dimmers[i].lowlimit) {
    brightness[i] = brightness[i] - fadeAmount;
  } else {
    brightness[i] = 0;
  }
}


void setup()
{
  Serial.begin(9600);
  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe(); 

  //sets the frequency for the specified pins
  bool success = true;
  
  for( unsigned int i = 0; i < NUMBER_OF_DIMMERS; ++i ) {
//    success &= SetPinFrequencySafe(dimmers[i].output, frequency);
      pinMode(dimmers[i].output, OUTPUT); // Just when developing

  }
  //if the pin frequency was set successfully, turn pin 13 on
  if(success) {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);
    delay(1000);    
    digitalWrite(13, LOW);
  }
  
  // Initialize all inputs as pullups
  for( unsigned int i = 0; i < NUMBER_OF_DIMMERS; ++i ) {
    pinMode(dimmers[i].upinput, INPUT_PULLUP);
  }  
  for( unsigned int i = 0; i < NUMBER_OF_DIMMERS; ++i ) {
    pinMode(dimmers[i].downinput, INPUT_PULLUP);
  }  
}

void loop()
{
  for( unsigned int i = 0; i < NUMBER_OF_DIMMERS; ++i ) {
    if( digitalRead( dimmers[i].upinput ) == 0 ) { // Up button pressed
      increase(i);
      upcounter[i]++;
      Serial.print(dimmers[i].name);
      Serial.print(": ");
      Serial.print(i);
      Serial.print(F("th circuit going up to "));
      Serial.println(brightness[i]);
    }
    if( digitalRead( dimmers[i].downinput ) == 0 ) { // Down button pressed
      decrease(i);
      downcounter[i]++;
      Serial.print(dimmers[i].name);
      Serial.print(": ");
      Serial.print(i);
      Serial.print(F("th circuit going down to "));
      Serial.println(brightness[i]);
    }  
    if( digitalRead( dimmers[i].upinput ) == 1 ) { // Up button released
      if( upcounter[i] > 0) {
        Serial.print(i);
        Serial.println(F(" up released."));
      }
      upcounter[i] = 0;
    }
    if( digitalRead( dimmers[i].downinput ) == 1 ) { // Down button released
      if( downcounter[i] > 0) {
        Serial.print(i);
        Serial.println(F(" down released."));
      }
      downcounter[i] = 0;
    }

    // Write the state to the LUD
    // pwmWrite(dimmers[i].output, brightness[i]);
    analogWrite(dimmers[i].output, brightness[i]); // Just when developing

  } 
  delay(20);
  digitalWrite(13, LOW);
}

