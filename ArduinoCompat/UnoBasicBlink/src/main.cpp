/* Copyright (C) Matthew Fatheree - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential.
 * Written by Matthew Fatheree <matthewfatheree@gmail.com>, 2018
 */
#include <Arduino.h>

int led_pin = 13;

void setup() {
    /* set pin 13 to be an output pin */
    pinMode(led_pin, OUTPUT);
}

void loop() {
    /* blink light once per second */
    digitalWrite(led_pin, HIGH);
    delay(1000);
    digitalWrite(led_pin, LOW);
    delay(1000);
}
