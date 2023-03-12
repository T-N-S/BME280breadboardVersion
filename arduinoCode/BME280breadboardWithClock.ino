/* ============================================

    Sketch for Breadboard with Clock NaWiSchool
    Output on SD-Card (data logger)
    [This code is used to read and save the data from BME280 sensor and Clock]
    Author: T. Schumann
    Date: 2022-01-11

    Dependencies:
    Adafruit_Sensor - https://github.com/adafruit/Adafruit_Sensor
    Adafruit_BME380 - https://github.com/adafruit/Adafruit_BME280_Library
    SSD1306AsciiWire - https://github.com/greiman/SSD1306Ascii
    RTClib - https://github.com/adafruit/RTClib
    
    All rights reserved. Copyright Tim Schumann 2021
  ===============================================
*/

#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SSD1306AsciiWire.h>
#include <RTClib.h>

//#define ENLOG

long millisSave = 0;  //variable to save last processor time 
long everykMillis = 2000; //delay counter

char filename[] = "LOGGER00.CSV";

// Create a Clock object
RTC_DS3231 rtc;

// Create a BME280 object
Adafruit_BME280 bme; 

// Create a OLED object
SSD1306AsciiWire oled;


void setup() {
  Wire.begin();
#ifdef ENLOG
  Serial.begin(115200);
#endif

  //start OLED
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(Adafruit5x7);
  oled.set2X();

#ifdef ENLOG
  Serial.println("BME280 with Clock By Tim Schumann");
  Serial.println();
#endif

  if (!SD.begin(10))
  {
    fail(1);
  }
  for (uint8_t i = 0; i < 100; i++) {     //create new file
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)) {
      File logfile = SD.open(filename, FILE_WRITE);
      logfile.println("CPU time in ms, year, month, day, hh:mm:ss, temp, press, hum");
      logfile.close();
      break; 
    }
  }
#ifdef ENLOG
  Serial.print("Logging to: ");
  Serial.println(filename);
#endif

  unsigned status;
  status = bme.begin(0x76, &Wire);
  if (!status) {
    fail(0);
  }
  if (! rtc.begin()) {
    fail(3);
  }
  if (rtc.lostPower()) {
#ifdef ENLOG
    Serial.println("RTC lost power, let's set the time!");
#endif
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  millisSave=millis();
}

void loop() {
  if (millis() > millisSave + everykMillis) { //safe values every 100 loops
    sdwrite();
    millisSave=millis();
  }
  delay(10);
}

//---------write data to sd-----------
void sdwrite() {

  DateTime now = rtc.now();
  File logfile = SD.open(filename, FILE_WRITE);
  // if the file is available, write to it:
  if (logfile) {
    logfile.print(millis());
    logfile.print(", ");
    logfile.print(now.year(),DEC);
    logfile.print(", ");
    logfile.print(now.month(),DEC);
    logfile.print(", ");
    logfile.print(now.day(),DEC);
    logfile.print(", ");
    logfile.print(now.hour());
    logfile.print(":");
    logfile.print(now.minute());
    logfile.print(":");
    logfile.print(now.second());
    logfile.print(", ");
    logfile.print(bme.readTemperature());
    logfile.print(", ");
    logfile.print(bme.readPressure() / 100.0F);
    logfile.print(", ");
    logfile.println(bme.readHumidity());
    logfile.close();
  }


  oled.clear(100, 130, 0, 4);
  oled.setCursor(5, 0);
  oled.print("T: ");
  oled.setCursor(40, 0);
  oled.print(bme.readTemperature());
  oled.print(" C");
  oled.setCursor(5, 2);
  oled.print("H: ");
  oled.setCursor(40, 2);
  oled.print(bme.readHumidity());
  oled.print(" %");
  oled.setCursor(5, 6);
  oled.print("t:");
  oled.print(now.hour());
  oled.print(":");
  oled.print(now.minute());
  oled.print(":");
  oled.print(now.second());
}

//------Error Message--------
void fail(int i) {
  oled.print("\n ");
  switch (i) {
    case 0:
#ifdef ENLOG
      Serial.println("BME-ERROR!");
#endif
      oled.println("BME-ERROR");
      break;
    case 1:
#ifdef ENLOG
      Serial.println("Card failed, or not present");
#endif
      oled.println("SD-Error");
      break;
    case 2:
#ifdef ENLOG
      Serial.println("Clock failed, or not present");
#endif
      oled.println("Clock-Error");
      break;

  }
  oled.println("   !!!!");
  while (1){}
}
