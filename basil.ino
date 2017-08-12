/*
  RaggedPi Project
  Arduino 3 "Basil" - MPPT Vent Controller
  Written by david durost <david.durost@gmail.com>
*/
/* Includes */
#include <SoftwareSerial.h>     // Serial library
#include <Relay.h>              // Relay library
#include <Pir.h>                // PIR library

/* Misc Constants */
#define SDA 2                   // sda
#define SDL 3                   // sdl
#define CS 10                   // chipselect
#define LED 13                  // led pin
#define MPPT 4                  // digital pin
#define SLEEP_TIME 90000        // ms

/* Enums */
enum Modes {
    MONITOR_MODE,
    SLEEP_MODE,
    MANUAL_MODE
};

/* Objects */
Relay fanRelay(RELAY4, LOW);    // relay
Relay lightRelay(RELAY3, LOW);  // relay
PIR pirSensor(PIR_SENSOR);      // sensor

/* Variables */
unsigned long sleep = 0;
Modes status = MONITOR_MODE;

/**
 * Read mppt pin
 * @return uint8_t
 */
uint8_t readMppt() {
    return digitalRead(MPPT);
}

/**
 * Monitor mode
  */
void monitorMode() {
    Serial.println("Monitoring.");
    uint8_t mppt = readMppt();
    Serial.print("mppt: ");
    Serial.println(mppt);

    while (mppt != fanRelay.isOn()) {
//        fanRelay.toggle();
        Serial.print("fan ");
        if (fanRelay.isOff()) {
            Serial.println("activated.");
            fanRelay.on();
        } else {
            Serial.println("deactivated.");
            fanRelay.off();
            status = SLEEP_MODE;
            sleep = millis();
        }
    }
}

/**
 * Sleep mode
 */
void sleepMode() {
    Serial.println("Sleeping.");

    while ((millis() - sleep) < SLEEP_TIME) {
        fanRelay.off();
    }
    status = MONITOR_MODE;
}

/**
 * Manual mode
 */
void manualMode() {
    Serial.print("vent fan relay manually ");
    if (fanRelay.isOn()) {
        Serial.println("deactivated.");
        status = SLEEP_MODE;
        sleep = millis();
    } else {
        Serial.println("activated.");
        status = MONITOR_MODE;
    }
}

/**
 * Check for motion
 */
void checkForMotion() {
    if (HIGH == pirSensor.read()) {
        Serial.println("Motion detected.");
        lightRelay.on();
    } else {
        lightRelay.off();
    }
}

/*******************************************************************************
** MAIN METHODS ****************************************************************
*******************************************************************************/
/**
 * Setup
 */
void setup() {
    Serial.begin(9600);
    while (!Serial);

    /* Set pins ***************************************************************/
    pinMode(MPPT, INPUT);
    pinMode(LED, OUTPUT);
    pinMode(CS, OUTPUT);

    Serial.println("RaggedPi Project Codename Basil Initializing...");
    
    /* Relay ******************************************************************/
    Serial.print("Initializing relays...");
    fanRelay.begin();
    Serial.println("[OK]");
    delay(600);
    /* Motion Sensor **********************************************************/
    Serial.print("Initializing motion sensor...");
    pirSensor.begin();
    Serial.println("[OK]");
    delay(600);
    /* System *****************************************************************/
    Serial.println("System initialized.");
    delay(600);
}

/**
 * Loop
 */
void loop() {
    checkForMotion();
    switch(status) {
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
    delay(10000);
}