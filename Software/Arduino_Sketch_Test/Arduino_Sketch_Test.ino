#include <Arduino.h>

//File System
#include "SPIFFS.h"

//Fast LED
#define NUM_LEDS 256
#define LED_PIN 22
#include <FastLED.h>
CRGB g_LEDs[NUM_LEDS] = {0};

//WiFi and Web
#include <WiFi.h>
#include <WiFiClient.h>
const char* ssid = "ZORGLUB";
const char* passwd = "Vive la Bretagne !";

//WebServer
#include <WebServer.h>
WebServer server(80);

//Pico-8 inspired color palette
const CRGB pico8_palette[16] = {
  CRGB(0x000000),             CHSV(HUE_BLUE,170,150),     CHSV(HUE_PURPLE,200,200) ,  CHSV(HUE_GREEN,180,120), 
  CHSV(HUE_ORANGE, 255, 100), CHSV(HUE_ORANGE, 20, 120),      CHSV(HUE_RED, 0, 190),      CHSV(HUE_ORANGE, 1, 255), 
  CHSV(HUE_RED, 230, 255),    CHSV(HUE_ORANGE, 220, 255), CHSV(HUE_YELLOW, 255, 255), CHSV(HUE_GREEN,255,255), 
  CHSV(HUE_BLUE, 130, 255),   CHSV(HUE_PURPLE,120,200),   CHSV(HUE_PINK,150,255),     CHSV(HUE_ORANGE, 160, 255)
};
                            
/**************************************************************************************
 *                                SETUP
 *************************************************************************************/
void setup() {

  //Turn built-in LED OFF
//  pinMode(LED_BUILTIN, OUTPUT);
//  bool bLED = 0;
//  digitalWrite(LED_BUILTIN, bLED);
  
  //Serial setup
  Serial.begin(115200);
  while(!Serial){};
  Serial.println("NODEMCU-ESP32 Serial UP");
  Serial.printf("CPU freq. : %u\n",ESP.getCpuFreqMHz());

  //Mount file System
  if(!SPIFFS.begin())
  {
    Serial.println("SPIFFS Mount failed");
  } else {
    Serial.println("SPIFFS Mount succesfull");
  }

  //FastLED setup
  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(g_LEDs, NUM_LEDS);
  FastLED.setCorrection(LEDColorCorrection::TypicalLEDStrip);
  FastLED.setTemperature(ColorTemperature::HighNoonSun);
  FastLED.setBrightness(50);

  //Station mode STA
  WiFi.begin(ssid, passwd);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to WiFi with IP : ");
  Serial.println(WiFi.localIP());

  //Http Server
  server.onNotFound([]() { 
    // If the client requests any URI
    if (!handleFileRead(server.uri())) // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });
  server.begin();
  Serial.println("HTTP server Running on port 80");
}

bool playmode = true;

/**************************************************************************************
 *                                MAIN LOOP
 *************************************************************************************/
void loop() {
  server.handleClient();
  FastLED.show();
  delay(250);

  if(playmode) {

    if (SPIFFS.exists("/sketches.txt")) {
      File file = SPIFFS.open("/sketches.txt", "r"); 

      //array to store a sketch
      //a sketch is maximum 758 chars
      char sketch[758] = {' '};
      char currentChar = ' ';
  
      uint16_t sketchIndex = 0;
      while(currentChar != '\n'){
        currentChar = file.read();
        sketch[sketchIndex] = currentChar;
        sketchIndex++;
      }

      setLEDsWithSketch(&sketch[0]);
    }
  }
}

/**************************************************************************************
 *                                TOOLS
 *************************************************************************************/

void setLEDsWithSketch(String sketch) {
  char p[sketch.length()];
  for (int i = 0; i < sizeof(p); i++) {
    p[i] = sketch[i];
  }

  Serial.println("setLEDsWithSketch: ");
  Serial.println(p);
  
  char *ptr = strtok(p, ",");
  uint16_t ledIndex = 0;
  while (ptr != NULL && ledIndex < NUM_LEDS)
  {
    Serial.print("(");
    Serial.print(ledIndex);
    Serial.print(",");
    g_LEDs[ledIndex] = pico8_palette[strtol(ptr, NULL, 10)];
    Serial.print(ptr);
    Serial.print(");");
    ptr = strtok(NULL, ",");
    ledIndex++;
  }
}

void setLEDsWithSketch(char* sketch) {
  char *ptr = strtok(sketch, ",");
  uint16_t ledIndex = 0;
  while (ptr != NULL && ledIndex < NUM_LEDS)
  {
    Serial.print("(");
    Serial.print(ledIndex);
    Serial.print(",");
    g_LEDs[ledIndex] = pico8_palette[strtol(ptr, NULL, 10)];
    Serial.print(ptr);
    Serial.print(");");
    ptr = strtok(NULL, ",");
    ledIndex++;
  }
}

void drawPalette() {
  for(int index=0; index < NUM_LEDS; index++){
    g_LEDs[index] = pico8_palette[0];  
  }

  g_LEDs[63] = pico8_palette[0];
  g_LEDs[62] = pico8_palette[1];
  g_LEDs[61] = pico8_palette[2];
  g_LEDs[60] = pico8_palette[3];
  g_LEDs[32] = pico8_palette[4];
  g_LEDs[33] = pico8_palette[5];
  g_LEDs[34] = pico8_palette[6];
  g_LEDs[35] = pico8_palette[7];
  g_LEDs[31] = pico8_palette[8];
  g_LEDs[30] = pico8_palette[9];
  g_LEDs[29] = pico8_palette[10];
  g_LEDs[28] = pico8_palette[11];
  g_LEDs[0] = pico8_palette[12];
  g_LEDs[1] = pico8_palette[13];
  g_LEDs[2] = pico8_palette[14];
  g_LEDs[3] = pico8_palette[15];
}

/**************************************************************************************
 *                                HTTP Server
 *************************************************************************************/
bool handleFileRead(String path) {
  //Serial.println("Requete reçue : " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r"); 
    size_t sent = server.streamFile(file, contentType);
    if(path == "/save.html"){
      String sketch = server.arg("sketch");
      Serial.print("Sketch reçu : ");
      Serial.println(sketch);
      setLEDsWithSketch(sketch);
    }
    file.close();
    return true;
  }
  return false; // If the file doesn't exist, return false
}

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  return "text/plain";
}
