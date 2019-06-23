#include <FastLED.h>
#include <LiquidCrystal.h>

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


CHSV leds[ NUM_LEDS ];

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

void scoreUp(int *a)
{
  ++*a;
  printScore();
};
void scoreDown(int *a)
{
  --*a;
  printScore();
};

#define NUM_BUTTONS 4
button_t buttons[NUM_BUTTONS] = {
    {9, 0, scoreUp},
    {10, 0, scoreDown},
    {11, 0, scoreUp},
    {12, 0, scoreDown}};

void checkButtons()
{
  for (int i = 0; i < NUM_BUTTONS; ++i)
  {
    int bs = digitalRead(buttons[i].pin);
    if (bs == HIGH && buttons[i].state == 0)
    {
      buttons[i].func((i < 2) ? (&red_score) : (&blue_score));
      buttons[i].state = 1;
    }
    else if (bs == LOW)
    {
      buttons[i].state = 0;
    }
  }
}

void printScore()
{
  if (blue_score < 0)
    blue_score = 0;

  if (red_score < 0)
    red_score = 0;

  lcd.setCursor(0, 0);
  String str = "Red:    " + String(red_score);
  lcd.print(str);
  Serial.print(str);

  lcd.setCursor(0, 1);
  str = "Blue:   " + String(blue_score);
  Serial.print(str);
  lcd.print(str);
  lcd.display();
}

void setLEDState(float percent, uint8_t hue)
{
  uint8_t offset = 0;
  fill_solid(leds, NUM_LEDS, CHSV(0,0,0) );
  

  // Show the leds
  FastLED.show();
}

void setup()
{
  FastLED.addLeds<NEOPIXEL, LED_SENSOR>(leds, NUM_LEDS);

  // Init break-beam pins
  pinMode(SENSORPIN, INPUT);
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup

  // Start serial output
  Serial.begin(9600);

  // Setup button pins
  pinMode(buttons[0].pin, INPUT);
  pinMode(buttons[1].pin, INPUT);
  pinMode(buttons[2].pin, INPUT);
  pinMode(buttons[3].pin, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  printScore();
  Serial.print("Setup...\n");
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

#define HOLED_TIME 2000

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

void loop()
{
  // read the state of the pushbutton value:
  sensorState = digitalRead(SENSORPIN);
  switch (currentState)
  {
  case PLAYING:
  {
    if (!beamBroken && (sensorState == LOW))
    {
      currentState = HOLED;
      beamBroken = true;
      stateStart = millis();
      Serial.print("Entering HOLED state...\n");
    }
  }
  break;
  case HOLED:
  {
    // iterLightShow();
    unsigned long timeFromStart = millis() - stateStart;
    setLEDState((float)timeFromStart / (float)HOLED_TIME)

    if ( timeFromStart >= HOLED_TIME)
    {
      Serial.print("Entering PLAYING state...\n");
      currentState = PLAYING;
      beamBroken = false;
    }
  }
  break;
  case VICTORY:
  {
    // if ( (blue_score >= 21) || (red_score >= 21) )
  }
  break;
  }
  checkButtons();
}
