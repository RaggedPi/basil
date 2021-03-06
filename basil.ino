/*
  RaggedPi Project
  Arduino 3 "Basil" - MPPT Vent Controller
  Written by david durost <david.durost@gmail.com>
*/
/* Includes */
#include <SoftwareSerial.h>     // Serial library
#include <Relay.h>              // Relay library
#include <OneButton.h>          // OneButton library

/* Constants */
// Misc
#define SDA 2                           // sda
#define SDL 3                           // sdl
#define CS 10                           // chipselect
#define LED 13                          // led pin
// Pins
#define MPPT_INPUT_PIN 4                // digital pin
#define BUTTON_PIN 5                    // digital pin
// Settings
#define READ_WAIT_TIME 60000            // ms

/* Variables */
Relay fan(RELAY3, HIGH);                // relay
OneButton button(BUTTON_PIN, true);     // button
bool override[2] = { false, false };    // overrides
unsigned long lastMpptReadTime = 0;     // ms


/* Methods */
/**
 * Fan on
 * @return {[type]} [description]
 */
void fanOn() {
    override[OFF] = false;
    override[ON] = true;    
}

/**
 * Fan off
 * @return {[type]} [description]
 */
void fanOff() {
    override[ON] = false;
    override[OFF] = true;
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

    button.attachClick(fanOn);
    button.attachDoubleClick(fanOff);
    fan.begin();

    delay(600);
}
/**
 * Loop
 */
void loop() {
    button.tick();
    if ((millis() - lastMpptReadTime) >= READ_WAIT_TIME) {
        if ( fan.isOff() && ( override[ON] || ( 0 > digitalRead(MPPT_INPUT_PIN) ) ) )    fan.on();
        if ( fan.isOn() && ( override[OFF] || ( 0 > digitalRead(MPPT_INPUT_PIN) ) ) )    fan.off();      
        lastMpptReadTime = millis();
    }
    delay(5000);
}
