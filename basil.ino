/*
  RaggedPi Project
  Arduino 3 "Basil" - MPPT Vent Controller
  Written by david durost <david.durost@gmail.com>
*/
#include <SoftwareSerial.h>     // Serial library
#include <TimeLib.h>            // Time library
#include <Wire.h>               // One Wire library
#include <DS1307RTC.h>          // Realtime Clock library
#include <SPI.h>                // SPI library
#include <SD.h>                 // SD library
#include <Relay.h>              // Relay library
#include <DHT.h>

// Debugging
// #define DEBUG                   // uncomment for debugger
//#define XBEE                    // uncomment to enable xbee transmission
#define LOG                     // uncomment to enable datalogging
#define SERIAL                  // uncomment to enable serial output
// Constants
#define SDA 2                   // sda
#define SDL 3                   // sdl
#define CS 10                   // chipselect
#define LED 13                  // led pin
#define MPPT 4                  // digital pin
#define DHTPIN 5                // digital pin
#define DHTFAHRENHEIT true      // Fahrenheit (true) or Celsius (false)

Relay fanRelay(RELAY1, LOW);    // relay
DHT dht(DHTPIN, DHT11);         // dht11
File logfile;                   // log file
SoftwareSerial XBee(SDA, SDL);  // XBee2
char* msg;                      // reused variable
tmElements_t rtc;               // RTC
bool timerStatus = false;   
uint8_t startTime, endTime;
const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

typedef struct {
    uint8_t month, week, day, hour, minute, second;
} timerResults;

void startTimer() {
    timerStatus = true;
    startTime = millis();
}

void stopTimer() {
    if (timeStatus) {
        timerStatus = false;
        endTime = millis();
    }
}

char* calculateTimerResults(uint8_t timespan, bool verbose = false) {
    timerResults timerResults;
    int x = 0;
    while (timespan % 2628002880) {
        x++;
        timespan -= 2628002880;
    } timerResults.month = x;
    x = 0;
    while (timespan % 604800000) {
        x++;
        timespan -= 604800000;
    } timerResults.week = x;
    x = 0;
    while (timespan % 86400000) {
        x++;
        timespan -= 86400000;
    } timerResults.day = x;
    x = 0;
    while (timespan % 3600000) {
        x++;
        timespan -= 3600000;
    } timerResults.hour = x;
    x = 0;
    while (timespan % 60000) {
        x++;
        timespan -= 60000;
    } timerResults.minute = x;
    x = 0;
    while (timespan % 1000) {
        x++;
        timespan -= 1000;
    } timerResults.second = x;
    x = 0;
    if (verbose || (0 < timerResults.month)) {
        sprintf(msg, "%d months", timerResults.month);
    }
    if (verbose || (0 < timerResults.week)) {
        sprintf(msg, "%d weeks", timerResults.week);
    }
    if (verbose || (0 < timerResults.day)) {
        sprintf(msg, "%d days", timerResults.day);
    }
    if (verbose || (0 < timerResults.week)) {
        sprintf(msg, "%d hours", timerResults.hour);
    }
    if (verbose || (0 < timerResults.minute)) {
        sprintf(msg, "%d minutes", timerResults.minute);
    }
    if (verbose || (0 < timerResults.second)) {
        sprintf(msg, "%d seconds", timerResults.second);
    }
    return msg;
}

uint8_t getTimerResults() {
    stopTimer();
    return (endTime - startTime);
}

/**
 * Set time from a string
 * @param  const char *str
 * @return bool
 */
bool setTimeFromString(const char *str) {
  int Hour, Min, Sec;

  if (sscanf(str, "%d:%d:%d", &Hour, &Min, &Sec) != 3) return false;
  rtc.Hour = Hour;
  rtc.Minute = Min;
  rtc.Second = Sec;
  return true;
}

/**
 * Set date from a string
 * @Param const char *str
 * @return bool
 */
bool setDateFromString(const char *str)
{
  char Month[12];
  int Day, Year;
  uint8_t monthIndex;

  if (sscanf(str, "%s %d %d", Month, &Day, &Year) != 3) return false;
  for (monthIndex = 0; monthIndex < 12; monthIndex++) {
    if (strcmp(Month, monthName[monthIndex]) == 0) break;
  }
  if (monthIndex >= 12) return false;
  rtc.Day = Day;
  rtc.Month = monthIndex + 1;
  rtc.Year = CalendarYrToTm(Year);
  return true;
}

/**
 * Get datetime string
 * @param  null|tmElements_t tm
 * @param  char* fmt
 * @return char*
 */
char* getDateTimeString(tmElements_t tm = rtc, char* fmt = NULL) {   
    sprintf(msg, "%d/%d/%d %d:%d:%d", tm.Month, tm.Day, tmYearToCalendar(tm.Year), tm.Hour, tm.Minute, tm.Second);
    return msg;
}

/**
 * Print
 * @param  char *str
 * @param  bool xbee
 * @param  bool serial
 */
void print(char *str, bool log=true, bool xbee=true, bool serial=true) {
    if (serial) {
        #ifdef SERIAL
        Serial.print(str);
        #endif
    }
    if (xbee) {
        #ifdef XBEE
        XBee.print(str);
        #endif
    }
    if (log) {
        #ifdef LOG
        logfile.print(str);
        #endif
    }
}

/**
 * Print line
 * @param  char *str
 * @param  bool xbee
 * @param  bool serial
 */
void println(char *str=NULL, bool log=true, bool xbee=true, bool serial=true) {
    print(str, log, xbee, serial);
    print("\n", log, xbee, serial);
}

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
 * Error
 * @param  char *str
 * @param  bool halt
 */
void error(char* str, bool halt=true) {
    blinkLED(LED);
    sprintf(str, "[ERROR] %s", str);
    println(str);
    if (halt)   while(1);
}

/**
 * Warning
 * @param  char *str
 */
void warning(char *str) {
    blinkLED(LED, 5);
    sprintf(str, "[WARNING] %s", str);
    println(str);
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
 * Setup
 */
void setup() {
    bool parse=false;
    bool config=false;

    Serial.begin(9600);
    while (!Serial);

    #ifdef XBEE
    XBee.begin(9600);
    #endif
    print("RaggedPi Project Codename Cloves Initializing");
    dotdotdot();
    println();
    
    // SPI
    SPI.begin();

    // RTC 
    print("Initializing RTC");
    dotdotdot(5);
    
    if (setDateFromString(__DATE__) && setTimeFromString(__TIME__)) {
        parse = true;
        if (RTC.write(rtc)) {
            config = true;
        }
    }
    delay(200);
    if (parse && config) {
    } else if (parse) {
        println("Failed");
        error("DS1307 Communication Error");
    } else {
        println("Failed");
        sprintf(msg, "Could not parse info from the compiler, Time=\"%s\", Date=\"%s\"", __TIME__, __DATE__);
        error(msg);
    }
    println("Initialized.");

    // SD
    print("Initializing SD card");
    dotdotdot(10, ".", 100, true);
    if (!SD.begin(CS))  error("SD card failed or is missing");
    println("Initialized.");

    // Logging
    #ifdef LOG
    char filename[] = "LOG00.csv";
    for (uint8_t i = 0; i < 100; i++) {
        filename[3] = i / 10 + '0';
        filename[4] = i % 10 + '0';
        if (!SD.exists(filename)) {
            logfile = SD.open(filename, FILE_WRITE);
            break;
        }
    }
    if (!logfile)   error("log file creation failed.  System will continue.", false);
    else {
        sprintf(msg, "Logfile: %s", filename);
        println(msg);
    }
    #endif

    // 1 WIRE
    Wire.begin();

    // Relay
    fanRelay.begin();

    // Set pins
    pinMode(MPPT, INPUT);
    pinMode(DHTPIN, INPUT);
    pinMode(LED, OUTPUT);
    pinMode(CS, OUTPUT);
    println("System initialized.");
}

/**
 * Loop
 */
void loop() {
    RTC.read(rtc);
    uint8_t mppt = digitalRead(MPPT);

    // Read DHT data (~250ms)
    float temp = dht.readTemperature(DHTFAHRENHEIT);
    if (isnan(temp))   warning("Failed to read from DHT sensor! (t)");   

    // Fan relay
    if ((HIGH == mppt && fanRelay.isOff()) || (LOW == mppt && fanRelay.isOn())) {
        fanRelay.toggle();
        sprintf(msg, "%s [%s] - fan", getDateTimeString(), temp);
        if (fanRelay.isOn()) {
            sprintf(msg,"%s %s", msg, "activated");
            startTimer();
        } else {
            stopTimer();
            sprintf(msg, "%s %s [%s]", msg, "deactivated", calculateTimerResults(endTime - startTime));
        }
        println(msg);
        delay(3000);
        logfile.close();
    }
}
