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
 
int LEDpin = 13;


void setup() {
  // put your setup code here, to run once:
  pinMode(LEDpin, OUTPUT );
  // start the serial port at 115200 Baud ( bps )
  Serial.begin(115200);
  delay(5000);
  Serial.println("Ready for user input...");
}

void loop() {
  int loopNumber;
  int i;
  if (Serial.available()>0) 
  {      
    loopNumber=Serial.parseInt();
    for( int i  = 0; i < loopNumber; i++ ) {
      Serial.print("blinking ");
      Serial.print(i);
      Serial.println(" times");
      digitalWrite(LEDpin, HIGH);
      delay(1000);
      digitalWrite(LEDpin, LOW);
    }
  }
}
