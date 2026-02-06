/*
  GPS and Environment Tracker
  Copyright 2026 Milo and Solomon, with help from Josh and Ben

  Current pin assignments are for the Wemos Lolin32 Lite (esp32) devlopment board
*/

///// LIBRARIES /////
#include <TinyGPSPlus.h> // TinyGPSPlus-esp32 by Mikal Hart
#include <AHT20.h> // AHT20 by dvarrel
#include <BMP280.h> // BMP280 by dvarrel
#include <Preferences.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"


///// PIN ASSIGNMENTS /////
// Built-in
#define PIN_LED        22

#define PIN_GPS_TX     18

#define PIN_I2C_SCL    17 
#define PIN_I2C_SDA    16

#define PIN_LORA_RX    19
#define PIN_LORA_TX    23 

#define PIN_SD_CS   14 // CS aka SS aka SDA
#define PIN_SD_SCK  13
#define PIN_SD_MISO 15
#define PIN_SD_MOSI 27


///// GLOBAL VARIABLES /////
TinyGPSPlus gps;
AHT20 aht20;
BMP280 bmp280;
unsigned long last_output;
char log_buffer[256]; // each line of the log output will get stored here before it 

SPIClass *hspi = nullptr; // SD card on HSPI interface
File log_file; // file handle for writing our csv logs
int log_file_ok = 0;

int lora_ok = 0;


///// ARDUINO ONE-TIME SETUP /////
void setup() {
  Serial.begin(115200); // connection between esp32 and the computer
  Serial1.begin(9600, SERIAL_8N1, PIN_GPS_TX); // connection between esp32 and the GPS module
  delay(2000);
  Serial.printf("\n\nLong live Bob II!\n");

  // SD card on HSPI interface
  //pinMode(PIN_SD_MISO, INPUT_PULLUP);
  hspi = new SPIClass(HSPI);
  hspi->begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (SD.begin(PIN_SD_CS, *hspi, 20000000)) {
    if (SD.cardType() != CARD_NONE) {
      Serial.printf("SD Card Present! %lluMB (%.2f%%) used of %lluMB available space\n", (SD.usedBytes()/1048576), ((float)SD.usedBytes()/(float)SD.totalBytes()), (SD.totalBytes()/1048576));

      // Increment a variable in prefs (esp32 nvram) to give us a unique log file every time we reboot
      Preferences prefs;                         // NVS handler
      prefs.begin("myapp", false);               // Open namespace "myapp", RW mode
      int counter = prefs.getInt("counter", 0);  // Read current value (default 0 if not found)
      prefs.putInt("counter", counter+1);  // Increment it and save it back
      prefs.end();

      // Temporarily use the log buffer to store the name of the log file we're about to set up, and create the file
      sprintf(log_buffer, "/log_%05d.csv", counter);
      log_file = SD.open(log_buffer, FILE_WRITE);
      if (log_file) {
        Serial.printf("Logging to [%s]\n", log_buffer);
        log_file.printf("GPS Timestamp, GPS Latitude, GPS Longitude, GPS Altitude, Satellites, AHT20 Temperature, AHT20 Humidity, BMP280 Temperature, BMP280 Pressure, BMP280 Altitude\n"); // CSV header row
        log_file_ok = 1; // Now it's finally safe to start logging to the file we just tried to create on the SD card

      } else Serial.printf("ERROR: Failed to create log file [%s]\n", log_buffer);
    } else Serial.println("ERROR: No SD card present or invalid format!");
  } else Serial.println("ERROR: SD card initialization failure!");

  // Initialize the i2c bus and all the sensors attached to it
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 800000);
  if (aht20.begin()) Serial.printf("AHT20 OK\n");
  else Serial.printf("ERROR:  Failed to initialize AHT20\n");
  if (bmp280.begin() == BMP280::eStatusOK) Serial.printf("BMP280 OK\n");
  else Serial.printf("ERROR:  Failed to initialize BMP280\n");

  // Initialize the GPIO pin controlling the built-in LED
  pinMode(PIN_LED, OUTPUT);

  // Initialize any other global variables we need to initialize
  last_output = millis();
  Serial.printf("Initialization complete\n\n");
  Serial.printf("GPS Timestamp, GPS Latitude, GPS Longitude, GPS Altitude, Satellites, AHT20 Temperature, AHT20 Humidity, BMP280 Temperature, BMP280 Pressure, BMP280 Altitude\n"); // CSV header row
}

///// ARDUINO CONTINUOUS LOOP /////
void loop() {
  // We may receive bytes from the GPS module at any time
  if (Serial1.available() > 0) gps.encode(Serial1.read());

  // Once every second, check the sensors and output formatted data
  if ((millis() - last_output) > 1000) {
    digitalWrite(PIN_LED, LOW); // Turn the built-in LED on for the duration of the loop

    

    // Take our measurements and assemble all our data into one big line of CSV data.
    // We could store each of these in its own variable if we needed to do anything with them
    // FORMAT: GPS Timestamp, GPS Latitude, GPS Longitude, GPS Altitude (m), Satellites, AHT20 Temperature (c), AHT20 Humidity (%), BMP280 Temperature (c), BMP280 Pressure (pascals), BMP280 Altitude (m)
    sprintf(log_buffer, "%02d/%02d/%02d %02d:%02d:%02d, %.6f, %.6f, %.6f, %d, %.2f, %.2f, %.2f, %ld, %d\n",
      gps.date.month(), gps.date.day(), gps.date.year(), gps.time.hour(), gps.time.minute(), gps.time.second(),
      gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.satellites.value(),
      aht20.getTemperature(), aht20.getHumidity(),
      bmp280.getTemperature(), bmp280.getPressure(), bmp280.calAltitude(bmp280.getPressure())
    );
    
    // Debug/live output to USB serial port
    Serial.printf(log_buffer);

    // Output to SD card if it's attached
    if (log_file_ok) {
      log_file.write((const unsigned char *)log_buffer, strlen(log_buffer));
      log_file.flush();
    }

    // Output to LORA Radio if it's attached
    if (lora_ok) {
    }

    last_output = millis();
    digitalWrite(PIN_LED, HIGH);
  }
}
