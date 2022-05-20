// Title: ESP8266_AD8232_MicroSD_RTC.ino
// Description: A portable ECG device with WiFi, MicroSD, and a RTC
// Tasks:
// 1) Wire MicroSD
// 2) Wire RTC
// 3) Intergrate MicroSD
// 4) Intergrate RTC
// 5) Create Client-Server
// 6) Create JS/Python Data Dashboard
// 7) Clean up code
// 8) Fix NTP Time duplicates problem (need miliseconds) (Possilbly with the RTC once It comes in
// 9) Create Database and design Data logging abilities
// 10) Send Serialized JSON to file

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <RTClib.h>


#define HTTP_REST_PORT 80 // Set port to 80
#define WIFI_RETRY_DELAY 500 //WiFi Retry Delay
#define MAX_WIFI_INIT_RETRY 50 //Limit for WiFi Connection Atempts

// SD card
#define CS_PIN D8

// Global Variables
// AD8232 ECG Module
uint8_t ecg_high = 5;
uint8_t ecg_low = 4;

//WebServer
// On-board LED Pin
uint8_t pin_led = 16;

// RTC Module
RTC_DS3231 rtc;
const byte interruptPin = D3;
volatile unsigned long start_sec;
int centi_mills;

// WiFi info
const char* wifi_ssid = "bluebear"; // Write here your router's username
const char* wifi_password = "92WdVejL@3d#"; // Write here your router's passward

// NTP server to request epoch time
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
// Variable to save current epoch time
unsigned long epochTime;

// SD Card Variables
File root;

void update_sec_start() {
    start_sec = millis();
  }

void rtcSetup() {
  #ifndef ESP8266
    while (!Serial);
  #endif

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
    }

    if (rtc.lostPower()) {
      Serial.println("RTC lost Power, Let's set the time!");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      }
  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));

  // Setup Milliseconds
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(0, update_sec_start, FALLING); // or RISING
  }

void rtcSetNow(){
  
  }

void initWiFi() {
  int retries = 0;

  Serial.println("Connecting to WiFi AP..........");

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  // check the status of WiFi connection to be WL_CONNECTED
  while ((WiFi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
    retries++;
    delay(WIFI_RETRY_DELAY);
    Serial.print("#");
  }
  //print a new line, then print WiFi connected and the IP address
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
  //return WiFi.status(); // return the WiFi connection status
}


void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}


void sdSetup() {
  pinMode(10, OUTPUT);
  SD.begin(CS_PIN);
  if (!SD.begin(CS_PIN)) {
    Serial.println("Failed, check if the card is present.");
  }
  else {
    Serial.println("SD Card Initialized");
  }
  root = SD.open("/");
  printDirectory(root, 0);
}


int sdRead(String fileName) {
  File dataFile = SD.open(fileName, FILE_READ);
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
    return 1;
  }
  else {
    Serial.println("error opening.txt");
    dataFile
    .close();
    return 0;
  }
}

int sdWrite(String fileName, String reading) {
  File dataFile = SD.open(fileName, FILE_WRITE);
  // if the file was opened correctly, write the data to it
  if (dataFile) {
    Serial.println("The file was opened successfully.");
    dataFile.println(reading); // writing line by line
    Serial.println("Writing sucsessful!");
    // Close the file
    dataFile.close();
    Serial.println("File Closed");
    delay(500);
    return 1;
  }
  else {
    // if the file could not be opened the data will not be written.
    Serial.println("Failed to open LOG.txt file");
    delay(500);
    dataFile.close();
    return 0;
  }
}


int sdExists(String fileName) {
  if (SD.exists(fileName)) {
    return 1;
  }
  else {
    return 0;
  }
}


int sdFileDelete(String fileName) {
  if (sdExists(fileName)) {
    SD.remove(fileName);
    Serial.println("file: " + fileName + " has been deleted");
    return 1;
  }
  else {
    Serial.println("file: " + fileName + " Does not Exist");
    return 0;
  }
}


int ecgLeadCheck() {
  delay(1);
  if ((digitalRead(ecg_high)) == 1 || (digitalRead(ecg_low) == 1)) {
    return 0;
  }
  else {
    return analogRead(A0);
  }
}


String getReading() {
  timeClient.update();
  return timeClient.getFormattedTime() + "," + ecgLeadCheck();
}


unsigned long getTime() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}



void setup() {
  // set onboard led function
  pinMode(pin_led, OUTPUT);

  // WiFi parameters to be configured
  Serial.begin(9600);

  // Initialize to WiFi
  initWiFi();

  // ECG AD8232 Setup
  // Setup AD8232 to 8266 pins
  pinMode(ecg_high, INPUT); // Setup for leads off detection LO +
  pinMode(ecg_low, INPUT); // Setup for leads off detection LO -


  // Time Library
  timeClient.begin();

  //RTC Module Setup
  rtcSetup();
  // SD Card Setup
  sdSetup();

  // Begin Serial Connection to Second ESP8266
  //serial.begin(BAUD_RATE, SWSERIAL_8N1, SoftwareSerial_RX, SoftwareSerial_TX, false); //, 95
  // Real Time Clock Setup

}


void loop() {
  // Datetime Keeper
  char buf1[20];
  DateTime now = rtc.now();
   
  //Updated now.day to now.date
  //sprintf(buf1, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year()); 
  // String str = String(now.year(), DEC) + '/' + String(now.month(), DEC) + '/' + String(now.day(), DEC) + " " + String(now.hour(), DEC) + ':' + String(now.minute(), DEC) + ':' + String(now.second(), DEC);
  centi_mills = (millis()-start_sec);
  Serial.print(F("Date/Time: "));
  //Serial.println(centi_mills);
  //sdWrite("LOG.txt", getReading());
  //sdRead("LOG.txt");
  
  //sdFileDelete("LOG.txt");
  delay(5);
}
