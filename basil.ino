
#include <Relay.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

#define MPPT 4
#define LED 13
#define CS 10
#define XBEE
#define SDA 2
#define SDL 3

Relay fanRelay(RELAY1, LOW);
File logFile;
SoftwareSerial XBee(SDA, SDL);

/**
 * Blink LED
 * @param  uint8_t pin
 * @param  int     times
 * @param  int     delay
 */
void blinkLED(uint8_t pin, int times=10, int wait=500) {
    for (int i=0; i < times; i++) {
        digitalWrite(pin, HIGH);
        delay(wait);
        digitalWrite(pin, LOW);
        if (times > (i + 1))    delay(wait);
    }
}

/**
 * Print
 * @param  char *str
 * @param  bool xbee
 * @param  bool serial
 */
void print(char *str, bool xbee=true, bool serial=true) {
    if (serial) {
        Serial.print(str);
    }
    #ifdef XBEE
    if (xbee) {
        XBee.print(str);
    }
    #endif
}

/**
 * Print line
 * @param  char *str
 * @param  bool xbee
 * @param  bool serial
 */
void println(char *str, bool xbee=true, bool serial=true) {
    print(str, xbee, serial);
    print("\n", xbee, serial);
}

/**
 * Dot dot dot
 * @param  uint8_t dot_total
 * @param  char dot
 * @param  uint8_t ms_per_dot
 */
void dotdotdot(uint8_t dot_total=10, char* dot=".", uint8_t ms_per_dot=100, bool newline=false) {
    for (int x = 0; x < dot_total; x++) {
        print(dot, false);
        delay(ms_per_dot);
    }
}

/**
 * Error
 * @param  char *str
 * @param  bool halt
 */
void error(char *str, bool halt=true) {
    blinkLED(LED);
    print("error: ");
    println(str);
    if (halt) {
        println("System halted.");
        while(1);
    }
}

/**
 * Warning
 * @param  char *str
 */
void warning(char *str) {
    blinkLED(LED, 5, 500);
    print("warning: ");
    println(str);
}

/**
 * Setup
 */
void setup() {
    Serial.begin(9600);
    XBee.begin(9600);
    // RTC
    setSyncProvider(RTC.get);
    if (timeStatus() != timeSet)  warning("Unable to sync with RTC");
    else    println("System synced to RTC");
    // SD
    print("Initializing SD card");
    dotdotdot(10, ".", 100, true);
    if (!SD.begin(CS))  error("SD car failed or is missing");
    println("SD card intialized.");
    // Logging
    char filename[] = "LOG00.csv";
    for (uint8_t i = 0; i < 100; i++) {
        filename[3] = i / 10 + '0';
        filename[4] = i % 10 + '0';
        if (!SD.exists(filename)) {
            logFile = SD.open(filename, FILE_WRITE);
            break;
        }
    }
    if (!logFile)   error("logFile creation failed.");
    print("Logging to: ");
    println(filename);
    // 1 WIRE
    Wire.begin();
    // Relay
    fanRelay.begin();
    // Set pins
    pinMode(MPPT, INPUT);
    pinMode(LED, OUTPUT);
    pinMode(CS, OUTPUT);
    println("System initialized.");
}

/**
 * Loop
 */
void loop() {
    time_t now = RTC.get();
    uint8_t mppt = digitalRead(MPPT);

    // Fan relay
    if ((HIGH == mppt && fanRelay.isOff()) || (LOW == mppt && fanRelay.isOn())) {
        fanRelay.toggle();
        if (fanRelay.isOn()) {
            logFile.print(now);
            logFile.println(" - fan activated.");
        } else {
            logFile.print(now);
            logFile.println(" - fan deactivated.");
        }
        delay(3000);
        logFile.close();
    }
}
