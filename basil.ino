/*
  RaggedPi Project
  Arduino 4 "basil"
  Written by david durost <david.durost@gmail.com>
*/
#define fanRelay 6  // digital pin
#define mppt 2      // digital pin
#define led 13      // led pin

void setup()
{
    Serial.begin(9600);
    pinMode(fanRelay, OUTPUT);
    pinMode(mppt, INPUT);
    pinMode(led, OUTPUT);
}

void loop() {
    pinMode(fanRelay, digitalRead(mppt));
}