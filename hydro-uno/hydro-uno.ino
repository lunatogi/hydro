// A0 -> Flammable Gas 
// A1 -> TDS
#define TDS_PIN A1
#define FLG_PIN A0
#define PH_PIN A5

#define CLK_PIN 11
#define DATA_PIN 10

#include "Adafruit_BMP085.h"    // BMP180 Air Pressure Sensor
#include "DFRobot_PH.h"         // DFTRobot Analog pH Sensor 

Adafruit_BMP085 bmp;
DFRobot_PH ph;

float temp;

uint32_t bitwiseAlt;
uint32_t bitwiseTemp;
uint32_t pressure;

uint32_t bitwisephValue;

uint32_t particleRaw;

uint32_t tds;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if(!bmp.begin()){
    Serial.println("BMP180 not found!");
  }

  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLK_PIN, LOW);
}

void loop() {         // temp/100, alt/100
  // put your main code here, to run repeatedly:

  readBMP();

  delay(100);

  sendBitwiseData(bitwiseTemp);


  
  //readFlyingFish();
  //readpH();
  //readTDS();

  delay(5000);
}

void sendBitwiseData(uint32_t data){
  bool send_bit;
  Serial.println("");
  Serial.println(data);

  for(int i = 31; i >= 0; i--){
    send_bit = (data >> i) & 1;
    Serial.print(send_bit);
    digitalWrite(DATA_PIN, send_bit);
    digitalWrite(CLK_PIN, HIGH);
    delay(5);
    digitalWrite(CLK_PIN, LOW);
    delay(5);
  }
  Serial.println("");
  digitalWrite(DATA_PIN, LOW);
}

void readBMP(){

  pressure = bmp.readPressure();
  float temperature = bmp.readTemperature();
  bitwiseTemp = temperature * 100;     // So we can get rid of .00 
  float alt = bmp.readAltitude();     
  bitwiseAlt = alt * 100;

  Serial.println("Air Pressure:");
  Serial.println(pressure);

  Serial.println("Temperature:");
  temp = temperature;
  Serial.println(bitwiseTemp);

  Serial.println("Altitude:");
  Serial.println(bitwiseAlt);
  Serial.println("");

  delay(10);
}

void readpH(){
  Serial.println("pH: ");
  float voltage = analogRead(PH_PIN)/1024.0*5000;
  float phValue = ph.readPH(voltage, temp);
  bitwisephValue = phValue * 100;
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

