#define CLK_PIN D7
#define DATA_PIN D6

bool prevClk = 0;

uint32_t data = 0;
int bitCounter = -1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("---ESP STARTED---");


  pinMode(CLK_PIN, INPUT);
  pinMode(DATA_PIN, INPUT);
}

void loop() {

  bool clk = digitalRead(CLK_PIN);
  
  if(clk && !prevClk){
    bitCounter++;
    bool takenData = digitalRead(DATA_PIN);
    Serial.println(takenData);
    data = (data << 1) | (takenData & 1);
    Serial.println(data);
    Serial.println(String("BC: ")+bitCounter);
    if(bitCounter == 32){
      float exactData = data/100;
      //Serial.println(exactData);
      data = 0;
      bitCounter = 0;
    }

  }



  prevClk = clk;
}
