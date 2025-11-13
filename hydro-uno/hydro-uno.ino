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

//#include "Adafruit_BMP085.h"  // BMP180 Air Pressure Sensor
#include "DFRobot_PH.h"         // DFTRobot Analog pH Sensor 
#include <OneWire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_AHTX0.h>
#include <DallasTemperature.h>
#include <SPI.h>

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature obj_tempSensor(&oneWire);

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

  bool increaseEnabled;
  bool decreaseEnabled;
};


Sensor temperature = {
  .name = "Temperature Sensor",
  .ref = 17.3f,
  .margin = 1.0f,
  .value = 0,
  .increaseEnabled = false,
  .decreaseEnabled = false
};

Sensor humidity = {
  .name = "Humidity Sensor",
  .ref = 73.0f,
  .margin = 3.0f,
  .value = 0,
  .increaseEnabled = false,
  .decreaseEnabled = false
};

Sensor pH = {
  .name = "pH Sensor",
  .ref = 8.8f,
  .margin = 1.0f,
  .value = 0,
  .increaseEnabled = false,
  .decreaseEnabled = false
};

Sensor pressure = {
  .name = "Pressure Sensor",
  .ref = 5.09f,
  .margin = 1.0f,
  .value = 0,
  .increaseEnabled = false,
  .decreaseEnabled = false
};

Sensor tds = {
  .name = "TDS Sensor",
  .ref = 313.0f,
  .margin = 1.0f,
  .value = 0,
  .increaseEnabled = false,
  .decreaseEnabled = false
};

Sensor ff = {
  .name = "Air Quality Sensor",
  .ref = 32.0f,
  .margin = 1.0f,
  .value = 0,
  .increaseEnabled = false,
  .decreaseEnabled = false
};

Sensor sensors[] = {temperature, humidity, pH, pressure, tds, ff};

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
  //Serial.print("Master: ");
  //Serial.println(message);
  //Serial.print("Slave: ");
  //Serial.println(retData);
  // Route by header letter: t=temperature, p=pH, r=pressure, d=dissolved solids (tds), f=particle (ff.value), h=humidity, m=margin of error temperature, n=margin of error humidity, e=engin
  switch (message[0]) {
    case 't':
      temperature.ref = retData.toFloat();
      Serial.print("InRef Temp: ");
      Serial.println(temperature.ref);
      Serial.println("");
      break;
    case 'p':
      pH.ref = retData.toFloat();
      Serial.print("InRef pH: ");
      Serial.println(pH.ref);
      Serial.println("");
      break;
    case 'r':
      pressure.ref = retData.toFloat();
      Serial.print("InRef Pressure: ");
      Serial.println(pressure.ref);
      Serial.println("");
      break;
    case 'd':
      tds.ref = retData.toFloat();
      Serial.print("InRef TDS: ");
      Serial.println(tds.ref);
      Serial.println("");
      break;
    case 'f':
      ff.ref = retData.toFloat();
      Serial.print("InRef Particle: ");
      Serial.println(ff.ref);
      Serial.println("");
      break;
    case 'h':
      humidity.ref = retData.toFloat();
      Serial.print("InRef Humidity: ");
      Serial.println(humidity.ref);
      Serial.println("");
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
      Serial.println("");
      break;
    default:
      Serial.print("No respond message: ");
      Serial.println(retData);
      Serial.println("");
      break;
  }
  //Serial.println("");
}

void SPIMasterSetup(){
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  esp.begin();
  sendESP("Hello Slave!");         // Unnecessary
}
///////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  obj_tempSensor.begin();    //OneWire temperature sensor
 	SPIMasterSetup();

  //if(!bmp.begin()){
  //  Serial.println("BMP180 not found!");
  //}
  pinMode(MOTOR_DATA, OUTPUT);
  pinMode(MOTOR_CLK, OUTPUT);
  digitalWrite(MOTOR_DATA, LOW);
  digitalWrite(MOTOR_CLK, LOW);

  while (aht10.begin() != true) //for ESP-01 use aht10.begin(0, 2);
  {
    Serial.println(F("AHT1x not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free

    delay(2000);
  }


  delay(3000);
}

void loop() {         // temperature.value/100, alt/100
  // put your main code here, to run repeatedly:
  

  //AdjustMotors(0b00000001);
  //AdjustMotors(0b00010110);
  //AdjustMotors(0);

  if((o_mtrID != mtrID) || (o_mtrData != mtrData)){
    uint8_t mtrData_L = mtrData & 0b11111111;
    uint8_t mtrdData_H = mtrData >> 8;
    AdjustMotors(mtrID);
    AdjustMotors(mtrData_L);
    AdjustMotors(mtrdData_H);
    o_mtrID = mtrID;
    o_mtrData = mtrData;
  }
  //AdjustMotors(mtrData);
  //mtrData++;
  ReadAndSendESP();
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

void ReadAndSendESP(){
  String ESPval = "";

  Serial.println("---");
  readOneWire();                    // Temperature
  ESPval = "t"+String(temperature.value);
  sendESP(ESPval.c_str());

  readpH();                         // pH
  ESPval = "p"+String(pH.value);
  sendESP(ESPval.c_str());

  readTDS();                        // Particle (TDS)
  ESPval = "d"+String(tds.value);
  sendESP(ESPval.c_str());

  readFlyingFish();
  ESPval = "f"+String(ff.value);
  sendESP(ESPval.c_str());

  readHumidity();
  ESPval = "h"+String(humidity.value);
  sendESP(ESPval.c_str());
  
  sendESP("e0");   // Takes motor adjustment values

  //Send Debug Values
  ESPval = "m"+String(temperature.margin);
  sendESP(ESPval.c_str());

  ESPval = "n"+String(humidity.margin);
  sendESP(ESPval.c_str());
}

void readOneWire(){
  obj_tempSensor.requestTemperatures(); 
  Serial.print("Temperature:");
  temperature.value = obj_tempSensor.getTempCByIndex(0) + 1.5;
  Serial.println(temperature.value); 
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
  pH.value = ph.readPH(voltage, temperature.value) + 0.3;
  Serial.println(pH.value);

  delay(10);
}

void readFlyingFish(){
  Serial.print("Flammable Gas: ");
  ff.value = analogRead(FLG_PIN);
  Serial.println(ff.value);

  delay(10);
}

void readTDS(){
  Serial.print("TDS: ");
  tds.value = analogRead(TDS_PIN);
  Serial.println(tds.value);

  delay(10);
}

void readHumidity(){
  aht10.getEvent(&aht10Hum, &aht10Temp);
  humidity.value = aht10Hum.relative_humidity;
  Serial.print("Humudity: ");
  Serial.println(humidity.value);
}

