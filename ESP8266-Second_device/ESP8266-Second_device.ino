#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

// Arduino Json
#define SAMPLES 20;
// dont forget to change to something else before up loading to GitHub
#define AP_SSID "bluebear"
// dont forget to change to something else before up loading to GitHub
#define AP_PASS "92WdVejL@3d#" 

// Which Serialport to connect to
#define SoftwareSerial_TX 1
#define SoftwareSerial_RX 3
#define BAUD_RATE 4800 // BAUD Rate for serial connection to another ESP8266


SoftwareSerial serial(SoftwareSerial_TX, SoftwareSerial_RX);

int data;

int serialPortSetup(){
  if(!serial) {
    Serial.println("pin config error: Invalid software serial pin");
    return 201;
    }
  else {
    return 1;
    }
  }


 int serialWriteNum(int num){
    if (serial.available()){
      serial.write(num);
      return 1;
    }
  else{
    Serial.println("SerialWriteNum: availablity error");
    return 202;
    }
  }


int serialRead(){
  
  }


int wifiSetup(){
  
  }

  
void setup() {
  
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(9600);
  // int portcheck = serialPortSetup();
  serial.begin(BAUD_RATE); //  SWSERIAL_8N1, SoftwareSerial_RX, SoftwareSerial_TX, false
}

void loop() {
  // put your main code here, to run repeatedly:
  //serialWriteNum(101);
  if (serial.available()) {
    data = serial.read();
    Serial.println(data);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  else {
    digitalWrite(BUILTIN_LED, LOW);
  }
  delay(1000);
}


  
