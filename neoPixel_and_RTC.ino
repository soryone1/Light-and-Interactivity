#include <ArduinoJson.h>
#include <ColorConverter.h>

//* set up for neoPixel*//
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#define PIN_MIN            6
#define NUMPIXELS_MIN      60
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS_MIN, PIN_MIN, NEO_GRB + NEO_KHZ800);
int MinPixel;

#define PIN_HOUR           8
#define NUMPIXELS_HOUR      24
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUMPIXELS_HOUR, PIN_HOUR, NEO_GRB + NEO_KHZ800);
int HourPixel;

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

//weather API related
WiFiClient client;
char server[] = "api.openweathermap.org";
String apiKey = SECRET_APIKEY; //SECRET_APIKEY;
ColorConverter converter;

int h = 15;
int h2 = 15;
int s = 100;
int i = 100;

unsigned long timer = 0;
unsigned long previousTimer = 0;

String locationID = "5128581"; //New York City ID

boolean updateWeather = true;
boolean dataReady = false;

String nextWeatherTime[] = {" ", " ", " "};
String nextWeather[] = {" ", " ", " "};
String nextTemp[] = {" ", " ", " "};

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

  //print out WiFi status:
  printWiFiStatus();

  //rtc stuff
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
    //    rtc.setEpoch(1556759815);
    Serial.println();
  }

  //* Initialize neoPixel*//
  strip.begin();
  strip.setBrightness(255);
  strip2.begin();
  strip2.setBrightness(255);
}

void loop() {
  //  printDate();
  printTime();

  Serial.println();
  //  Serial.println(apiKey);
  delay(1000);

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

  //set neopixel color based on weather

  ///////////////////////////////////////////////
  //verify the conditions.
  // 0 - clear; 1 - rain; 2 - coulds; 3 - snow;
  ///////////////////////////////////////////////

  //Current Weather
  if (nextWeather[0] == "Clear") {
    //    strip.setPixelColor(0,color.red, color.green, color.blue);
    h = 45;
    s = 77;
  }
  else if (nextWeather[0] == "Rain") {
    //    strip.setPixelColor(1,color.red, color.green, color.blue);
    h = 208;
    s = 77;
  }
  else if (nextWeather[0] == "Clouds") {
    //    strip.setPixelColor(2,color.red, color.green, color.blue);
    h = 208;
    s = 0;
  }
  else if (nextWeather[0] == "Snow") {
    //    strip.setPixelColor(3,color.red, color.green, color.blue);
    h = 208;
    s = 0;
  }

  RGBColor color = converter.HSItoRGB(h, s, i);
  RGBColor colorDimmed = converter.HSItoRGB(h, s, i - 70);

  //Outter Minute Ring
  for (int i = 0; i < NUMPIXELS_MIN; i++) {
    strip.setPixelColor(i, strip.Color(color.red, color.green, color.blue));
  }

  //offset positoin on the fixture
  if (MinPixel < 31) {
    MinPixel += 29;
  } else {
    MinPixel -= 31;
  }

  if (MinPixel == 0) {
    strip.setPixelColor(MinPixel + 59, strip.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
    strip.setPixelColor(MinPixel, strip.Color(0, 0, 0));
    strip.setPixelColor(MinPixel + 1, strip.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
  } else if (MinPixel == 59) {
    strip.setPixelColor(MinPixel - 1, strip.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
    strip.setPixelColor(MinPixel, strip.Color(0, 0, 0));
    strip.setPixelColor(MinPixel - 59, strip.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
  } else {
    strip.setPixelColor(MinPixel - 1, strip.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
    strip.setPixelColor(MinPixel, strip.Color(0, 0, 0));
    strip.setPixelColor(MinPixel + 1, strip.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
  }
  strip.show();

  //Inner Hour Ring
  for (int i = 0; i < NUMPIXELS_HOUR; i++) {
    strip2.setPixelColor(i, strip2.Color(color.red, color.green, color.blue));
  }

  //offset positoin on the fixture
  if (HourPixel < 19) {
    HourPixel += 5;
  } else {
    HourPixel -= 19;
  }

  if (HourPixel == 0) {
    strip2.setPixelColor(HourPixel + 11, strip2.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
    strip2.setPixelColor(HourPixel, strip2.Color(0, 0, 0));
    strip2.setPixelColor(HourPixel + 1, strip2.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
  } else if (HourPixel == 11) {
    strip2.setPixelColor(HourPixel - 1, strip2.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
    strip2.setPixelColor(HourPixel, strip2.Color(0, 0, 0));
    strip2.setPixelColor(HourPixel - 11, strip2.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
  } else {
    strip2.setPixelColor(HourPixel - 1 , strip2.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
    strip2.setPixelColor(HourPixel, strip2.Color(0, 0, 0));
    strip2.setPixelColor(HourPixel + 1, strip2.Color(colorDimmed.red, colorDimmed.green, colorDimmed.blue));
  }

  strip2.show();

}

void getWeather() {

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    Serial.println();
    // Make a HTTP request:
    client.print("GET /data/2.5/forecast/hourly?");
    client.print("id=" + locationID);
    client.print("&APPID=" + apiKey);
    client.print("&units=metric");
    client.println("&cnt=3");

    client.println("Host: api.openweathermap.org");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("unable to connect");
  }
  delay(1000);
  String line = "";

  while (client.connected()) {
    line = client.readStringUntil('\n');
    int line_len = line.length() + 1;
    char input[line_len];
    line.toCharArray(input, line_len);

    Serial.println("JSON Received");
    Serial.println(line);
    Serial.println();

    //parse JSON values
    Serial.println("Parsing Values");

    const size_t capacity = 3 * JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(0) + 8 * JSON_OBJECT_SIZE(1) + 4 * JSON_OBJECT_SIZE(2) + 4 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 6 * JSON_OBJECT_SIZE(8) + 1090;
    StaticJsonDocument<capacity> doc;

    auto error = deserializeJson(doc, input);

    if (error) {
      Serial.println("Parse Failed");
      Serial.println(error.c_str());
      return;
    } else {
      dataReady = true;
      Serial.println("Parse Succeeded");
    }

    JsonObject obj = doc.as<JsonObject>();

    if (obj.containsKey("list")) {
      Serial.println("contains list");
    }

    //get the data from the json tree
    String nextWeatherTime0 = doc["list"][0]["dt_txt"];
    String nextWeather0 = doc["list"][0]["weather"][0]["main"];
    float nextTemp0 = doc["list"][0]["main"]["temp"];

    String nextWeatherTime1 = doc["list"][1]["dt_txt"];
    String nextWeather1 = doc["list"][1]["weather"][0]["main"];
    float nextTemp1 = doc["list"][1]["main"]["temp"];

    String nextWeatherTime2 = doc["list"][2]["dt_txt"];
    String nextWeather2 = doc["list"][2]["weather"][0]["main"];
    float nextTemp2 = doc["list"][2]["main"]["temp"];

    nextWeatherTime[0] = nextWeatherTime0;
    nextWeatherTime[1] = nextWeatherTime0;
    nextWeatherTime[2] = nextWeatherTime0;

    nextWeather[0] = nextWeather0;
    nextWeather[1] = nextWeather1;
    nextWeather[2] = nextWeather2;

    nextTemp[0] = nextTemp0;
    nextTemp[1] = nextTemp1;
    nextTemp[2] = nextTemp2;

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
    Serial.println(nextWeatherTime0);
    Serial.println(nextWeather0);
    Serial.println(nextTemp0);
    Serial.println(nextWeatherTime1);
    Serial.println(nextWeather1);
    Serial.println(nextTemp1);
    Serial.println(nextWeatherTime2);
    Serial.println(nextWeather2);
    Serial.println(nextTemp2);
  }
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

void printTime()
{
  Hour = rtc.getHours() + GMT;
  if (Hour < 0) {
    Hour += 24;
  }
  print2digits(Hour);
  //  Serial.print(Hour);
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.println();
  Min = rtc.getMinutes();
  //  Min = rtc.getSeconds();
  //  Serial.println(Min);
  MinToPixel();
  HourToPicel();
  //  Serial.print(":");
  //  print2digits(rtc.getSeconds());
  //  Serial.println();
}

void MinToPixel() {
  MinPixel = Min;
  Serial.print("Minute Pixel corresponding to current minute is: ");
  Serial.println(MinPixel);
}

void HourToPicel() {
//  if (Hour >= 12) {
//    HourPixel = Hour - 12;
//    HourPixel = Min - 12;
//  } else {
////    HourPixel = Hour;
//    HourPixel = Min;
//  }
  HourPixel = Hour;
  
  Serial.print("Hour Pixel corresponding to current minute is: ");
  Serial.println(HourPixel);
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
