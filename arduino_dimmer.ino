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
  byte nightlevel;
};

// Initialize the system with background knowledge
struct Dimmer dimmers[] = {
  /* name       output  upinput  downinput  lowlimit nightlevel */
  { "Kitchen",       2,      33,        35,       53,         0 },
  { "Unassig",       3,      37,        39,        5,         0 },
  { "Dining",        5,      41,        43,       55,         0 },
  { "Living",        6,      45,        47,       25,        35 },
  { "Master",        7,      32,        34,       12,         0 },
  { "ULiving",       8,      36,        38,       37,        57 },
  { "Bathrm2",       9,      40,        42,       42,        40 },
  { "Bathrm1",      11,      44,        46,       38,        38 }
};

#define NUMBER_OF_DIMMERS 8

unsigned int prevlevel[8] = {100,100,100,100,100,100,100,100};    // The previous brightness of the LEDS
unsigned int brightness[8] = {100,100,100,100,100,100,100,100};    // how bright the LEDs are
int fadeAmount = 1;         // how many points to fade a LED by
int32_t frequency = 100;    // frequency (in Hz)
int maxlevel = 255 * 0.9;
unsigned int upcounter[8] = {0,0,0,0,0,0,0,0}; // A proxy to see if the up button is kept pressed
unsigned int downcounter[8] = {0,0,0,0,0,0,0,0}; // A proxy to see if the down button is kept pressed
int quickclick = 5;         // How many counts counts as a quick click

const byte nightpin = 30;
const byte awaypin = 28;

boolean isaway = 1;

void setPrevLevel(int i)
{
  if(brightness[i] > dimmers[i].lowlimit) {
    prevlevel[i] = brightness[i];
  }
}

void increase(int i)
{ 
  upcounter[i]++;
  if (upcounter[i] > quickclick) {
    setPrevLevel(i);
  }
  digitalWrite(13, HIGH);
  if (brightness[i] < dimmers[i].lowlimit) {
    brightness[i] = dimmers[i].lowlimit;
  }
  else if (brightness[i] < maxlevel - fadeAmount - int(upcounter[i]/8)) {
    brightness[i] = brightness[i] + fadeAmount + int(upcounter[i]/8);
  } else {
    brightness[i] = maxlevel;
  }
}

void decrease(int i)
{
  downcounter[i]++;
  if (downcounter[i] > quickclick) {
    setPrevLevel(i);
  }
  digitalWrite(13, HIGH);
  if(brightness[i] >= fadeAmount + dimmers[i].lowlimit + int(downcounter[i]/8)) {
    brightness[i] = brightness[i] - fadeAmount - int(downcounter[i]/8);
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
    success &= SetPinFrequencySafe(dimmers[i].output, frequency);
    //  pinMode(dimmers[i].output, OUTPUT); // Just when developing

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

  // Init special pins
  pinMode(nightpin, INPUT_PULLUP);
  pinMode(awaypin, INPUT_PULLUP);

  if ( digitalRead( awaypin ) == 1 ) {
    isaway = 0;
  }
  Serial.println("reset");
}

void loop()
{
  for( unsigned int i = 0; i < NUMBER_OF_DIMMERS; ++i ) {
    if( digitalRead( dimmers[i].upinput ) == 0 ) { // Up button pressed
      increase(i);
      Serial.print(dimmers[i].name);
      Serial.print(": ");
      Serial.print(i);
      Serial.print(F("th circuit going up to b, c "));
      Serial.print(brightness[i]);
      Serial.print(", ");
      Serial.println(upcounter[i]);
    }
    if( digitalRead( dimmers[i].downinput ) == 0 ) { // Down button pressed
      decrease(i);
      Serial.print(dimmers[i].name);
      Serial.print(": ");
      Serial.print(i);
      Serial.print(F("th circuit going down to b, c "));
      Serial.print(brightness[i]);
      Serial.print(", ");
      Serial.println(downcounter[i]);
    }  
    if( digitalRead( dimmers[i].upinput ) == 1 ) { // Up button released
      if( upcounter[i] > 0) {
        digitalWrite(13, LOW);
        Serial.print(i);
        Serial.println(F(" up released."));
        if( upcounter[i] <= quickclick) {
          Serial.print(F("Returning to previous state "));
          Serial.print(prevlevel[i]);
          Serial.print(" on ");
          Serial.println(dimmers[i].name);
          brightness[i] = prevlevel[i];
        }
      }
      upcounter[i] = 0;
    }
    if( digitalRead( dimmers[i].downinput ) == 1 ) { // Down button released
      if( downcounter[i] > 0) {
        digitalWrite(13, LOW);
        Serial.print(i);
        Serial.println(F(" down released."));
        if( downcounter[i] <= quickclick) {
          Serial.print(F("Turning off "));
          Serial.println(dimmers[i].name);
          setPrevLevel(i);
          brightness[i] = 0;
        }
      }
      downcounter[i] = 0;
    }

    if( digitalRead( nightpin ) == 0 ) { // Night button pressed
      for( unsigned int i = 0; i < NUMBER_OF_DIMMERS; ++i ) {
        setPrevLevel(i);
        brightness[i] = dimmers[i].nightlevel;
      }
      Serial.println(F("Setting all dimmers in night state"));
      delay(100); // So that prevlevel won't be reset at loop
    }

    if( (! isaway) && digitalRead( awaypin ) == 0 ) { // Away status is newly set
      isaway = 1;
      for( unsigned int i = 0; i < NUMBER_OF_DIMMERS; ++i ) {
        setPrevLevel(i);
        brightness[i] = 0;
      }
      Serial.println(F("Setting all dimmers off while away"));
    }

    if( (isaway) && digitalRead( awaypin ) == 1 ) { // We just returned
      isaway = 0;
      for( unsigned int i = 0; i < NUMBER_OF_DIMMERS; ++i ) {
        brightness[i] = dimmers[i].nightlevel;
        if( dimmers[i].nightlevel > 0 ) {
          brightness[i]+=20; // Add a bit to avoid problems with just one going on
        }
      }
      Serial.println(F("Just back, setting all dimmers at nightlevel + 20"));
    }

      

    // Write the state to the LUD
    pwmWrite(dimmers[i].output, brightness[i]);
    // analogWrite(dimmers[i].output, brightness[i]); // Just when developing

  } 
  delay(20);
}

