#include <Arduino.h>

//File System
#include <FS.h>

//Fast LED
#define FASTLED_INTERNAL
#define NUM_LEDS 256
#define LED_PIN 5
#include <FastLED.h>
CRGB g_LEDs[NUM_LEDS] = {0};

//WiFi and Web
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
const char* ssid = "ZORGLUB";
const char* passwd = "Vive la Bretagne !";

//WebServer
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);

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

  system_update_cpu_freq(160);

  //Turn built-in LED OFF
  pinMode(LED_BUILTIN, OUTPUT);
  bool bLED = 0;
  digitalWrite(LED_BUILTIN, bLED);

  //Serial setup
  Serial.begin(115200);
  while(!Serial){};
  Serial.println("NODEMCU Serial UP");
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

/**************************************************************************************
 *                                MAIN LOOP
 *************************************************************************************/
void loop() {
  FastLED.show();
  delay(25);
  server.handleClient();
}

/**************************************************************************************
 *                                TOOLS
 *************************************************************************************/

void setLEDsWithSketch(String sketch) {
  char p[sketch.length()];
  for (int i = 0; i < sizeof(p); i++) {
    p[i] = sketch[i];
  }

  char *ptr = strtok(p, ",");
  uint8_t ledIndex = 0;
  while (ptr != NULL && ledIndex < NUM_LEDS)
  {
    g_LEDs[ledIndex] = pico8_palette[strtol(ptr, NULL, 10)];
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
