//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// hapStak DemoProgram
// version 0.1
// 
// Copyright (C) 2021 Foster Electric Co., Ltd.
// 
//////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <M5Atom.h>
#include <math.h>

#include "FS.h"
#include "SPIFFS.h"

#include "AudioFileSourceSPIFFS.h"
#include "AudioFileSourceID3.h"
#include "AudioGeneratorWAVRepeatable.h"
#include "AudioOutputI2S.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FLG_WRITE_FILES 1 // 信号ファイルの書き込みを行うかどうか。信号ファイルを初めて書き込むときや変更したときは1、それ以外は0にする。

#define FILE_NAME "/file%02d.wav"  // 振動信号ファイルのファイル名。"%02d"に2桁でNo.が入る。先頭に"/"が必要。
#define BOOT_NAME "/boot.wav"  // 起動音ファイルのファイル名。先頭に"/"が必要。

const float DEFAULT_GAIN = 0.5; // 振動信号のデフォルトゲイン。0.0～1.0
const float BEEP_GAIN = 0.3; // Beep音、起動音のゲイン。0.0～1.0

const float ACCEL_THRESHOLD = 3.0;  // Swingトリガー時の閾値加速度[G]

const int INITIAL_DELAY = 0;  // 動作開始前のdelay値
const int LOOP_DELAY = 5; // メインループのdelay値

const int FADEIN_STEP = int(1000 / LOOP_DELAY); // フェードインのステップ数
const int FADEOUT_STEP = int(250 / LOOP_DELAY); // フェードアウトのステップ数

const long LONG_COUNT = int(300 / LOOP_DELAY);  // 長押しの反応時間
const long LONG_LONG_COUNT = int(900 / LOOP_DELAY); // 超長押しの反応時間
const long MULTI_COUNT = int(50 / LOOP_DELAY);  // ダブル・トリプルクリックの反応時間

//////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
  int dispNumber;
  int fileNumber;
  int triggerMode;
  float gain;
  uint8_t dispColor[3];
} TrackData;

const int TRIG_LOOP = 0;
const int TRIG_BUTTON = 1;
const int TRIG_SWING = 2;

#define COLOR_BLACK { 0x00, 0x00, 0x00 }
#define COLOR_WHITE { 0xFF, 0xFF, 0xFF }
#define COLOR_RED { 0xFF, 0x00, 0x00 }
#define COLOR_GREEN { 0x00, 0xFF, 0x00 }
#define COLOR_BLUE { 0x00, 0x00, 0xFF }

const int BUTTON_FIRST = 0;
const int ACCEL_FIRST = 3;
const int LOOP_FIRST = 8;


const int TRACK_COUNT = 14; // 振動再生モード（トラック）の数

// トラックデータ
// LED表示の番号(0～F), 振動波形ファイルの番号, 駆動モードの種類, 再生ゲイン, LED表示色
TrackData trackData[TRACK_COUNT] = {
  // One Shot
  { 0x1, 7, TRIG_BUTTON, DEFAULT_GAIN, COLOR_RED }, // BUTTON_FIRST
  { 0x2, 8, TRIG_BUTTON, DEFAULT_GAIN, COLOR_RED }, 
  { 0x3, 9, TRIG_BUTTON, DEFAULT_GAIN, COLOR_RED },
  // Swing
  { 0x4, 10, TRIG_SWING, DEFAULT_GAIN, COLOR_GREEN }, // ACCEL_FIRST
  { 0x5, 11, TRIG_SWING, DEFAULT_GAIN, COLOR_GREEN },
  { 0x6, 12, TRIG_SWING, DEFAULT_GAIN, COLOR_GREEN },
  { 0x7, 13, TRIG_SWING, DEFAULT_GAIN, COLOR_GREEN },
  { 0x8, 14, TRIG_SWING, DEFAULT_GAIN, COLOR_GREEN },
  // Loop
  { 0x9, 1, TRIG_LOOP, DEFAULT_GAIN, COLOR_BLUE },  // LOOP_FIRST
  { 0xA, 2, TRIG_LOOP, DEFAULT_GAIN, COLOR_BLUE },
  { 0xB, 3, TRIG_LOOP, DEFAULT_GAIN, COLOR_BLUE },
  { 0xC, 4, TRIG_LOOP, DEFAULT_GAIN, COLOR_BLUE },
  { 0xD, 5, TRIG_LOOP, DEFAULT_GAIN, COLOR_BLUE },
  { 0xE, 6, TRIG_LOOP, DEFAULT_GAIN, COLOR_BLUE },
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CONFIG_I2S_BCK_PIN      19
#define CONFIG_I2S_LRCK_PIN     33
#define CONFIG_I2S_DATA_PIN     22

AudioGeneratorWAVRepeatable *wav;
AudioFileSourceSPIFFS *file;
AudioOutputI2S *out;
AudioFileSourceID3 *id3;

int currentIndex = 0;
TrackData currentData;

double curGain = 0.0; // フェードイン・アウト時のゲイン
double fadeDelta = 0.0; // フェードイン・アウト時の変化量

bool isIMUInit = false;
bool isError = false;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  M5.begin(true, false, true);
  delay(50);

  delay(INITIAL_DELAY);
  Serial.begin(115200);

  Serial.printf("SPIFF init...\n");
  SPIFFS.begin();

  #if FLG_WRITE_FILES
  WriteFiles();
  #endif

  Serial.printf("WAV playback init...\n");

  audioLogger = &Serial;
  
  out = new AudioOutputI2S();
  out->SetPinout(CONFIG_I2S_BCK_PIN, CONFIG_I2S_LRCK_PIN, CONFIG_I2S_DATA_PIN);
  out->SetChannels(1);
  out->SetGain(DEFAULT_GAIN);
  if (!out) {
    isError = true;
    Serial.printf("I2S init FAILED!\n");
  }

  wav = new AudioGeneratorWAVRepeatable();
  if (!wav) {
    isError = true;
    Serial.printf("generator init FAILED!\n");
  }

  Serial.printf("IMU init...\n");

  if (M5.IMU.Init() != 0) {
    isIMUInit = true;
    Serial.printf("FAILED!\n");
  } else {
    isError = false;
    isIMUInit = true;
  }
  
  FillDisplay(0xFFFFFF);
  
  LoadFile(BOOT_NAME, BEEP_GAIN, false);
  Play();
  while (wav->loop()) { /*noop*/ }
  Stop();

  SelectTrack(0);
}

void loop()
{
  AudioLoop();
  ButtonLoop();
  AccelLoop();
  DisplayLoop();
  
  delay(LOOP_DELAY);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void ButtonLoop()
{
  static unsigned long pressCnt = 0;
  static unsigned long releaseCnt = ULONG_MAX;
  static int multiCnt = 0;

  M5.update();
  
  if (M5.Btn.isPressed()) {
    if (pressCnt < ULONG_MAX) {
      pressCnt ++;
    }

    if (pressCnt == 1) {
      SingleDown();  // 押したとき
    } else if (pressCnt == LONG_COUNT) {
      LongPress();
    } else if (pressCnt == LONG_LONG_COUNT) {
      LongLongPress();
    }

    releaseCnt = 0;
  } else {
    if (releaseCnt < ULONG_MAX) {
      releaseCnt ++;
    }

    if ((pressCnt > 0) && (pressCnt < LONG_COUNT)) {
      multiCnt ++;
    }
    
    if (releaseCnt == MULTI_COUNT) {
      if (multiCnt == 1) {
        SinglePress();  // 離したとき
      } else if (multiCnt == 2) {
        DoublePress();
      } else if (multiCnt >= 3) {
        TriplePress();
      }
      multiCnt = 0;
    }
    
    pressCnt = 0;
  }
}

void SingleDown() {
  if (currentData.triggerMode == TRIG_BUTTON) {
    LoadTrack();
    Play();
  }
}

void SinglePress() {
  if (currentData.triggerMode == TRIG_LOOP) {
    if (!(wav->isRunning())) {
      LoadTrack();
      FadePlay();
      //Play();
    } else {
      FadeStop();
      //Stop();
    }
  }
}

void DoublePress() {

}

void TriplePress() {
  SelectTrack(0);
}

void LongPress() {
  NextTrack();
}

void LongLongPress() {
  if (currentIndex < ACCEL_FIRST) {
    SelectTrack(ACCEL_FIRST);
  } else if (currentIndex < LOOP_FIRST) {
    SelectTrack(LOOP_FIRST);
  } else {
    SelectTrack(BUTTON_FIRST);
  }  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void DisplayLoop()
{
  static int num = -1;

  if (currentData.dispNumber != num) {
    DisplayNumber(currentData.dispNumber, currentData.dispColor);
    num = currentData.dispNumber;
  }
  
  DisplayState();
}

const uint8_t digitData[16][25] = {
  { 0,1,1,0,0, 1,0,0,1,0, 1,0,0,1,0, 1,0,0,1,0, 0,1,1,0,0, },  // "0"
  { 0,0,1,0,0, 0,1,1,0,0, 0,0,1,0,0, 0,0,1,0,0, 0,1,1,1,0, },  // "1"
  { 0,1,1,0,0, 1,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 1,1,1,1,0, },  // "2"
  { 1,1,1,0,0, 0,0,0,1,0, 0,1,1,0,0, 0,0,0,1,0, 1,1,1,0,0, },  // "3"
  { 0,0,1,0,0, 0,1,1,0,0, 1,0,1,0,0, 1,1,1,1,0, 0,0,1,0,0, },  // "4"
  { 1,1,1,1,0, 1,0,0,0,0, 1,1,1,0,0, 0,0,0,1,0, 1,1,1,0,0, },  // "5"
  { 0,1,1,0,0, 1,0,0,0,0, 1,1,1,0,0, 1,0,0,1,0, 0,1,1,0,0, },  // "6"
  { 1,1,1,1,0, 0,0,0,1,0, 0,0,1,0,0, 0,1,0,0,0, 0,1,0,0,0, },  // "7"
  { 0,1,1,0,0, 1,0,0,1,0, 0,1,1,0,0, 1,0,0,1,0, 0,1,1,0,0, },  // "8"
  { 0,1,1,0,0, 1,0,0,1,0, 0,1,1,1,0, 0,0,0,1,0, 0,1,1,0,0, },  // "9"
  { 0,1,1,0,0, 1,0,0,1,0, 1,1,1,1,0, 1,0,0,1,0, 1,0,0,1,0, },  // "A"
  { 1,1,1,0,0, 1,0,0,1,0, 1,1,1,0,0, 1,0,0,1,0, 1,1,1,0,0, },  // "B"
  { 0,1,1,0,0, 1,0,0,1,0, 1,0,0,0,0, 1,0,0,1,0, 0,1,1,0,0, },  // "C"
  { 1,1,1,0,0, 1,0,0,1,0, 1,0,0,1,0, 1,0,0,1,0, 1,1,1,0,0, },  // "D"
  { 1,1,1,1,0, 1,0,0,0,0, 1,1,1,0,0, 1,0,0,0,0, 1,1,1,1,0, },  // "E"
  { 1,1,1,1,0, 1,0,0,0,0, 1,1,1,0,0, 1,0,0,0,0, 1,0,0,0,0, },  // "F"
};


void DisplayNumber(int num, uint8_t* color) {  
  uint8_t buf[2+25*3];

  buf[0] = 5;
  buf[1] = 5;

  for (int i = 0; i < 25; i ++) {
    for (int j = 0; j < 3; j ++) {
      buf[2+i*3+j] = (digitData[num][i] == 0) ? 0x00 : color[j];
    }
  }

  M5.dis.displaybuff(buf, 0, 0);
}

void DisplayState() {
  M5.dis.drawpix(4, isError ? 0x00FF00 : 0x000000);
  
  if (!wav) { return; }
  M5.dis.drawpix(24, wav->isRunning() ? 0xFFFFFF : 0x000000);
}

void FillDisplay(uint32_t grb) {
  uint8_t buf[2+25*3];

  buf[0] = 5;
  buf[1] = 5;

  for (int i = 0; i < 25; i ++) {
    buf[2+i*3+0] = (grb & 0xFF00) >> 8;
    buf[2+i*3+1] = (grb & 0xFF0000) >> 16;
    buf[2+i*3+2] = grb & 0xFF;
  }

  M5.dis.displaybuff(buf, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void AccelLoop()
{
  float accX = 0, accY = 0, accZ = 0;
  float gyroX = 0, gyroY = 0, gyroZ = 0;
  float temp = 0;

  if (isIMUInit == false) {
    return;
  }
    
  M5.IMU.getAccelData(&accX, &accY, &accZ);
  //M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
  //M5.IMU.getTempData(&temp);
  
  //Serial.printf("%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\r\n", accX, accY, accZ, gyroX, gyroY, gyroZ, temp);
  //Serial.printf("%.2f,%.2f,%.2f\r\n", accX, accY, accZ);
  
  float accTotal = sqrt(accX * accX + accY * accY + accZ * accZ);

  if ((currentData.triggerMode == TRIG_SWING) && (accTotal > ACCEL_THRESHOLD)) {
    LoadTrack();
    Play();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void AudioLoop()
{
  if (!wav) { return; }
  if (!(wav->isRunning())) { return; }

  if (!(wav->loop())) { // wav->loop()はリピートしない場合のみfalseを返す
    wav->stop();
  }

  if (!(wav->Repeat)) { return; }

  if (fadeDelta < 0) {  // フェードアウト処理
    curGain += fadeDelta;
    if (curGain > 0) {
      out->SetGain(curGain);
    } else {
      fadeDelta = 0.0;
      wav->stop();
    }
  } else if (fadeDelta > 0) { // フェードイン処理
    curGain += fadeDelta;
    if (curGain < currentData.gain) {
      out->SetGain(curGain);
    } else {
      out->SetGain(currentData.gain);
      fadeDelta = 0.0;
    }
  }
}

void LoadTrack()
{
  char fileName[32];
  sprintf(fileName, FILE_NAME, currentData.fileNumber);
    
  LoadFile(fileName, currentData.gain, currentData.triggerMode == TRIG_LOOP);
}

void SelectTrack(int index)
{
  if (!wav) { return; }
  if (wav->isRunning()) {
    Stop();
  }
  
  currentIndex = index;
  currentData = trackData[currentIndex];

  Serial.printf("SelectTrack:%d\r\n", currentData.dispNumber);

  if (currentData.triggerMode == TRIG_LOOP) {
    LoadTrack();
    FadePlay(); 
  }
}

void NextTrack()
{  
  SelectTrack((currentIndex + 1) % TRACK_COUNT);
}

void PrevTrack()
{
  SelectTrack((currentIndex + TRACK_COUNT - 1) % TRACK_COUNT);
}

void LoadFile(char* fileName, float gain, bool repeat)
{
  if (!wav) { return; }
  if (wav->isRunning()) {
    Stop();
  }
  
  Serial.printf("Load:%s\r\n", fileName);
  
  file = new AudioFileSourceSPIFFS(fileName);
  if (!file) {
    Serial.printf("file ERROR!\r\n");
  }
  
  id3 = new AudioFileSourceID3(file);
  if (!id3) {
    Serial.printf("id3 ERROR!\r\n");
  }

  out->SetGain(gain);
  wav->Repeat = repeat;
}

void Play()
{
  if (wav->isRunning()) {
    return;
  }
  
  Serial.print("Play\r\n");
  bool result = wav->begin(id3, out);
  if (!result) {
    isError = true;
    Serial.print("wav begin ERROR!\r\n");
  }
}

void Stop()
{
  Serial.print("Stop\r\n");
  wav->stop();
}

void FadePlay()
{
  Serial.print("FadePlay\r\n");
  curGain = 0.0;
  out->SetGain(curGain);
  bool result = wav->begin(id3, out);
  if (!result) {
    isError = true;
    Serial.print("wav begin ERROR!\r\n");
  }
  fadeDelta = currentData.gain / FADEIN_STEP;
}

void FadeStop()
{
  Serial.print("FadeStop\r\n");
  curGain = currentData.gain;
  fadeDelta = -currentData.gain / FADEOUT_STEP;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

#if FLG_WRITE_FILES

#include "./wav/file01.h"
#include "./wav/file02.h"
#include "./wav/file03.h"
#include "./wav/file04.h"
#include "./wav/file05.h"
#include "./wav/file06.h"
#include "./wav/file07.h"
#include "./wav/file08.h"
#include "./wav/file09.h"
#include "./wav/file10.h"
#include "./wav/file11.h"
#include "./wav/file12.h"
#include "./wav/file13.h"
#include "./wav/file14.h"
#include "./wav/file15.h"
#include "./wav/file16.h"
#include "./wav/boot.h"

const int FILE_COUNT = 16;  // 振動ファイルの数（起動音は含めない）。
const unsigned char* wavePtr[FILE_COUNT] = { file01, file02, file03, file04, file05, file06, file07, file08, file09, file10, file11, file12, file13, file14, file15, file16, };
int waveSize[FILE_COUNT] = { sizeof(file01), sizeof(file02), sizeof(file03), sizeof(file04), sizeof(file05), sizeof(file06), sizeof(file07), sizeof(file08), sizeof(file09), sizeof(file10), sizeof(file11), sizeof(file12), sizeof(file13), sizeof(file14), sizeof(file15), sizeof(file16), };

bool WriteFile(char* fileName, const unsigned char* wavePtr, int waveSize)
{
    Serial.println(fileName);
    Serial.printf("data size = %d\n", waveSize);

    Serial.println("file open");
    File fp = SPIFFS.open(fileName, FILE_WRITE);
  
    Serial.println("file write start");
    fp.write(wavePtr, waveSize);
    fp.close();
    Serial.println("file write end");
  
    fp = SPIFFS.open(fileName, FILE_READ);
    size_t fileSize = fp.size();
    Serial.printf("file size = %d\n", fileSize);

    bool result = fileSize == waveSize;

    Serial.printf("file write %s\n", result ? "successed" : "FAILED");
    Serial.printf("\n");

    return result;
}

void WriteFiles()
{  
  FillDisplay(0xFFFFFF);
  delay(3000);
  
  Serial.println("SPIFFS file write");

  Serial.println("SPIFFS format start");
  SPIFFS.format();
  Serial.println("SPIFFS format end");

  FillDisplay(0x000000);

  int i;
  bool result;
  
  for (i = 0; i < FILE_COUNT; i ++) {
    char fileName[32];
    sprintf(fileName, FILE_NAME, i + 1);
    result = WriteFile(fileName, wavePtr[i], waveSize[i]);
    M5.dis.drawpix(i, result ? 0xFF0000 : 0x00FF00);
  }

  result = WriteFile(BOOT_NAME, boot, sizeof(boot));
  M5.dis.drawpix(i, result ? 0xFF0000 : 0x00FF00);

  delay(5000);
}

#endif

