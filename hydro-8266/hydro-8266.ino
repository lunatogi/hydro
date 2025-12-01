#include <SPISlave.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>
#include <EEPROM.h>
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

enum {
  IDX_TEMP = 0,
  IDX_HUM,
  IDX_PH,
  IDX_PRESS,
  IDX_TDS,
  IDX_FF
};

Sensor sensors[] = {
  { "Temperature Sensor", 25.0f, 1.0f, 0 },
  { "Humidity Sensor",    73.0f, 3.0f, 0 },
  { "pH Sensor",           7.0f, 1.0f, 0 },
  { "Pressure Sensor",     1.5f, 1.0f, 0 },
  { "TDS Sensor",        300.0f, 1.0f, 0 },
  { "Air Quality Sensor",  50.0f, 1.0f, 0 }
};

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
        sensors[IDX_TEMP].ref = valStr.toFloat();
        UpdateEEPROM(0, sensors[IDX_TEMP].ref);
      } else if(msg.startsWith("refpH")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        sensors[IDX_PH].ref = valStr.toFloat();
        UpdateEEPROM(8, sensors[IDX_PH].ref);
      } else if(msg.startsWith("refPres")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        sensors[IDX_PRESS].ref = valStr.toFloat();
        UpdateEEPROM(12, sensors[IDX_PRESS].ref);
      } else if(msg.startsWith("refTDS")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        sensors[IDX_TDS].ref = valStr.toInt();
        UpdateEEPROM(16, sensors[IDX_TDS].ref);
      } else if(msg.startsWith("refFF")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        sensors[IDX_FF].ref = valStr.toFloat();
        UpdateEEPROM(20, sensors[IDX_FF].ref);
      } else if(msg.startsWith("refHum")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        sensors[IDX_HUM].ref = valStr.toFloat();
        UpdateEEPROM(4, sensors[IDX_HUM].ref);
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
  readings["temperature"] = sensors[IDX_TEMP].value;
  readings["ph"] = sensors[IDX_PH].value;
  readings["pressure"] = sensors[IDX_PRESS].value;
  readings["tds"] = sensors[IDX_TDS].value;
  readings["ff"] = sensors[IDX_FF].value;
  readings["humidity"] = sensors[IDX_HUM].value;

  readings["refTemperature"] = sensors[IDX_TEMP].ref;
  readings["refpH"] = sensors[IDX_PH].ref;
  readings["refPressure"] = sensors[IDX_PRESS].ref;
  readings["refTDS"] = sensors[IDX_TDS].ref;
  readings["refFF"] = sensors[IDX_FF].ref;
  readings["refHum"] = sensors[IDX_HUM].ref;

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
  readings["consoleTempMOE"] = sensors[IDX_TEMP].margin;
  readings["consoleHumMOE"] = sensors[IDX_HUM].margin;
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
        sensors[IDX_TEMP].value = atof(rawData);
        Serial.println(sensors[IDX_TEMP].value);
        //Send ref value of the data
        sValue = String(sensors[IDX_TEMP].ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'p':
        sensors[IDX_PH].value = atof(rawData);
        Serial.println(sensors[IDX_PH].value);
        //Send ref value of the data
        sValue = String(sensors[IDX_PH].ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'r':
        sensors[IDX_PRESS].value = atof(rawData);
        Serial.println(sensors[IDX_PRESS].value);
        //Send ref value of the data
        sValue = String(sensors[IDX_PRESS].ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'd':
        sensors[IDX_TDS].value = atoi(rawData);
        Serial.println(sensors[IDX_TDS].value);
        //Send ref value of the data
        sValue = String(sensors[IDX_TDS].ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'f':
        sensors[IDX_FF].value = atof(rawData);
        Serial.println(sensors[IDX_FF].value);
        //Send ref value of the data
        sValue = String(sensors[IDX_FF].ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'h':
        sensors[IDX_HUM].value = atof(rawData);
        Serial.println(sensors[IDX_HUM].value);
        //Send ref value of the data
        sValue = String(sensors[IDX_HUM].ref);
        SPISlave.setData(sValue.c_str());
        break;
      case 'm':
        sensors[IDX_TEMP].margin = atof(rawData);
        Serial.println(sensors[IDX_TEMP].margin);
        break;
      case 'n':
        sensors[IDX_HUM].margin = atof(rawData);
        Serial.println(sensors[IDX_HUM].margin);
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

///////////////////////// EEPROM //////////////////////
void UpdateEEPROM(int startAddress, float value){
  union{
    float val;
    uint8_t b[4];   // little-endian -> b[0] LSB
  }conv;

  conv.val = value;

  for(int j = 3; j >= 0; j--){
    EEPROM.write(startAddress++, conv.b[j]);
  }
  
  EEPROM.commit();
}

void EEPROMSetup(){
  EEPROM.begin(128);

  union{
    uint32_t u;
    float f;
  } conv;

  int eepromAddress = 0;

  for(int i = 0; i < MAX_SENSOR; i++){
    conv.u = 0;
    for (int j = 0; j < 4; j++) {
        uint8_t eByte = EEPROM.read(eepromAddress++);
        conv.u = (conv.u << 8) | eByte;
    }
    sensors[i].ref = conv.f;
  }

}

//////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ESP Started");

  EEPROMSetup();
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