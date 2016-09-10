/*
  RaggedPi Project
  Arduino 4 "basil"
  Written by david durost <david.durost@gmail.com>
*/

#include <Relay.h>

#define FAN 1       // relay number [1-4]
#define MPPT 2      // digital pin
#define LED 13      // led pin

Relay fanRelay(FAN, LOW);

void setup()
{
    Serial.begin(9600);

    pinMode(MPPT, INPUT);
    pinMode(LED, OUTPUT);
}

void loop() {
    if ((HIGH == MPPT && fanRelay.isOff()) || (LOW == MPPT && fanRelay.isOn())){
        fanRelay.toggle();
    }
}