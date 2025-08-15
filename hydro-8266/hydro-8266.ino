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

float temp = 25.0;
float ph = 21.1;
float pres = 1.54;

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
      if(strcmp((char*)data, "updateSensorValue") == 0){
        temp += 5;
        ph += 5;
        pres += 5;
      }
      String sensorReadings = getSensorReadings();
      Serial.println(sensorReadings);
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

  server.serveStatic("/", LittleFS, "/");

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
    if(index == 't'){
      temp = atof(rawData);
      Serial.println(temp);
    }else if(index == 'p'){
      ph = atof(rawData);
      Serial.println(ph);
    }else{
      String message = String(cData+1);      // So that we can get rid of 
      Serial.println(message);
    }
    
    SPISlave.setData("Ask me a question!");
  });

  SPISlave.onDataSent([]() {
    Serial.println("Answer Sent");
  });

  SPISlave.begin();
}

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Started");

  WebSocketSetup();
  SPISlaveSetup();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    Serial.print(sensorReadings);
    notifyClients(sensorReadings);
    lastTime = millis();
  }

  ws.cleanupClients();
}