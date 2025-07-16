// A0 -> Flammable Gas 
// A1 -> TDS
#include "Adafruit_BMP085.h"    // For BMP180 Air Pressure Sensor

Adafruit_BMP085 bmp;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if(!bmp.begin()){
    Serial.println("BMP180 not found!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:


  Serial.println("TDS: ");
  int tds = analogRead(A1);
  Serial.println(tds);
  Serial.println("---");
  Serial.println("");

  delay(10);

  // Flammable Gas Sensor
  Serial.println("Flammable Gas: ");
  int particleRaw = analogRead(A0);
  Serial.println(particleRaw);
  Serial.println("---");
  Serial.println("");

  delay(10);

  Serial.println("Air Pressure: ");
  int pressure = bmp.readPressure();
  Serial.println(pressure);
  Serial.println("---");
  Serial.println("");

  delay(10);

  Serial.println("Temperature: ");
  int temp = bmp.readTemperature();
  Serial.println(temp);
  Serial.println("---");
  Serial.println("");

  delay(10);

  Serial.println("Altitude: ");
  int alt = bmp.readAltitude();
  Serial.println(alt);
  Serial.println("---");
  Serial.println("");

  delay(10);

  delay(1000);
}
