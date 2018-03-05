int stepPin = 9;
int dirPin = 8;
int waitTime = 2500; //in micro seconds
int stepPerRev = 1000; //steps per rev, set on the motor controller

void setup() {
  //set both pins to outputs
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);

  //the motor starts as off
  digitalWrite(dirPin, LOW);
  digitalWrite(stepPin, LOW);

   //open serial connection to the computer with a baud rate of 9600
  Serial.begin(9600);
}

void loop() {
  //writing dirPin high, to control direction of motor
  digitalWrite(dirPin, HIGH);

  if(Serial.available() > 0){ //Check if the serial port successfully opened
    double rps = Serial.readString().toFloat(); //Read a value (revs per second)
    waitTime = 1000000/(stepPerRev*rps); //calculate the microseconds between pulses
    //Serial.println(waitTime);
  }
  digitalWrite(stepPin, HIGH); //writing stepPin high, moving the motor 1/1000 of a rev
  delayMicroseconds(waitTime); //wait the calculated delay
  digitalWrite(stepPin, LOW); //write stepPin low, stopping the motor
}
