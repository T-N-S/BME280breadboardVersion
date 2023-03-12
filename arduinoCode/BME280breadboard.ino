/* ============================================

    Sketch for PCB NaWiSchool
    Output on SD-Card (data logger)
    [This code is used to read and save the data from BME280 sensor and GPS module]
    Author: T. Schumann
    Date: 2021-10-09

    Dependencies:
    TinyGPS++ - https://github.com/neosarchizo/TinyGPS
    Adafruit_Sensor - https://github.com/adafruit/Adafruit_Sensor
    Adafruit_BME380 - https://github.com/adafruit/Adafruit_BME280_Library
    SSD1306AsciiWire - https://github.com/greiman/SSD1306Ascii

    All rights reserved. Copyright Tim Schumann 2021
  ===============================================
*/
#include <SoftwareSerial.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <TinyGPS++.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <SSD1306AsciiWire.h>

//#define ENLOG

byte d = 0; //delay counter
#define PowerLed 7
#define GPSLed 6

bool blinkState = false;
byte failflag = 0;
char filename[] = "LOGGER00.CSV";


TinyGPSPlus gps;

SoftwareSerial ss(3, 2);

Adafruit_BME280 bme; // I2C

SSD1306AsciiWire oled;


void setup() {
  Wire.begin();
#ifdef ENLOG
  Serial.begin(115200);
#endif
  pinMode(GPSLed, OUTPUT);
  pinMode(PowerLed, OUTPUT);
  digitalWrite(PowerLed, HIGH);

  ss.begin(4800);

  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(Adafruit5x7);
  oled.set2X();

#ifdef ENLOG
  Serial.println("BME280 PCB By Tim Schumann and Maximillian Kalff");
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
      logfile.println("CPU time in ms, lat, lng, alt, hh:mm:ss, temp, press, hum");
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
}

void loop() {
  while (ss.available() > 0)
    gps.encode(ss.read());
  delay(10);
  if (d > 100) {
    sdwrite();
    d = 0;
  }
  d++;

}

//---------write data to sd-----------
void sdwrite() {

  File logfile = SD.open(filename, FILE_WRITE);
  // if the file is available, write to it:
  if (logfile) {
    logfile.print(millis());
    logfile.print(", ");
    logfile.print(String(gps.location.lat(), 6));
    logfile.print(", ");
    logfile.print(String(gps.location.lng(), 6));
    logfile.print(", ");
    logfile.print(gps.altitude.meters());
    logfile.print(", ");
    logfile.print(gps.time.hour());
    logfile.print(":");
    logfile.print(gps.time.minute());
    logfile.print(":");
    logfile.print(gps.time.second());
    logfile.print(", ");
    logfile.print(bme.readTemperature());
    logfile.print(", ");
    logfile.print(bme.readPressure() / 100.0F);
    logfile.print(", ");
    logfile.println(bme.readHumidity());
    logfile.close();
  }

  oled.clear(100, 130, 0, 6);
  oled.setCursor(5, 0);
  oled.print("T: ");
  oled.setCursor(40, 0);
  oled.print(bme.readTemperature());
  oled.print(" C");
  oled.setCursor(5, 4);
  oled.print("H: ");
  oled.setCursor(40, 4);
  oled.print(bme.readHumidity());
  oled.print(" %");
  oled.setCursor(5,6);
  oled.print("Sat.: ");
  oled.print(gps.satellites.value());

  if (gps.altitude.meters() && bme.readTemperature() != 0) {
    blinkState = !blinkState;
    digitalWrite(GPSLed, blinkState);
  }
  else {
    digitalWrite(GPSLed, LOW);
  }
}

//------Error Message--------
void fail(int i) {
  failflag = 1;
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
  }
  oled.println("   !!!!");
  while (1) {   //output card failed
    digitalWrite(PowerLed, HIGH);
    delay(500);
    digitalWrite(PowerLed, LOW);
    delay(100);
  }
}
