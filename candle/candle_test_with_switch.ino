#include <ColorConverter.h>
#include <Adafruit_Circuit_Playground.h>

#include <Interval.h>
#include <Adafruit_NeoPixel.h>

Interval flickerInterval1;
Interval flickerInterval2;
Interval flickerRestoreInterval;
Interval randomDimInterval;
ColorConverter converter;

//switch
float overallIntensityScale = 0;
int ignitionCount = 0;

int dir = 0; //0 = netrual, 1 = tilted north, 5 = tilted south, 9 directions in total, clock-wise
int strongBaseLED = 5;
int weakBaseLED = 0;
int strongUpperLED = 3;
int weakUpperLED = 6;

//base ring
const int neoPixelPin1 = 9;
const int pixelCount1 = 10;
float baseH = 18; // from RGB 255, 83, 12
float baseS = 100;
float baseI = 53;
float baseChange = 0.1;
float upperChange = 0.1;


//upper ring
const int neoPixelPin2 = 10;
const int pixelCount2 = 7;
float upperH = 18;
float upperS = 100;
float upperI = 53;
float coreH = 240;
float coreS = 100;
float coreI = 50;

float baseIntensityScale = 0.7;
float upperIntensityScale = 1;
float coreIntensityScale = 0.3;

float defaultDirIntensity = 10;
float dirIntensity = 100;
float dirChange = 2;

boolean isRandomDim = false;
float randomIntensityOffset = 0;
long randomDimTime;

//float flickerIntensity = 255;
int flickerDim = 0;
boolean flickerRestored = true;
long flickerTime;

// set up upper ring:
Adafruit_CPlay_NeoPixel strip2 = Adafruit_CPlay_NeoPixel(pixelCount2, neoPixelPin2, NEO_GRBW + NEO_KHZ800);

int prevX, prevY, prevZ;

void setup() {

  CircuitPlayground.begin();

  strip2.begin();    // initialize pixel strip
  strip2.clear();    // turn all LEDs off
  Serial.begin(9600);

  //flicer randomly
  flickerInterval1.setInterval(flickerDim1, 7000);
  flickerInterval2.setInterval(flickerDim2, 9890);
  randomDimInterval.setInterval(randomDim, 8000);

}

void loop() {


  //accel--------------------------------------------

  int x = CircuitPlayground.motionX();
  int y = CircuitPlayground.motionY();
  int z = CircuitPlayground.motionZ();

  Serial.print("x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" z: ");
  Serial.println(z);

  int accel = abs(x - prevX) + abs(y - prevY) + abs(z - prevZ);
  Serial.print("total: ");
  Serial.println(accel);

  prevX = x;
  prevY = y;
  prevZ = z;

  //ignition--------------------------
  if (accel > 3 && z >= 7) {
    ignitionCount++;
    Serial.print("ignition count: ");
    Serial.println(ignitionCount);
  }
  if (ignitionCount == 80) {
    overallIntensityScale = 0.5;
  } else if (ignitionCount == 81) {
    overallIntensityScale = 0;
  } else if (ignitionCount == 100) {
    overallIntensityScale = 0.8;
  } else if (ignitionCount == 101) {
    overallIntensityScale = 0;
  } else if (ignitionCount == 120) {
    overallIntensityScale = 0.8;
  } else if (ignitionCount == 121) {
    overallIntensityScale = 0;
  }else if (ignitionCount >= 150) {
    overallIntensityScale = 1;
  }

  //turn off------------------------
  if (z <=-9) {
    ignitionCount = 0;
    overallIntensityScale = 0;
  }

  //direction control--------------------

  if (x <= -2 && y == 0) {
    dir = 1;
  } else if (x <= -2 && y >= 2) {
    dir = 2;
  } else if (x == 2 && y >= 2) {
    dir = 3;
  } else if (x >= 2 && y >= 2) {
    dir = 4;
  } else if (x >= 2 && y == 0) {
    dir = 5;
  } else if (x >= 2 && y <= -2) {
    dir = 6;
  } else if (x == 0 && y <= -2) {
    dir = 7;
  } else if (x <= -2 && y <= -2) {
    dir = 8;
  } else {
    dir = 0;
  }
  //--------------------------------------------
  //flickerDim
  if (!flickerRestored) {
    Serial.println("needs restore!");
    //    flickerRestoreInterval.setTimeout(flickerRestore, 49);
    if (millis() > flickerTime + 49) {
      flickerRestore();
    }
  }

  //randomDim
  if (isRandomDim) {
    Serial.println("needs restore random!");
    randomIntensityOffset = random(0, 3);
    if (millis() > randomDimTime + 2500) {
      randomDimRestore();
    }
  }


  switch (dir) {
    case 0:
      //do noting to change the strong, weak LED
      strongBaseLED = 99;
      weakBaseLED = 99;
      strongUpperLED = 99;
      weakUpperLED = 99;
      break;
    case 1:
      strongBaseLED = 5;
      weakBaseLED = 0;
      strongUpperLED = 3;
      weakUpperLED = 6;
      break;
    case 2:
      strongBaseLED = 3;
      weakBaseLED = 8;
      strongUpperLED = 4;
      weakUpperLED = 1;
      break;
    case 3:
      strongBaseLED = 2;
      weakBaseLED = 7;
      strongUpperLED = 4;
      weakUpperLED = 1;
      break;
    case 4:
      strongBaseLED = 0;
      weakBaseLED = 5;
      strongUpperLED = 5;
      weakUpperLED = 2;
      break;
    case 5:
      strongBaseLED = 0;
      weakBaseLED = 5;
      strongUpperLED = 6;
      weakUpperLED = 3;
      break;
    case 6:
      strongBaseLED = 9;
      weakBaseLED = 4;
      strongUpperLED = 6;
      weakUpperLED = 3;
      break;
    case 7:
      strongBaseLED = 7;
      weakBaseLED = 2;
      strongUpperLED = 1;
      weakUpperLED = 4;
      break;
    case 8:
      strongBaseLED = 6;
      weakBaseLED = 1;
      strongUpperLED = 2;
      weakUpperLED = 5;
      break;

  }

  //change intensity based on direction
  if (dir == 0) {
    if (dirIntensity >= 25) {
      dirIntensity = dirIntensity - dirChange;
    }
  } else {
    if (dirIntensity < 50) {
      dirIntensity = dirIntensity + dirChange;
    }
  }


  delay(2);

  //determine the final RGB value

  RGBColor baseColorStrong = converter.HSItoRGB(baseH, baseS, (dirIntensity - flickerDim - randomIntensityOffset) * baseIntensityScale * overallIntensityScale);
  RGBColor baseColorWeak = converter.HSItoRGB(baseH, baseS, (50 - dirIntensity - flickerDim - randomIntensityOffset) * baseIntensityScale * overallIntensityScale);
  RGBColor baseColorNeutral = converter.HSItoRGB(baseH, baseS, (defaultDirIntensity - flickerDim - randomIntensityOffset) * baseIntensityScale * overallIntensityScale);
  RGBColor upperColorStrong = converter.HSItoRGB(upperH, upperS, (dirIntensity - flickerDim - randomIntensityOffset) * upperIntensityScale * overallIntensityScale);
  RGBColor upperColorWeak = converter.HSItoRGB(upperH, upperS, (50 - dirIntensity - flickerDim - randomIntensityOffset) * upperIntensityScale * overallIntensityScale);
  RGBColor upperColorNeutral = converter.HSItoRGB(upperH, upperS, (defaultDirIntensity - flickerDim - randomIntensityOffset) * upperIntensityScale * overallIntensityScale);
  RGBColor coreColor = converter.HSItoRGB(coreH, coreS, (defaultDirIntensity - flickerDim - randomIntensityOffset) * coreIntensityScale * overallIntensityScale);


  //add a constant shifting of HUE for two rings
  baseH = baseH + baseChange;
  if (baseH < 16 || baseH > 22) {
    baseChange = -baseChange;
  }

  upperH = upperH + upperChange;
  if (upperH < 13 || upperH > 22) {
    upperChange = -upperChange;
  }



  for (int pixel = 0; pixel < pixelCount1; pixel++) {
    CircuitPlayground.setBrightness(255);

    //base ring
    if (pixel == strongBaseLED) {
      //      CircuitPlayground.strip.setPixelColor(pixel, baseColor.red * orangeScale * flickerIntensity / 255 * dirIntensity / 255, baseColor.green * orangeScale * flickerIntensity / 255 * dirIntensity / 255, baseColor.blue * orangeScale * flickerIntensity / 255 * dirIntensity / 255 ); // set the color for this pixel
      CircuitPlayground.strip.setPixelColor(pixel, baseColorStrong.red, baseColorStrong.green, baseColorStrong.blue); // set the color for this pixel
    } else if (pixel == weakBaseLED) {
      //      CircuitPlayground.strip.setPixelColor(pixel, baseColor.red * orangeScale * flickerIntensity / 255 * (1 - dirIntensity / 255), baseColor.green * flickerIntensity / 255 * orangeScale * (1 - dirIntensity / 255), baseColor.blue * flickerIntensity / 255 * orangeScale * (1 - dirIntensity / 255)); // set the color for this pixel
      CircuitPlayground.strip.setPixelColor(pixel, baseColorWeak.red, baseColorWeak.green, baseColorWeak.blue); // set the color for this pixel
    } else {
      //      CircuitPlayground.strip.setPixelColor(pixel, baseColor.red * orangeScale * flickerIntensity / 255 * defaultDirIntensity / 255, baseColor.green * orangeScale * flickerIntensity / 255 * defaultDirIntensity / 255, baseColor.blue * orangeScale * flickerIntensity / 255 * defaultDirIntensity / 255); // set the color for this pixel
      CircuitPlayground.strip.setPixelColor(pixel, baseColorNeutral.red, baseColorNeutral.green, baseColorNeutral.blue); // set the color for this pixel
    }

    //upper ring
    if (pixel == 0) {
      //      strip2.setPixelColor(pixel, 0 * blueScale  * flickerIntensity / 255, 0 * blueScale * flickerIntensity / 255, 255 * blueScale * flickerIntensity / 255); // set inner most blue
      strip2.setPixelColor(pixel, coreColor.red, coreColor.green, coreColor.blue); // set the color for this pixel
    } else if (pixel == strongUpperLED) {
      //      strip2.setPixelColor(pixel, red * redScale * flickerIntensity / 255 * dirIntensity / 255, green * redScale * flickerIntensity / 255 * dirIntensity / 255, blue * redScale * flickerIntensity / 255 * dirIntensity / 255 ); // set the color for this pixel
      strip2.setPixelColor(pixel, upperColorStrong.red, upperColorStrong.green, upperColorStrong.blue); // set the color for this pixel

    } else if (pixel == weakUpperLED) {
      //      strip2.setPixelColor(pixel, red * redScale * flickerIntensity / 255 * (1 - dirIntensity / 255), green * redScale * flickerIntensity / 255  * (1 - dirIntensity / 255), blue * redScale * flickerIntensity / 255 * (1 - dirIntensity / 255)); // set the color for this pixel
      strip2.setPixelColor(pixel, upperColorWeak.red, upperColorWeak.green, upperColorWeak.blue); // set the color for this pixel
    }
    else {
      //      strip2.setPixelColor(pixel, red * redScale * flickerIntensity / 255 * defaultDirIntensity / 255, green * redScale * flickerIntensity / 255 * defaultDirIntensity / 255, blue * redScale * flickerIntensity / 255 * defaultDirIntensity / 255 ); // set the color for this pixel
      strip2.setPixelColor(pixel, upperColorNeutral.red, upperColorNeutral.green, upperColorNeutral.blue); // set the color for this pixel
    }
    //delay(500);

  }
  CircuitPlayground.strip.show();    // refresh the strip
  strip2.show();    // refresh the strip

  flickerInterval1.check();
  flickerInterval2.check();
  flickerRestoreInterval.check();
  randomDimInterval.check();

}

void flickerDim1() {
  flickerDim = 50;
  flickerRestored = false;
  flickerTime = millis();
}

void flickerDim2() {
  flickerDim = 50;
  flickerRestored = false;
  flickerTime = millis();
}

void flickerRestore() {
  Serial.print("restored!");
  flickerDim = 0;
  flickerRestored = true;
}

void randomDim() {
  isRandomDim = true;
  randomDimTime = millis();
}

void randomDimRestore() {
  randomIntensityOffset = 0;
  isRandomDim = false;
}
