/* Copyright (C) Matthew Fatheree - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 * Written by Matthew Fatheree <greymattr@gmail.com>, 2018
 */


#include <Keypad.h>

const byte tone1Pin=9; // pin for tone 1
const byte tone2Pin=13; // pin for tone 2
//byte PhoneNumber[]={8,6,7,5,3,0,9}; // for special characters: 10=*, 11=#, 12=1sec delay
char PhoneNumber[]= {8,6,7,5,3,0,9}; // for special characters: 10=*, 11=#, 12=1sec delay
byte PhoneNumberLength = 7;  // adjust to length of phone number

const byte ROWS = 4; // Four rows
const byte COLS = 6; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'3','1','2','a','b','c'},
  {'d','e','0','*','g','#'},
  {'h','i','8','j','7','9'},
  {'k','4','5','l','m','6'},
};

// frequencies adopted from: https://en.wikipedia.org/wiki/Dual-tone_multi-frequency_signaling
int DTMF[13][2]= {
  {941,1336}, // frequencies for touch tone 0
  {697,1209}, // frequencies for touch tone 1
  {697,1336}, // frequencies for touch tone 2
  {697,1477}, // frequencies for touch tone 3
  {770,1209}, // frequencies for touch tone 4
  {770,1336}, // frequencies for touch tone 5
  {770,1477}, // frequencies for touch tone 6
  {852,1209}, // frequencies for touch tone 7
  {852,1336}, // frequencies for touch tone 8
  {852,1477}, // frequencies for touch tone 9
  {941,1209}, // frequencies for touch tone *
  {941,1477}, // frequencies for touch tone #
  {0,0} // pause
};

// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 2, 8, 10, 5};
//byte rowPins[ROWS] = { 1, 4, 7, 8, 9 };

// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 3, 4, 6, 7, 11, 12 };
//byte colPins[COLS] = { 2, 3, 5, 6, 10, 11 };

// Create the Keypad
Keypad kpd = Keypad( makeKeymap( keys ), rowPins, colPins, ROWS, COLS );

void playDTMF( char digit, byte duration )
{
  boolean tone1state=false;
  boolean tone2state=false;
  int tone1delay=( 500000/DTMF[digit][0] )-10; // calculate delay (in microseconds) for tone 1 (half of the period of one cycle). 10 is a fudge factor to raise the frequency due to sluggish timing.
  int tone2delay=( 500000/DTMF[digit][1] )-10; // calculate delay (in microseconds) for tone 2 (half of the period of one cycle). 10 is a fudge factor to raise the frequency due to sluggish timing.
  unsigned long tone1timer=micros();
  unsigned long tone2timer=micros();
  unsigned long timer=millis(); // for timing duration of a single tone
  if( digit==12 ) {
    delay( 1000 ); // one second delay if digit is 12
  } else {
    while( millis()-timer<duration ) {
      if( micros()-tone1timer>tone1delay ) {
        tone1timer=micros(); // reset the timer
        tone1state=!tone1state; // toggle tone1state
        digitalWrite( tone1Pin, tone1state );
      }
      if( micros()-tone2timer>tone2delay ) {
        tone2timer=micros(); // reset the timer
        tone2state=!tone2state; // toggle tone2state
        digitalWrite( tone2Pin, tone2state );
      }
    }
    digitalWrite( tone1Pin,LOW );
    digitalWrite( tone2Pin,LOW );
  }
}

void playNumber( char number )
{
  playDTMF( number, 100 ); // 100 msec duration of tone
  delay( 100 ); // 100 msec pause between tones
}

void playTone( char number )
{
  playDTMF( number, 400 ); // 100 msec duration of tone
  delay( 100 ); // 100 msec pause between tones
}

void mary_had_a_little_lamb()
{
  playTone( 3 );
  playTone( 2 );
  playTone( 1 );
  playTone( 2 );
  playTone( 3 );
  playTone( 3 );
  playTone( 3 );
  delay( 300 );
  playTone( 2 );
  playTone( 2 );
  playTone( 2 );
  delay( 300 );
  playTone( 3 );
  playTone( 9 );
  playTone( 9 );
  delay( 300 );
  playTone( 3 );
  playTone( 2 );
  playTone( 1 );
  playTone( 2 );
  playTone( 3 );
  playTone( 3 );
  playTone( 3 );
  //delay(300);
  playTone( 3 );
  playTone( 2 );
  playTone( 2 );
  playTone( 3 );
  playTone( 2 );
  playTone( 1 );

}

void happy_birthday()
{
  playTone( 1 );
  playTone( 1 );
  playTone( 2 );
  playTone( 1 );
  playTone( 6 );
  playTone( 3 );
  delay( 300 );
  playTone( 1 );
  playTone( 1 );
  playTone( 2 );
  playTone( 1 );
  playTone( 9 );
  playTone( 6 );
  delay( 300 );
  playTone( 1 );
  playTone( 1 );
  playTone( 11 );
  playTone( 9 );
  playTone( 6 );
  playTone( 3 );
  playTone( 2 );
  delay( 300 );
  playTone( 9 );
  playTone( 6 );
  playTone( 9 );
  playTone( 3 );
  playTone( 6 );
  playTone( 3 );

}

void setup()
{
//  pinMode(ledpin,OUTPUT);
//  digitalWrite(ledpin, HIGH);
  pinMode( tone1Pin,OUTPUT ); // Output for Tone 1
  pinMode( tone2Pin,OUTPUT ); // Output for Tone 2
  Serial.begin( 115200 );
  delay( 500 );

  mary_had_a_little_lamb();
  //happy_birthday();

}

void loop()
{
  char key = kpd.getKey();
  if( key ) { // Check for a valid key.
    Serial.println( key );
    switch( key ) {
    case '0':
      playNumber( 0 );
      break;
    case '1':
      playNumber( 1 );
      break;
    case '2':
      playNumber( 2 );
      break;
    case '3':
      playNumber( 3 );
      break;
    case '4':
      playNumber( 4 );
      break;
    case '5':
      playNumber( 5 );
      break;
    case '6':
      playNumber( 6 );
      break;
    case '7':
      playNumber( 7 );
      break;
    case '8':
      playNumber( 8 );
      break;
    case '9':
      playNumber( 9 );
      break;
    case '*':
      playNumber( 10 );
      break;
    case '#':
      playNumber( 11 );
      break;
    }
    //playNumber(atoi(key));
  }
}
