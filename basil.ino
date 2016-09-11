/*
  RaggedPi Project
  Arduino 4 "basil"
  Written by david durost <david.durost@gmail.com>
*/

#include <Relay.h>

#define MPPT 2      // digital pin
#define LED 13      // led pin

Relay fanRelay(RELAY1, LOW);

void setup()
{
    Serial.begin(9600);
    Serial.println("RaggedPi Project Codename Basil Initialized.");
  
    fanRelay.begin();

    pinMode(MPPT, INPUT);
    pinMode(LED, OUTPUT);

    Serial.print("Fan Relay: ");
    Serial.println(fanRelay.state());
    Serial.print("MPPT Input State: ");
    Serial.println(digitalRead(MPPT));
}

void loop() {
    uint8_t mppt = digitalRead(MPPT);
    if ((HIGH==mppt && fanRelay.isOff()) || (LOW==mppt && fanRelay.isOn())){
        fanRelay.toggle();
    }
}