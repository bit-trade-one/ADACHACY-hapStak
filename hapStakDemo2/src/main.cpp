/*
  hapStak DemoProgram
  version 1.0

  Copyright (C) 2021 Foster Electric Co., Ltd.

  version 1.0 changes:
  - PlatformIO project
  - M5Unified (ATOM Matrix / AtomS3 / AtomS3R auto detection)
  - WAV data is embedded in flash (PROGMEM), no SPIFFS write step required
  - LCD GUI for AtomS3 / AtomS3R

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <M5Unified.h>
#include <math.h>

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorWAVRepeatable.h"
#include "AudioOutputI2S.h"

#include "ui.h"

// 振動波形データ（フラッシュ埋め込み）
#include "wav/boot.h"
#include "wav/file01.h"
#include "wav/file02.h"
#include "wav/file03.h"
#include "wav/file04.h"
#include "wav/file05.h"
#include "wav/file06.h"
#include "wav/file07.h"
#include "wav/file08.h"
#include "wav/file09.h"
#include "wav/file10.h"
#include "wav/file11.h"
#include "wav/file12.h"
#include "wav/file13.h"
#include "wav/file14.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////

// hapStakモジュール(裏面ソケット)のI2Sピン
#if defined(CONFIG_IDF_TARGET_ESP32S3)
// AtomS3 / AtomS3R : ATOM Matrixと物理位置互換のピン (G19->G6, G33->G8, G22->G5)
const int CONFIG_I2S_BCK_PIN  = 6;
const int CONFIG_I2S_LRCK_PIN = 8;
const int CONFIG_I2S_DATA_PIN = 5;
#else
// ATOM Matrix
const int CONFIG_I2S_BCK_PIN  = 19;
const int CONFIG_I2S_LRCK_PIN = 33;
const int CONFIG_I2S_DATA_PIN = 22;
#endif

const float DEFAULT_GAIN = 1.0; // 振動信号のデフォルトゲイン。0.0～1.0
const float BEEP_GAIN = 0.5;    // Beep音、起動音のゲイン。0.0～1.0

const float ACCEL_THRESHOLD = 3.0;  // Swingトリガー時の閾値加速度[G]
const int SWING_COOLDOWN = 300;     // Swing再生終了後の再トリガー禁止時間[ms]（自身の振動による誤トリガー防止）

const int INITIAL_DELAY = 0;  // 動作開始前のdelay値
const int LOOP_DELAY = 5;     // メインループのdelay値

const int FADEIN_STEP = int(1000 / LOOP_DELAY); // フェードインのステップ数
const int FADEOUT_STEP = int(250 / LOOP_DELAY); // フェードアウトのステップ数

const long LONG_COUNT = int(300 / LOOP_DELAY);      // 長押しの反応時間
const long LONG_LONG_COUNT = int(900 / LOOP_DELAY); // 超長押しの反応時間
const long MULTI_COUNT = int(250 / LOOP_DELAY);     // ダブル・トリプルクリックの反応時間（クリック間隔がこれ以内なら連打と判定）

//////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
  const unsigned char* data;
  size_t size;
} WavRef;

#define WAV_REF(arr) { arr, sizeof(arr) }

typedef struct {
  int dispNumber;
  WavRef wav;
  const char* name;
  int triggerMode;
  float gain;
  uint8_t dispColor[3];
} TrackData;

#define COLOR_RED { 0xFF, 0x00, 0x00 }
#define COLOR_GREEN { 0x00, 0xFF, 0x00 }
#define COLOR_BLUE { 0x00, 0x00, 0xFF }

const int BUTTON_FIRST = 0;
const int ACCEL_FIRST = 3;
const int LOOP_FIRST = 8;

const int TRACK_COUNT = 14; // 振動再生モード（トラック）の数

// トラックデータ
// LED/LCD表示の番号(0～F), 振動波形データ, 信号名, 駆動モードの種類, 再生ゲイン, 表示色
const TrackData trackData[TRACK_COUNT] = {
  // One Shot
  { 0x1, WAV_REF(file07), "Pistol",              TRIG_BUTTON, DEFAULT_GAIN, COLOR_RED },   // BUTTON_FIRST
  { 0x2, WAV_REF(file08), "Machine Gun",         TRIG_BUTTON, DEFAULT_GAIN, COLOR_RED },
  { 0x3, WAV_REF(file09), "Bazooka",             TRIG_BUTTON, DEFAULT_GAIN, COLOR_RED },
  // Swing
  { 0x4, WAV_REF(file10), "Tennis",              TRIG_SWING,  DEFAULT_GAIN, COLOR_GREEN }, // ACCEL_FIRST
  { 0x5, WAV_REF(file11), "Wooden Bat",          TRIG_SWING,  DEFAULT_GAIN, COLOR_GREEN },
  { 0x6, WAV_REF(file12), "Iron Bat",            TRIG_SWING,  DEFAULT_GAIN, COLOR_GREEN },
  { 0x7, WAV_REF(file13), "Sword Slash (Blank)", TRIG_SWING,  DEFAULT_GAIN, COLOR_GREEN },
  { 0x8, WAV_REF(file14), "Sword Slash (Object)",TRIG_SWING,  DEFAULT_GAIN, COLOR_GREEN },
  // Loop
  { 0x9, WAV_REF(file01), "Low Vibe (60Hz)",     TRIG_LOOP,   DEFAULT_GAIN, COLOR_BLUE },  // LOOP_FIRST
  { 0xA, WAV_REF(file02), "Crispy Pulse",        TRIG_LOOP,   DEFAULT_GAIN, COLOR_BLUE },
  { 0xB, WAV_REF(file03), "Up Sweep",            TRIG_LOOP,   DEFAULT_GAIN, COLOR_BLUE },
  { 0xC, WAV_REF(file04), "Alpha Wave (60Hz)",   TRIG_LOOP,   DEFAULT_GAIN, COLOR_BLUE },
  { 0xD, WAV_REF(file05), "Dinosaur Walk",       TRIG_LOOP,   DEFAULT_GAIN, COLOR_BLUE },
  { 0xE, WAV_REF(file06), "Heart Beat",          TRIG_LOOP,   DEFAULT_GAIN, COLOR_BLUE },
};

const WavRef bootWav = WAV_REF(boot);

//////////////////////////////////////////////////////////////////////////////////////////////////////////

AudioGeneratorWAVRepeatable *wav;
AudioFileSourcePROGMEM *file;
AudioOutputI2S *out;

int currentIndex = 0;
TrackData currentData;

double curGain = 0.0;   // フェードイン・アウト時のゲイン
double fadeDelta = 0.0; // フェードイン・アウト時の変化量

bool isError = false;

void AudioLoop();
void ButtonLoop();
void AccelLoop();
void DisplayLoop();
void SingleDown();
void SinglePress();
void DoublePress();
void TriplePress();
void LongPress();
void LongLongPress();
void SelectTrack(int index);
void NextTrack();
void PrevTrack();
void LoadTrack();
void LoadFile(const WavRef& ref, float gain, bool repeat);
void Play();
void Stop();
void FadePlay();
void FadeStop();

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  auto cfg = M5.config();
  cfg.internal_spk = false; // 内蔵スピーカー・マイクは使わない（I2SはhapStak用）
  cfg.internal_mic = false;
  M5.begin(cfg);
  Serial.begin(115200);
  delay(50);
  delay(INITIAL_DELAY);

  Serial.printf("hapStakDemo (%s)\n",
#if defined(CONFIG_IDF_TARGET_ESP32S3)
    M5.getBoard() == m5::board_t::board_M5AtomS3R ? "AtomS3R" : "AtomS3"
#else
    "ATOM Matrix"
#endif
  );

  UiInit();
  UiSplash();

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

  Serial.printf("board=%d\n", (int)M5.getBoard());
  Serial.printf("IMU: %s (type=%d)\n", M5.Imu.isEnabled() ? "OK" : "not found", (int)M5.Imu.getType());
  if (!M5.Imu.isEnabled()) {
    isError = true; // IMU未検出: Swingトリガーが使用不可
  }

  LoadFile(bootWav, BEEP_GAIN, false);
  Play();
  while (wav->loop()) { /*noop*/ }
  Stop();

  SelectTrack(0);
}

void loop()
{
  M5.update();

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

  if (M5.BtnA.isPressed()) {
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
    } else {
      FadeStop();
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
  UiState st;
  st.trackIndex = currentIndex;
  st.trackCount = TRACK_COUNT;
  st.dispNumber = currentData.dispNumber;
  st.name = currentData.name;
  st.triggerMode = currentData.triggerMode;
  st.color = currentData.dispColor;
  st.playing = wav && wav->isRunning();
  st.error = isError;

  UiUpdate(st);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void AccelLoop()
{
  float accX = 0, accY = 0, accZ = 0;

  if (!M5.Imu.isEnabled()) {
    return;
  }

  M5.Imu.getAccel(&accX, &accY, &accZ);

  float accTotal = sqrt(accX * accX + accY * accY + accZ * accZ);

  if (currentData.triggerMode == TRIG_SWING) {
    // 加速度ピーク値のモニタ（静止時は約1.00G。0.00ならIMU読み出し異常）
    static float peak = 0.0f;
    static uint32_t lastReport = 0;
    if (accTotal > peak) { peak = accTotal; }
    if (millis() - lastReport >= 1000) {
      Serial.printf("accel peak: %.2f G\r\n", peak);
      peak = 0.0f;
      lastReport = millis();
    }

    // 再生中および再生終了直後は再トリガーしない
    // （hapStak自身の振動をIMUが拾い、鳴り続けてしまうのを防ぐ）
    static bool wasRunning = false;
    static uint32_t quietUntil = 0;
    bool running = wav && wav->isRunning();
    if (wasRunning && !running) {
      quietUntil = millis() + SWING_COOLDOWN;
    }
    wasRunning = running;
    if (running || (millis() < quietUntil)) {
      return;
    }

    if (accTotal > ACCEL_THRESHOLD) {
      LoadTrack();
      Play();
    }
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
  LoadFile(currentData.wav, currentData.gain, currentData.triggerMode == TRIG_LOOP);
}

void SelectTrack(int index)
{
  if (!wav) { return; }
  if (wav->isRunning()) {
    Stop();
  }

  currentIndex = index;
  currentData = trackData[currentIndex];

  Serial.printf("SelectTrack:%X %s\r\n", currentData.dispNumber, currentData.name);

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

void LoadFile(const WavRef& ref, float gain, bool repeat)
{
  if (!wav) { return; }
  if (wav->isRunning()) {
    Stop();
  }

  delete file;
  file = new AudioFileSourcePROGMEM(ref.data, ref.size);
  if (!file) {
    Serial.printf("file ERROR!\r\n");
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
  bool result = wav->begin(file, out);
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
  bool result = wav->begin(file, out);
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
