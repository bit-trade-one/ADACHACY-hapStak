/*
  hapStakDemo - AtomS3 / AtomS3R (ESP32-S3) 128x128 LCD GUI表示
  機種の判別・LCD初期化はM5Unifiedが自動で行う
*/
#include <sdkconfig.h>

#if defined(CONFIG_IDF_TARGET_ESP32S3)

#include <M5Unified.h>
#include "ui.h"

static M5Canvas canvas(&M5.Display);

static const int W = 128;
static const int H = 128;
static const int HEADER_H = 20;

static const uint32_t ANIM_INTERVAL = 120;  // 再生中アニメーションの更新間隔[ms]

static uint16_t rgb(const uint8_t* c) { return canvas.color565(c[0], c[1], c[2]); }
static uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) { return canvas.color565(r, g, b); }

static const char* BoardName()
{
  switch (M5.getBoard()) {
    case m5::board_t::board_M5AtomS3R: return "AtomS3R";
    case m5::board_t::board_M5AtomS3:  return "AtomS3";
    default:                           return "ESP32-S3";
  }
}

static const char* ModeLabel(int triggerMode)
{
  switch (triggerMode) {
    case TRIG_BUTTON: return "SHOT";
    case TRIG_SWING:  return "SWING";
    default:          return "LOOP";
  }
}

static const char* ModeHint(int triggerMode)
{
  switch (triggerMode) {
    case TRIG_BUTTON: return "CLICK!";
    case TRIG_SWING:  return "SHAKE!";
    default:          return "REPEAT";
  }
}

void UiInit()
{
  M5.Display.setBrightness(200);
  canvas.setColorDepth(16);
  canvas.createSprite(W, H);
}

void UiSplash()
{
  canvas.fillSprite(rgb(10, 12, 20));

  canvas.setTextDatum(middle_center);
  canvas.setTextColor(TFT_WHITE);
  canvas.setFont(&fonts::FreeSansBold18pt7b);
  canvas.setTextSize(1.0f);
  canvas.drawString("hapStak", W / 2, 48);

  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(rgb(140, 150, 170));
  canvas.drawString("Haptic Demo", W / 2, 76);
  canvas.setTextColor(rgb(90, 100, 120));
  canvas.drawString(BoardName(), W / 2, 110);

  canvas.pushSprite(0, 0);
}

static void Draw(const UiState& st, uint32_t tick)
{
  uint16_t cBg     = rgb(10, 12, 20);
  uint16_t cPanel  = rgb(22, 26, 38);
  uint16_t cHeader = rgb(30, 34, 48);
  uint16_t cGray   = rgb(120, 130, 150);
  uint16_t cAccent = rgb(st.color);
  uint16_t cDim    = rgb(st.color[0] / 4, st.color[1] / 4, st.color[2] / 4);

  canvas.fillSprite(cBg);

  // ---- ヘッダ ----
  canvas.fillRect(0, 0, W, HEADER_H, cHeader);
  canvas.setTextDatum(middle_left);
  canvas.setFont(&fonts::Font2);
  canvas.setTextSize(1.0f);
  canvas.setTextColor(TFT_WHITE);
  canvas.drawString("hapStak", 5, HEADER_H / 2);

  if (st.error) {
    canvas.fillCircle(86, HEADER_H / 2, 6, rgb(220, 40, 40));
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(TFT_WHITE);
    canvas.drawString("!", 86, HEADER_H / 2);
  }

  if (st.playing) {
    // 再生中: イコライザ風アニメーション
    for (int i = 0; i < 4; i++) {
      int h = 4 + (int)(5.5f * (1.0f + sinf(tick * 0.011f + i * 1.9f)));
      canvas.fillRect(102 + i * 6, HEADER_H - 3 - h, 4, h, rgb(80, 220, 120));
    }
  } else {
    // 停止中: 停止アイコン
    canvas.fillRect(108, 6, 9, 9, cGray);
  }

  // ---- トラック番号パネル ----
  canvas.fillRoundRect(5, 26, 62, 70, 8, cPanel);
  canvas.drawRoundRect(5, 26, 62, 70, 8, cAccent);
  char num[3];
  snprintf(num, sizeof(num), "%X", st.dispNumber);
  canvas.setTextDatum(middle_center);
  canvas.setFont(&fonts::FreeSansBold24pt7b);
  canvas.setTextSize(1.5f);
  canvas.setTextColor(cAccent);
  canvas.drawString(num, 36, 62);

  // ---- カテゴリチップ ----
  canvas.fillRoundRect(72, 30, 52, 20, 5, cAccent);
  canvas.setFont(&fonts::Font2);
  canvas.setTextSize(1.0f);
  canvas.setTextColor(TFT_BLACK);
  canvas.drawString(ModeLabel(st.triggerMode), 72 + 26, 40);

  // ---- トリガーヒント・トラック位置 ----
  canvas.setTextColor(cGray);
  canvas.drawString(ModeHint(st.triggerMode), 72 + 26, 62);
  char pos[8];
  snprintf(pos, sizeof(pos), "%d/%d", st.trackIndex + 1, st.trackCount);
  canvas.setTextColor(rgb(80, 90, 110));
  canvas.drawString(pos, 72 + 26, 82);

  // ---- 信号名 ----
  canvas.setFont(&fonts::Font2);
  canvas.setTextColor(TFT_WHITE);
  int tw = canvas.textWidth(st.name);
  canvas.setTextSize((tw > W - 6) ? (float)(W - 6) / tw : 1.0f);
  canvas.drawString(st.name, W / 2, 106);
  canvas.setTextSize(1.0f);

  // ---- トラックドット ----
  int pitch = 8;
  int x0 = (W - pitch * (st.trackCount - 1)) / 2;
  for (int i = 0; i < st.trackCount; i++) {
    if (i == st.trackIndex) {
      canvas.fillCircle(x0 + i * pitch, 121, 3, cAccent);
    } else {
      canvas.fillCircle(x0 + i * pitch, 121, 1, cDim);
    }
  }

  canvas.pushSprite(0, 0);
}

void UiUpdate(const UiState& st)
{
  static int lastIndex = -1;
  static bool lastPlaying = false;
  static bool lastError = false;
  static uint32_t lastAnim = 0;

  uint32_t now = millis();
  bool changed = (st.trackIndex != lastIndex) ||
                 (st.playing != lastPlaying) ||
                 (st.error != lastError);
  bool animate = st.playing && (now - lastAnim >= ANIM_INTERVAL);

  if (!changed && !animate) {
    return;
  }
  lastIndex = st.trackIndex;
  lastPlaying = st.playing;
  lastError = st.error;
  lastAnim = now;

  Draw(st, now);
}

#endif // CONFIG_IDF_TARGET_ESP32S3
