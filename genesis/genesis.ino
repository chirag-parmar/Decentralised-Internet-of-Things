#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266mDNS.h>
#include <Crypto.h>
#include <SPI.h>
#include <SD.h>

void setup() {

  SD.begin(4);
  // put your setup code here, to run once:
    SHA256 hasher;
    byte hash[SHA256_SIZE];
    
    String fileName = "";
    String from = "";
    String to = "";
    String amount = "";
    String description = "";
    String prevHash = "" ;
    String curHash;
    
    String compiled = from + to + fileName + amount + description + prevHash;
    char charBuf[200];
    compiled.toCharArray(charBuf, compiled.length());
    hasher.doUpdate(charBuf);
    hasher.doFinal(hash);
    
    for(int j=0;j<32;j++){
      curHash += String(hash[j], HEX);
    }

    StaticJsonBuffer<500> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["from"] = from;
      root["to"] = to;
      root["fileName"] = fileName;
      root["amount"] = amount;
      root["description"] = description;
      root["prevHash"] = prevHash;
      root["curHash"] = curHash;
      
      String groot;
      root.printTo(groot);
      File newFile = SD.open("balance.txt", FILE_WRITE);
      if(newFile){
        newFile.println(groot);
        newFile.close();
      }
      else
        Serial.println("Error opening file..");
}

void loop() {
  // put your main code here, to run repeatedly:

}
