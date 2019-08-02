#include <FastLED.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

//BT1 98d3,81,fd652c
// test
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

CRGB leds[NUM_LEDS];

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

enum STATES
{
  PLAYING,
  HOLED,
  VICTORY,
  DISTANCE,
  MAX_STATES
};

int currentState = PLAYING;

void checkButtons()
{
  int d0 = digitalRead(buttons[0].pin);
  int d1 = digitalRead(buttons[1].pin);
  int d2 = digitalRead(buttons[2].pin);
  int d3 = digitalRead(buttons[3].pin);

  if ( (d1 == LOW) && (d3 == LOW) )
  {
    currentState = DISTANCE;
    buttons[1].state = 1;
    buttons[3].state = 1;
    Serial.print("Entering DISTANCE state...\n");
    return;
  }

  for (int i = 0; i < NUM_BUTTONS; ++i)
  {
    int bs = digitalRead(buttons[i].pin);
    if (bs == LOW && buttons[i].state == 0)
    {
      buttons[i].state = 1;
      if (currentState == DISTANCE)
      {
        currentState = PLAYING;
      }
      else
      {
        buttons[i].func((i < 2) ? (&red_score) : (&blue_score));
        printScore();
      }
    }
    else if (bs == HIGH)
    {
      buttons[i].state = 0;
    }
  }
}

SoftwareSerial Bluetooth(A1, A0);
void printScore()
{
  Serial.print("Print score...");
  char line[16] = {};
  char line2[16] = {};

  if (blue_score < 0)
    blue_score = 0;

  if (red_score < 0)
    red_score = 0;

  lcd.setCursor(0, 0);
  snprintf(line, 16, "Red:    %d  ", red_score);
  lcd.print(line);

  lcd.setCursor(0, 1);
  snprintf(line2, 16, "Blue:   %d  ", blue_score);
  lcd.print(line2);
  lcd.display();

  char bt[8] = {};
  snprintf(bt, 8, "%d:%d\n", red_score, blue_score);
  Bluetooth.write(bt, 8);
}

float velocity = 30.0f;
float dist = 0;

void setLEDState(unsigned long dt)
{
  dist += velocity * ((float)dt / 1000.0f);

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

int trigPin = A2;
int echoPin = A3;

void setup()
{
  FastLED.addLeds<NEOPIXEL, LED_SENSOR>(leds, NUM_LEDS);

  // Init break-beam pins
  pinMode(SENSORPIN, INPUT);
  digitalWrite(SENSORPIN, HIGH); // turn on the pullup

  // Start serial output
  Serial.begin(9600);
  Bluetooth.begin(38400);

  // Setup button pins
  pinMode(buttons[0].pin, INPUT_PULLUP);
  pinMode(buttons[1].pin, INPUT_PULLUP);
  pinMode(buttons[2].pin, INPUT_PULLUP);
  pinMode(buttons[3].pin, INPUT_PULLUP); 

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  printScore();
  Serial.print("Setup...\n");
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

unsigned long stateStart = 0;
unsigned long lightShowIter = 0;
unsigned long timeFromStart = 0;
unsigned long lastFrame = 0;

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
      beamBroken = true;
    }

    if (beamBroken && (sensorState == HIGH))
    {
      currentState = HOLED;
      stateStart = millis();
      lastFrame = stateStart;
      timeFromStart = 0;
      Serial.print("Entering HOLED state...\n");
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

    if (timeFromStart >= HOLED_TIME)
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
  case DISTANCE:
  {
    long duration;
    // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(15);
    digitalWrite(trigPin, LOW);

    // Read the signal from the sensor: a HIGH pulse whose
    // duration is the time (in microseconds) from the sending
    // of the ping to the reception of its echo off of an object.
    pinMode(echoPin, INPUT);
    duration = pulseIn(echoPin, HIGH);

    float feet = ((float)(duration / 2.0f) * 0.0135f) / 12.0f; // Divide by 74 or multiply by 0.0135

    char line[16] = {};

    lcd.setCursor(0, 0);
    lcd.print("Distance:       ");
    lcd.setCursor(0, 1);
    snprintf(line, 16, "%.2f feet    ", feet);
    lcd.print(line);
    lcd.display();
  }
  break;
  }
  checkButtons();
  //delay(10);
}
