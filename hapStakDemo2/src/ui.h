/*
  hapStakDemo - UI interface
  ATOM Matrix : 5x5 LED matrix (ui_matrix.cpp)
  AtomS3/S3R  : 128x128 LCD GUI (ui_lcd.cpp)
*/
#pragma once

#include <stdint.h>

// 駆動モードの種類
const int TRIG_LOOP = 0;
const int TRIG_BUTTON = 1;
const int TRIG_SWING = 2;

typedef struct {
  int trackIndex;         // 現在のトラック番号 (0～)
  int trackCount;         // トラック総数
  int dispNumber;         // 表示番号 (0x1～0xE)
  const char* name;       // 信号名
  int triggerMode;        // TRIG_LOOP / TRIG_BUTTON / TRIG_SWING
  const uint8_t* color;   // カテゴリ色 RGB
  bool playing;           // 再生中か
  bool error;             // エラー発生か
} UiState;

void UiInit();                    // 表示デバイスの初期化
void UiSplash();                  // 起動画面（起動音再生中の表示）
void UiUpdate(const UiState& st); // 毎ループ呼び出し（変化時のみ再描画）
