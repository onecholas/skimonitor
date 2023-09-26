// Version 4.6: Graphing complete

// Import required libraries
#include <FS.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <cstdint>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP3XX.h>
#include <Adafruit_Sensor.h>
#include <TinyGPSPlus.h>

// Graphing
String graph_top_run_vel;

// Data storage
double all_top_run_vel[100];

// File
File file_write;
File file_read;

// Battery
const float minVoltage = 3;  // Replace with your battery's specifications
const float  maxVoltage = 4.2;  // Replace with your battery's specifications
float batteryVoltage = 0;
float batteryPercentage = 0;
String displaybatteryPercentage = "N/A";

// Reduce event sending
unsigned long prevMillis = 0;
unsigned long currMillis = 0;
const unsigned long intervalMillis = 1000;

// Multithreading
TaskHandle_t ledTask;

// GPS Related
TinyGPSPlus gps;
double run_vel = 0;
String displayrun_vel = "N/A";
double top_run_vel = 0;
String displaytop_run_vel = "N/A";
double overall_top_run_vel = 0;
String displayoverall_top_run_vel = "N/A";

double alt_gps = 0;
String displayalt_gps = "N/A";
// Not initialized by getNEO
double elevationStart_gps = 0;
double elevationDiff_gps = 0;
String displayelevationDiff_gps = "N/A";
double totalelevationDiff_gps = 0;
String displaytotalelevationDiff_gps = "N/A";

double run_dist = 0;
String displayrun_dist = "N/A";
double total_dist = 0;
String displaytotal_dist = "N/A";
// Not initialized by getNEO
double prev_lat = 0;
double prev_long = 0;

bool neoworking = false;
String displayneoworking;
double dist_diff;

void getNEO(){
  if (millis() > 5000 && gps.charsProcessed() < 10){
      Serial.println(F("No GPS detected: check wiring."));
      neoworking = false;
  }
//  Serial.print("Serial1.available: ");
//  Serial.println(Serial1.available());
//  Serial.print("gps.encode(Serial1.read()): ");
//  Serial.println(gps.encode(Serial1.read()));
//  Serial.print("gps.charsProcessed(): ");
//  Serial.println(gps.charsProcessed());
  if (Serial1.available() > 0 && gps.encode(Serial1.read())) {
    if (gps.location.isValid()){
      neoworking = true;
      run_vel = gps.speed.mps();
      displayrun_vel = String(run_vel);
      Serial.print("run_vel: ");
      Serial.println(displayrun_vel);
      if (run_vel > top_run_vel) {
        top_run_vel = run_vel;
        displaytop_run_vel = String(top_run_vel);
        Serial.print("top_run_vel: ");
        Serial.println(displaytop_run_vel);
      }
      alt_gps = gps.altitude.meters();
      displayalt_gps = String(alt_gps);
      Serial.print("alt_gps: ");
      Serial.println(displayalt_gps);
      if (elevationStart_gps != 0) {
        elevationDiff_gps = elevationStart_gps - alt_gps;
        displayelevationDiff_gps = String(elevationDiff_gps);
        Serial.print("elevationDiff_gps: ");
        Serial.println(displayelevationDiff_gps);
      }
      if (prev_lat != 0 && prev_long != 0) {
        dist_diff = gps.distanceBetween(prev_lat, prev_long, gps.location.lat(), gps.location.lng());
        if (dist_diff > 1) {run_dist += dist_diff;}
        displayrun_dist = String(run_dist);
        Serial.print(displayrun_dist);
      }
      prev_lat = gps.location.lat();
      prev_long = gps.location.lng();
    }
    else {
      neoworking = false;
      Serial.println("INVALID READING... MOVE TO OPEN SPACE");
    }
  }
}

// Timer Related
unsigned long startTime = 0; // Variable to store the start time
bool timerPressed = false;
bool timerState = false;    // Flag to track if the stopwatch is running
String displayTime = "00:00";
unsigned long currentTime = 0;
unsigned long elapsedTime = 0;
String displaytotalTime = "00:00";
unsigned long totalTime = 0;
String displayavgTime = "00:00";
unsigned long avgTime = 0;
String displaytotalRuns = "0";
unsigned long totalRuns = 0;

// Timer-Controlled Related
float elevationStart = 0;
float elevationDiff = 0;
String displayelevationDiff = "N/A";

void getTIMER() {
  currentTime = millis();
  elapsedTime = (currentTime - startTime) / 1000;
  displayTime = String() + String(elapsedTime/60 % 60 < 10 ? "0" : "") + String(int(floor(elapsedTime/60))) + ":" + String(elapsedTime % 60 < 10 ? "0" : "") + String(elapsedTime % 60);
  
  // Display the elapsed time in the console
//  Serial.print("Elapsed Time: ");
//  Serial.println(displayTime); // Display elapsed seconds
}

// BMP390 Related

Adafruit_BMP3XX bmp;
AsyncEventSource events("/events");
float temp;
float pres;
float alt;

#define SEALEVELPRESSURE_HPA (1013.25)  

void getBMP(){
  if (! bmp.performReading()) {
    Serial.println("Failed to perform reading");
    return;
  }
  temp = bmp.temperature;
  pres = bmp.pressure / 100.0;
  alt = bmp.readAltitude(SEALEVELPRESSURE_HPA);
//  Serial.print("Pressure = ");
//  Serial.print(temp);
//  Serial.print("   Altitude = ");
//  Serial.print(alt);
//  Serial.print("   Temperature = ");
//  Serial.print(temp);
//  Serial.println("");
}

// Web Server Related

// Network Credentials
const char* ssid = "APMonitor";
const char* password = "testpassword";

// Testing output LED
const int output = 13;
int ledState = digitalRead(output);

// Current state of LEDs (defaults to high for Rainbow)
std::vector<int> ledModeVec = {HIGH, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int currledMode = 0;
// Length of ledModeVec
int ledModeVeclen = 9;
// Parameters to interface between HTML and ESP32
const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Updates Current LED Mode
void updateledModeVec(int index) {
  Serial.print("ledModeVec Updated:");
  for (int i = 0; i < ledModeVeclen; i++) {
    if (i != index) {
      Serial.print(" LOW:");
      Serial.print(i);
      ledModeVec[i] = LOW;
    }
    else {
      Serial.print(" HIGH:");
      Serial.print(i);
      ledModeVec[i] = HIGH;
      currledMode = i;
    }
  }
  Serial.println("");
}

// Replaces placeholder with appropriate string
String processor(const String& var){
  if (var == "LED_ONOFF") {
    // By using "checked," can select/unselect checkboxes and radio buttons
    return (ledState == HIGH) ? "checked" : "";
  }
  else if (var == "MODE0") {
//    Serial.println("MODE0");
    return (ledModeVec[0] == HIGH) ? "checked" : "";
  }
  else if (var == "MODE1") {
//    Serial.println("MODE1");
    return (ledModeVec[1] == HIGH) ? "checked" : "";
  }
  else if (var == "MODE2") {
//    Serial.println("MODE2");
    return (ledModeVec[2] == HIGH) ? "checked" : "";
  }
  else if (var == "MODE3") {
//    Serial.println("MODE3");
    return (ledModeVec[3] == HIGH) ? "checked" : "";
  }
  else if (var == "MODE4") {
//    Serial.println("MODE4");
    return (ledModeVec[4] == HIGH) ? "checked" : "";
  }
  else if (var == "MODE5") {
//    Serial.println("MODE5");
    return (ledModeVec[5] == HIGH) ? "checked" : "";
  }
  else if (var == "MODE6") {
//    Serial.println("MODE6");
    return (ledModeVec[6] == HIGH) ? "checked" : "";
  }
  else if (var == "MODE7") {
//    Serial.println("MODE7");
    return (ledModeVec[7] == HIGH) ? "checked" : "";
  }
  else if (var == "MODE8") {
//    Serial.println("MODE8");
    return (ledModeVec[8] == HIGH) ? "checked" : "";
  }
  else if (var == "CURR_TEMP"){
    return String(temp);
  }
  else if (var == "CURR_ELEV"){
    return String(displayelevationDiff_gps);
//    return String(displayelevationDiff);
  }
  else if (var == "CURR_TIME") {
    return String(displayTime);
  }
  else if (var == "TIMER_ONOFF") {
    return (timerState == HIGH) ? "checked" : "";
  }
  else if (var == "RUNS_COMPLETED") {
    return String(displaytotalRuns);
  }
  else if (var == "AVG_RUN_TIME") {
    return String(displayavgTime);
  }
  else if (var == "TOTAL_RUN_TIME") {
    return String(displaytotalTime);
  }
  else if (var == "CURR_SPEED") {
    return String(displayrun_vel);
  }
  else if (var == "CURR_TOP_SPEED") {
    return String(displaytop_run_vel);
  }
  else if (var == "CURR_DIST") {
    return String(displayrun_dist);
  }
  else if (var == "OVERALL_ELEV") {
    return String(displaytotalelevationDiff_gps);
  }
  else if (var == "OVERALL_DIST") {
    return String(displaytotal_dist);
  }
  else if (var == "OVERALL_TOP_SPEED") {
    return String(displayoverall_top_run_vel);
  }
  else if (var == "GPS_STATUS") {
    return (neoworking) ? "Working" : "Not Connected";
  }
  else if (var == "BATTERY_STATUS") {
    return String(displaybatteryPercentage);
  }
  return String();
}

// Print all files uploaded through LittleFS
void listAllFiles(){
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while(file){
      Serial.print("FILE: ");
      Serial.println(file.name());
      file = root.openNextFile();
  }
}

// Interrupt

//bool server_interrupt = true;

// LED Settings Related

#define PIN 33
#define BRIGHTNESS 100
#define NUM_LEDS 20
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);
 

// Rainbow effect

void rainbow_effect(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i < NUM_LEDS; i++) { 
      int pixelHue = firstPixelHue + (i * 65536L / NUM_LEDS);
      pixels.setPixelColor(i, pixels.gamma32(pixels.ColorHSV(pixelHue, 100, 100)));
    }
    pixels.show();
    delay(wait);
  }
}

// Teal Effect

void teal_effect(int wait) {
  for(int i=0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(20, 140, 128));
    pixels.show();
    delay(wait*10);
  }
  for(int i=0; i < NUM_LEDS; i++) {
    pixels.setPixelColor(i, pixels.Color(75, 32, 153));
    pixels.show();
    delay(wait*10);
  }
}

// Speed Color Definitions:

uint8_t colorred;
uint8_t colorgreen;
uint8_t colorblue;
float conversion;
void color_def() {
// Input speed in terms of meters per second:
  conversion = (run_vel / 4.47) * 10;
  // Color palette from: https://coolors.co/palette/7400b8-6930c3-5e60ce-5390d9-4ea8de-48bfe3-56cfe1-64dfdf-72efdd-80ffdb
  if (conversion < 5) {
    colorred = 82;
    colorgreen = 55;
    colorblue = 158;
    Serial.println("5");
    Serial.println(conversion);
  }
  else if (conversion >= 5 && conversion < 10) {
    colorred = 55;
    colorgreen = 100;
    colorblue = 158;
    Serial.println("10");
  }
  else if (conversion >= 10 && conversion < 15) {
    colorred = 55;
    colorgreen = 148;
    colorblue = 158;
    Serial.println("15");
  }
  else if (conversion >= 15 && conversion < 20) {
    colorred = 55;
    colorgreen = 158;
    colorblue = 136;
    Serial.println("20");
  }
  else if (conversion >= 20 && conversion < 25) {
    colorred = 79;
    colorgreen = 158;
    colorblue = 55;
    Serial.println("25");
  }
  else if (conversion >= 25 && conversion < 30) {
    colorred = 151;
    colorgreen = 158;
    colorblue = 55;
    Serial.println("30");
  }
  else if (conversion >= 30 && conversion < 35) {
    colorred = 158;
    colorgreen = 106;
    colorblue = 55;
    Serial.println("35");
  }
  else if (conversion >= 35 && conversion < 40) {
    colorred = 158;
    colorgreen = 64;
    colorblue = 55;
    Serial.println("40");
  }
  else {
    colorred = 100;
    colorgreen = 100;
    colorblue = 100;
    Serial.println("Other");
  }
}

// Speed Fade

void speed_fade(int wait) {
  color_def();
  for(int k=0; k < 256; k++) {
    for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(colorred * k/256, colorgreen * k/256, colorblue * k/256));
    }
    delay(wait);
    pixels.show();
  }
  delay(wait*10); 
  for(int k=256; k > 0; k--) {
    for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(colorred * k/256, colorgreen * k/256, colorblue * k/256));
    }
    delay(wait);
    pixels.show();
  }
}

// Speed Bounce

int EyeSize = 3;

void speed_bounce(int wait) {
  color_def();
  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
    pixels.setPixelColor(i, pixels.Color(colorred/10, colorgreen/10, colorblue/10));
    for(int j = 1; j <= EyeSize; j++) {
      pixels.setPixelColor(i+j, pixels.Color(colorred, colorgreen, colorblue));
    }
    pixels.setPixelColor(i+EyeSize+1, pixels.Color(colorred/10, colorgreen/10, colorblue/10));
    pixels.show();
    delay(wait*10);
  }

  delay(wait);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.setPixelColor(i, pixels.Color(colorred/10, colorgreen/10,colorblue/10));
    for(int j = 1; j <= EyeSize; j++) {
      pixels.setPixelColor(i+j, pixels.Color(colorred, colorgreen, colorblue));
    }
    pixels.setPixelColor(i+EyeSize+1, pixels.Color(colorred/10, colorgreen/10, colorblue/10));
    pixels.show();
    delay(wait*10);
  }
 
  delay(wait*10);
}

uint32_t oldColor;
uint8_t r, g, b;
int value;

void fadeToBlack(int ledNo, byte fadeValue) {
  oldColor = pixels.getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);

  r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
  g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
  b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
 
  pixels.setPixelColor(ledNo, pixels.Color(r,g,b));
}

// Speed Meteor

byte meteorSize = 3;
byte meteorTrail = 4;

void speed_meteor(int wait) {
  color_def();
  for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      fadeToBlack(j, meteorTrail);
    }
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        pixels.setPixelColor(i-j, pixels.Color(colorred, colorgreen, colorblue));
      }
    }
    pixels.show();
    delay(wait*10);
  }
}

// Custom Colors

uint8_t customred = 0;
uint8_t customgreen = 0;
uint8_t customblue = 0;

// Custom Fade

void custom_fade(int wait) {
  for(int k=0; k < 256; k++) {
    for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(customred * k/256, customgreen * k/256, customblue * k/256));
    }
    delay(wait);
    pixels.show();
  }
  delay(wait*10); 
  for(int k=256; k > 0; k--) {
    for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(customred * k/256, customgreen * k/256, customblue * k/256));
    }
    delay(wait);
    pixels.show();
  }
}

// Custom Bounce

void custom_bounce(int wait) {
  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
    pixels.setPixelColor(i, pixels.Color(customred/10, customgreen/10, customblue/10));
    for(int j = 1; j <= EyeSize; j++) {
      pixels.setPixelColor(i+j, pixels.Color(customred, customgreen, customblue));
    }
    pixels.setPixelColor(i+EyeSize+1, pixels.Color(customred/10, customgreen/10, customblue/10));
    pixels.show();
    delay(wait*10);
  }

  delay(wait);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.setPixelColor(i, pixels.Color(customred/10, customgreen/10,customblue/10));
    for(int j = 1; j <= EyeSize; j++) {
      pixels.setPixelColor(i+j, pixels.Color(customred, customgreen, customblue));
    }
    pixels.setPixelColor(i+EyeSize+1, pixels.Color(customred/10, customgreen/10, customblue/10));
    pixels.show();
    delay(wait*10);
  }
 
  delay(wait*10);
}

// Custom Meteor

void custom_meteor(int wait) {
  for(int i=0; i < NUM_LEDS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      fadeToBlack(j, meteorTrail);
    }
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        pixels.setPixelColor(i-j, pixels.Color(customred, customgreen, customblue));
      }
    }
    pixels.show();
    delay(wait*10);
  }
}

// Custom Solid

void custom_solid(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i < NUM_LEDS; i++) { 
        pixels.setPixelColor(i, pixels.Color(customred, customgreen, customblue));
    }
    pixels.show();
    delay(wait);
  }
}


// LEDs Off

void leds_off(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i < NUM_LEDS; i++) { 
        pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // Off
    }
    pixels.show();
    delay(wait);
  }
}

void setup(){

  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(output, OUTPUT);
  digitalWrite(output, LOW);

  for (int vec_counter = 0; vec_counter < 100; vec_counter++) {
    all_top_run_vel[vec_counter] = -1;
  }

  // Initialize LED task
  xTaskCreatePinnedToCore(
                    ledTaskcode,   /* Task function. */
                    "ledTask",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    0,           /* priority of the task */
                    &ledTask,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */  
                       
  // Initialize LittleFS
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  file_write = LittleFS.open("/outputdata.csv", "a");
  if (!file_write) {
    Serial.println("Failed to open CSV file.");
    return;
  }
  file_write.close();

  file_read = LittleFS.open("/outputdata.csv");
  if(!file_read){
    Serial.println("Failed to open file for reading");
    return;
  }
  String datapoint = "";
  int counter = 0;
  char datap;
  Serial.println("Initial CSV file reading");
  while (file_read.available()) {
    char datap = file_read.read();
    if (datap != '\n' && datap != ',' && datap != ' ') {        
      datapoint += datap;
    }
    else {
      if (datapoint.length() > 0) {
        // Number of Runs: totalRuns (unsigned long)
        if (counter == 0) {
          totalRuns = datapoint.toInt();
          displaytotalRuns = String(totalRuns);
          counter++;
        }
        // Top Speed: overall_top_run_vel (double)
        else if (counter == 1) {
          counter++;
          if (datapoint.toDouble() > overall_top_run_vel) {
            overall_top_run_vel = datapoint.toDouble();
            displayoverall_top_run_vel = datapoint.toDouble();
          }
          for (int i = 0; i < 50; i++) {
            if (all_top_run_vel[i] != -1) {
              all_top_run_vel[i] = datapoint.toDouble();
            }
          }
        }
        // Distance: total_dist (double)
        else if (counter == 2) {
          total_dist += datapoint.toDouble();
          displaytotal_dist = String(total_dist);
          counter++;
        }
        // Elevation: totalelevationDiff_gps (double)
        else if (counter == 3) {
          totalelevationDiff_gps += datapoint.toDouble();
          displaytotalelevationDiff_gps = String(totalelevationDiff_gps);
          counter++;
        }
        // Time: totalTime (unsigned long)
        else if (counter == 4) {
          totalTime += datapoint.toInt();
          avgTime = totalTime / totalRuns;
          displaytotalTime = String() + String(totalTime/3600 % 60 < 10 ? "0" : "") + String(int(floor(totalTime/3600))) + ":" + String(totalTime/60 % 60 < 10 ? "0" : "") + String(totalTime/60 % 60);
          displayavgTime = String() + String(avgTime/60 % 60 < 10 ? "0" : "") + String(int(floor(avgTime/60))) + ":" + String(avgTime % 60 < 10 ? "0" : "") + String(avgTime % 60);
          counter = 0;
        }
        Serial.print("Datapoint: ");
        Serial.println
        (datapoint);
        datapoint = "";
      }
    }
  }
  file_read.close();

  listAllFiles();

  // Access Point Mode
  WiFi.softAP(ssid, password);
  Serial.print("\nIP address: ");
  Serial.println(WiFi.softAPIP());

  // BMP Setup
  if (!bmp.begin_I2C()) {   // hardware I2C mode, can pass in address & alt Wire
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//    server_interrupt = false;
    request->send(LittleFS, "/index.html", "text/html", false, processor);
//    server_interrupt = true;
  });

  server.serveStatic("/", LittleFS, "/");

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
//    server_interrupt = false;
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      if (inputMessage1 == "onoff") {
        ledState = !ledState;
        // Remove after testing
        digitalWrite(output, ledState);
        Serial.print("LED Status: ");
        Serial.println(ledState);
        if (inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("0");
        updateledModeVec(0);
        }
      }
      else if (inputMessage1 == "option0" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("0");
        updateledModeVec(0);
      }
      else if (inputMessage1 == "option1" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("1");
        updateledModeVec(1);
      }
      else if (inputMessage1 == "option2" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("2");
        updateledModeVec(2);
      }
      else if (inputMessage1 == "option3" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("3");
        updateledModeVec(3);
      }
      else if (inputMessage1 == "option4" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("4");
        updateledModeVec(4);
      }
      else if (inputMessage1 == "option5" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("5");
        updateledModeVec(5);
      }
      else if (inputMessage1 == "option6" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("6");
        updateledModeVec(6);
      }
      else if (inputMessage1 == "option7" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("7");
        updateledModeVec(7);
      }
      else if (inputMessage1 == "option8" && inputMessage2 == "1") {
        Serial.print("updateledModeVec: ");
        Serial.println("8");
        updateledModeVec(8);
      }
      else if (inputMessage1 == "colorpickerred") {
        Serial.print("colorpickerred: ");
        Serial.println(inputMessage2);
        customred = inputMessage2.toInt();        
      }
      else if (inputMessage1 == "colorpickergreen") {
        Serial.print("colorpickergreen: ");
        Serial.println(inputMessage2);
        customgreen = inputMessage2.toInt();        
      }
      else if (inputMessage1 == "colorpickerblue") {
        Serial.print("colorpickerblue: ");
        Serial.println(inputMessage2);
        customblue = inputMessage2.toInt();        
      }
      else if (inputMessage1 == "timer") {
        timerPressed = true;
      }
      else {
        inputMessage1 = "inputMessage1 does not match";
        Serial.println(inputMessage1);
      }
    }
    else {
      inputMessage1 = "No message sent";
      Serial.println(inputMessage1);
    }    
    request->send(LittleFS, "text/plain", "OK");
//    server_interrupt = true;
  });

  // Send a GET request to <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
//    server_interrupt = false;
    request->send(LittleFS, "text/plain", String(digitalRead(output)).c_str());
//    server_interrupt = true;

  });

  // Start server
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);

    // Graphing Initial
    for (int run_counter = 0; run_counter < 100; run_counter++) {
      if (all_top_run_vel[run_counter] != -1) {
        graph_top_run_vel = String(run_counter)  + " " + String(all_top_run_vel[run_counter]);
        events.send(graph_top_run_vel.c_str(), "graphtoprunvel", millis());
      }
    }
  });
  server.addHandler(&events);
  server.begin();
  // Start NeoPixel library
  pixels.begin();
}

void ledTaskcode( void * pvParameters ){
  Serial.print("ledTask running on core ");
  Serial.println(xPortGetCoreID());

  while(true) {
//    Serial.print("ledState: ");
//    Serial.println(ledState);
//    Serial.print("currledMode: ");
//    Serial.println(currledMode);
//    if (server_interrupt) {
      if (ledState == HIGH) {
        if (currledMode == 0) {rainbow_effect(4);}
        else if (currledMode == 1) {teal_effect(4);}
        else if (currledMode == 2) {speed_fade(4);}
        else if (currledMode == 3) {speed_bounce(4);}
        else if (currledMode == 4) {speed_meteor(4);}
        else if (currledMode == 5) {custom_fade(4);}
        else if (currledMode == 6) {custom_bounce(4);}
        else if (currledMode == 7) {custom_meteor(4);}
        else if (currledMode == 8) {custom_solid(4);}
      }
      else {leds_off(4);}
//    }
  }
}


void loop(){

  currMillis = millis();
  if (currMillis - prevMillis >= intervalMillis) {
    prevMillis = currMillis;
  
  //  Serial.print("timerState: ");
  //  Serial.println(timerState);
    getBMP();
    events.send("ping",NULL,millis());
    displayneoworking = (neoworking) ? "Working" : "Not Connected";
    events.send(displayneoworking.c_str(), "gps", millis());
    events.send(String(temp).c_str(),"temperature",millis());
  
    getNEO();

    // Read battery voltage
    batteryVoltage = analogRead(A13) * ((2* 3.3)/4095);  // Replace A0 with your actual analog pin
    batteryPercentage = ((batteryVoltage - minVoltage) / (maxVoltage - minVoltage)) * 100;
    displaybatteryPercentage = String(batteryPercentage) + " Percent";
//    Serial.print("Battery Percentage: ");
//    Serial.println(displaybatteryPercentage);
//    Serial.print("Battery Voltage: ");
//    Serial.println(batteryVoltage);
    events.send(displaybatteryPercentage.c_str(), "battery", millis());
  
    // Start the timer
    if (timerPressed) {
      timerPressed = false;
      // If timer was started previously
      if (timerState) {
        timerState = false;
        // Reset the start time
        startTime = 0;
        // Update overall statistics here
        if (elapsedTime > 0) {
        
          totalRuns++;
          totalTime += elapsedTime;
          avgTime = totalTime / totalRuns;
          displaytotalRuns = String(totalRuns);
          displaytotalTime = String() + String(totalTime/3600 % 60 < 10 ? "0" : "") + String(int(floor(totalTime/3600))) + ":" + String(totalTime/60 % 60 < 10 ? "0" : "") + String(totalTime/60 % 60);
          displayavgTime = String() + String(avgTime/60 % 60 < 10 ? "0" : "") + String(int(floor(avgTime/60))) + ":" + String(avgTime % 60 < 10 ? "0" : "") + String(avgTime % 60);
          events.send(displaytotalRuns.c_str(), "totalruns", millis());
          events.send(displaytotalTime.c_str(), "totaltime", millis());
          events.send(displayavgTime.c_str(), "avgtime", millis());
  
          total_dist += run_dist;
          displaytotal_dist = String(total_dist);
          events.send(displaytotal_dist.c_str(), "overalldist", millis());
          totalelevationDiff_gps += elevationDiff_gps;
          displaytotalelevationDiff_gps = String(totalelevationDiff_gps);
          events.send(displaytotalelevationDiff_gps.c_str(), "overallelev", millis());
          if (top_run_vel > overall_top_run_vel) {
            overall_top_run_vel = top_run_vel;
            displayoverall_top_run_vel = String(overall_top_run_vel);
            events.send(displayoverall_top_run_vel.c_str(), "overalltoprunvel", millis());
          }

          
          // Send overall statistics to CSV file
          file_write = LittleFS.open("/outputdata.csv", "a");
          if (file_write.println(displaytotalRuns + ", " + String(top_run_vel) + ", " + String(run_dist) + ", " + String(elevationDiff_gps) + ", " + String(elapsedTime))) {
            Serial.println("File Printed:");
          }
          file_write.flush();
          file_write.close();

          // Graphing Loop
          graph_top_run_vel = displaytotalRuns + " " + String(top_run_vel);
          events.send(graph_top_run_vel.c_str(), "graphtoprunvel", millis());

          file_read = LittleFS.open("/outputdata.csv");
          if(!file_read){
            Serial.println("Failed to open file for reading");
            return;
          }
          while (file_read.available()) {
            Serial.write(file_read.read());
          }
          file_read.close();
          
        }
      } 
      // Begin current run statistics here
      // If timer was stopped previously
      else {
        timerState = true;
        if (!startTime) {
          startTime = millis();
          getBMP();
          elevationStart = alt;
          elevationStart_gps = gps.altitude.meters();
          prev_lat = gps.location.lat();
          prev_long = gps.location.lng();
        }
      }
    }
  
    // If timer has been started
    // Update current run statistics here
    if (timerState) {
      getTIMER();
      events.send(displayTime.c_str(),"time",millis());
      if (elevationDiff < elevationStart - alt) {
        elevationDiff = elevationStart - alt;
        displayelevationDiff = String(elevationDiff);
      }
      events.send(displayelevationDiff.c_str(),"altitude",millis());
      getNEO();
      events.send(displayrun_vel.c_str(),"runvelocity",millis());
      events.send(displaytop_run_vel.c_str(),"toprunvelocity",millis());
      events.send(displayelevationDiff_gps.c_str(),"elevationdiffgps",millis());
      events.send(displayrun_dist.c_str(),"rundist",millis());
    }
  }
}
