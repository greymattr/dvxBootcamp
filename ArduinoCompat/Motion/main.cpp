/* PIR motion sensor example */​

#include <Arduino.h>​

​

int ledPin = 13;                 // choose the pin for the LED​

int sensorPin = 2;               // choose the input pin (for PIR sensor)​

int pirState = LOW;              // we start, assuming no motion detected​

int val = 0;                     // variable for reading the pin status​

​

void setup()
{
  ​

  pinMode( ledPin, OUTPUT );     // set ledPin LED as output​

  pinMode( sensorPin, INPUT );   // set sensorPin as input​

  Serial.begin( 9600 );
  ​

}​

​

void loop()
{
  ​

  val = digitalRead( sensorPin );         // read the sensorPin​

  Serial.println( val );
  ​

  if ( val == HIGH ) {                    // check if the input is HIGH​

    digitalWrite( ledPin, HIGH );         // turn LED ON​

    if ( pirState == LOW ) {              // Motion detected pin goes LOW​

      Serial.println( "Motion detected!" );
      ​

      pirState = HIGH;                    // the pin will stay low for a few seconds​

    }​

  } else {
    ​

    digitalWrite( ledPin, LOW );          // turn LED OFF​

    if ( pirState == HIGH ) {             // motion detection ended​

      Serial.println( "Motion ended!" );
      ​

      pirState = LOW;
      ​

    }​

  }​

}​