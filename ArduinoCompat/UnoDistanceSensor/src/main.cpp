/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Belkin/Linksys <dvxbootcamp@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <Arduino.h>

int trigger_pin = 2;        // use pin #2 as the trigger pin
int measurement_pin = 3;    // use pin #3 to measure the echo

long duration;      // The amount of time it took for the unit to get an echo response
int distance;       // The calculated distance

void setup()
{
  pinMode( trigger_pin, OUTPUT );   // Sets the trigger_pin as an Output
  pinMode( measurement_pin, INPUT ); // Sets the measurement_pin as an Input
  Serial.begin( 9600 );             // Start the serial port for displaying measurement values
}

void loop()
{
  digitalWrite( trigger_pin, LOW ); // Clear the trigger_pin
  delayMicroseconds( 2 );           // trigger_pin in transition time, stay low for 2 clocks
  digitalWrite( trigger_pin, HIGH ); // Sets the trigger_pin on HIGH state for 10 micro seconds
  delayMicroseconds( 10 );
  digitalWrite( trigger_pin, LOW ); // Sets the trigger_pin on LOW
  duration = pulseIn( measurement_pin, HIGH );  // Reads the measurement_pin ( sound wave travel time in microseconds )
  distance= ( duration*0.034 )/2;   // Calculate the distance
  Serial.print( "Distance: " );     // Print the distance on the Serial Monitor
  Serial.print( distance );
  Serial.println( "cm" );
  delay( 1000 );                    // wait 1 second before repeating loop
}
