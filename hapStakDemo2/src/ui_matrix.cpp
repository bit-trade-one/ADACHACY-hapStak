/*
  hapStakDemo - ATOM Matrix (ESP32) 5x5 LEDマトリクス表示
*/
#include <sdkconfig.h>

#if defined(CONFIG_IDF_TARGET_ESP32)

#include <Arduino.h>
#include <driver/rtc_io.h>
#include <FastLED.h>
#include "ui.h"

static const int LED_PIN = 27;
static const int NUM_LEDS = 25;
static const uint8_t LED_BRIGHTNESS = 40;

static CRGB leds[NUM_LEDS];

// 0～Fの5x5フォント
static const uint8_t digitData[16][25] = {
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

void UiInit()
{
  // M5Unifiedのボード判別がGPIO27(LEDデータ線)をタッチセンサとして使うため、
  // デジタルGPIOに戻してLOWで安定させてから初期化する
  // （そのままだと先頭LEDが化けデータを拾い、表示全体が1ピクセルずれる）
  rtc_gpio_deinit((gpio_num_t)LED_PIN);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(1);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  FastLED.clear(true);
}

void UiSplash()
{
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
}

void UiUpdate(const UiState& st)
{
  static int lastNumber = -1;
  static bool lastPlaying = false;
  static bool lastError = false;

  if ((st.dispNumber == lastNumber) &&
      (st.playing == lastPlaying) &&
      (st.error == lastError)) {
    return;
  }
  lastNumber = st.dispNumber;
  lastPlaying = st.playing;
  lastError = st.error;

  CRGB color = CRGB(st.color[0], st.color[1], st.color[2]);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = digitData[st.dispNumber][i] ? color : CRGB::Black;
  }

  if (st.error) {
    leds[4] = CRGB::Red;      // 右上: エラー表示
  }
  if (st.playing) {
    leds[24] = CRGB::White;   // 右下: 再生中表示
  }

  FastLED.show();
}

#endif // CONFIG_IDF_TARGET_ESP32
