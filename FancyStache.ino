#include "Adafruit_NeoPixel.h"
#include "colortable.h"
#include <EEPROM.h>

#define LOW 0.0625
#define MED 0.125
#define HIGH 0.5

float levels[] = 
{
  HIGH, MED, LOW
};

#define NUM_P 10

#if defined(__AVR_ATtiny85__ )
 #define NEO_PIN 0
#else
 #define NEO_PIN 17
#endif

#define NUM_ANIMATIONS 9  //STARTS AT 0 (0,1,2,etc) How many different animations are there?


Adafruit_NeoPixel stache = Adafruit_NeoPixel(NUM_P,NEO_PIN, NEO_GRB + NEO_KHZ800);

int _colorMode = 0;

int rate = 100;  //making this a variable allows for per-animation speed adjustment

static long rndSeed = 7890L;
long rnd(long maxVal)
{
  //LCG Algorithm with glibc values: http://en.wikipedia.org/wiki/Linear_congruential_generator
  rndSeed = ((1103515245L * rndSeed + 12345L));
  return (rndSeed % maxVal) - 1;
}
void setup()
{
  _colorMode = (int)EEPROM.read(0);    //Read value from EEPROM

  if (_colorMode >= (NUM_ANIMATIONS)){
    _colorMode = 0;
  }  

  EEPROM.write(0, (byte)(_colorMode+1));  

  stache.begin();
  //_colorMode=8;    //Uncomment and change value to lock animation for testing
}

#define C(r, g, b) ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b

uint32_t Wheel(uint16_t WheelPos, float level)
{
  byte r, g, b;
  switch(WheelPos / 128)
  {
  case 0:
    r = 127 - WheelPos % 128;   //Red down
    g = WheelPos % 128;      // Green up
    b = 0;                  //blue off
    break; 
  case 1:
    g = 127 - WheelPos % 128;  //green down
    b = WheelPos % 128;      //blue up
    r = 0;                  //red off
    break; 
  case 2:
    b = 127 - WheelPos % 128;  //blue down 
    r = WheelPos % 128;      //red up
    g = 0;                  //green off
    break; 
  }

  r *= 2 * level;
  g *= 2 * level;
  b *= 2 * level;

  return(C(r,g,b));
}

uint32_t wheelHelper(uint16_t pos, uint8_t length, float level)
{
  return Wheel((((pos * 384 / length)) % 384), level);
}

void setPixel(uint8_t i, uint32_t c)
{
  if(i < NUM_P)
    stache.setPixelColor(i, c);
}

void setPixel(uint8_t i, uint8_t r, uint8_t g, uint8_t b)
{
  setPixel(i, C(r,g,b));
}

__attribute__((noinline)) uint8_t colorVal(uint8_t mode, int8_t level)
{
  return _colorMode == mode ? levels[abs(level)] * 255 : 0;
}

__attribute__((noinline)) bool TimeElapsed(unsigned long ref, unsigned long wait)
{
  static unsigned long now = 0;
  static bool result;
  result = false;
  now = millis();

  if(now < ref || ref == 0) //for the 50 day rollover or first boot
    result = true;  
  else if((now - ref) > wait)
    result = true;

  return result;
}

void clearAll()
{
  for(uint8_t i=0; i<NUM_P; i++) setPixel(i, 0, 0, 0); 
}

void loop()
{
  static int8_t s = 0;
  static bool dir = true;

  if(_colorMode != 8)  //don't clear for time larson
    clearAll();

  rate = 100;

  if(_colorMode==0) //Rainbow 'Stache
  {
    for(int8_t i = -2; i<=2; i++)
      setPixel(s + i, wheelHelper(s, NUM_P, levels[abs(i)]));
  }
  else if(_colorMode==1 || _colorMode==2 || _colorMode==3) //Red, Green, & Blue Larson Scanner
  {
    for(int8_t i = -2; i<=2; i++)
      setPixel(s + i, 
      colorVal(1, i), 
      colorVal(2, i), 
      colorVal(3, i));
  }
  else if(_colorMode==4) //Your Moustache is Party Mode!
  {
    for(int8_t i=0; i<5; i++)
      setPixel(rnd(NUM_P + 1), Wheel(rnd(384), 0.5));
  }
  else if(_colorMode==5 || _colorMode==6 || _colorMode==7) //Double Larsons! What does it mean?!
  {
    rate = 50;
    for(int8_t i = -1; i<=1; i++)
    {
      setPixel(((NUM_P/2)-1-s/2)+i, 
      colorVal(5, i), 
      colorVal(6, i), 
      colorVal(7, i)); 
      setPixel(((NUM_P/2)+s/2)+i, 
      colorVal(5, i), 
      colorVal(6, i), 
      colorVal(7, i)); 
    }
  }
  else if(_colorMode==8)
  {
    static uint8_t pos = 0;
    static bool timeDir = true;

    rate = 1; 
    static unsigned long secRef = 0;
    static unsigned long timeRef = 0;

    static uint8_t second = 0;
    static RGB timeColor;

    if(TimeElapsed(secRef, 1000))
    {
      secRef = millis();
      second++;
      if(second >= 60) second = 0;
    }

    if(TimeElapsed(timeRef, 1000 / NUM_P))
    {
      timeRef = millis();
      clearAll();
      for(int8_t i = -2; i<=2; i++)
        setPixel(pos + i, Wheel((384/60) * second, levels[abs(i)]));

      //needs its own position and direction
      pos += timeDir ? 1 : -1;
      if(pos>=NUM_P || pos<=0) 
      {
        timeDir = !timeDir;  
      }
    }
  }


  //Increment step counter, change direction if needed
  s += dir ? 1 : -1;
  if(s>=NUM_P || s<=0) 
  {
    dir = !dir;  
  }

  //Send bytes to the lights!
  stache.show();

  delay(rate);
}















