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
  setColor(255, 255, 255); // Red Color
  delay(1000);

 setColor(0, 255, 200); // Green Color
  delay(1000);

  setColor(250, 0, 250); // Blue Color
  delay(1000);

}

void setColor(int redMode, int greenMode, int blueMode) {
  analogWrite(R, 255 - redMode);
  analogWrite(G, 255 - greenMode);
  analogWrite(B, 255 - blueMode);
}