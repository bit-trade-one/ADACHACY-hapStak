#include <SD.h>
#include <ArduinoSound.h>

const char filename[] = "SOUND.WAV";
SDWaveFile waveFile;

void setup() {
  Serial.begin(9600);
  Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card is valid.");

  waveFile = SDWaveFile(filename);

  if (!waveFile) {
    Serial.println("wave file is invalid!");
    while (1); // do nothing
  }

  long duration = waveFile.duration();
  Serial.print("Duration = ");
  Serial.print(duration);
  Serial.println(" seconds");

  if (!AudioOutI2S.canPlay(waveFile)) {
    Serial.println("unable to play wave file using I2S!");
    while (1);
  }

  Serial.println("looping file");
  AudioOutI2S.loop(waveFile);
}

void loop() {
  AudioOutI2S.volume(100);
}
