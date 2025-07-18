// A0 -> Flammable Gas 
// A1 -> TDS
#define TDS_PIN A1
#define FLG_PIN A0
#define PH_PIN A5

#include "Adafruit_BMP085.h"    // BMP180 Air Pressure Sensor
#include "DFRobot_PH.h"         // DFTRobot Analog pH Sensor 

Adafruit_BMP085 bmp;
DFRobot_PH ph;

float temp;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if(!bmp.begin()){
    Serial.println("BMP180 not found!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:


  readBMP();
  //readFlyingFish();
  //readpH();
  //readTDS();

  delay(500);
}

void readBMP(){

  float pressure = bmp.readPressure();
  float temperature = bmp.readTemperature();
  float alt = bmp.readAltitude();

  Serial.println("<ST>");
  Serial.println("Air Pressure: ");
  Serial.println(pressure);

  delay(10);

  /*
  Serial.println("Temperature: ");
  
  temp = temperature;
  Serial.println(temp);
  Serial.println("---");
  Serial.println("");

  delay(10);

  Serial.println("Altitude: ");
  
  Serial.println(alt);
  Serial.println("---");
  Serial.println("");

  delay(10);
  */
}

void readpH(){
  Serial.println("pH: ");
  float voltage = analogRead(PH_PIN)/1024.0*5000;
  float phValue = ph.readPH(voltage, temp); 
  Serial.println(phValue);
  Serial.println("---");

  delay(10);
}

void readFlyingFish(){
  Serial.println("Flammable Gas: ");
  float particleRaw = analogRead(FLG_PIN);
  Serial.println(particleRaw);
  Serial.println("---");
  Serial.println("");

  delay(10);
}

void readTDS(){
  Serial.println("TDS: ");
  int tds = analogRead(TDS_PIN);
  Serial.println(tds);
  Serial.println("---");
  Serial.println("");

  delay(10);
}

