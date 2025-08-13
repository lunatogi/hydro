#define CLK_PIN D7
#define RXDATA_PIN D6
#define TXDATA_PIN D5

#define IDLE_STATE 0
#define RECEIVE_STATE 1
#define SEND_STATE 2

bool clk = 0;
bool prevClk = 0;

//Communication
uint16_t rx_data = 0;
uint16_t bitCounter = 0;
uint16_t sentBitCounter = 0;

uint16_t maxBitPerData = 16;

int state = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("---ESP STARTED---");


  pinMode(CLK_PIN, INPUT);
  pinMode(RXDATA_PIN, INPUT);
  pinMode(TXDATA_PIN, OUTPUT);
  digitalWrite(TXDATA_PIN, LOW);

  state = SEND_STATE;
}

void loop() {


  switch(state){
    case IDLE_STATE:
      Serial.println("IDLE STATE");
      break;
    case RECEIVE_STATE:
      //Serial.println("RECEIVE STATE");
      
      break;
    case SEND_STATE: 
      Serial.println("SEND STATE");
      sendBitwiseData(43690);
      break;
  }

  
  delay(1);
}

void sendUNO(){

}

void sendBitwiseData(uint16_t data){
  bool send_bit;
  //Serial.println("Trying to send");
  if(sentBitCounter < maxBitPerData){
    //Serial.println("Bit counter less than max bit");
    clk = digitalRead(CLK_PIN);
    //Serial.println(clk);
    if(clk && !prevClk){                                                //Send at each posedge clk
      Serial.println("Posedge");
      send_bit = (data >> (maxBitPerData-(1 + sentBitCounter))) & 1;    //Starting from MSB
      Serial.print("Sent bit: ");
      Serial.println(send_bit);
      Serial.println(sentBitCounter);
      digitalWrite(TXDATA_PIN, send_bit);
      sentBitCounter++;
      if(sentBitCounter == maxBitPerData){
        sentBitCounter = 0;
        digitalWrite(TXDATA_PIN, LOW);
      }
    }
    prevClk = clk;
  }
  
}


void listenUNO(){
  clk = digitalRead(CLK_PIN);
  if(clk && !prevClk){
    bitCounter++;
    bool rx_bit = digitalRead(RXDATA_PIN);
    //Serial.println(rx_bit);
    rx_data = (rx_data << 1) | (rx_bit & 1);
    //Serial.println(data);
    //Serial.println(String("BC: ")+bitCounter);
    if(bitCounter == maxBitPerData){
      float exactData = (float)rx_data/100;
      Serial.println(exactData);
      rx_data = 0;
      bitCounter = 0;
    }

  }
  prevClk = clk;
}
