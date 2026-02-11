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

uint8_t tx_buffer[48] = {0};
uint8_t rx_buffer[48] = {0};

uint8_t STMDataCount = 0;
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

    uint8_t raw[6];       // Be carefull about this size
} SingleSPIData_t;

SingleSPIData_t messageQueue[6] = {0};
uint8_t queueCounter = 0;

void QueueMessage(uint8_t _idx, uint8_t _type, float _payload){     // _idx'in type'ı SensorIndex_t yapılabilir sonrasında
  messageQueue[queueCounter].frame.id = _idx;
  messageQueue[queueCounter].frame.type = _type;
  messageQueue[queueCounter].frame.payload = _payload;
  queueCounter++;
}

void BytesToBits(const uint8_t *bytes, uint8_t *bits, uint8_t byteSize)
{
    for (int byte = 0; byte < byteSize; byte++)
    {
        uint8_t value = bytes[byte];

        for (int bit = 0; bit < 8; bit++)
        {
            bits[byte * 8 + bit] = (value >> (7 - bit)) & 0x01;
        }
    }
}

void FillTxBufferFromQueue(){      // Check this functions functionality
    if(queueCounter > 0){
      BytesToBits(messageQueue[0].raw, tx_buffer, spiDataLength);
      for(int i = 1; i < queueCounter; i++){
        messageQueue[i-1] = messageQueue[i];
      }
      queueCounter--;
    }
    digitalWrite(misoPin, tx_buffer[0] & 1);
}

void ClearQueue(){
  for(int i = 0; i < 6; i++)      // Bunu hard coded halinden değiştir
  {
    for(int k = 0; k < 6; k++){
      messageQueue[i].raw[k] = 0;
    }
  }
  queueCounter = 0;
}

void ClearRxBuffer(){
  for(int i = 0; i < maxBits; i++){     // Clear RX Buffer
    rx_buffer[i] = 0;
  }
}

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
  if (bitCounter < maxBits && digitalRead(csPin) == LOW) {
    rx_buffer[bitCounter] = digitalRead(mosiPin);
    if (bitCounter + 1 < maxBits) digitalWrite(misoPin, tx_buffer[bitCounter+1] & 1);
    bitCounter++;
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

void PrintRxBuffer(){
  Serial.print("Rx Buffer: ");
  for(int i = 0; i < maxBits; i++){
    Serial.print(rx_buffer[i]);
  }
  Serial.println("");
}

String uint8ToBinaryString(uint8_t value) {
  String out;
  out.reserve(8);

  for (int i = 7; i >= 0; --i) {
    out += ((value >> i) & 0x01) ? '1' : '0';
  }

  return out;
}

void CommManager(){
  if(firstDataTaken == 0 && bitCounter == 8){
    BitsToBytes(rx_buffer, &STMDataCount, 1);
    maxDataCount = max(STMDataCount, queueCounter);
    Serial.print("MaxDataCounter: ");
    Serial.println(maxDataCount);
    firstDataTaken = 1;
    bitCounter = 0;
    FillTxBufferFromQueue();
    ClearRxBuffer();
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
    switch(spiData.frame.type){
      case 0:
        sensors[spiData.frame.id].value = spiData.frame.payload;
        Serial.print("Inside Sensor Value: ");
        Serial.println(sensors[spiData.frame.id].value);
        break;
      case 1:
        Serial.println("Type 1: Reference value");
        break;
      case 2:
        switchMatrixStr = uint8ToBinaryString(spiData.raw[2]);
        break;
      default:
        Serial.println("Invalid data!");
    }
    

    //Serial.println(spiData.frame.payload);
    
    PrintRxBuffer();
    
    digitalWrite(misoPin, tx_buffer[0]);

    if(dataCounter == maxDataCount){
      Serial.println("All data is taken");
      firstDataTaken = 0;
      dataCounter = 0;
      ClearQueue();
    }else{
      FillTxBufferFromQueue();
    }
    ClearRxBuffer();
    bitCounter = 0;
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

void UpdateRefValue(uint8_t _idx, uint8_t _memoryLoc, float _newValue){
  if(queueCounter < 6){         // Change this hard coded format
    QueueMessage(_idx, 1, _newValue);             // Communication can be corrupted if some data comes from web while communicating
    Serial.print("QueueCounter: ");
    Serial.println(queueCounter);
    BytesToBits(&queueCounter, tx_buffer, 1);
    digitalWrite(misoPin, tx_buffer[0] & 1);
    sensors[_idx].ref = _newValue;
    UpdateEEPROM(_memoryLoc, sensors[_idx].ref);
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
        float newValue = valStr.toFloat();
        UpdateRefValue(IDX_TEMP, 0, newValue);
      } else if(msg.startsWith("refpH")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        float newValue = valStr.toFloat();
        UpdateRefValue(IDX_PH, 12, newValue);
      } else if(msg.startsWith("refAlt")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        float newValue = valStr.toFloat();
        UpdateRefValue(IDX_ALT, 4, newValue);
      } else if(msg.startsWith("refTDS")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        float newValue = valStr.toFloat();
        UpdateRefValue(IDX_TDS, 16, newValue);
      } else if(msg.startsWith("refFF")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        float newValue = valStr.toFloat();
        UpdateRefValue(IDX_FF, 20, newValue);
      } else if(msg.startsWith("refHum")){
        valStr = msg.substring(msg.indexOf(':') + 1);
        float newValue = valStr.toFloat();
        UpdateRefValue(IDX_HUM, 8, newValue);
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

  readings["switch_temp_up"] = switchMatrixStr[4] - '0';      // Need this "-0" for javascript side
  readings["switch_temp_down"] = switchMatrixStr[5] - '0';
  readings["switch_alt_up"] = switchMatrixStr[6] - '0';
  readings["switch_alt_down"] = switchMatrixStr[7] - '0';
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

  

  //if(digitalRead(csPin) == LOW) return;

  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    Serial.println(sensorReadings);
    notifyClients(sensorReadings);
    lastTime = millis();
  }

  if(digitalRead(csPin) == LOW){
    CommManager();
    return;
  } 

  //ws.cleanupClients();
}