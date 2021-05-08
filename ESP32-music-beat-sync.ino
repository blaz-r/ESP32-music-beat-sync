/*
  Beat sync, kind of works for most songs. It's self adjusting frequency, that still needs to be below 250
  and self adjusting magnitude. Works quite well for some songs, doesn't work so good for others.

  Everything was done with esp32, rgbw ledstrip and max9814 microphone.
*/

#include <arduinoFFT.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 13
#define NUM 29

#define MIC_PIN 32

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM, LED_PIN, NEO_GRBW + NEO_KHZ800);

const uint16_t samples = 256;               // This value MUST ALWAYS be a power of 2
const uint16_t samplingFrequency = 20000;   

double vReal[samples];
double vImag[samples];

arduinoFFT FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency);

void setup() {
  strip.begin();
  strip.show();

  Serial.begin(115200);
  delay(500);

}

unsigned long updateTime = 0; // limit adjustment time

// current and previus max magnitued, used for dynamic adjustment
double magMaxPrev = 0;
double magMax = 0;

// current and previus max frequency, used for dynamic adjustment
double freqMaxPrev = 0;
double freqMax = 0;

double lastBeat = 0; // time of last beat in millis()

double prevMag = 0; // previous magnitude
double freq, mag; // peak frequency and magnitude

void analyzeMusic() {
  // read samples into array
  for (uint16_t i = 0; i < samples; i++) {
    vReal[i] = analogRead(MIC_PIN);
    vImag[i] = 0.0;
  }

  FFT.DCRemoval();  // remove DC
  FFT.Windowing(FFT_WIN_TYP_BLACKMAN, FFT_FORWARD); // blackman windowing works quite well, could try others
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();

  FFT.MajorPeak(&freq, &mag); // save current peak frequency and magnitued

  // set magnitude and frequency
  magMax = max(mag, magMax);
  // ifnore frequencies above 250, since we want bass
  if(freq < 250)
    freqMax = max(freq, freqMax);

  // dynamic adjustment of limits, values are empircal and can be tweaked, this setup works quite good
  if(millis() - updateTime > 4200){
    /**
     * Adjust max magnitude if the difference from previus max magnitude is more than 13%
     * or if difference between max magnitude and current calcuated magnitued is more than 50%
     */
    if(abs(magMaxPrev - magMax) > magMax*0.13 || abs(magMax - mag) > magMax*0.5) {
      magMax *= 0.6; // 60% of previous value
      magMaxPrev = magMax;  // save it for further adjsutments
    }
    /**
     * Adjust max frequency if the difference from previus max frequency is more than 20%
     * or if difference between max magnitude and current calcuated magnitued is more than 69%
     */
    if(abs(freqMaxPrev - freqMax) > freqMax*0.2 || abs(freqMax - freq) > freqMax*0.69) {
      freqMax *= 0.8;
      freqMaxPrev = freqMax;
    }
    
    updateTime = millis();  // save time of last update
  }
}

void logData() {
  Serial.print("f0=");
  Serial.print(freq, 6);
  Serial.print("Hz -> ");
  Serial.print(mag, 6);
  Serial.print(", Vmax:");
  Serial.print(magMax);
  Serial.print(", Fmax:");
  Serial.println(freqMax);   
}

int fade = 0; // fading of leds

/* dynamically adjust the required limits for beat to count
 * values here are mostly empirically set, we want frequency to be lover than 65% of peak frequency
 * difference between previous beat magnitued and current needs to be at least 31%, and magnitued needs
 * to be at least 42% of peak magnitude
 */
void controlLed() {
  if(freq < freqMax*0.65 && abs(mag - prevMag) > magMax * 0.314 && mag > magMax * 0.42) {
    logData();
     
    uint32_t color = strip.Color(0, 0, 0, 100);
    strip.fill(color);

    fade = 100;
    lastBeat = millis();
  } else {
    uint32_t color = strip.Color(0, 0, 0, fade);
    strip.fill(color);
  }

  // every iteration reduces the fade
  if(fade > 0)
    fade -= 1; 
  
  strip.show(); 
}

void loop() {
  // only calcualte beat every 300ms, this suffices for music up to 200BPM
  if(millis() - lastBeat > 300)
    analyzeMusic();
  controlLed();
  prevMag = mag; // save magnitude for use in next cycle
}
