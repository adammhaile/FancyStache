#include "Adafruit_NeoPixel.h"
#include <EEPROM.h>

#define NUM_P 10

//#define NEO_PIN 0
#define NEO_PIN 17


Adafruit_NeoPixel stache = Adafruit_NeoPixel(NUM_P,NEO_PIN, NEO_GRB + NEO_KHZ800);

bool _colorMode = false;
void setup()
{
  _colorMode = (bool)EEPROM.read(0);
  EEPROM.write(0, (byte)(!_colorMode));

  stache.begin();
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

static uint32_t wheelHelper(uint16_t pos, uint8_t length, float level)
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
  setPixel(i, Adafruit_NeoPixel::Color(r,g,b));
}

void loop()
{
  static int8_t s = 0;
  static bool dir = true;

  for(uint8_t i=0; i<NUM_P; i++) setPixel(i, 0, 0, 0);

  if(_colorMode)
  {
    setPixel(s - 2, wheelHelper(s, NUM_P, 0.12));
    setPixel(s - 1, wheelHelper(s, NUM_P, 0.25));
    setPixel(s + 0, wheelHelper(s, NUM_P, 0.50));
    setPixel(s + 1, wheelHelper(s, NUM_P, 0.25));
    setPixel(s + 2, wheelHelper(s, NUM_P, 0.12));
  }
  else
  {
//    setPixel(s - 2, 16, 0, 0);
//    setPixel(s - 1, 48, 0, 0);
//    setPixel(s + 0, 128, 0, 0);
//    setPixel(s + 1, 48, 0, 0);
//    setPixel(s + 2, 16, 0, 0);

    setPixel(s - 2, 0, 16, 0);
    setPixel(s - 1, 0, 48, 0);
    setPixel(s + 0, 0, 128, 0);
    setPixel(s + 1, 0, 48, 0);
    setPixel(s + 2, 0, 16, 0);
  }

  s += dir ? 1 : -1;
  if(s>=NUM_P || s<=0) 
  {
    dir = !dir;
  }

  stache.show();

  delay(100);
}





