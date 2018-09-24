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