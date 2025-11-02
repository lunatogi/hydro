#include "SPISlave.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <FS.h>
//#include <Adafruit_BME280.h>
//#include <Adafruit_Sensor.h>

// Replace with your network credentials
const char* ssid = "KET0";
const char* password = "keto4522";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 10000;

// Create a sensor object
//Adafruit_BME280 bme;         // BME280 connect to ESP32 I2C (GPIO 21 = SDA, GPIO 22 = SCL)

//Current and ref sensor values
float temp = 25.0f;
float ph = 21.1;
float pres = 1.54f;
int tds = 10;
float ff = 33;
float hum = 55.3f;

float refTemp = 27.4f;
float refpH = 7.0f;
float refPres = 1.12f;
int refTDS = 131;
float refFF = 23.0f;
float refHum = 70.0f;

float margin_Temp = 0.8f;
float margin_Hum = 0.6f;


//Motor Values
uint8_t motorID = 1;
uint16_t motorValue = 1;
String motorMsg = "1:1";
////////////////////////////


////////////////////// WEB SOCKET /////////////////////////
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      data[len] = 0;

      String msg = (char*)data;

      Serial.print("Message from web: ");
      Serial.println(msg);
      String valStr = "";
      if(msg.startsWith("refTemp")) {
        valStr = msg.substring(msg.indexOf(':') + 1);
        refTemp = valStr.toFloat();
      } else if(msg.startsWith("refpH")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        refpH = valStr.toFloat();
      } else if(msg.startsWith("refPres")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        refPres = valStr.toFloat();
      } else if(msg.startsWith("refTDS")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        refTDS = valStr.toInt();
      } else if(msg.startsWith("refFF")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        refFF = valStr.toFloat();
      } else if(msg.startsWith("refHum")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        refHum = valStr.toFloat();
      }else if (msg[0] >= '0' && msg[0] <= '9'){
        motorMsg = msg;
        String motorIDStr = msg.substring(0, msg.indexOf(':'));
        String motorValueStr = msg.substring(msg.indexOf(':') + 1);
        motorID = motorIDStr.toInt();
        motorValue = motorValueStr.toInt();
        Serial.print("Motor ID: ");
        Serial.println(motorID);
        Serial.print("Motor Value: ");
        Serial.println(motorValue);
      }

      String sensorReadings = getSensorReadings();
      //Serial.println(sensorReadings);
      notifyClients(sensorReadings);
    //}
  }
}

void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
   Serial.println("LittleFS mounted successfully");
  }
}

void listFS(){
  Dir dir = LittleFS.openDir("/");
  while (dir.next()){
    Serial.printf("FS: %s (%u bytes)\n", dir.fileName().c_str(), dir.fileSize());
  }
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  readings["temperature"] = temp;
  readings["ph"] = ph;
  readings["pressure"] = pres;
  readings["tds"] = tds;
  readings["ff"] = ff;
  readings["humidity"] = hum;

  readings["refTemperature"] = refTemp;
  readings["refpH"] = refpH;
  readings["refPressure"] = refPres;
  readings["refTDS"] = refTDS;
  readings["refFF"] = refFF;
  readings["refHum"] = refHum;

  readings["consoleLastMotorID"] = motorID;
  readings["consoleLastMotorData"] = motorValue;
  readings["consoleTempMOE"] = margin_Temp;
  readings["consoleHumMOE"] = margin_Hum;
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

void WebSocketSetup(){
  initWiFi();
  initFS();
  listFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
    //request->send(200, "text/html", "<h1>ESP8266 is alive ✅</h1>");
  });

  server.onNotFound([](AsyncWebServerRequest *req){
    Serial.printf("404: %s %s\n", req->methodToString(), req->url().c_str());
    if (req->method() == HTTP_OPTIONS) { req->send(204); return; }
    req->send(404, "text/plain", "Not found: " + req->url());
  });

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // Start server
  server.begin();
}
/////////////////////////////////////////////////////////////////////

void SPISlaveSetup(){
  SPISlave.onData([](uint8_t *data, size_t len) {
    char * cData = (char *)data;
    char * rawData = cData + 1;     //Drop the first letter which says the value type
    char index = cData[0];
    Serial.print("Mesaj: ");
    String sValue = "";
    switch(index){                  //  t->temperature, p->pH, r->pressure, d=dissolved solids (tds), f=particle (ff), h=humidity, m=margin of error temperature, n=margin of error humidity, i=last motor id, k=last motor value
      case 't':
        temp = atof(rawData);
        Serial.println(temp);
        //Send ref value of the data
        sValue = String(refTemp);
        SPISlave.setData(sValue.c_str());
        break;
      case 'p':
        ph = atof(rawData);
        Serial.println(ph);
        //Send ref value of the data
        sValue = String(refpH);
        SPISlave.setData(sValue.c_str());
        break;
      case 'r':
        pres = atof(rawData);
        Serial.println(pres);
        //Send ref value of the data
        sValue = String(refPres);
        SPISlave.setData(sValue.c_str());
        break;
      case 'd':
        tds = atoi(rawData);
        Serial.println(tds);
        //Send ref value of the data
        sValue = String(refTDS);
        SPISlave.setData(sValue.c_str());
        break;
      case 'f':
        ff = atof(rawData);
        Serial.println(ff);
        //Send ref value of the data
        sValue = String(refFF);
        SPISlave.setData(sValue.c_str());
        break;
      case 'h':
        hum = atof(rawData);
        Serial.println(hum);
        //Send ref value of the data
        sValue = String(refHum);
        SPISlave.setData(sValue.c_str());
        break;
      case 'm':
        margin_Temp = atof(rawData);
        Serial.println(margin_Temp);
        break;
      case 'n':
        margin_Hum = atof(rawData);
        Serial.println(margin_Temp);
        break;
      default:
        SPISlave.setData(motorMsg.c_str());         // Send motor adjustment values
        Serial.println(motorMsg);
    }
    
  });

  SPISlave.onDataSent([]() {
    //Serial.println("Answer Sent");
  });

  SPISlave.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ESP Started");

  WebSocketSetup();
  SPISlaveSetup();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    Serial.println(sensorReadings);
    notifyClients(sensorReadings);
    lastTime = millis();
  }

  ws.cleanupClients();
}