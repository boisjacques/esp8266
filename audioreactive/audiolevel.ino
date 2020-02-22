/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/

#include <FastLED.h>
#include <math.h>

#define ANZAHL_LEDS 10
#define LED_PIN 5
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
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level

   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;

   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 5.0) / 1024;  // convert to volts

   Serial.println(volts);

   unsigned int level;
   level = floor(volts * NUM_LEDS);

   for (int i = 0; i < NUM_LEDS; i++){
      leds[i] = CRGB::Black;
   }

   for (int i = 0; i < level; i++) {
    leds[i] = CRGB::Green;
    FastLED.show();
  }

  delay(100);

}
