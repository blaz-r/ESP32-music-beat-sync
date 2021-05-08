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

/*  
 *  following are defines used for updating limits, you can adjust if you want
 *  it's all calcualted as X < Y * define, where X is current difference, 
 *  Y is reference value of which we want to get percentage of
 *  example: 
 *    #define LED_FREQ_LIM 0.65
 *    freq < freqMax*LED_FREQ_LIM 
 *    
 *  means that frequency needs to be smaller than 65% of peak frequency.
 *  following defines tell that percentage factor:
 */
 
// difference between current max magnitude and previous max magnitude needs to be greater than magMax*MAX_MAG_DIFF
#define MAX_MAG_DIFF 0.13
// difference between current max magnitude and current calculated magnitude needs to be greater than magMax*CURR_MAG_DIFF
#define CURR_MAG_DIFF 0.5
// difference between current max frequency and previous max frequency needs to be greater than freqMax*MAX_FREQ_DIFF
#define MAX_FREQ_DIFF 0.2  
// difference between current max frequency and current calculated frequency needs to be greater than freqMax*CURR_FREQ_DIFF
#define CURR_FREQ_DIFF 0.7 

/*
 * following are all defines for setting limits that count as beat, calculated same as above
 */
// frequency needs to be lower than freqMax*LED_FREQ_LIM
#define LED_FREQ_LIM 0.65
// difference between current max magnitude and current calculated magnitude needs to be greater than magMax*LED_MAG_DIFF_LIM
#define LED_MAG_DIFF_LIM 0.31
// current magnitued needs to be greater than magMax*LED_MAG_LIM
#define LED_MAG_LIM 0.42

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

  // update max magnitude and frequency
  magMax = max(mag, magMax);
  // ignore frequencies above 250, since we want bass
  if(freq < 250)
    freqMax = max(freq, freqMax);

  // dynamic adjustment of limits, values are empircal and can be tweaked, this setup works quite good
  if(millis() - updateTime > 4200){
    /**
     * Adjust max magnitude if the difference from previus max magnitude is more than 13%
     * or if difference between max magnitude and current calcuated magnitued is more than 50%
     */
    if(abs(magMaxPrev - magMax) > magMax*MAX_MAG_DIFF || abs(magMax - mag) > magMax*CURR_MAG_DIFF) {
      magMax *= 0.6; // 60% of previous value
      magMaxPrev = magMax;  // save it for further adjsutments
    }
    /**
     * Adjust max frequency if the difference from previus max frequency is more than 20%
     * or if difference between max magnitude and current calcuated magnitued is more than 69%
     */
    if(abs(freqMaxPrev - freqMax) > freqMax*MAX_FREQ_DIFF || abs(freqMax - freq) > freqMax*CURR_FREQ_DIFF) {
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
  if(freq < freqMax*LED_FREQ_LIM && abs(mag - prevMag) > magMax*LED_MAG_DIFF_LIM && mag > magMax*LED_MAG_LIM) {
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
