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

#define COMMON_ANODE    // Comment this line out when using common cathode RGB LED.

int R = 9;  // OUTPUT PIN red
int G = 10; // OUTPUT PIN green
int B = 11; // OUTPUT PIN blue
int ground = 8;


void setup() {
   // set all LED pins to output signals
   pinMode(R, OUTPUT);
   pinMode(G, OUTPUT);
   pinMode(B, OUTPUT);
   pinMode(ground, OUTPUT);
   digitalWrite(ground, HIGH);
}

void loop() {
#if 0
  setColor(255, 0, 0); // Red Color
  delay(1000);
  setColor(0, 0, 0);
  delay(1000);

  setColor(0, 255, 0); // Green Color
  delay(1000);
  setColor(0, 0, 0);
  delay(1000);

  setColor(0, 0, 255); // Blue Color
  delay(1000);
  setColor(0, 0, 0);
  delay(1000);
#endif

	/* uncomment the functions below to see some different things the RGB LED can do */

  // fadeLED();

  // randomColorChange();

  // RGBChange();

  // PoliceLights();

  // TrafficSignal();

}

void setColor(int redMode, int greenMode, int blueMode) {
  #ifdef COMMON_ANODE
    redMode = 255 - redMode;
    greenMode = 255 - greenMode;
    blueMode = 255 - blueMode;
  #endif
  analogWrite(R, redMode);
  analogWrite(G, greenMode);
  analogWrite(B, blueMode);
}

/* user functions below this comment */

void fadeLED() {
  int i;				// we will only use one value, it will be assigned to all RGB pins
  
  for(i=0; i<255; i++){  // for loop to go from dark to bright
    setColor(i, i, i);
    delay(10);
  }
  for(i=255; i>0; i--){  // for loop to go from bright to dark
    setColor(i, i, i);
    delay(10);
  }
}

void randomColorChange() {
  int red;
  int green;
  int blue;
  
  red = rand() % 255;   // assign a random value from 0 to 255 to the pins
  green = rand() % 255;
  blue = random() % 255;
  
  setColor(red, green, blue);
  delay(500);
}

void RGBChange () {
  setColor(255, 0, 0); // set led to red
  delay(2000);
  setColor(0, 255, 0); // set led to green
  delay(2000);
  setColor(0, 0, 255); // set led to blue
  delay(2000);
}

void PoliceLights() {
  setColor(255, 0, 0);  // set LED to bright RED
  delay(150);
  setColor(255, 255, 255);  // set LED to WHITE
  delay(150);
  setColor(0, 0, 255);  // set LED to bright BLUE
  delay(150);
}

void TrafficSignal() {
  setColor(255, 0, 0);  // set LED to bright RED
  delay(2000);
  setColor(0, 255, 0);  // set LED to GREEN
  delay(3000);
  setColor(200, 255, 0);  // set LED to YELLOW
  delay(1000);
}

