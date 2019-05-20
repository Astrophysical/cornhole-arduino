#include <FastLED.h>

// variables will change:
int sensorState = 0, lastState = 0;

#define SENSORPIN 4
#define NUM_LEDS 10
CRGB leds[NUM_LEDS];

void setLedColor(CRGB color) {
  for ( int i = 0; i < 10; ++i)
  {
    leds[i] = color;
  }
  FastLED.show();
}

void lightShow() {
  int count = 0;
  while ( count < 20 ) {
    for ( int i = 0; i < NUM_LEDS; i += 2) {
      int offset = (i + count) % NUM_LEDS;
      
      leds[offset].r = rand()%128;
      leds[offset].g = rand()%255;
      leds[offset].b = rand()%255;

      leds[offset + 1].r = rand()%255;
      leds[offset + 1].g = rand()%255;
      leds[offset + 1].b = rand()%128;
    }
    FastLED.show();
    delay(100);
    count++;
  }
}

void errorFlash() {
  while ( digitalRead(SENSORPIN) == LOW )
  {
    Serial.print("Warning...\n");
    setLedColor(CRGB::Red);
    FastLED.show();
    delay(500);
    setLedColor(CRGB::Black);
    FastLED.show();
    delay(500);
  }
}

void setup() {
  FastLED.addLeds<NEOPIXEL, 5>(leds, NUM_LEDS);

  // initialize the sensor pin as an input:
  pinMode(SENSORPIN, INPUT);
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup
  Serial.begin(9600);

  setLedColor(3);

  sensorState = digitalRead(SENSORPIN);
  if ( sensorState == LOW )
  {
    errorFlash();
  }
}

void loop() {
  // read the state of the pushbutton value:
  sensorState = digitalRead(SENSORPIN);

  if (sensorState == LOW) {
    // turn LED on:
    lightShow();
  }
  else
  {
     setLedColor(CRGB::White);
  }

}
