#define CLK_PIN D7
#define RXDATA_PIN D6
#define TXDATA_PIN D5

bool clk = 0;
bool prevClk = 0;
bool dataFull = false;

uint16_t data = 0;
uint16_t bitCounter = 0;

uint16_t maxBitPerData = 16;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("---ESP STARTED---");


  pinMode(CLK_PIN, INPUT);
  pinMode(RXDATA_PIN, INPUT);

}

void loop() {
  //Serial.println("CLK: ");
  //Serial.println(clk);
  //Serial.println("PrevCLK");
  //Serial.println(prevClk);
  listenUNO();
}



void listenUNO(){
  clk = digitalRead(CLK_PIN);
  if(clk && !prevClk){
    bitCounter++;
    bool takenData = digitalRead(RXDATA_PIN);
    //Serial.println(takenData);
    data = (data << 1) | (takenData & 1);
    //Serial.println(data);
    //Serial.println(String("BC: ")+bitCounter);
    if(bitCounter == maxBitPerData){
      float exactData = (float)data/100;
      Serial.println(exactData);
      data = 0;
      bitCounter = 0;
      dataFull = true;
    }

  }
  prevClk = clk;
}
