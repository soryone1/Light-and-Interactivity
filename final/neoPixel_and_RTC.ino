#include <ColorConverter.h>

//* set up for neoPixel*//
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN_MIN            6
#define NUMPIXELS_MIN      30
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS_MIN, PIN_MIN, NEO_GRB + NEO_KHZ800);
int MinPixel;

//*set up for wifi RTC*//
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <RTCZero.h>

RTCZero rtc;

#include "arduino_secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int keyIndex = 0;
int status = WL_IDLE_STATUS;
const int GMT = -4;        //new york should be -5, but -4 works

int Hour;
int Min;

//weather related
WiFiClient client;
char server[] = "api.openweathermap.org";
//char server[] = "www.google.com";  
String apiKey= SECRET_APIKEY; //SECRET_APIKEY;
ColorConverter converter;

int h = 15;
int h2 = 15;
int s = 100;
int i = 100;

int red = 50;
int green = 50;
int blue = 50;

//generate time for the cos move
unsigned long timer = 0;
unsigned long previousTimer = 0;

String locationID = "5128581"; //New York City ID

boolean updateWeather = true;
boolean dataReady = false;

String nextWeatherTime[]={" "," "," "};
String nextWeather[] ={" "," "," "};
String nextTemp[] ={" "," "," "};

void setup() {
  Serial.begin(115200);

  // check if the WiFi module works
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the status:
  printWiFiStatus();

  rtc.begin();

  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  }
  while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries > maxTries) {
    Serial.print("NTP unreachable!!");
    while (1);
  }
  else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);

    Serial.println();
  }

  //* Initialize neoPixel*//
  strip.begin();
  strip.setBrightness(250);
}

void loop() {
  //  printDate();
  printTime();

  Serial.println();
  Serial.println(apiKey);
  delay(1000);

  for (int i = 0; i < NUMPIXELS_MIN; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
    strip.setPixelColor(MinPixel - 2, strip.Color(red, green, blue));
    strip.setPixelColor(MinPixel - 1, strip.Color(0, 0, 0));
    strip.setPixelColor(MinPixel, strip.Color(red, green, blue));
  }

  //weather related stuff
  timer = millis(); // updating time

  if (updateWeather == false ) {
    if (timer - previousTimer >= 300000) {
      previousTimer = timer;
      updateWeather = true;
    }
  } else if (updateWeather == true) {
    getWeather();
    if (dataReady == true) {
      updateWeather = false;
      dataReady = false;
      previousTimer = timer;
    }
  }

  //  RGBColor color = converter.HSItoRGB(h, s, i);


    ///////////////////////////////////////////////
  //verify the conditions.
  // 0 - clear; 1 - rain; 2 - coulds; 3 - snow;
  /////////////////////////////////////////////// 
  
  //Current Weather
  if (nextWeather[0]=="Clear"){ 
//    strip.setPixelColor(0,color.red, color.green, color.blue);
    red = 50;
    green = 0;
    blue = 0;
  } 
  else if (nextWeather[0]=="Rain") {
//    strip.setPixelColor(1,color.red, color.green, color.blue);
        red = 50;
    green = 0;
    blue = 0;
  } 
  else if (nextWeather[0]=="Clouds") {
//    strip.setPixelColor(2,color.red, color.green, color.blue);
        red = 50;
    green = 0;
    blue = 0;
  } 
  else if (nextWeather[0]=="Snow") {
//    strip.setPixelColor(3,color.red, color.green, color.blue);
        red = 50;
    green = 0;
    blue = 0;
  }     
      

    strip.show();
}

void printTime()
{
  //  print2digits(rtc.getHours() + GMT);
  Hour = rtc.getHours() + GMT;
  //  Serial.print(Hour);
  //  Serial.print(":");
  //  print2digits(rtc.getMinutes());
  //  Min = rtc.getMinutes();
  Min = rtc.getSeconds();
  //  Serial.println(Min);
  MinToPixel();
  //  Serial.print(":");
  //  print2digits(rtc.getSeconds());
  //  Serial.println();
}

void MinToPixel() {
  MinPixel = Min / 2;
  Serial.println(MinPixel);
}
void printDate()
{
  Serial.print(rtc.getDay());
  Serial.print("/");
  Serial.print(rtc.getMonth());
  Serial.print("/");
  Serial.print(rtc.getYear());

  Serial.print(" ");
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0");
  }
  Serial.print(number);
}

void getWeather() {

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.print("GET /data/2.5/forecast/hourly?");
//    client.println("GET /search?q=arduino HTTP/1.1");
    client.print("id="+locationID);
    client.println("&APPID="+apiKey);
//    client.print("&cnt=3");
//    client.println("&units=metric");
    client.println("Host: api.openweathermap.org");
//     client.println("Host: www.google.com");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("unable to connect");
  }
  delay(1000);
  String line = "";

  while (client.connected()) {
    line = client.readStringUntil('\n');
    Serial.println(line);
    Serial.println("parsingValues");
    //create a json buffer where to store the json data
//    StaticJsonBuffer<5000> jsonBuffer;
//    JsonObject& root = jsonBuffer.parseObject(line);
//    if (root.success()){
//      dataReady = true;
//    } else if (!root.success()) {
//      Serial.println("parseObject() failed");
//      return;
//    }

  //get the data from the json tree
  // get the weather description & temprature at current time and 6 hours later
  
//  String nextWeatherTime0 = root["list"][0]["dt_txt"];
//  String nextWeather0 = root["list"][0]["weather"][0]["main"];
//  float nextTemp0 = root["list"][0]["main"]["temp"];
//
//  String nextWeatherTime1 = root["list"][1]["dt_txt"];
//  String nextWeather1 = root["list"][1]["weather"][0]["main"];
//  float nextTemp1 = root["list"][1]["main"]["temp"];
//  
//  String nextWeatherTime2 = root["list"][2]["dt_txt"];
//  String nextWeather2 = root["list"][2]["weather"][0]["main"];
//  float nextTemp2 = root["list"][2]["main"]["temp"];
//  
//  nextWeatherTime[0]=nextWeatherTime0;
//  nextWeatherTime[1]=nextWeatherTime0;
//  nextWeatherTime[2]=nextWeatherTime0;
//
//  nextWeather[0]=nextWeather0;
//  nextWeather[1]=nextWeather1;
//  nextWeather[2]=nextWeather2;
//
//  nextTemp[0]=nextTemp0;
//  nextTemp[1]=nextTemp1;
//  nextTemp[2]=nextTemp2;

//   //Set the color of the light depends on temp
//  if (nextTemp0 > 10) {
//    h = 15;
//  } else if (nextTemp0 >= 30 && nextTemp0 < 35 ) {
//    h = 35;
//  } else if (nextTemp0 >= 20 && nextTemp0 < 30 ) {
//    h = 55;
//  } else if (nextTemp0 >= 10 && nextTemp0 < 20 ) {
//    h = 180;
//  } else if (nextTemp0 >= 0 && nextTemp0 < 10 ) {
//    h = 200;
//  } else {
//    h = 220;
//  } 

  //Set the color of the light depends on temp in 6 hours
//  if (nextTemp2 > 10) {
//    h2 = 15;
//  } else if (nextTemp2 >= 30 && nextTemp0 < 35 ) {
//    h2 = 35;
//  } else if (nextTemp2 >= 20 && nextTemp0 < 30 ) {
//    h2 = 55;
//  } else if (nextTemp2 >= 10 && nextTemp0 < 20 ) {
//    h2 = 180;
//  } else if (nextTemp2 >= 0 && nextTemp0 < 10 ) {
//    h2 = 200;
//  } else {
//    h2 = 220;
//  } 
  // Print values.
//  Serial.println(nextWeatherTime0);
//  Serial.println(nextWeather0);
//  Serial.println(nextTemp0);
//  Serial.println(nextWeatherTime1);
//  Serial.println(nextWeather1);
//  Serial.println(nextTemp1);  
//  Serial.println(nextWeatherTime2);
//  Serial.println(nextWeather2);
//  Serial.println(nextTemp2);
  }
}
