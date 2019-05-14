#include <ColorConverter.h>
#include <Adafruit_NeoPixel.h>
#include <Interval.h>

//------------------Digital-in Pins--------------------
int pushbuttons[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
//int pushbuttons[3] = {1, 2, 3};

//----------------Mode Control
int mode = 0; //0: individual control, 1: pre-centralized, 2: centralized-control 3: glowing & flowing
int modeThreeType = 0; // 0:glowing, 1:flowing

//----------------Debounce & Long Press Detection
int buttonStates[9] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int lastButtonStates[9] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int pressStates[9] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
int lastPressStates[9] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.

long debounceDelay = 150;    // the debounce time; increase if the output flickers
long longPressDelay = 1500;    // the debounce time; increase if the output flickers
long lastDebounceTimes[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
long lastLongPressTimes[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};


//-------------------NeoPixel Tiles------------------------
ColorConverter converter;
float hue = 18; // from RGB 255, 83, 12
float saturation = 100;
float intensity = 50;

const int intensityNone = 0;
const int intensityWeak = 50;
const int intensityStrong = 150;

//Tile 1
const int neoPixelPin = 10;
const int pixelCount = 28;
float stripIntensity = 0;
float stripChange = 1;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, neoPixelPin, NEO_GRB + NEO_KHZ800);

//Tile 2
const int neoPixelPin2 = 11;
const int pixelCount2 = 39;
float stripIntensity2 = 250;
float stripChange2 = 1;
int modeThreePixel = 0;
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(pixelCount2, neoPixelPin2, NEO_GRB + NEO_KHZ800);


void setup() {

  for (int i = 0; i < 9; i++) {
    int pin = pushbuttons[i];
    pinMode(pin, INPUT);
  }

  strip.begin();    // initialize pixel strip
  strip.clear();    // turn all LEDs off
  strip2.begin();    // initialize pixel strip
  strip2.clear();    // turn all LEDs off
  Serial.begin(9600);

}

void loop() {
  //-----------------------------------------------------------------------------
  //read sensor
  //-----------------------------------------------------------------------------
  int sensorValues[9];

  for (int i = 0; i < 9; i++) {
    sensorValues[i] = digitalRead(pushbuttons[i]);
    if (i == 3) {
      //      Serial.println(sensorValues[i]);
    }
  }

  //    Serial.print("mode:");
  //    Serial.println(mode);
  //-----------------------------------------------------------------------------
  //logic handling for different modes
  //-----------------------------------------------------------------------------
  switch (mode) {

    //inividual control
    case 0:
      for (int i = 0; i < 9; i++) {
        // If the switch changed, due to noise or pressing:
        if (sensorValues[i] != lastButtonStates[i]) {
          lastDebounceTimes[i] = millis();
        }

        if (sensorValues[i] != lastPressStates[i]) {
          lastLongPressTimes[i] = millis();
        }

        //see if it's a touch
        if ((millis() - lastDebounceTimes[i]) > debounceDelay) {

          // if the button state has changed:
          if (sensorValues[i] != buttonStates[i]) {
            buttonStates[i] = sensorValues[i];

            // only change the intensity if the new button state is HIGH
            if (buttonStates[i] == HIGH) {
              Serial.println("touch!");
              //change the intensity of the strip

              if (stripIntensity == intensityNone) {
                stripIntensity = intensityWeak;
              } else if (stripIntensity == intensityWeak) {
                stripIntensity = intensityStrong;
              } else {
                stripIntensity = 0;
              }
            }
          }
        }

        //see if it's long press. If it is, overwrite the intensity and go to mode 1
        if ((millis() - lastLongPressTimes[i]) > longPressDelay) {

          // if the button state has changed:
          if (sensorValues[i] != pressStates[i]) {
            pressStates[i] = sensorValues[i];

            // only change the mode if the new button state is HIGH and the test tile is long pressed
            if (pressStates[i] == HIGH && i == 3) {
              Serial.println("long touch! go to mode 3 flowing!");
              //turn off all strips
              stripIntensity = intensityNone;
              for (int pixel = 0; pixel < pixelCount; pixel++) {
                strip.setPixelColor(pixel, stripIntensity, stripIntensity, stripIntensity);
              }
              strip.show();
              for (int pixel = 0; pixel < pixelCount2; pixel++) {
                strip2.setPixelColor(pixel, stripIntensity, stripIntensity, stripIntensity);
              }
              strip2.show();

              //change the mode
              //              mode = 1;
              //              Serial.println("go to mode 1!!");
              mode = 3;
              Serial.println("go to mode 3!!");
            }
          }
        }

        // save the reading. Next time through the loop, it'll be the lastButtonState:
        lastButtonStates[i] = sensorValues[i];
        // save the reading. Next time through the loop, it'll be the lastPressState:
        lastPressStates[i] = sensorValues[i];
      }
      break;

    //pre-centralized control
    case 1:
      //if the led intensity options has been fully shown, go to mode 2
      if (stripIntensity == intensityStrong) {
        mode = 2;
        Serial.println("go to mode 2!!");
      }
      break;

    //centralized control
    case 2:
      for (int i = 0; i < 9; i++) {
        // If the switch changed, due to noise or pressing:
        if (sensorValues[i] != lastButtonStates[i]) {
          lastDebounceTimes[i] = millis();
        }

        if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
          // if the button state has changed:
          if (sensorValues[i] != buttonStates[i]) {
            buttonStates[i] = sensorValues[i];

            // only change the intensity if the new button state is HIGH
            if (buttonStates[i] == HIGH) {
              //change the intensity of the all the strips
              stripIntensity = intensityNone;

              //change the mode back to 0
              mode = 0;
              Serial.println("go to mode 0!!");
            }
          }
        }

        // save the reading. Next time through the loop, it'll be the lastButtonState:
        lastButtonStates[i] = sensorValues[i];
      }
      break;

    //flowing
    case 3:
      for (int i = 0; i < 9; i++) {

        if (sensorValues[i] != lastPressStates[i]) {
          lastLongPressTimes[i] = millis();
        }

        //see if it's long press. If it is, overwrite the intensity and go to mode 1
        if ((millis() - lastLongPressTimes[i]) > longPressDelay) {

          // if the button state has changed:
          if (sensorValues[i] != pressStates[i]) {
            pressStates[i] = sensorValues[i];

            // only change the mode if the new button state is HIGH and the test tile is long pressed
            if (pressStates[i] == HIGH && i == 3) {
              Serial.println("long touch! go back to mode 0");
              stripIntensity = intensityNone;

              //change the mode
              mode = 0;
              Serial.println("go to mode 0!!");
            }
          }
        }

        // save the reading. Next time through the loop, it'll be the lastButtonState:
        lastButtonStates[i] = sensorValues[i];
        // save the reading. Next time through the loop, it'll be the lastPressState:
        lastPressStates[i] = sensorValues[i];
      }
      break;
  }


  //-----------------------------------------------------------------------------
  //show led for different modes
  //-----------------------------------------------------------------------------
  RGBColor color = converter.HSItoRGB(hue, saturation, stripIntensity);
  RGBColor color2 = converter.HSItoRGB(hue, saturation, stripIntensity2);
  RGBColor color2Weak = converter.HSItoRGB(hue, saturation, 100);
  RGBColor color2SuperWeak = converter.HSItoRGB(hue, saturation, 50);

  switch (mode) {
    //individual control mode
    case 0:
      for (int pixel = 0; pixel < pixelCount; pixel++) {
        strip.setPixelColor(pixel, color.red, color.green, color.blue);
      }
      for (int pixel = 0; pixel < pixelCount2; pixel++) {
        strip2.setPixelColor(pixel, color.red, color.green, color.blue);
      }
      break;

    //pre-centralized mode
    case 1:
      if (stripIntensity < intensityStrong) {
        stripIntensity++;
      }
      for (int pixel = 0; pixel < pixelCount; pixel++) {
        strip.setPixelColor(pixel, color.red, color.green, color.blue);
      }
      delay(2);
      break;

    case 2:
      //centralized control mode
      for (int pixel = 0; pixel < pixelCount; pixel++) {
        strip.setPixelColor(pixel, color.red, color.green, color.blue);
      }
      break;

    case 3:
      if (modeThreeType == 0) {
        //glowing mode
        if (stripIntensity > intensityStrong) {
          stripChange = -stripChange;
        }
        stripIntensity += stripChange;
        for (int pixel = 0; pixel < pixelCount; pixel++) {
          strip.setPixelColor(pixel, color.red, color.green, color.blue);
        }
//        Serial.print("mode 3 glowing at: ");
//        Serial.println(stripIntensity);

        if (stripIntensity == 0) {
          modeThreeType = 1; //change to flowing
          stripChange = -stripChange;
          Serial.println("chang to flowing!");
        }
        delay(5);
      } else {
        //flowing mode
        for (int pixel = 0; pixel < pixelCount2; pixel++) {
          strip2.setPixelColor(pixel, 0, 0, 0);
        }

        if (modeThreePixel == 0) {
          strip2.setPixelColor(modeThreePixel, color2.red, color2.green, color2.blue);
        } else if (modeThreePixel == 1) {
          strip2.setPixelColor(modeThreePixel, color2.red, color2.green, color2.blue);
          strip2.setPixelColor(modeThreePixel - 1, color2Weak.red, color2Weak.green, color2Weak.blue);
        } else {
          strip2.setPixelColor(modeThreePixel, color2.red, color2.green, color2.blue);
          strip2.setPixelColor(modeThreePixel - 1, color2Weak.red, color2Weak.green, color2Weak.blue);
          strip2.setPixelColor(modeThreePixel - 2, color2SuperWeak.red, color2SuperWeak.green, color2SuperWeak.blue);
        }
        modeThreePixel++;
        if (modeThreePixel == pixelCount2 + 3) {
          strip2.setPixelColor(modeThreePixel - 1, 0, 0, 0);
          modeThreeType = 0; //change back to glowing
          modeThreePixel = 0; //reset pixel
          Serial.println("chang to glowing!");
        }
        delay(70);
      }

      break;
  }

  strip.show();
  strip2.show();
}
