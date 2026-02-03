#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>
#include <EEPROM.h>
#include <FS.h>
#include <math.h>
//#include <Adafruit_BME280.h>
//#include <Adafruit_Sensor.h>

#define MAX_SENSOR 6

// Replace with your network credentials
const char* ssid = "KET0";
const char* password = "keto4522";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Static IP address
IPAddress local_IP(192, 168, 68, 37);
// Gateway IP address
IPAddress gateway(192, 168, 68, 1);
// Subnet IP address
IPAddress subnet(255, 255, 252, 0);

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
  IDX_ALT,
  IDX_HUM,
  IDX_PH,
  IDX_TDS,
  IDX_FF
};

Sensor sensors[] = {
  { "Temperature Sensor", 25.0f, 1.0f, 0 },
  { "Altitude Sensor",   160.0f, 10.0f, 0 },
  { "Humidity Sensor",    73.0f, 3.0f, 0 },
  { "pH Sensor",           7.0f, 1.0f, 0 },
  { "TDS Sensor",        300.0f, 1.0f, 0 },
  { "Air Quality Sensor",  50.0f, 1.0f, 0 }
};

////////////////////// COMMUNICATION //////////////////////
uint8_t csPin = 4;
uint8_t clkPin = 14;
uint8_t mosiPin = 13;
uint8_t misoPin = 12;

uint8_t tx_buffer[48] = {0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1};
uint8_t rx_buffer[48] = {0};

uint8_t maxDataCount = 0;
uint8_t firstDataTaken = 0;
uint8_t dataCounter = 0;
uint8_t spiDataLength = 6;

uint8_t bitCounter = 0;
uint8_t maxBits = 48;

typedef union
{
    struct __attribute__((packed))
    {
        uint8_t id;
        uint8_t type;
        float   payload;
    } frame;

    uint8_t raw[6];       // Be carefull about this ize
} SingleSPIData_t;

void CommSetup(){
  pinMode(csPin, INPUT_PULLUP);
  pinMode(clkPin, INPUT);
  pinMode(mosiPin, INPUT);
  pinMode(misoPin, OUTPUT);
  attachInterrupt(clkPin, commISR, RISING);
  digitalWrite(misoPin, tx_buffer[0]);
}

// Interrupt Service Routine
void ARDUINO_ISR_ATTR commISR(){
  rx_buffer[bitCounter] = digitalRead(mosiPin);
  digitalWrite(misoPin, tx_buffer[bitCounter+1] & 1);
  bitCounter++;
}

void ClearRxBuffer(){
  for(int i = 0; i < maxBits; i++){     // Clear RX Buffer
    rx_buffer[i] = 0;
  }
}

void CommManager(){
  if(firstDataTaken == 0 && bitCounter == 8){
    BitsToBytes(rx_buffer, &maxDataCount, 1);
    Serial.print("MaxDataCounter: ");
    Serial.println(maxDataCount);
    firstDataTaken = 1;
    bitCounter = 0;
    CleanRxBuffer();
  }

  if(bitCounter >= maxBits && firstDataTaken == 1){
    dataCounter++;
    Serial.println(bitCounter);
    Serial.print("Message: ");

    SingleSPIData_t spiData = {0};
    BitsToBytes(rx_buffer, spiData.raw, spiDataLength);
    //Serial.print("Raw: ");
    //for(int k = 0; k < spiDataLength; k++){
    //  Serial.print(spiData.raw[k]);
    //  Serial.print(" ");
    //}
    //Serial.println("");
    //Serial.print("ID: ");
    //Serial.println(spiData.frame.id);
    //Serial.print("Type: ");
    //Serial.println(spiData.frame.type);
    sensors[spiData.frame.id].value = spiData.frame.payload;
    Serial.print("Inside Sensor Value: ");
    Serial.println(sensors[spiData.frame.id].value);
    //Serial.println(spiData.frame.payload);
    
    for(int i = 0; i < maxBits; i++){
      Serial.print(rx_buffer[i]);
    }
    Serial.println("");
    
    digitalWrite(misoPin, tx_buffer[0]);

    if(dataCounter == maxDataCount){
      Serial.println("All data is taken");
      firstDataTaken = 0;
      dataCounter = 0;
    }
    CleanRxBuffer();
    bitCounter = 0;
  }
}

void BitsToBytes(const uint8_t *bits, uint8_t *bytes, uint8_t byteSize)
{
    for (int byte = 0; byte < byteSize; byte++)
    {
        uint8_t value = 0;

        for (int bit = 0; bit < 8; bit++)
        {
            value <<= 1;
            value |= bits[byte * 8 + bit] & 0x01;
        }
        bytes[byte] = value;
    }
}


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
      } else if(msg.startsWith("refAlt")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        sensors[IDX_ALT].ref = valStr.toFloat();
        UpdateEEPROM(12, sensors[IDX_ALT].ref);
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
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
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
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
   Serial.println("LittleFS mounted successfully");
  }
}

void listFS(){
  File root = LittleFS.open("/");
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open LittleFS root");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    Serial.printf("FS: %s (%u bytes)\n", file.name(), (unsigned)file.size());
    file = root.openNextFile();
  }
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
  readings["temperature"] = sensors[IDX_TEMP].value;
  readings["ph"] = sensors[IDX_PH].value;
  readings["altitude"] = sensors[IDX_ALT].value;
  readings["tds"] = sensors[IDX_TDS].value;
  readings["ff"] = sensors[IDX_FF].value;
  readings["humidity"] = sensors[IDX_HUM].value;

  readings["refTemperature"] = sensors[IDX_TEMP].ref;
  readings["refpH"] = sensors[IDX_PH].ref;
  readings["refAlt"] = sensors[IDX_ALT].ref;
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

    if(!isnan(conv.f)) sensors[i].ref = conv.f; 
  }

}

//////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ESP Started");

  EEPROMSetup();
  WebSocketSetup();
  CommSetup();
}

void loop() {

  CommManager();

  ws.cleanupClients();

  //if(digitalRead(csPin) == LOW) return;

  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    Serial.println(sensorReadings);
    notifyClients(sensorReadings);
    lastTime = millis();
  }
}