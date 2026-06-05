#include <Wire.h>
#include <EEPROM.h>
#include <FastLED.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
Adafruit_SSD1306 display (
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

#define TOUCH_PIN 2
#define LED_PIN 6

#define NUM_LEDS 4

CRGB leds[NUM_LEDS];

struct TrophyStats
{
  unsigned long totalTouches;
  unsigned long powerUps;
  byte favoriteMode;
  byte lastMode;
};

TrophyStats stats;

int currentMode = 0;

unsigned long lastTouchTime = 0;
unsigned long buttonDownTime = 0;

bool touchState = false;
bool previousTouchState = false;

int touchCounter = 0;

unsigned long lastInteraction = 0;

bool sleeping = false;

unsigned long tapTimer = 0;
byte tapCount = 0;

const unsigned long doubleTapWindow = 450;
const unsigned long longPressTime = 1500;
const unsigned long sleepAfter = 300000;

String pokerHands[] = 
{
  "HIGH CARD",
  "PAIR",
  "TWO PAIR",
  "THREE KIND",
  "STRAIGHT",
  "FLUSH",
  "FULL HOUSE",
  "FOUR KIND",
  "STRAIGHT FLUSH",
  "ROYAL FLUSH"
};

void loadStats()
{
  EEPROM.get(0, stats);
  if (stats.powerups > 100000)
  {
    stats.totalTouches = 0;
    stats.powerUps = 0;
    stats.favoriteMode = 0;
    stats.lastMode = 0;
  }
}

void saveStats()
{
  EEPROM.put(0, stats);
}

void centerText(String text, int y, int size)
{
  int16_t x1;
  int16_t y1;

  uint16_t w;
  uint16_t h;

  display.setTextSize(size);

  display.getTextBounds(
    text,
    0,
    0,
    &x1,
    &y1,
    &w,
    &h,
  );

  display.setCursor(
    (SCREEN_WIDTH - w) / 2,
    y
  );

  display.print(text);
}

void showSplash()
{
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  centerText("POKER", 8, 2);
  centerText("CHAMPION", 30, 2);

  display.display();

  for (int - i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB::Gold;
    FastLED.show();
    delay(180);
  }

  delay(1200);
}

void showWelcomeScreen()
{
  display.clearDisplay();
  
  centerText("WELCOME", 0, 2);

  display.setTextSize(1);

  display.setCursor(10, 30);
  display.print("Power Ups: ");
  display.print(stats.powerUps);

  display.setCursor(10, 45);
  display.print("Touches: ");
  display.print(stats.totalTouches); 

  display.display();
}

void showModeScreen(String modeName)
{
  display.clearDisplay();

  centerText(modeName, 10, 2);

  display.setTextSize(1);

  display.setCursor(20, 45);
  display.print("Mode ");
  display.print(currentMode + 1 );

  display.display();
}

void readTouch()
{
  touchState = digitalRead(TOUCH_PIN);

  if (touchState && !previousTouchState)
  {
    buttonDownTime = millis();
  }

  if (!touchState && previousTouchState)
  {
    unsigned long heldFor = 
      millis() - buttonDownTime;

    if (heldFor >= longPressTime)
    {
      handleLongPress();
    }
    else
    {
      tapCount++;
      tapTimer = millis();
    }
  }

  previousTouchState = touchState;
}

void process()
{
  if (
    tapCount > 0 &&
    millis() - tapTimer > doubleTapWindow
  )

  {
    if (tapCount == 1)
    {
      handleSingleTap();
    }
    else if (tapCount == 2)
    {
      handleDoubleTap();
    }
    else if (tapCount >= 3)
    {
      handleTripleTap();
    }

    tapCount = 0;
  }
}

void  hangleSingleTap()
{
  stats.totalTouches++;

  currentMode++;

  if (currentMode > 9)
  {
    currentMode = 0;
  }

  stats.lastMode = currentMode;

  saveStats();

  lastInteraction = millis();
}

void handleDoubleTap()
{
  stats.total_touches++;
  showPokerHand();
  lastInteraction = millis();
}

void handleTripleTap()
{
  stats.totalTouches++;
  showStatistics();
  saveStats();
  lastInteraction = millis();
}

void handleLongPress()
{
  sleeping = !sleeping;

  if (sleeping)
  {
    display.clearDisplay();
    display.display();

    FastLED.clear();
    FastLED.show();
  }

  lastInteraction = millis();
}

void showPokerHand()
{
  int hand = random(0, 10);
  display.clearDisplay();

  centerText("YOUR HAND", 0, 1);
  centerText(pokerHands[hand], 24, 2);

  display.display();

  for (int i = 0; i < 8; i++)
  {
    leds[random(NUM_LEDS)] = CRGB::White;
    FastLED.show();
    delay(80);

    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
  }
  delay(2000)
}

void showStatistics()
{
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0,0);
  display.print("Touches:");
  display.print(stats.totalTouches);

  display.setCursor(0,15);
  display.print("PowerUps:");
  display.print(stats.powerUps);

  display.setCursor(0,30);
  display.print("Mode:");
  display.print(currentMode + 1);

  display.setCursor(0,45);
  display.print("Favorite:");
  display.print(stats.favoriteMode + 1);

  display.display();

  delay(3000);
}

void championGold()
{
  static uint8_t glow = 0;
  static int direction = 1;

  glow += direction;

  if(glow > 180) direction = -1;
  if(glow < 30) direction = 1;

  full_solid(
    leds,
    NUM_LEDS,
    CRGB(glow, glow * 0.7, 0)
  );

  FastLED.show();

  display.clearDisplay();
  centerText("CHAMPION", 10, 2);
  centerText("POKER NIGHT", 40, 1);
  display.display();

  delay(10);
}

void royalFlush()
{
  display.clearDisplay();

  centerText("ROYAL", 6, 2);
  centerText("FLUSH", 30, 2);

  display.display();

  for(int i=0; i<NUM_LEDS; i++)
  {
    leds[i] = CRGB::White;
    FastLED.show();
    delay(50);

    leds[i] = CRGB::Gold; 
  }

  FastLED.show();
}

void chipLeader()
{
  static int activeLed = 0;
  static unsigned long lastMove = 0;

  if(millis() - lastMove > 250)
  {
    lastMove = millis();
    FastLED.clear();
    
    for(int i=0; i<=activeLED; i++)
    {
      leds[i] = CRGB::RED;
    }

    FastLED.show();
    activeLED++;

    if(activeLed >= NUM_LEDS)
    {
      activeLed = 0;
    }
  }

  display.clearDisplay();

  centerText("CHIP", 8, 2);
  centerText("LEADER", 34, 2);

  display.display();
}

void allIn()
{
  static bool pulse = false;

  if(pulse)
  {
    fill_solid(
      leds,
      NUM_LEDS,
      CRGB::RED
    );
  }
  else
  {
    fill_solid(
      leds,
      NUM_LEDS,
      CRGB::Black
    );
  }

  pulse = !pulse;

  FastLED.show();

  display.clearDisplay();

  centerText("ALL IN", 20, 2);

  display.display();

  delay(200);
}

void jackpot()
{
  for(int i=0; i<NUM_LEDS ; i++)
  {
    leds[i]=CHSV(
      random(225),
      255, 
      255
    );
  }

  FastLED.show();
  display.clearDisplay();
  centerText("JACKPOT", 20, 2);
  display.display();
  delay(70;
}

void rainbowMode()
{
  static uint8_t hue = 0;

  fill_rainbow(
    leds,
    NUM_LEDS,
    hue,
    20
  );

  hue++;

  FastLED.show();
  display.clearDisplay();
  centerText("RAINBOW", 20, 2);
  display.display();
  delay(25);
}

void winnerMode()
{
  fill_solid(
    leds,
    NUM_LEDS,
    CRGB::Gold
  );

  FastLED.show();
  display.clearDisplay();
  centerText("WINNER", 5, 2);
  centerText("RvBrothers11");

  display.display();
}

void highRoller()
{
  display.clearDisplay();

  centerText("HIGH", 8, 2);
  centerText("ROLLER", 34, 2);

  display.display();

  for(int i=0; i<NUM_LEDS; i++)
  {
    leds[i] = CRGB::PURPLE;
  }

  FastLED.show();
}

void touchCounterMode()
{
  display.clearDisplay();
  centerText("TOUCHES", 0, 1);
  display.setTextSize(2);
  display.setCursor(20,28);
  display.print(stats.totalTouches);

  display.display();

  fill_solid(
    leds,
    NUM_LEDS,
    CRGB::BLUE
  );

  FastLED.show();
}

void secretMode()
{
  display.clearDisplay();
  centerText("HOUSE", 8,2);
  centerText("ALWAYS WINS". 36, 1);

  display.display();

  for(int i=0; i<NUM_LEDS; i++)
  {
    leds[i] = CHSV(
      millis()/5 + i*40,
      255,
      255
    );
  }

  FastLED.show();
}

void runCurrentMode()
{
  switch(currentMode)
  {
    case 0:
      championGold();
      break;
    
    case 1:
      royalFlush();
      break;

    case 2:
      chipLeader;
      break;

    case 3:
      allIn;
      break;

    case 4:
      jackpot();
      break;

    case 5:
      rainbowMode();
      break;

    case 6:
      winnerMode;
      break;

    case 7:
    highRoller;
    break;

    case 8:
      touchCounterMode;
      break;

    case 9:
      secretMode;
      break;
  }

  if(stats.totalTouches > 50)
  {
    stats.favoriteMode = currentMode;
  }
}

void runCurrentMode()
{

}

void setup()
{
  pinMode(TOUCH_PIN, INPUT);

  FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);

  FastLED.setBrightness(80);

  display.begin(
    SSD1306_SWITCHCAPVCC,
    0x3C
  );

  display.setRotation(2); 
  randomSeed(analogRead(A0));
  loadStats();
  stats.powerUps++;
  saveStats();
  showSplash();
  showWelcomeScreen();
  delay(2000)
  lastInteraction = millis();
}

void loop()
{
  readTouch();
  processTaps();

  if (!sleeping)
  {
    runCurrentMode();
  }

  if (
    millis() - lastInteraction > sleepAfter
  )
  {
    sleeping = true;
    FastLED.clear();
    FastLED.show();
    
    display.clearDisplay();
    display.display();
  }
}
