#include <Arduino.h>
#include <string.h>

//File System
#include "SPIFFS.h"
const char* sketchDirPath = "/sketches";
File sketchDir;

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


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>HTML Form to Input Data</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem; color: #FF0000;}
  </style>
  </head><body>
  <h2>HTML Form to Input Data</h2> 
  <form action="/get">
    Enter a string: <input type="text" name="input_string">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    Enter an integer: <input type="text" name="input_integer">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    Enter a floating value: <input type="text" name="input_float">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";


/**************************************************************************************
 *                                SETUP
 *************************************************************************************/

void setup() {
  
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
    sketchDir = SPIFFS.open(sketchDirPath);
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

bool playmode = true;
bool firstBoot = true;
uint32_t lastImageShowedMillis = 0;
uint8_t lastImageShowedIndex = 0;

void loop() {
  server.handleClient();
  FastLED.show();
  delay(10);
  
  if(firstBoot || (playmode && (millis() - lastImageShowedMillis >= 5000)) ) {
    firstBoot = false;
    File sketchFile = sketchDir.openNextFile();
    if(sketchFile) {
      Serial.print("Showing sketch: ");
      Serial.println(sketchFile.name());
      
      //array to store a sketch
      //a sketch is 256 chars
      char sketch[NUM_LEDS];
      char currentChar = ' ';
      uint16_t sketchIndex = 0;
      while(sketchIndex < NUM_LEDS){
        currentChar = sketchFile.read();
        sketch[sketchIndex] = currentChar;
        sketchIndex++;
      }
      setLEDsWithSketch(sketch);
      lastImageShowedMillis = millis();
      sketchFile.close();
    } else {
      sketchDir.close();
      sketchDir = SPIFFS.open(sketchDirPath);
    }
  }
}

/**************************************************************************************
 *                                TOOLS
 *************************************************************************************/

bool validateSketch(const char* sketch) {
  if(strlen(sketch) != NUM_LEDS){
    Serial.print("Invalide length: ");
    Serial.println(strlen(sketch));
    return false;
  }
  
  for(int i = 0; i < NUM_LEDS; i++){
    //Check if all values are between 0 and 15
    if( (int)sketch[i]-65 < 0 || (int)sketch[i]-65 > 15 ){
      Serial.print("Invalide character: ");
      Serial.print(sketch[i]);
      Serial.print(" , index: ");
      Serial.println((int)sketch[i]-65);
      return false;
    }
  }
  return true;
}

void setLEDsWithSketch(const char* sketch) {
  for(int ledIndex = 0; ledIndex < NUM_LEDS; ledIndex++) {
    g_LEDs[ledIndex] = pico8_palette[(int)sketch[ledIndex] - 65]; //-65 because ASCII code for A = 65 
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
  Serial.println("Requete reçue : " + path);
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r"); 
    size_t sent = server.streamFile(file, contentType);


    //Display sketch
    if(path == "/display.html"){
      String sketch = server.arg("sketch");
      if(validateSketch(sketch.c_str())){
        setLEDsWithSketch(sketch.c_str());
        FastLED.show();
        delay(5000);
      } else {
        Serial.println("Sketch invalid :");
        Serial.println(sketch);
      }
    }

    //Save sketch
    if(path == "/save.html"){
      String sketch = server.arg("sketch");
      String fileName = server.arg("fileName");

      if(validateSketch(sketch.c_str())) {

        char sketch_char[NUM_LEDS];
        for (int i = 0; i < NUM_LEDS; i++) {
          sketch_char[i] = sketch[i];
        }
        char sketchSavePath[100];
        strcat(sketchSavePath, sketchDirPath);
        strcat(sketchSavePath, "/");
        strcat(sketchSavePath, fileName.c_str());
        strcat(sketchSavePath, ".txt");
              
        File file = SPIFFS.open(sketchSavePath, FILE_WRITE);
        if(file){
          file.println(sketch.c_str());
          file.close();
          Serial.print("File saved :");
          Serial.print(sketchSavePath);
        } else {
          Serial.print("Error opening file");
        }
        setLEDsWithSketch(sketch.c_str());
        FastLED.show();
        delay(5000);
      } else {
        Serial.println("Sketch invalid :");
        Serial.println(sketch);
      }
    }

    //List sketches
    if(path == "/list.html"){
      
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
