/*
  Beat sync, kind of works for most songs. It's self adjusting frequency, that still needs to be below 250
  and self adjusting magnitude. Works quite well for some songs, doesn't work so good for others.

  Everything was done with esp32, rgbw ledstrip and max9814 microphone.
*/

#include <arduinoFFT.h>
#include <Adafruit_NeoPixel.h>
#include <driver/adc.h>

#define LED_PIN 13
#define NUM 29

#define MIC_PIN 32

// frequency needs to be lower than LED_FREQ_LIM
#define LED_FREQ_LIM 150

// magnitude needs to be greater than 1.15*avgMag
#define LED_MAG_LIM 1.15

// average magnitude is calculated by dividing with avgSampleCount, which is limited to AVG_COUNT_LIMIT
#define AVG_COUNT_LIMIT 500

// once avgSampleCount reaches AVG_COUNT_LIMIT, it gets reset back to AVG_COUNT_LOWER
#define AVG_COUNT_LOWER 100

#define BPM 180

// Define this to use reciprocal multiplication for division and some more speedups that might decrease precision
#define FFT_SPEED_OVER_PRECISION

// Define this to use a low-precision square root approximation instead of the regular sqrt() call
// This might only work for specific use cases, but is significantly faster. Only works for ArduinoFFT<float>.
// #define FFT_SQRT_APPROXIMATION

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM, LED_PIN, NEO_GRBW + NEO_KHZ800);

const uint16_t samples = 1024;               // This value MUST ALWAYS be a power of 2
const uint16_t samplingFrequency = 25000;   // sampling freq = (1 / sampling period)

float vReal[samples];
float vImag[samples];

float weighingFactors[samples];

ArduinoFFT<float> FFT = ArduinoFFT<float>(vReal, vImag, samples, samplingFrequency, weighingFactors);

void setup() {
  strip.begin();
  strip.show();

  Serial.begin(115200);
  delay(500);

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
}

float magAvg = 0;
int avgSampleCount = 1;

float lastBeat = 0;  // time of last beat in millis()

float freq, mag;     // peak frequency and magnitude

void analyzeMusic() {
  // read samples into array
  for (uint16_t i = 0; i < samples; i++) {
    // uncomment this to find sampling period
    //unsigned long old_time = micros();
    vReal[i] = adc1_get_raw(ADC1_CHANNEL_4);
    vImag[i] = 0.0;
    //unsigned long new_time = micros();
    //Serial.println(new_time - old_time);
  }

  FFT.dcRemoval();  // remove DC offset
  // commented out since it works fine w/o and we get less delay
  // FFT.Windowing(FFT_WIN_TYP_BLACKMAN, FFT_FORWARD); // blackman windowing works quite well, could try others
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  FFT.majorPeak(freq, mag); // save current peak frequency and magnitude

  // calculate avg of 10 low frequencies
  mag = 0;
  for (int b = 0; b < 10; b++) {
    mag += vReal[b];
  }
  mag /= 10;

  // adjust running average
  magAvg = magAvg + (mag - magAvg) / avgSampleCount;
  avgSampleCount++;

  // when sample count reaches limit, reset to lower value, this way we avoid slow change on music change
  if (avgSampleCount > AVG_COUNT_LIMIT) {
    avgSampleCount = AVG_COUNT_LOWER;
  }
}

void logData() {
  Serial.print("f0=");
  Serial.print(freq, 6);
  Serial.print("Hz -> ");
  Serial.print(mag, 6);
  Serial.print(", Favg:");
  Serial.println(magAvg);  
}

int fade = 0; // fading of leds
double beatTime = 60.0 / BPM * 1000;

/* dynamically adjust the required limits for beat to count
 * values/limits here are mostly empirically set
 * for values to count as beat, we want frequency to be below LED_FREQ_LIM
 * and magnitude needs to be at least magAvg * LED_AVG_MAG_LIM, where LED_AVG_MAG_LIM is by default 1.15 so 115%
 */
void controlLed() {
  if(freq < LED_FREQ_LIM && millis() - lastBeat > beatTime && mag > magAvg * LED_MAG_LIM) {
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
  // only calcualte beat every 333ms, this suffices for music up to 180BPM
  if(millis() - lastBeat > beatTime)
    analyzeMusic();
    
  controlLed();
}
