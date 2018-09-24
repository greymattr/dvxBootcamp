
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
