/****************************************
Example Sound Level Sketch for the 
Adafruit Microphone Amplifier
****************************************/
#include <FastLED.h>
#include <math.h>
#define NUM_LEDS 131
#define LED_PIN 4
#define UPDATES_PER_SECOND 100
#define MIN 400
#define MAX 1023
#define PERIOD 300
#define FLT_MAX 3.40282e+38

CRGB leds[NUM_LEDS];
uint8_t samples[PERIOD];
int decay;
int iteration; 
int peak;
int decay_timer;

float relation(int a, int b) {
    
}

void setup() 
{
   delay( 3000 ); // power-up safety delay  
   FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
   iteration = 0;
   peak = 0;
   decay_timer=0;
   decay = 254;
   for (int i = 0; i < PERIOD; i++) {
      samples[i] = 0;
   }

   Serial.begin(115200);
}

void loop() 
{
   int val = analogRead(0);
   int numLedsToLight = map(val, MIN, MAX, 0, NUM_LEDS);
   int color = map(val, MIN, MAX, 10, 255);
   int average = 2;
   if (numLedsToLight > peak) {
      peak = numLedsToLight;
      decay = 254;
      decay_timer = 0;
   }
   samples[iteration] = numLedsToLight;
   for (int i = 0; i < PERIOD; i++) {
      average = average + samples[i];
   }
   average = average / PERIOD;
   if (average > NUM_LEDS) {
      average = NUM_LEDS;
   }

   float decay_koefficient = pow(1 - 0.0005, decay_timer);
   float peak_koefficient = pow(1 - 0.0000001, decay_timer);
   decay_timer++;
   decay = decay * decay_koefficient;
   peak = peak * peak_koefficient;
   
   // First, clear the existing led values
   FastLED.clear();
   
   for (int led = 0; led < peak; led++) { 
      leds[led].setHSV(color, decay, decay);
   }
   leds[peak - 1].setHSV(color, 255, decay);

   for (int led = 0; led < numLedsToLight; led++) { 
      leds[led].setHSV(color, 255, color);
   }

   for (int i = 0; i < 5; i++) {
      leds[average - 2 + i].setHSV(0,127,255);
   }



   FastLED.show();

   iteration = (iteration + 1) % PERIOD;
}

