#include <I2S.h>

const int frequency = 1760; // frequency of square wave in Hz
const int amplitude = 1200; // amplitude of square wave
const int sampleRate = 44100; // sample rate in Hz

const int halfWavelength = (sampleRate / frequency); // half wavelength of square wave

short sample = amplitude; // current sample value
int count = 0;
extern const unsigned char audio[364808];

void setup() {
  Serial.begin(9600);
  Serial.println("I2S simple tone");
  //Serial.println(I2S_PHILIPS_MODE);

  // start I2S at the sample rate with 16-bits per sample
  if (!I2S.begin(I2S_PHILIPS_MODE, sampleRate, 16)) {
  //if (!I2S.begin(I2S_LEFT_JUSTIFIED_MODE, sampleRate, 16)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }
}

void loop() {
  /*if (count % halfWavelength == 0) {
    // invert the sample every half wavelength count multiple to generate square wave
    sample = -1 * sample;
  }//*/
  int ii = count % 103372;
   sample = (short) (audio[ii*2+1]*256+audio[ii*2]);


  // write the same sample twice, once for left and once for the right channel
  I2S.write(sample);
  I2S.write(sample);

  // increment the counter for the next sample
  count++;
}
