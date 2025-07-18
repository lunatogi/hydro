#include <SoftwareSerial.h>

SoftwareSerial arduinoSerial(D7, D6); // rx = D7, tx = D6

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  arduinoSerial.begin(9600);
  Serial.println("---ESP STARTED---");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("From ESP: ");
  //char buffer[20] = "";
  
  if (arduinoSerial.available()){
    for(int i = 0; i < 2; i++){
      String buffer = arduinoSerial.readStringUntil('\n');
      Serial.println(buffer);
    }
    Serial.println("");
  }else{
    Serial.println("No message from Arduino");
  }
  delay(500);
}
