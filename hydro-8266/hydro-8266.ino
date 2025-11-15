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

#define MAX_SENSOR 6

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

String switchMatrixStr = "";

//Motor Values
uint8_t motorID = 1;
uint16_t motorValue = 1;
String motorMsg = "1:1";

////////////////////// SENSORS /////////////////////////
struct Sensor {
  const char* name;

  float ref;      
  float margin;

  float value;
};

Sensor temperature = {
  .name = "Temperature Sensor",
  .ref = 17.3f,
  .margin = 1.0f,
  .value = 0,
};

Sensor humidity = {
  .name = "Humidity Sensor",
  .ref = 73.0f,
  .margin = 3.0f,
  .value = 0,
};

Sensor pH = {
  .name = "pH Sensor",
  .ref = 8.8f,
  .margin = 1.0f,
  .value = 0,
};

Sensor pressure = {
  .name = "Pressure Sensor",
  .ref = 5.09f,
  .margin = 1.0f,
  .value = 0,
};

Sensor tds = {
  .name = "TDS Sensor",
  .ref = 313.0f,
  .margin = 1.0f,
  .value = 0,
};

Sensor ff = {
  .name = "Air Quality Sensor",
  .ref = 32.0f,
  .margin = 1.0f,
  .value = 0,
};

Sensor sensors[] = {temperature, humidity, pH, pressure, tds, ff};

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
        temperature.ref = valStr.toFloat();
      } else if(msg.startsWith("refpH")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        pH.ref = valStr.toFloat();
      } else if(msg.startsWith("refPres")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        pressure.ref = valStr.toFloat();
      } else if(msg.startsWith("refTDS")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        tds.ref = valStr.toInt();
      } else if(msg.startsWith("refFF")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        ff.ref = valStr.toFloat();
      } else if(msg.startsWith("refHum")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        humidity.ref = valStr.toFloat();
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

String deleteEmptyFlagBits(String flags){
  return flags.substring(16-(MAX_SENSOR*2));
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  readings["temperature"] = temperature.value;
  readings["ph"] = pH.value;
  readings["pressure"] = pressure.value;
  readings["tds"] = tds.value;
  readings["ff"] = ff.value;
  readings["humidity"] = humidity.value;

  readings["refTemperature"] = temperature.ref;
  readings["refpH"] = pH.ref;
  readings["refPressure"] = pressure.ref;
  readings["refTDS"] = tds.ref;
  readings["refFF"] = ff.ref;
  readings["refHum"] = humidity.ref;

  switchMatrixStr = deleteEmptyFlagBits(switchMatrixStr);
  readings["switch_temp_up"] = switchMatrixStr[0] - '0';      // Need this "-0" for javascript side
  readings["switch_temp_down"] = switchMatrixStr[1] - '0';
  readings["switch_hum_up"] = switchMatrixStr[2] - '0';
  readings["switch_hum_down"] = switchMatrixStr[3] - '0';
//  readings["switch_ph_up"] = 
//  readings["switch_ph_down"] = 
//  readings["switch_press_up"] = 
//  readings["switch_press_down"] = 
//  readings["switch_tds_up"] = 
//  readings["switch_tds_down"] = 
//  readings["switch_ff_up"] = 
//  readings["switch_ff_down"] = 

  readings["consoleLastMotorID"] = motorID;
  readings["consoleLastMotorData"] = motorValue;
  readings["consoleTempMOE"] = temperature.margin;
  readings["consoleHumMOE"] = humidity.margin;
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
    switch(index){                  //  t-> temperature, p-> pH, r -> pressure, d-> dissolved solids (tds), f-> particle (ff), h-> humidity, m-> margin of error temperature, n-> margin of error humidity, i-> last motor id, k-> last motor value, s-> motor off on switches
      case 't':
        temperature.value = atof(rawData);
        Serial.println(temperature.value);
        //Send ref value of the data
        sValue = String(temperature.ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'p':
        pH.value = atof(rawData);
        Serial.println(pH.value);
        //Send ref value of the data
        sValue = String(pH.ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'r':
        pressure.value = atof(rawData);
        Serial.println(pressure.value);
        //Send ref value of the data
        sValue = String(pressure.ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'd':
        tds.value = atoi(rawData);
        Serial.println(tds.value);
        //Send ref value of the data
        sValue = String(tds.ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'f':
        ff.value = atof(rawData);
        Serial.println(ff.value);
        //Send ref value of the data
        sValue = String(ff.ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'h':
        humidity.value = atof(rawData);
        Serial.println(humidity.value);
        //Send ref value of the data
        sValue = String(humidity.ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'm':
        temperature.margin = atof(rawData);
        Serial.println(temperature.margin);
        break;
      case 'n':
        humidity.margin = atof(rawData);
        Serial.println(temperature.margin);
        break;
      case 's':
        switchMatrixStr = rawData;
        Serial.println(rawData);
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