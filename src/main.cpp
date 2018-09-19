#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#define NPROGRAMS 7
#define NUMPIXELS 7
#define STRIPPIN 6
#define POTPIN A5
#define OPTIONSPOTPIN A4
#define OPTIONSBUTTON 2

byte rgb[3];                // max [255,255,255]
float hsv[3];               // max [359.0, 100.0 ,100.0]
int hue, saturation, value; // global hue for steplike loopcolor
bool reset = true;          // reset global values when first entering program
bool direction = false;
bool hide = true; // settings for randomWalk
bool runReturn = true;
bool changeColor = true;
int dimValue = 33;
int maxBrightness = 50; // 0 is maximal maxBrightness, 255 just below maximum
// int dimValue = 10; int maxBrightness = 255; // 0 is maximal maxBrightness,
// 255 just below maximum
int ledRunLight;
int program = 1;
int lastPotValue;
enum Options { Hue, Delay, Chance, LongDelay};

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(
    NUMPIXELS, STRIPPIN, NEO_GRB + NEO_KHZ800); // Led Strip Initialiseren

int wrap(int i, int max) { return i % max; }
int wrap(int i, int min, int max) { return min + (i - min) % (max - min); }

void printColor() {
  Serial.print("rgb: ");
  Serial.print(rgb[0]);
  Serial.print(", ");
  Serial.print(rgb[1]);
  Serial.print(", ");
  Serial.println(rgb[2]);
  Serial.print("hsv: ");
  Serial.print(hsv[0]);
  Serial.print(", ");
  Serial.print(hsv[1]);
  Serial.print(", ");
  Serial.println(hsv[2]);
}

// set led and show effect
void setLed(int led) {
  pixels.setPixelColor(led, rgb[0], rgb[1], rgb[2]);
  pixels.show();
}

// set led without showing
void setSingleLed(int led) {
  pixels.setPixelColor(led, rgb[0], rgb[1], rgb[2]);
}

void setAllLeds() {
  pixels.clear();
  for (int led = 0; led < NUMPIXELS; led++) {
    pixels.setPixelColor(led, rgb[0], rgb[1], rgb[2]);
  }
  pixels.show();
}

void setRGB(int r, int g, int b) {
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

// HSV waarden (360,100,100) uit hsv[] omzetten tot RGB waarden (255) in rgb[]
void HSVtoRGB() {
  float h = hsv[0];
  float s = hsv[1];
  float v = hsv[2];
  int i;
  float f, p, q, t;

  h = max(0.0, min(360.0, h));
  s = max(0.0, min(100.0, s));
  v = max(0.0, min(100.0, v));

  s /= 100;
  v /= 100;

  if (s == 0) {
    // Achromatic (grey)
    rgb[0] = rgb[1] = rgb[2] = round(v * 255);
    return;
  }

  h /= 60; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));
  switch (i) {
  case 0:
    rgb[0] = round(255 * v);
    rgb[1] = round(255 * t);
    rgb[2] = round(255 * p);
    break;
  case 1:
    rgb[0] = round(255 * q);
    rgb[1] = round(255 * v);
    rgb[2] = round(255 * p);
    break;
  case 2:
    rgb[0] = round(255 * p);
    rgb[1] = round(255 * v);
    rgb[2] = round(255 * t);
    break;
  case 3:
    rgb[0] = round(255 * p);
    rgb[1] = round(255 * q);
    rgb[2] = round(255 * v);
    break;
  case 4:
    rgb[0] = round(255 * t);
    rgb[1] = round(255 * p);
    rgb[2] = round(255 * v);
    break;
  default: // case 5:
    rgb[0] = round(255 * v);
    rgb[1] = round(255 * p);
    rgb[2] = round(255 * q);
  }
}

// RGB waarden (range 255) uit rgb[] omzetten tot HSV waarden (360,100,100) in
// hsv[]
void RGBtoHSV() {
  float r = rgb[0];
  float g = rgb[1];
  float b = rgb[2];
  r /= 255.0;
  g /= 255.0;
  b /= 255.0;
  float max = max(max(r, g), b), min = min(min(r, g), b);
  float h, s, v;

  float d = max - min;
  s = max == 0 ? 0 : d / max;

  if (max == min) {
    h = 0; // achromatic
  } else {
    if (max == r)
      h = (g - b) / d + (g < b ? 6 : 0);
    else if (max == g)
      h = (b - r) / d + 2;
    else
      h = (r - g) / d + 4;
    h *= 60;
  }
  v = max;

  hsv[0] = h;
  hsv[1] = s * 100.0;
  hsv[2] = v * 100.0;
}

void setHSV(float h, float s, float v) {
  hue = h;
  saturation = s, value = v;
  hsv[0] = h;
  hsv[1] = s;
  hsv[2] = v;
  HSVtoRGB();
}

void setHue(int h) {
  hsv[0] = h;
  hue = h;
  HSVtoRGB();
}

// Loop hue value
void loopColor(int delayTime) {
  hue = wrap(++hue, 360);
  // Serial.print("Hue Value : "); Serial.println(hue);
  setHue(hue);
  setAllLeds();
  pixels.show();
  delay(delayTime);
}

void getColor(int led) {
  uint32_t rgbColor =
      pixels.getPixelColor(led); // reads rgb color in white,red,green,blue
  rgb[0] = (rgbColor >> 16) & 0x000000FF;
  rgb[1] = (rgbColor >> 8) & 0x000000FF;
  rgb[2] = rgbColor & 0x000000FF;
  printColor();
}

// Set rainbow gradient
void rainbowLeds() {
  if (reset) {
    for (int led = 0; led < NUMPIXELS; led++) {
      float hue = (float)led / (NUMPIXELS - 1) * 300;
      setHSV(hue, 100, 100);
      setHue(hue);
      pixels.setPixelColor(led, rgb[0], rgb[1], rgb[2]);
    }
    pixels.show();
    reset = false;
  }
}

void fillColor() {
  if (reset) {
    setHSV(50, 100, 100);
    reset = false;
  }
  setHue(hue);
  setAllLeds();
}

// Blink all leds
void blink(int hue, int delayTime) {
  setRGB(0, 0, 0);
  setAllLeds();
  delay(delayTime);
  setHSV(hue, 100, 100);
  setAllLeds();
  delay(delayTime);
}

void runLight(int startHue, int delayTime) {
  if (reset) {
    reset = false;
    setRGB(0, 0, 0);
    setAllLeds();
    direction = false;
    hue = startHue;
  }
  setRGB(0, 0, 0);
  setAllLeds();
  if (runReturn && direction) {
    ledRunLight = wrap(--ledRunLight, NUMPIXELS);
    if (ledRunLight == 0) {
      direction = false;
      if (changeColor) {
        hue = wrap(hue + 10, 360);
      }
    }
  } else {
    ledRunLight = wrap(++ledRunLight, NUMPIXELS);
    if (ledRunLight == NUMPIXELS - 1) {
      direction = true;
      if (changeColor) {
        hue = wrap(hue + 10, 360);
      }
    }
  }
  Serial.print("led"); Serial.print(" : "); Serial.println(ledRunLight);
  setHSV(hue, 100, dimValue);
  setLed(ledRunLight - 1);
  setHSV(hue, 100, 100);
  setLed(ledRunLight);
  setHSV(hue, 100, dimValue);
  setLed(ledRunLight + 1);
  delay(delayTime);

  // for (int led = 0; led < NUMPIXELS; led++) {
  //   setRGB(0, 0, 0); setAllLeds();
  //   setHSV(hue, 100,100); setLed(led);
  //   delay(delayTime);
  // }
  // for (int led = NUMPIXELS-1; led > 0; led--) {
  //   setRGB(0, 0, 0); setAllLeds();
  //   setHSV(hue, 100,100); setLed(led);
  //   delay(delayTime);
  // }
}

/** Random walk from left to right and back
 * @param hue       light getColor
 * @param chance    1/chance 'chance' of changing direction
 * @param delayTime time (ms) to wait between steps
 */
void randomWalk(int hue, int chance, int delayTime) {
  if (reset) {
    setRGB(0, 0, 0);
    setAllLeds();
    ledRunLight = 0;
    reset = false;
  }
  setRGB(0, 0, 0);
  setLed(ledRunLight);
  bool changeDirection = random(1, chance + 1) / chance;
  if (changeDirection)
    direction = !direction;
  ledRunLight = max( 0 - hide, min(NUMPIXELS - 1 + hide, ledRunLight - 1 + (2 * direction)));
  setHSV(110, 100, 100);
  setLed(ledRunLight);
  delay(delayTime);
}

int readOptions(Options option) {
  int value = analogRead(OPTIONSPOTPIN);
  switch (option) {
    case Hue:
      value = map(value, 0, 1023, 0, 359);
      break;
    case Delay:
      value = map(value, 0, 1023, 100, 5);
      break;
    case Chance:
      value = map(value, 0, 1023, 1, 20);
      break;
    case LongDelay:
      value = map(value, 0, 1023, 1000, 100);
      break;
  }
  return value;
}

void selectProgram(){
  int potValue = analogRead(POTPIN);
  if (abs(potValue-lastPotValue) > 10) {
    program = min((potValue + 10) / (1023/(NPROGRAMS)),NPROGRAMS-1); // +10 als marge omwille van afronden
    lastPotValue = potValue;
    reset = true; // program changed so reset of initial parameters needed
    // Serial.print("potValue"); Serial.print(" : "); Serial.print(potValue);
    Serial.print(" - program"); Serial.print(" : "); Serial.println(program);
  }
    switch (program) {
    case 0: // turn off leds
      setRGB(0, 0, 0);
      setAllLeds();
      break;
    case 1: // loopColor
      loopColor(readOptions(Delay));
      break;
    case 2:
      randomWalk(110, readOptions(Chance), 15);
      break;
    case 3:
      rainbowLeds();
      break;
    case 4:
      hue = readOptions(Hue);
      fillColor();
      break;
    case 5:
      //hue = readOptions(Hue);
      blink(hue,readOptions(LongDelay));
      break;
    case 6:
      runLight(hue, readOptions(Delay));
      break;
  }
}

void optionBtn() {
  /* code */
}

void setup() {
  Serial.begin(9600);
  randomSeed(digitalRead(A0));
  pinMode(OPTIONSBUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(OPTIONSBUTTON), optionBtn, FALLING);

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.clear();
  pixels.show();
  pixels.setBrightness(maxBrightness);

  setHSV(100, 100, 100);
  for (int i = 0; i < NUMPIXELS; i++) {
    setLed(i); delay(300);
  }  
}


void loop() {
  selectProgram(); // check potentiometer value to check which program to run
  // fillColor();
  // loopColor(5);
  // randomWalk(110, 10, 15);
  // rainbowLeds();
  // blink(250,200);
  // runLight(180, 50);
}
