/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/

#include <FastLED.h>
#include <math.h>

#define NUM_LEDS 59
#define LED_PIN 4
#define UPDATES_PER_SECOND 100

CRGB leds[NUM_LEDS];
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup() 
{
   delay( 3000 ); // power-up safety delay  
   FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

   
   Serial.begin(115200);
}


void loop() 
{

   int val = analogRead(0);
   int numLedsToLight = map(val, 0, 1023, 0, NUM_LEDS);
   int colorOffset = map(val, 0, 1023, 0, 100);

   // First, clear the existing led values
   FastLED.clear();
   for(int led = 0; led < numLedsToLight; led++) { 
      leds[led].setHue(100 - colorOffset); 
   }
   FastLED.show();
   Serial.println(val);
}
