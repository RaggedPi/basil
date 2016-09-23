/*
  RaggedPi Project
  Arduino 4 "basil"
  Written by david durost <david.durost@gmail.com>
*/

#include <Relay.h>
#include <TimeLib.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <SPI.h>
#include <SD.h>

#define MPPT 2      // digital pin
#define LED 13      // led pin
#define CHIPSELECT 10
#define ECHO_TO_SERIAL

Relay fanRelay(RELAY1, LOW);

File logfile;
 
void error(char *str)
{
    Serial.print("error: ");
    Serial.println(str);

    digitalWrite(LED, HIGH);

    while(1);
}

void setup()
{
    Serial.begin(9600);
    Serial.println("RaggedPi Project Codename Basil Initialized.");
  
    setSyncProvider(RTC.get);
    if(timeStatus()!= timeSet) {
        Serial.println("Unable to sync with the RTC");
    } else {
        Serial.println("RTC has set the system time");
    }

    Serial.print("Initializing SD card...");
    if (!SD.begin(CHIPSELECT)) {
        Serial.println("Card failed, or missing.");
        return;
    }
    Serial.println("Card initialized.");

    char filename[] = "LOGGER00.CSV";
    for (uint8_t i = 0; i < 100; i++) {
        filename[6] = i/10 + '0';
        filename[7] = i%10 + '0';
        if (!SD.exists(filename)) {
            logfile = SD.open(filename, FILE_WRITE); 
            break;
        }
    }

    if (! logfile) {
        error("logfile creation failed.");
    }

    Serial.print("Logging to: ");
    Serial.println(filename);
    
     Wire.begin();  
    
    logfile.println("millis,time,light,temp");    
    #ifdef ECHO_TO_SERIAL
    Serial.println("millis,time,light,temp");
    #endif

    fanRelay.begin();

    pinMode(MPPT, INPUT);
    pinMode(LED, OUTPUT);
    pinMode(CHIPSELECT, OUTPUT);

    Serial.print("Fan Relay: ");
    Serial.println(fanRelay.state());
    Serial.print("MPPT Input State: ");
    Serial.println(digitalRead(MPPT));
}

void loop() {
    time_t now;
    uint8_t mppt = digitalRead(MPPT);

    // log ms since start
    uint32_t m = millis();
    logfile.print(m);         // ms since start
    logfile.print(", ");    
    #ifdef ECHO_TO_SERIAL
    Serial.print(m);         // ms since start
    Serial.print(", ");  
    #endif
 
    now = RTC.get();

    if ((HIGH==mppt && fanRelay.isOff()) || (LOW==mppt && fanRelay.isOn())){
        fanRelay.toggle();
        if (fanRelay.isOn()) {
            logfile.print(now);
            logfile.println(" Fan turned on.");
            #ifdef ECHO_TO_SERIAL  
            Serial.print("[LOG][");
            Serial.print(now);
            Serial.println("] Fan turned on.");
            #endif
        } else {
            logfile.print(now);
            logfile.println(" Fan turned off.");
            #ifdef ECHO_TO_SERIAL  
            Serial.print("[LOG][");
            Serial.print(now);
            Serial.println("] Fan turned off.");
            #endif
        }

        logfile.close();
    }
}