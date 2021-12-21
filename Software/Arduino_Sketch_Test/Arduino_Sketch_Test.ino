//General
#define EEPROM_SIZE 2

//Fast LED
#define NUM_LEDS 256
#define FASTLED_INTERNAL

//PINS Config
#define LED_PIN 22
#define BR_PLUS_PIN 23
#define BR_MINUS_PIN 21
#define POWER_SW 19

#include <Arduino.h>
#include <string.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <WiFiManager.h>
#include "SPIFFS.h"
#include <WebServer.h>

//File System
const char* sketchDirPath = "/imgs";
File sketchDir;

//FastLED
CRGB g_LEDs[NUM_LEDS] = {0};
uint8_t brightness = 50;
uint8_t desiredBrightness = 50;

//WebServer
WebServer server(80);

//Pico-8 inspired color palette
const CRGB pico8_palette[16] = {
  CRGB(0x000000),             CHSV(HUE_BLUE,170,150),     CHSV(HUE_PURPLE,200,200) ,  CHSV(HUE_GREEN,180,120), 
  CHSV(HUE_ORANGE, 255, 100), CHSV(HUE_ORANGE, 20, 120),      CHSV(HUE_RED, 0, 190),      CHSV(HUE_ORANGE, 1, 255), 
  CHSV(HUE_RED, 230, 255),    CHSV(HUE_ORANGE, 220, 255), CHSV(HUE_YELLOW, 255, 255), CHSV(HUE_GREEN,255,255), 
  CHSV(HUE_BLUE, 130, 255),   CHSV(HUE_PURPLE,120,200),   CHSV(HUE_PINK,150,255),     CHSV(HUE_ORANGE, 160, 255)
};

const int refreshRateValues[6] = {
  1000,
  5000,
  10000,
  30000,
  60000,
  300000
};

int refreshRateIndex = 2;

/**************************************************************************************
 *                                SETUP
 *************************************************************************************/

void setup() {

  EEPROM.begin(EEPROM_SIZE);
  brightness = EEPROM.read(0);
  refreshRateIndex = EEPROM.read(1);

  //PINS
  pinMode(LED_PIN, OUTPUT);
  pinMode(BR_PLUS_PIN, INPUT);
  pinMode(BR_MINUS_PIN, INPUT);
  pinMode(POWER_SW, INPUT);

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
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(g_LEDs, NUM_LEDS);
  FastLED.setCorrection(LEDColorCorrection::TypicalLEDStrip);
  FastLED.setTemperature(ColorTemperature::HighNoonSun);
  FastLED.setBrightness(brightness);

  //Wifi Manager
  WiFiManager wm;
  bool connectionStatus;
  connectionStatus = wm.autoConnect("Cadre Magique");
  if(!connectionStatus) {
    Serial.println("Failed to connect");
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
  
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

int maxPower = 1500;
int minPower = 100;

//buttons
unsigned long debounceDelay = 50;
int lastPlusState = LOW;
int lastMinusState = LOW;
int plusState = LOW;
int minusState = LOW;
unsigned long lastPlusDebounceTime = 0;
unsigned long lastMinusDebounceTime = 0;

void loop() {
  
  //Brightness buttons
  int plusRead = digitalRead(BR_PLUS_PIN);
  int minusRead = digitalRead(BR_MINUS_PIN);
  
  if (plusRead != lastPlusState) {
    lastPlusDebounceTime = millis();
  }

  if (minusRead != lastMinusState) {
    lastMinusDebounceTime = millis();
  }

  if ((millis() - lastPlusDebounceTime) > debounceDelay) {
    if (plusRead != plusState) {
      plusState = plusRead;
      if (plusState == HIGH) {
        Serial.println("Brightness +");

        if(desiredBrightness < brightness) {
          desiredBrightness = brightness;
        }
        
        if(desiredBrightness >= 245) {
          desiredBrightness = 255;
        } else {
          desiredBrightness += 10;
        }

        //save selected brightness to eeprom ( flash)
        EEPROM.write(0, desiredBrightness);
        EEPROM.commit();
        displayPicture();
      }
    }
  }

  if ((millis() - lastMinusDebounceTime) > debounceDelay) {
    if (minusRead != minusState) {
      minusState = minusRead;
      if (minusState == HIGH) {
        Serial.println("Brightness -");

        if(desiredBrightness > brightness) {
          desiredBrightness = brightness;
        }
        
        if(desiredBrightness <= 10) {
          desiredBrightness = 0;
        } else {
          desiredBrightness -= 10;
        }

        //save selected brightness to eeprom ( flash)
        EEPROM.write(0, desiredBrightness);
        EEPROM.commit();
        displayPicture();
      }
    }
  }
  
  lastPlusState = plusRead;
  lastMinusState = minusRead;
  
  //HTTP Server
  server.handleClient();

  //Picture play loop
  if(firstBoot || (playmode && (millis() - lastImageShowedMillis >= refreshRateValues[refreshRateIndex])) ) {
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
      displayPicture();
      lastImageShowedMillis = millis();
      sketchFile.close();
    } else {
      sketchDir = SPIFFS.open(sketchDirPath); //Re-open to start listing files from beginning
    }
  }
}

/**************************************************************************************
 *                                TOOLS
 *************************************************************************************/

void displayPicture(){
  //Limit brightness if power isn't plugged (working only on USB)
  int powerPlugged = digitalRead(POWER_SW);
  
  if ( powerPlugged == HIGH ) {
    brightness = calculate_max_brightness_for_power_vmA(g_LEDs, NUM_LEDS, desiredBrightness, 5, maxPower); 
  } else {
    brightness = calculate_max_brightness_for_power_vmA(g_LEDs, NUM_LEDS, desiredBrightness, 5, minPower); 
  }

  Serial.print("Desired brightness: ");
  Serial.println(desiredBrightness);
  Serial.print("Real brightness: ");
  Serial.println(brightness);

  //Display
  FastLED.setBrightness(brightness);
  FastLED.show();
  delay(10);
}

bool validateSketch(const char* sketch) {
  Serial.println(sketch);
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
    file.close();
    return true;
  } else {
    //Display sketch
    if(path == "/display.html"){
      String sketch = server.arg("sketch");
      if(validateSketch(sketch.c_str())){
        setLEDsWithSketch(sketch.c_str());
        displayPicture();
        server.send(200, "text/plain", "Sketch diplayed !");
        delay(5000);
      } else {
        Serial.println("Sketch invalid :");
        Serial.println(sketch);
        server.send(500, "text/plain", "Error, sketch invalid !");
      }
      return true;
    }

    //Save sketch
    if(path == "/save.html"){
      String sketch = server.arg("sketch");
      String fileName = server.arg("fileName");

      if(validateSketch(sketch.c_str())) {
        char sketchSavePath[31];
        strcat(sketchSavePath, sketchDirPath);
        strcat(sketchSavePath, "/");
        strcat(sketchSavePath, fileName.c_str());
        strcat(sketchSavePath, ".txt");
        Serial.print("Saving to : ");
        Serial.println(sketchSavePath);      
        File file = SPIFFS.open(sketchSavePath, FILE_WRITE);
        if(file){
          file.println(sketch.c_str());
          file.close();
          Serial.print("File saved :");
          Serial.println(sketchSavePath);
          setLEDsWithSketch(sketch.c_str());
          displayPicture();
          server.send(200, "text/plain", "File saved !");
          delay(5000);
        } else {
          Serial.println("Error opening file");
          server.send(500, "text/plain", "Error, file not saved :(");
        }
      } else {
        Serial.println("Sketch invalid :");
        Serial.println(sketch);
        server.send(500, "text/plain", "Error, sketch invalid !");
      }
      return true;
    }

    //List sketches
    if(path == "/getList.html"){
      String response;
      sketchDir = SPIFFS.open(sketchDirPath);
      File sketchFile = sketchDir.openNextFile();
      while(sketchFile) {
        Serial.print("Getting file ");
        Serial.println(sketchFile.name());
        response += sketchFile.name();
        response += ",";
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
        response += sketch;
        response += "|";
        sketchFile = sketchDir.openNextFile();
      }
      
      server.send(200, "text/plain", response);
      return true;
    }

    //Delete sketches
    if(path == "/delete.html"){
      sketchDir.close();
      String sketchNameHttp = server.arg("sketchName");
      Serial.println(sketchNameHttp);
 
      String filePath = "/imgs/";
      filePath += sketchNameHttp;
      filePath += ".txt";
      Serial.print("Deleting : ");
      Serial.println(filePath);
      SPIFFS.remove(filePath);

      sketchDir = SPIFFS.open(sketchDirPath);
      server.send(200, "text/plain", "Ok");
      return true;
    }

    //Save config
    if(path == "/saveconfig.html"){
      String tempo = server.arg("tempo");
      refreshRateIndex = atoi(tempo.c_str());
      
      //save selected refresh to eeprom ( flash)
      EEPROM.write(1, refreshRateIndex);
      EEPROM.commit();
        
      String response = "<!DOCTYPE html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'/><html><body>";
      response += "Configuration appliquée...<br/><br/>";
      response += "<br/><br/>";
      response += "<a href='index.html'>Retour à la page d'acceuil<a></body></html>";
      server.send(200, "text/html", response);
      return true;
    }
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
