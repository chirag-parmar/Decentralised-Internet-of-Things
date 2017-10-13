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
#define DBG_OUTPUT_PORT Serial
struct coins{
  int iot;
  int iota;
}value;
struct block{
  String from;
  String to;
  String fileName;
  String amount;
  String description;
  String prevHash;
  String curHash;
}blocks[50];
int blockindex = 0;
const char* ssid = "OP3T";
const char* password = "batman1234";
const char* host = "iota";
int balance = 0;
long start;
byte previoushash[32];
ESP8266WebServer server(80);
static bool hasSD = false;
File uploadFile;
const int chipSelect = 4;
void returnOK() {
  server.send(200, "text/plain", "");
}
void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}
bool loadFromSdCard(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SD.open(path.c_str());
  if(dataFile.isDirectory()){
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
  }
  if (!dataFile)
    return false;
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    DBG_OUTPUT_PORT.println("Sent less data than expected!");
  }
  dataFile.close();
  return true;
}

void checkCoins()
{
  File newFile = SD.open("coins.txt",FILE_READ);
    String fileline = newFile.readString();
    StaticJsonBuffer<100> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(fileline);
    DBG_OUTPUT_PORT.println(fileline);
    if (!root.success()) {
      DBG_OUTPUT_PORT.println("JsonParser.parse() failed");
      return;
    }
    value.iot = root["iot"];
    DBG_OUTPUT_PORT.println("IoT's coin count:");
    DBG_OUTPUT_PORT.println(value.iot);
    //value.iot = (value.iot).toInt();
    value.iota = root["iota"];
    DBG_OUTPUT_PORT.println("IoTa's coin count:");
    DBG_OUTPUT_PORT.println(value.iota);
    //value.iota = (value.iota).toInt();
  newFile.close();
}
void updateCoins()
{
      StaticJsonBuffer<100> jsonBuffer;
      JsonObject& root = jsonBuffer.createObject();
      root["iot"] = String(value.iot);
      root["iota"] = String(value.iota);
      String groot;
      root.printTo(groot);
      if(SD.exists("coins.txt"))
        SD.remove("coins.txt");
      File newFile = SD.open("coins.txt", FILE_WRITE);
      if(newFile){
        newFile.println(groot);
        newFile.close();
      }
      else
        DBG_OUTPUT_PORT.println("Error opening file..");
}

void addFile(){
      if(server.args() != 7)                          //Add a new line
        {
          DBG_OUTPUT_PORT.println(server.args());
          server.send(200, "text/plain", "BAD ARG");
          return;
        }
      DBG_OUTPUT_PORT.println(server.args());
      DBG_OUTPUT_PORT.println("Received GET request");
      
      SHA256 hasher;
      byte hash[SHA256_SIZE];
    
      String from = server.arg("from");
      String to = server.arg("to");
      String fileName = server.arg("fileName");
      String amount = server.arg("amount");
      String description = server.arg("description");
      String prevHash = server.arg("prevHash");
      String curHash = server.arg("curHash");
      String compiled = from + to + fileName + amount + description + prevHash;
      String calculatedHash;
      char charBuf[200];
      compiled.toCharArray(charBuf, compiled.length());
      hasher.doUpdate(charBuf);
      hasher.doFinal(hash);
      for(int j=0;j<32;j++){
        calculatedHash += String(hash[j], HEX);
       }
      //check if previous hash equals to the current hash of the previous transaction
      if(prevHash != blocks[blockindex -1].curHash){
        returnFail("invalid prev hash");
        return;
      }
      else if(calculatedHash != curHash){
        returnFail("invalid current hash");
        return;
      }
      updateBlock(from,to,fileName,amount,description,prevHash,curHash);
    
      if(from == "10.1.26.123")
        {
          value.iot += amount.toInt();
          value.iota -= amount.toInt();
        }
        else
        {
          value.iot += amount.toInt();
          value.iota -= amount.toInt();
        }
        updateCoins();
        checkCoins();
      
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
        DBG_OUTPUT_PORT.println("Error opening file..");
      server.send(200,"text/plain", "New data added..");
    }



void buyFile(){
      if(server.args() != 7)                          //Add a new line
        {
          DBG_OUTPUT_PORT.println(server.args());
          server.send(200, "text/plain", "BAD ARG");
          return;
        }
      DBG_OUTPUT_PORT.println(server.args());
      DBG_OUTPUT_PORT.println("Received GET request for buying file");
      
      SHA256 hasher;
      byte hash[SHA256_SIZE];
    
      String from = server.arg("from");
      String to = server.arg("to");
      String fileName = server.arg("fileName");
      String amount = server.arg("amount");
      String description = server.arg("description");
      String prevHash = server.arg("prevHash");
      String curHash = server.arg("curHash");
      String compiled = from + to + fileName + amount + description + prevHash;
      String calculatedHash;
      char charBuf[200];
      compiled.toCharArray(charBuf, compiled.length());
      hasher.doUpdate(charBuf);
      hasher.doFinal(hash);
      for(int j=0;j<32;j++){
        calculatedHash += String(hash[j], HEX);
       }
      //check if previous hash equals to the current hash of the previous transaction
      if(prevHash != blocks[blockindex -1].curHash){
        returnFail("invalid prev hash");
        return;
      }
      else if(calculatedHash != curHash){
        returnFail("invalid current hash");
        return;
      }
   //   updateBlock(from,to,fileName,amount,description,prevHash,curHash);
    
      //ADD COIN TRANSACTION CODE
      String url = "http://" + to + "/" + fileName;      
     // t_httpUpdate_return ret = ESPhttpUpdate.update("https://raw.githubusercontent.com/sureshwaitforitkumar/Basic-Automation-using-NodeMCU/master/blinkESP.bin"); 
    DBG_OUTPUT_PORT.println("Buying file worked...");

      
     /* StaticJsonBuffer<500> jsonBuffer;
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
        DBG_OUTPUT_PORT.println("Error opening file..");
      server.send(200,"text/plain", "Buying new file...");
    */}

    
void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    if(SD.exists((char *)upload.filename.c_str())) return;
    uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
    DBG_OUTPUT_PORT.print("Upload: START, filename: "); DBG_OUTPUT_PORT.println(upload.filename);
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: "); DBG_OUTPUT_PORT.println(upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(uploadFile) uploadFile.close();
    
    SHA256 hasher;
    byte hash[SHA256_SIZE];
    
    DBG_OUTPUT_PORT.print("Upload: END, Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
    String fileName = String(upload.filename.c_str());
    fileName.remove(0,1);
    String from = WiFi.localIP().toString();
    String to = from;
    String amount = "1";
    String description = "getItFromHtml";
    String prevHash = blocks[blockindex -1].curHash; 
    String curHash;
    
    String compiled = from + to + fileName + amount + description + prevHash;
    DBG_OUTPUT_PORT.println(compiled);
    char charBuf[200];
    compiled.toCharArray(charBuf, compiled.length());
    hasher.doUpdate(charBuf);
    hasher.doFinal(hash);
    
        updateCoins();
        checkCoins();
    
    DBG_OUTPUT_PORT.println("CURRENT HASH:");
    for(int j=0;j<32;j++){
      DBG_OUTPUT_PORT.print(hash[j],HEX);
      curHash += String(hash[j], HEX);
      DBG_OUTPUT_PORT.print(" ");
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
        DBG_OUTPUT_PORT.println("Error opening file..");
    
    DBG_OUTPUT_PORT.println("Sending request..");
    updateBlock(from,to,fileName,amount,description,prevHash,curHash);
    GETRequest("/newfile",from,to,fileName,amount,description,prevHash,curHash);
    DBG_OUTPUT_PORT.println("Request sent..");
  }
  
}
void updateBlock(String from_,String to_,String fileName_, String amount_,String description_,String prevHash_,String curHash_){
   blocks[blockindex].from = from_;
   blocks[blockindex].to = to_;
   blocks[blockindex].fileName = fileName_;
   blocks[blockindex].amount = amount_;
   blocks[blockindex].description = description_;
   blocks[blockindex].prevHash = prevHash_;
   blocks[blockindex].curHash = curHash_;
    
   blockindex += 1; 
}
void deleteRecursive(String path){
  File file = SD.open((char *)path.c_str());
  if(!file.isDirectory()){
    file.close();
    SD.remove((char *)path.c_str());
    return;
  }
  file.rewindDirectory();
  while(true) {
    File entry = file.openNextFile();
    if (!entry) break;
    String entryPath = path + "/" +entry.name();
    if(entry.isDirectory()){
      entry.close();
      deleteRecursive(entryPath);
    } else {
      entry.close();
      SD.remove((char *)entryPath.c_str());
    }
    yield();
  }
  SD.rmdir((char *)path.c_str());
  file.close();
}
void handleDelete(){
  if(server.args() == 0) return returnFail("BAD ARGS");
  String path = server.arg(0);
  if(path == "/" || !SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  deleteRecursive(path);
  returnOK();
}
void handleCreate(){
  if(server.args() == 0) return returnFail("BAD ARGS");
  String path = server.arg(0);
  if(path == "/" || SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  if(path.indexOf('.') > 0){
    File file = SD.open((char *)path.c_str(), FILE_WRITE);
    if(file){
      file.write((const char *)0);
      file.close();
    }
  } else {
    SD.mkdir((char *)path.c_str());
  }
  returnOK();
}

void GETRequest(String getPath, String from,String to,String fileName, String amount,String description,String prevHash,String curHash)
{   
    DBG_OUTPUT_PORT.println("The contents of the file are:");
    readFile();
    String ip = "10.1.26.43";
    String port = "80";
    String path = "/newfile?from=" + from + "&to=" + to + "&fileName=" + fileName + "&amount=" + amount + "&description=" + description + "&prevHash=" + prevHash + "&curHash=" + curHash;
    String url = "http://" + ip + ":80" + path;
    Serial.println(url);
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin(url);  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request
    if (httpCode > 0) { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload
    }
    else
      Serial.println(httpCode);
    http.end();   //Close connection
  }
}  
void printDirectory() {
  if(!server.hasArg("dir")) return returnFail("BAD ARGS");
  String path = server.arg("dir");
  if(path != "/" && !SD.exists((char *)path.c_str())) return returnFail("BAD PATH");
  File dir = SD.open((char *)path.c_str());
  path = String();
  if(!dir.isDirectory()){
    dir.close();
    return returnFail("NOT DIR");
  }
  dir.rewindDirectory();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  WiFiClient client = server.client();
  server.sendContent("[");
  for (int cnt = 0; true; ++cnt) {
    File entry = dir.openNextFile();
    if (!entry)
    break;
    String output;
    if (cnt > 0)
      output = ',';
    output += "{\"type\":\"";
    output += (entry.isDirectory()) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += entry.name();
    output += "\"";
    output += "}";
    server.sendContent(output);
    entry.close();
 }
 server.sendContent("]");
 dir.close();
}
void handleNotFound(){
  if(hasSD && loadFromSdCard(server.uri())){
    return;
  }
  DBG_OUTPUT_PORT.println("SD CARD ERROR PRINTING FLAGS");
  DBG_OUTPUT_PORT.println(hasSD);
  DBG_OUTPUT_PORT.println(loadFromSdCard(server.uri()));
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  DBG_OUTPUT_PORT.print(message);
}
void readFile()
{ 
         // buffer must be big enough to hold the whole JSON string
  File newFile = SD.open("balance.txt",FILE_READ);
  while(newFile.available()){
    String fileline = newFile.readStringUntil('\n');
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(fileline);
    
    if (!root.success()) {
      DBG_OUTPUT_PORT.println("JsonParser.parse() failed");
      return;
    }
    blocks[blockindex].from = root["from"].as<String>();
    blocks[blockindex].to = root["to"].as<String>();
    blocks[blockindex].fileName = root["fileName"].as<String>();
    blocks[blockindex].amount = root["amount"].as<String>();
    blocks[blockindex].description = root["description"].as<String>();
    blocks[blockindex].prevHash = root["prevHash"].as<String>();
    blocks[blockindex].curHash = root["curHash"].as<String>();
    
    blockindex += 1;
  }
  newFile.close();
}

void setup(void){
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print("\n");
  WiFi.begin(ssid, password);
  DBG_OUTPUT_PORT.print("Connecting to ");
  DBG_OUTPUT_PORT.println(ssid);
  // Wait for connection
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 20) {//wait 10 seconds
    delay(500);
  }
  if(i == 21){
    DBG_OUTPUT_PORT.print("Could not connect to");
    DBG_OUTPUT_PORT.println(ssid);
    while(1) delay(500);
  }
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());
  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
    DBG_OUTPUT_PORT.println("MDNS responder started");
    DBG_OUTPUT_PORT.print("You can now connect to http://");
    DBG_OUTPUT_PORT.print(host);
    DBG_OUTPUT_PORT.println(".local");
  }
  server.on("/newfile", addFile);
  server.on("/buyFile", buyFile);
  server.on("/list", HTTP_GET, printDirectory);
  server.on("/edit", HTTP_DELETE, handleDelete);
  server.on("/edit", HTTP_PUT, handleCreate);
  server.on("/edit", HTTP_POST, [](){ returnOK(); }, handleFileUpload);
  server.on("/update", [](){
      t_httpUpdate_return ret = ESPhttpUpdate.update("https://raw.githubusercontent.com/sureshwaitforitkumar/Basic-Automation-using-NodeMCU/master/blinkESP.bin","","CC AA 48 48 66 46 0E 91 53 2C 9C 7C 23 2A B1 74 4D 29 9D 33"); 
  });
  server.onNotFound(handleNotFound);
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");
  if (SD.begin(chipSelect)){
     DBG_OUTPUT_PORT.println("SD Card initialized.");
     hasSD = true;
  }
  updateCoins();
  checkCoins();
  start = millis();
  readFile();
}
void loop(void){
  server.handleClient();
}
