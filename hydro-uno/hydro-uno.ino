// A0 -> Flammable Gas 
// A1 -> TDS
#define TDS_PIN A1
#define FLG_PIN A2
#define PH_PIN A0

#define CLK_PIN 11
#define TXDATA_PIN 10

// Data wire is conntec to the Arduino digital pin 4
#define ONE_WIRE_BUS 13

//#include "Adafruit_BMP085.h"    // BMP180 Air Pressure Sensor
#include "DFRobot_PH.h"         // DFTRobot Analog pH Sensor 
#include <OneWire.h>
#include <DallasTemperature.h>

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

//Adafruit_BMP085 bmp;
DFRobot_PH ph;

float temp = 30.0;
uint16_t pressure;
uint16_t tds;

//Communication variables     (bitwise suffix actually means its for communication)
uint16_t maxBitPerData = 16;

uint16_t bitwisephValue;
uint16_t bitwiseAlt;
uint16_t bitwiseTemp;
uint16_t particleRaw;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //if(!bmp.begin()){
  //  Serial.println("BMP180 not found!");
  //}

  pinMode(CLK_PIN, OUTPUT);
  pinMode(TXDATA_PIN, OUTPUT);
  digitalWrite(TXDATA_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);

  sensors.begin();
}

void loop() {         // temp/100, alt/100
  // put your main code here, to run repeatedly:

  


  

  readOneWire();
  readpH();

  //digitalWrite(CLK_PIN, HIGH);
  sendBitwiseData(bitwiseTemp);
  delay(1);
  sendBitwiseData(bitwisephValue);

  //readBMP();
  //readTDS();
  //readFlyingFish();
  delay(2000);
  //delay(5000);
}

void sendBitwiseData(uint32_t data){
  bool send_bit;
  Serial.println("");
  Serial.println(data);

  for(int i = maxBitPerData - 1; i >= 0; i--){
    send_bit = (data >> i) & 1;
    Serial.print(send_bit);
    digitalWrite(TXDATA_PIN, send_bit);
    digitalWrite(CLK_PIN, HIGH);
    delay(1);
    digitalWrite(CLK_PIN, LOW);
    delay(1);
  }
  Serial.println("");
  digitalWrite(TXDATA_PIN, LOW);
}

void readOneWire(){
  sensors.requestTemperatures(); 
  Serial.println("Temperature:");
  temp = sensors.getTempCByIndex(0) + 1.5;
  bitwiseTemp = (float)temp * 100;
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
  Serial.println("pH: ");
  float voltage = analogRead(PH_PIN)/1024.0*5000;
  float phValue = ph.readPH(voltage, temp) + 0.3;
  bitwisephValue = (float)phValue * 100;
  Serial.println(phValue);
  Serial.println("---");

  delay(10);
}

void readFlyingFish(){
  Serial.println("Flammable Gas: ");
  particleRaw = analogRead(FLG_PIN);
  Serial.println(particleRaw);
  Serial.println("---");
  Serial.println("");

  delay(10);
}

void readTDS(){
  Serial.println("TDS: ");
  tds = analogRead(TDS_PIN);
  Serial.println(tds);
  Serial.println("---");
  Serial.println("");

  delay(10);
}

