/*          SPI CONNECTIONS
    GPIO    NodeMCU   Name  |   Uno
   ===================================
     15       D8       SS   |   D10
     13       D7      MOSI  |   D11
     12       D6      MISO  |   D12
     14       D5      SCK   |   D13
*/
// Analog Pins
#define PH_PIN A0
#define TDS_PIN A1
#define FLG_PIN A2

//Digital Pins
#define ONE_WIRE_BUS 8          // Data wire is conntec to the Arduino digital pin 4
#define MOTOR_DATA 7
#define MOTOR_CLK 6

#define MAX_SENSOR 6

//#include <Adafruit_BMP085.h>  // BMP180 Air Pressure Sensor
#include <DFRobot_PH.h>         // DFTRobot Analog pH Sensor 
#include <OneWire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <SPI.h>

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature obj_tempSensor(&oneWire);

//Real-time Clock Module
ThreeWire threeWire(6, 7, 9);   // DATA, CLK, RST
RtcDS1302<ThreeWire> Rtc(threeWire);

//Adafruit_BMP085 bmp;
DFRobot_PH ph;

//Adafruit AHT10 settings
Adafruit_AHTX0 aht10;
sensors_event_t aht10Temp, aht10Hum;

//Current and ref sensor values
float temp = 30.0f;
float pH_ = 0.0f;
float pres = 2.21f;
float tds_ = 2.0f;
float ff_ = 1.0f;
float hum = 0.0f;

float refTemp = 17.3f;
float refpH = 8.8f;
float refPres = 5.09f;
float refTDS = 313.0f;
float refFF = 32.0f;
float refHum = 73.0f;

float margin_Temp = 1.0f;
float margin_Hum = 3.0f;
/////////////MOTOR/////////////
uint8_t mtrID = 0b00000000;
uint8_t o_mtrID = 0b00000001;
uint16_t mtrData = 0b1010101010101010;
uint16_t o_mtrData = 0b0101010101010101;

struct Sensor {
  const char* name;

  float ref;      
  float margin;

  float value;

  uint8_t upMotorID;
  uint8_t downMotorID;

  bool increaseEnabled;
  bool decreaseEnabled;

  float minValue;
  float maxValue;

  int increasePin;
  int decreasePin;
};

enum {
  IDX_TEMP = 0,
  IDX_HUM,
  IDX_PH,
  IDX_PRESS,
  IDX_TDS,
  IDX_FF
};

Sensor sensors[MAX_SENSOR] = {
  { "Temperature Sensor", 30.0f, 1.0f, 0, 40, 45, false,  false, 25.0f, 45.0f,  2, 3  },
  { "Humidity Sensor",    73.0f, 3.0f, 0, 50, 55, false,  false, 30.0f, 90.0f,  4, 0  },
  { "pH Sensor",          8.8f,  1.0f, 0, 60, 65, false,  false, 4.0f,  9.0f,   0, 0  },
  { "Pressure Sensor",    5.09f, 1.0f, 0, 70, 75, false,  false, 0.5f,  3.0f,   0, 0  },
  { "TDS Sensor",         313.0f,1.0f, 0, 80, 85, false,  false, 20.0f, 100.0f, 0, 0  },
  { "Air Quality Sensor", 32.0f, 1.0f, 0, 90, 95, false,  false, 50.0f, 600.0f, 0, 0  },
};
///////////////////// SPI ////////////////////////
class ESPMaster {
private:
  uint8_t _ss_pin;
  void _pulseSS() {
    digitalWrite(_ss_pin, HIGH);
    delayMicroseconds(5);
    digitalWrite(_ss_pin, LOW);
  }

public:
  ESPMaster(uint8_t pin)
    : _ss_pin(pin) {}
  void begin() {
    pinMode(_ss_pin, OUTPUT);
    _pulseSS();
  }

  void readData(uint8_t *data) {
    _pulseSS();
    SPI.transfer(0x03);
    SPI.transfer(0x00);
    for (uint8_t i = 0; i < 32; i++) { data[i] = SPI.transfer(0); }
    _pulseSS();
  }

  void writeData(uint8_t *data, size_t len) {
    uint8_t i = 0;
    _pulseSS();
    SPI.transfer(0x02);
    SPI.transfer(0x00);
    while (len-- && i < 32) { SPI.transfer(data[i++]); }
    while (i++ < 32) { SPI.transfer(0); }
    _pulseSS();
  }

  String readData() {
    char data[33];
    data[32] = 0;
    readData((uint8_t *)data);
    return String(data);
  }

  void writeData(const char *data) {
    writeData((uint8_t *)data, strlen(data));
  }
};

ESPMaster esp(SS);

void sendESP(const char *message) {
  esp.writeData(message);
  delay(10);
  String retData = String(esp.readData());
  float retDataF = retData.toFloat();
  //Serial.print("Master: ");
  //Serial.println(message);
  //Serial.print("Slave: ");
  //Serial.println(retData);
  // Route by header letter: t=temperature, p=pH, r=pressure, d=dissolved solids (tds), f=particle (ff.value), h=humidity, m=margin of error temperature, n=margin of error humidity, e=engin, s=on or off positions for motors
  switch (message[0]) {
    case 't':
      if(retDataF <= sensors[IDX_TEMP].maxValue && retDataF >= sensors[IDX_TEMP].minValue) sensors[IDX_TEMP].ref = retDataF;
      Serial.print("InRef Temp: ");
      Serial.println(sensors[IDX_TEMP].ref);
      break;
    case 'p':
      if(retDataF <= sensors[IDX_PH].maxValue && retDataF >= sensors[IDX_PH].minValue) sensors[IDX_PH].ref = retDataF;
      Serial.print("InRef pH: ");
      Serial.println(sensors[IDX_PH].ref);
      break;
    case 'r':
      if(retDataF <= sensors[IDX_PRESS].maxValue && retDataF >= sensors[IDX_PRESS].minValue) sensors[IDX_PRESS].ref = retDataF;
      Serial.print("InRef Pressure: ");
      Serial.println(sensors[IDX_PRESS].ref);
      break;
    case 'd':
      if(retDataF <= sensors[IDX_TDS].maxValue && retDataF >= sensors[IDX_TDS].minValue) sensors[IDX_TDS].ref = retDataF;
      Serial.print("InRef TDS: ");
      Serial.println(sensors[IDX_TDS].ref);
      break;
    case 'f':
      if(retDataF <= sensors[IDX_FF].maxValue && retDataF >= sensors[IDX_FF].minValue) sensors[IDX_FF].ref = retDataF;
      Serial.print("InRef Particle: ");
      Serial.println(sensors[IDX_FF].ref);
      break;
    case 'h':
      if(retDataF <= sensors[IDX_HUM].maxValue && retDataF >= sensors[IDX_HUM].minValue) sensors[IDX_HUM].ref = retDataF;
      Serial.print("InRef Humidity: ");
      Serial.println(sensors[IDX_HUM].ref);
      break;
    case 'e':
      //Motor Adjutment Values
      String retDataStr = retData;
      String motorIDStr = retDataStr.substring(0, retData.indexOf(':'));
      String motorDataStr = retDataStr.substring(retData.indexOf(':') + 1);
      mtrID = motorIDStr.toInt();
      mtrData = motorDataStr.toInt();
      
      Serial.print("Motor ID: ");
      Serial.println(mtrID);
      Serial.print("Motor Data: ");
      Serial.println(mtrData);
      break;
    case 's':       // No process needed for switch flag send
      break;
    default:
      Serial.print("No respond message: ");
      Serial.println(retData);
      Serial.println("");
      break;
  }
  //Serial.println("");
  delay(1);
}

void SPIMasterSetup(){
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  esp.begin();
  sendESP("Hello Slave!");         // Unnecessary
}
///////////////////////// EEPROM //////////////////////
void UpdateEEPROM(){
  union{
    float val;
    uint8_t b[4];   // little-endian -> b[0] LSB
  }conv;

  int eepromAddress = 0;
  for(int i = 0; i < MAX_SENSOR; i++){
    conv.val = sensors[i].ref;
    for(int j = 3; j >= 0; j--){
      EEPROM.update(eepromAddress++, conv.b[j]);
    }
  }
}

void EEPROMSetup(){
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

void PinConfiguration(){
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);       // Using for LEDs right now 
}

void RTCSetup(){
  Rtc.Begin();

  //Taking inital time from PC
  RtcDateTime cdt = RtcDateTime(__DATE__, __TIME__);

  //Taking initial time manually
  //RtcDateTime cdt = RtcDateTime("Dec 11 2025", "14:40:00");

  Rtc.SetDateTime(cdt);

  // Check for pin correctness
  digitalWrite(5, HIGH);
  delay(1000);
  digitalWrite(5, LOW);
  delay(1000);
  digitalWrite(5, HIGH);
  delay(1000);
  digitalWrite(5, LOW);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  PinConfiguration();
  EEPROMSetup();
  obj_tempSensor.begin();    //OneWire temperature sensor
 	SPIMasterSetup();
  RTCSetup();

  //if(!bmp.begin()){
  //  Serial.println("BMP180 not found!");
  //}
  
  // Manuel Motor Adjustment
  //pinMode(MOTOR_DATA, OUTPUT);
  //pinMode(MOTOR_CLK, OUTPUT);
  //digitalWrite(MOTOR_DATA, LOW);
  //digitalWrite(MOTOR_CLK, LOW);

  while (aht10.begin() != true) //for ESP-01 use aht10.begin(0, 2);
  {
    Serial.println(F("AHT1x not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free

    delay(2000);
  }


  delay(3000);
}

void loop() {         // temperature.value/100, alt/100
  // put your main code here, to run repeatedly:

  // Manuel Motor Adjustment
  /*
  if((o_mtrID != mtrID) || (o_mtrData != mtrData)){
    uint8_t mtrData_L = mtrData & 0b11111111;
    uint8_t mtrdData_H = mtrData >> 8;
    AdjustMotors(mtrID);
    AdjustMotors(mtrData_L);
    AdjustMotors(mtrdData_H);
    o_mtrID = mtrID;
    o_mtrData = mtrData;
  }
  */

  ReadSensors();
  processMotors();
  SendStatesToESP();
  UpdateEEPROM();
  CheckRealTime();
}

void CheckRealTime(){
  RtcDateTime pdt = Rtc.GetDateTime();
  Serial.println("---DateTime---");
  printDateTime(pdt);
  Serial.println("--------------");
  if(pdt.Hour() >= 6 && pdt.Hour() < 20){
    digitalWrite(5, HIGH);
  }else{
    digitalWrite(5, LOW);
  }
}

// NOT USED RIGHT NOW
void printDateTime(const RtcDateTime& dt) {
  //Day of the week
  Serial.print("Day of the week: ");
  if (dt.DayOfWeek() == 1) {
    Serial.println("Monday");
  }
  else if (dt.DayOfWeek() == 2) {
    Serial.println("Tuesday");
  }
  else if (dt.DayOfWeek() == 3) {
    Serial.println("Wednesday");
  }
  else if (dt.DayOfWeek() == 4) {
    Serial.println("Thursday");
  }
  else if (dt.DayOfWeek() == 5) {
    Serial.println("Friday");
  }
  else if (dt.DayOfWeek() == 6) {
    Serial.println("Saturday");
  }
  else if (dt.DayOfWeek() == 7) {
    Serial.println("Sunday");
  }
  // Current Date
  Serial.print("Current Date: ");
  if (dt.Day() < 10) {
    Serial.print("0");
    Serial.print(dt.Day());
  }
  else {
    Serial.print(dt.Day());
  }

  //one tab
  Serial.print("/");
  if (dt.Month() < 10) {
    Serial.print("0");
    Serial.print(dt.Month());
  }
  else {
    Serial.print(dt.Month());
  }
  Serial.print("/");
  Serial.println(dt.Year());
  //Current Time
  Serial.print("Current Time: ");
  if (dt.Hour() < 10) {
    Serial.print("0");
    Serial.print(dt.Hour());
  }
  else {
    Serial.print(dt.Hour());
  }
  Serial.print(":");
  if (dt.Minute() < 10) {
    Serial.print("0");
    Serial.print(dt.Minute());
  }
  else {
    Serial.print(dt.Minute());
  }

  //one tab
  Serial.print(":");
  if (dt.Second() < 10) {
    Serial.print("0");
    Serial.print(dt.Second());
    Serial.println();
  }else {
    Serial.print(dt.Second());
    Serial.println();
  }
}


void AdjustMotors(uint8_t data){
  for(int i = 0; i < 8; i++){
    bool bit = (data >> i) & 1;
    digitalWrite(MOTOR_DATA, bit);
    digitalWrite(MOTOR_CLK, HIGH);
    delay(1);
    digitalWrite(MOTOR_CLK, LOW);
    delay(1);
  }
}

void ReadSensors(){
  Serial.println("---");
  readOneWire();                    // Temperature
  readpH();                         // pH
  readTDS();                        // Particle (TDS)
  readFlyingFish();
  readHumidity();
}

void SendStatesToESP(){
  String ESPval = "";

  ESPval = "t"+String(sensors[IDX_TEMP].value);
  sendESP(ESPval.c_str());

  ESPval = "p"+String(sensors[IDX_PH].value);
  sendESP(ESPval.c_str());

  ESPval = "d"+String(sensors[IDX_TDS].value);
  sendESP(ESPval.c_str());

  ESPval = "f"+String(sensors[IDX_FF].value);
  sendESP(ESPval.c_str());

  ESPval = "h"+String(sensors[IDX_HUM].value);
  sendESP(ESPval.c_str());

  sendESP("e0");   // Takes motor adjustment values

  //Send Debug Values
  ESPval = "m"+String(sensors[IDX_TEMP].margin);
  sendESP(ESPval.c_str());

  ESPval = "n"+String(sensors[IDX_HUM].margin);
  sendESP(ESPval.c_str());

  ESPval = "s";
  ESPval = ESPval + sumSwitches();
  sendESP(ESPval.c_str());
}

void processMotors(){
  for(int i = 0; i < MAX_SENSOR; i++){
    
    float curValue = sensors[i].value;
    float refValue = sensors[i].ref;
    float marginValue = sensors[i].margin;
    bool increase = sensors[i].increaseEnabled;
    bool decrease = sensors[i].decreaseEnabled;
    uint8_t up_ID = sensors[i].upMotorID;
    uint8_t down_ID = sensors[i].downMotorID; 
    int p_increase = sensors[i].increasePin;
    int p_decrease = sensors[i].decreasePin;

    if(p_increase < 2) continue;          // Pass unused sensors (for debug)

    if((curValue < (refValue - marginValue)) && !increase){
      //AdjustMotors(up_ID);         // Open upMotor
      //AdjustMotors(123);          // Low bits
      //AdjustMotors(0);            // High bits

      //AdjustMotors(down_ID);         // Close downMotor
      //AdjustMotors(5);          // Low bits
      //AdjustMotors(0);            // High bits

      digitalWrite(p_increase, HIGH);
      digitalWrite(p_decrease, LOW);
      sensors[i].increaseEnabled = true;
      sensors[i].decreaseEnabled = false;
    }else if((curValue > (refValue + marginValue)) && !decrease){
      //AdjustMotors(down_ID);           // Open downMotor
      //AdjustMotors(123);            // Low bits
      //AdjustMotors(0);            // High bits

      //AdjustMotors(up_ID);           // Open downMotor
      //AdjustMotors(5);            // Low bits
      //AdjustMotors(0);            // High bits

      digitalWrite(p_increase, LOW);
      digitalWrite(p_decrease, HIGH);
      sensors[i].increaseEnabled = false;
      sensors[i].decreaseEnabled = true;
    }else{
      if(increase && (curValue >= (refValue - marginValue))){
        uint8_t m_ID = sensors[i].upMotorID;
        //AdjustMotors(m_ID);
        //AdjustMotors(5);          // Low bits
        //AdjustMotors(0);            // High bits
        sensors[i].increaseEnabled = false;
        digitalWrite(p_increase, LOW);
      }else if(decrease && (curValue <= (refValue + marginValue))){
        uint8_t m_ID = sensors[i].downMotorID;
        //AdjustMotors(m_ID);
        //AdjustMotors(5);          // Low bits
        //AdjustMotors(0);            // High bits
        sensors[i].decreaseEnabled = false;
        digitalWrite(p_decrease, LOW);
      }
    }
  }
}

String deleteEmptyFlagBits(String flags){
  return flags.substring(16-(MAX_SENSOR*2));
}

String byteToBinary(uint16_t b) {
  String s = "";
  for (int i = 15; i >= 0; i--) {
    s = s + ((b & (1 << i)) ? '1' : '0');
  }
  return s;
}

String sumSwitches(){         // If any motor is ON or OFF
  String flagStr = "";
  uint16_t flags = 0;
  for(int i = 0; i < MAX_SENSOR; i++){
    flags = flags << 1;
    if (sensors[i].increaseEnabled) flags |= 1;
    flags = flags << 1;
    if (sensors[i].decreaseEnabled) flags |= 1;
  }

  flagStr = byteToBinary(flags);
  flagStr = deleteEmptyFlagBits(flagStr);
  Serial.print("Switch Matrix: ");
  Serial.println(flagStr);
  return flagStr;
}

void readOneWire(){
  obj_tempSensor.requestTemperatures(); 
  Serial.print("Temperature:");
  sensors[IDX_TEMP].value = obj_tempSensor.getTempCByIndex(0) + 1.5;
  Serial.println(sensors[IDX_TEMP].value);

  delay(10);
}

/*
void readBMP(){

  pressure = bmp.readPressure();
  float temperature = bmp.readTemperature();
  bitwiseTemp = temperature * 100;     // So we can get rid of .00 
  float alt = bmp.readAltitude();     
  bitwiseAlt = alt * 100;

  //Serial.println("Air Pressure:");
  //Serial.println(pressure);

  Serial.println("Temperature BMP:");
  temperature.value = temperature;
  Serial.println(temperature.value);

  //Serial.println("Altitude:");
  //Serial.println(bitwiseAlt);
  //Serial.println("");

  delay(10);
}
*/

void readpH(){
  Serial.print("pH: ");
  float voltage = analogRead(PH_PIN)/1024.0*5000;
  sensors[IDX_PH].value = ph.readPH(voltage, sensors[IDX_TEMP].value) + 0.3;
  Serial.println(sensors[IDX_PH].value);

  delay(10);
}

void readFlyingFish(){
  Serial.print("Flammable Gas: ");
  sensors[IDX_FF].value = analogRead(FLG_PIN);
  Serial.println(sensors[IDX_FF].value);

  delay(10);
}

void readTDS(){
  Serial.print("TDS: ");
  sensors[IDX_TDS].value = analogRead(TDS_PIN);
  Serial.println(sensors[IDX_TDS].value);

  delay(10);
}

void readHumidity(){
  aht10.getEvent(&aht10Hum, &aht10Temp);
  sensors[IDX_HUM].value = aht10Hum.relative_humidity;
  Serial.print("Humudity: ");
  Serial.println(sensors[IDX_HUM].value);

  delay(10);
}

