/*
  RaggedPi Project
  Arduino 3 "Basil" - MPPT Vent Controller
  Written by david durost <david.durost@gmail.com>
*/
/* Includes */
#include <SoftwareSerial.h>     // Serial library
#include <Relay.h>              // Relay library

/* Misc Constants */
#define SDA 2                   // sda
#define SDL 3                   // sdl
#define CS 10                   // chipselect
#define LED 13                  // led pin
#define MPPT_INPUT_PIN 8        // digital pin
#define SLEEP_TIME 10000        // sleep time ms)

/* Enums */
enum Modes {
    MONITOR_MODE,
    SLEEP_MODE,
    MANUAL_MODE
};

/* Objects */
Relay fanRelay(RELAY3, LOW);    // relay

/* Variables */
bool override = false;
Modes state = MONITOR_MODE;

/**
 * Monitor mode
  */
void monitorMode() {
    Serial.println("Monitoring.");

    switch (digitalRead(MPPT_INPUT_PIN)) {
        case false:
            if (fanRelay.isOn() && !override) {
                fanRelay.off();
                Serial.println("fan deactivated.");
                state = SLEEP_MODE;
            }
            break;
        case true:
            if (fanRelay.isOff()) {
                fanRelay.on();
                Serial.println("fan activated.");
            }
            break;
    }
}

/**
 * Sleep mode
 */
void sleepMode() {
    Serial.println("Sleeping...");
    
    fanRelay.off();
    delay(SLEEP_TIME);
    state = MONITOR_MODE;
}

/**
 * Manual mode
 */
void manualMode() {
    if (override && fanRelay.isOff()) {
        fanRelay.on();
        Serial.println("fan manually activated.");
    } else if (override && fanRelay.isOn())   state = state;
    else {
        fanRelay.off();
        Serial.println("fan manually deactivated.");
        state = MONITOR_MODE;
    }
}

/**
 * Setup
 */
void setup() {
    Serial.begin(9600);
    while (!Serial);

    pinMode(MPPT_INPUT_PIN, INPUT);
    pinMode(LED, OUTPUT);
    pinMode(CS, OUTPUT);

    Serial.println("RaggedPi Project Codename Basil Initializing...");
    
    Serial.print("Initializing relays...");
    fanRelay.begin();
    Serial.println("[OK]");
    delay(600);
    Serial.println("System initialized.");
    delay(600);
}

/**
 * Loop
 */
void loop() {
    switch(state) {
        case MONITOR_MODE:
            monitorMode();
            break;
        case SLEEP_MODE:
            sleepMode();
            break;
        case MANUAL_MODE:
            manualMode();
            break;
    }
    delay(5000);
}