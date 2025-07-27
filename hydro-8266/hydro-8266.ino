#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>     

#define CLK_PIN D7
#define DATA_PIN D6

//WiFi Config
const char* ssid = "KET0";
const char* password = "keto4522";

ESP8266WebServer server(80);    // port num 80

bool clk = 0;
bool prevClk = 0;
bool dataFull = false;

uint32_t data = 0;
uint32_t bitCounter = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("---ESP STARTED---");


  pinMode(CLK_PIN, INPUT);
  pinMode(DATA_PIN, INPUT);

  initWebServer();
}

void loop() {
  server.handleClient();
}

void initWebServer(){
  //Web Server
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(WiFi.status());
    delay(500);
    //Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP Started");
}



void handle_OnConnect() {

  float temp = 27.6;
  float hum = 11.2;

  // Generate HTML content
  String html = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  html += "<!DOCTYPE html><html><head><title>Sensor Data</title></head><body>";
  html += "<h1> Sensor Readings</h1>";
  html += "<p>Temperature: " + String(temp) + " °C</p>";
  html += "<p>Humidity: " + String(hum) + " %</p>";
  html += "</body></html>";

  server.send(200, "text/html", html); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

void sendData(){
  pinMode(DATA_PIN, OUTPUT);
  delay(5);
  digitalWrite(DATA_PIN, HIGH);
  
  delay(5);
  dataFull = false;
}

void listenClk(){
  if(clk && !prevClk){
    bitCounter++;
    bool takenData = digitalRead(DATA_PIN);
    Serial.println(takenData);
    data = (data << 1) | (takenData & 1);
    Serial.println(data);
    Serial.println(String("BC: ")+bitCounter);
    if(bitCounter == 32){
      float exactData = (float)data/100;
      Serial.println(exactData);
      data = 0;
      bitCounter = 0;
      dataFull = true;
    }

  }
  prevClk = clk;
}
