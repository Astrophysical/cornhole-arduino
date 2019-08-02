#include <FastLED.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

// variables will change:
int sensorState = HIGH;

// Break Beam sensor
#define SENSORPIN 8

// LED Defines
#define LED_SENSOR 13
#define NUM_LEDS 119
#define FRAME_START 0
#define FRAME_END 101
#define RING_START 102
#define RING_END 118

#define BLUE_BITS 0xFF
#define RED_BITS 0xFF00


CRGB leds[ NUM_LEDS ];

// Init LCD
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Set initial scores
int blue_score = 0;
int red_score = 0;

// Button struct
typedef struct
{
  const int pin;
  int state;
  void (*func)(int *);
} button_t;

void setDisplay();

void printScore(int red, int blue)
{
  char line[16] = {};

  lcd.setCursor(0, 0);
  snprintf(line, 16, "Red:    %d  ", red);
  lcd.print(line);

  lcd.setCursor(0, 1);
  snprintf(line, 16, "Blue:   %d  ", blue);
  lcd.print(line);
  lcd.display();
}

float velocity = 30.0f;
float dist = 0;

void setLEDState(unsigned long dt)
{
  dist += velocity * ((float)dt/1000.0f);
  
  for (int i = FRAME_START; i <= FRAME_END; ++i)
  {
    leds[i].setHue(((i + (int)dist) % 12) * 30);
  }

  for (int i = RING_START; i <= RING_END; ++i)
  {
    leds[i].setHue(((i + (int)dist) % 12) * 30);
  }
  // Show the leds
  FastLED.show();
}

SoftwareSerial Bluetooth(A1, A0);

void setup()
{
  FastLED.addLeds<NEOPIXEL, LED_SENSOR>(leds, NUM_LEDS);

  // Init break-beam pins
  pinMode(SENSORPIN, INPUT);
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup

  // Start serial output
  Serial.begin(9600);
  Bluetooth.begin(38400);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  printScore(0,0);
  setWhite();
}

// Sets all LEDs to white
void setWhite()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = (CRGB::White);
  }
  FastLED.show();
}

#define HOLED_TIME 3000

bool beamBroken = false;
bool lightShow = false;

enum STATES
{
  PLAYING,
  HOLED,
  VICTORY,
  MAX_STATES
};

int currentState = PLAYING;
unsigned long stateStart = 0;
unsigned long lightShowIter = 0;
unsigned long timeFromStart = 0;
unsigned long lastFrame = 0;

#define INPUT_SIZE 8
void loop()
{
  if(Bluetooth.available()){ // Checks whether data is comming from the serial port
      char input[INPUT_SIZE + 1];
      byte size = Bluetooth.readBytes(input, INPUT_SIZE);
      input[size] = 0;
      Serial.write(input);
      char* score = strtok(input, ":");
      int red =0, blue = 0;
      if (score != 0)
      {
          red = atoi(score);
          score = strtok(NULL, ":");
      }
      if (score != 0)
      {
          blue = atoi(score);
      }
      printScore(red, blue);
  }
  // read the state of the pushbutton value:
  sensorState = digitalRead(SENSORPIN);
  switch (currentState)
  {
  case PLAYING:
  {
    if (!beamBroken && (sensorState == LOW))
    {
      beamBroken = true;
    }

    if (beamBroken && (sensorState == HIGH))
    {
      currentState = HOLED;
      stateStart = millis();
      lastFrame = stateStart;
      timeFromStart =0; 
    }
  }
  break;
  case HOLED:
  {
    // iterLightShow();
    unsigned long thisFrame = millis();
    unsigned long dt = thisFrame - lastFrame;
    timeFromStart += dt;
    lastFrame = thisFrame;

    setLEDState(dt);

    if ( timeFromStart >= HOLED_TIME)
    {
      Serial.print("Entering PLAYING state...\n");
      currentState = PLAYING;
      beamBroken = false;
      setWhite();
    }
  }
  break;
  case VICTORY:
  {
    // if ( (blue_score >= 21) || (red_score >= 21) )
  }
  break;
  }
}
