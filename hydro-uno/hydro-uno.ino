/*          SPI CONNECTIONS
    GPIO    NodeMCU   Name  |   Uno
   ===================================
     15       D8       SS   |   D10
     13       D7      MOSI  |   D11
     12       D6      MISO  |   D12
     14       D5      SCK   |   D13
*/
#define PH_PIN A0
#define TDS_PIN A1
#define FLG_PIN A2

// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 8

//#include "Adafruit_BMP085.h"    // BMP180 Air Pressure Sensor
#include "DFRobot_PH.h"         // DFTRobot Analog pH Sensor 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

//Adafruit_BMP085 bmp;
DFRobot_PH ph;

//Current and ref sensor values
float temp = 30.0;
float phValue = 0;
float pres = 2.21;
float tds = 2;
float ff = 1;

float refTemp = 17.3;
float refpH = 8.8;
float refPres = 5.09;
int refTDS = 313;
float refFF = 32;
///////////////////////////////

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
  // Route by header letter: t=temperature, p=pH, r=pressure, d=dissolved solids (tds), f=particle (ff)
  switch (message[0]) {
    case 't':
      refTemp = retData.toFloat();
      Serial.print("InRef Temp: ");
      Serial.println(refTemp);
      break;
    case 'p':
      refpH = retData.toFloat();
      Serial.print("InRef pH: ");
      Serial.println(refpH);
      break;
    case 'r':
      refPres = retData.toFloat();
      Serial.print("InRef Pressure: ");
      Serial.println(refPres);
      break;
    case 'd':
      refTDS = retData.toInt();
      Serial.print("InRef TDS: ");
      Serial.println(refTDS);
      break;
    case 'f':
      refFF = retData.toFloat();
      Serial.print("InRef Particle: ");
      Serial.println(refFF);
      break;
    default:
      Serial.println("Unknown message header");
      break;
  }
  Serial.println("");
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
  sensors.begin();    //OneWire temperature sensor
 	SPIMasterSetup();

  //if(!bmp.begin()){
  //  Serial.println("BMP180 not found!");
  //}
}

void loop() {         // temp/100, alt/100
  // put your main code here, to run repeatedly:
  Run();
  delay(1000);
}

void Run(){
  Serial.println("---");
  readOneWire();                    // Temperature
  String tempS = "t"+String(temp);
  sendESP(tempS.c_str());

  readpH();                         // pH
  String phS = "p"+String(phValue);
  sendESP(phS.c_str());

  readTDS();                        // Particle (TDS)
  String tdsS = "d"+String(tds);
  sendESP(tdsS.c_str());

  readFlyingFish();
  String ffS = "f"+String(ff);
  sendESP(ffS.c_str());
}


void readOneWire(){
  sensors.requestTemperatures(); 
  Serial.print("Temperature:");
  temp = sensors.getTempCByIndex(0) + 1.5;
  Serial.println(temp); 
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
  temp = temperature;
  Serial.println(temp);

  //Serial.println("Altitude:");
  //Serial.println(bitwiseAlt);
  //Serial.println("");

  delay(10);
}
*/

void readpH(){
  Serial.print("pH: ");
  float voltage = analogRead(PH_PIN)/1024.0*5000;
  phValue = ph.readPH(voltage, temp) + 0.3;
  Serial.println(phValue);

  delay(10);
}

void readFlyingFish(){
  Serial.print("Flammable Gas: ");
  ff = analogRead(FLG_PIN);
  Serial.println(ff);

  delay(10);
}

void readTDS(){
  Serial.print("TDS: ");
  tds = analogRead(TDS_PIN);
  Serial.println(tds);

  delay(10);
}

