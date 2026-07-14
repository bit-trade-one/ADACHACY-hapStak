# ADACHACY - hapStakDemo

触感デバイス開発モジュール **hapStak** と M5Stack ATOMシリーズの構成で、14種のSignal Patterns（触覚振動）を試せるデモプログラムです。

本バージョンは旧Arduino IDE版（M5Atomライブラリ + ESP32ボード1.0.6）を全面的に更新したものです。

## 対応ハードウェア

| ボード | SoC | 表示 | ビルド環境名 |
|---|---|---|---|
| ATOM Matrix | ESP32 | 5x5 LEDマトリクス | `m5stack-atom` |
| AtomS3 | ESP32-S3 | 128x128 LCD GUI | `m5stack-atoms3` |
| AtomS3R | ESP32-S3 | 128x128 LCD GUI | `m5stack-atoms3` |

AtomS3とAtomS3R（IMU: MPU6886 / BMI270 の違い等）は、**M5Unifiedが実行時に自動判別**するため同一ファームウェアで動作します。

## 旧版からの変更点

- **PlatformIOプロジェクト化** — Arduino IDEのバージョン固定（ESP32 1.0.6 / M5Atom 0.0.9）が不要になりました
- **M5Unified対応** — ATOM Matrix / AtomS3 / AtomS3R を1つのコードでサポート
- **WAVデータのフラッシュ埋め込み再生（PROGMEM）** — SPIFFSへの書き込みが不要になり、旧版の「`WRITEFILES 1`で書き込み→`0`に戻して再書き込み」という2段階手順を撤廃。**1回の書き込みだけで動作します**
- **AtomS3/S3R用のLCD GUI** — トラック番号・カテゴリ・信号名・再生アニメーションを表示

## ファイルリスト

- `README.md` — 本ドキュメント
- `TESTING.md` — テスト一覧（期待動作・各Signalの意味の一覧）
- `LICENSE.txt` — ライセンス条項（GPLv3）
- `hapStakDemo Instruction.pdf` — 使用方法（旧版の手順書。操作方法の参考）
- `platformio.ini` — PlatformIOプロジェクト設定
- `src/main.cpp` — メインプログラム
- `src/ui.h` / `src/ui_matrix.cpp` / `src/ui_lcd.cpp` — 表示処理（LEDマトリクス / LCD GUI）
- `src/AudioGeneratorWAVRepeatable.{h,cpp}` — シームレスループ再生対応WAVジェネレータ
- `src/wav/*.h` — 振動波形データ（C配列化されたWAV）

## ビルドとインストール

[PlatformIO](https://platformio.org/)（VSCode拡張 または CLI）が必要です。ライブラリ（M5Unified / ESP8266Audio / FastLED）は初回ビルド時に自動でインストールされます。

```sh
# AtomS3 / AtomS3R
pio run -e m5stack-atoms3 -t upload

# ATOM Matrix
pio run -e m5stack-atom -t upload

# シリアルモニタ (115200bps)
pio device monitor
```

> **Note (AtomS3/S3R):** 書き込みに失敗する場合は、本体側面のリセットボタンを緑色LEDが点灯するまで（約2秒）長押ししてダウンロードモードに入れてから書き込んでください。

## 使用方法

1. hapStakモジュールをATOM裏面のソケットに接続し、Atom TailBatまたはUSB-Cから給電すると起動音が鳴りStandby状態になります（起動画面 → Signal 1の表示）。
2. Trigger Switch（ATOM Matrix: 画面パネル / AtomS3系: 画面）で操作します。

| 操作 | 動作 |
|---|---|
| 短押し（クリック） | One Shot: 振動を1回発振 / LOOP: 再生・停止の切り替え（フェードイン・アウト付き） |
| 本体を振る（約3G以上） | Swing: 振動を1回発振 |
| 長押し（約0.3秒） | 次のSignalへ切り替え |
| 長押し（約1秒） | Action Categoryごとにスキップ（One Shot → Swing → LOOP → …） |
| トリプルクリック | Signal 1に戻る |

### 画面表示

**ATOM Matrix（5x5 LED）**
- Signal番号を表示（色はAction Category: One Shot=赤 / Swing=緑 / LOOP=青）
- 右下LED: 再生中（白点灯）
- 右上LED: エラー（赤点灯）

**AtomS3 / AtomS3R（LCD GUI）**
- 起動時: スプラッシュ画面（機種名を自動表示）
- Signal番号（カテゴリ色）／カテゴリチップ（SHOT / SWING / LOOP）
- 信号名（Pistol、Heart Beat等）とトリガーヒント（CLICK! / SHAKE! / REPEAT）
- 14トラックのポジションドット、再生中はイコライザ風アニメーション

## Signal Patterns 仕様

全波形とも **モノラル / 8bit PCM / 8kHz** のWAVデータです。起動音のみ16kHzです。

### One Shot（クリックで1回発振・赤）

| No. | 信号名 | 長さ | データ |
|---|---|---|---|
| 1 | Pistol | 1.5秒 | `file07.h` |
| 2 | Machine Gun | 2.0秒 | `file08.h` |
| 3 | Bazooka | 3.3秒 | `file09.h` |

### Swing（振ると1回発振・緑）

| No. | 信号名 | 長さ | データ |
|---|---|---|---|
| 4 | Tennis | 0.2秒 | `file10.h` |
| 5 | Wooden Bat | 1.4秒 | `file11.h` |
| 6 | Iron Bat | 0.9秒 | `file12.h` |
| 7 | Sword Slash (Blank) | 0.5秒 | `file13.h` |
| 8 | Sword Slash (Object) | 0.3秒 | `file14.h` |

### LOOP（選択中は繰り返し発振・青）

| No. | 信号名 | 長さ(1周期) | データ |
|---|---|---|---|
| 9 | Low Vibe (60Hz) | 0.1秒 | `file01.h` |
| A | Crispy Pulse | 0.08秒 | `file02.h` |
| B | Up Sweep | 0.5秒 | `file03.h` |
| C | Alpha Wave (60Hz) | 10秒 | `file04.h` |
| D | Dinosaur Walk | 1.3秒 | `file05.h` |
| E | Heart Beat | 1.8秒 | `file06.h` |

その他: 起動音 = `boot.h`（2.0秒 / 16kHz）。`file15.h` / `file16.h` はLow Vibeの複製（予備・未使用）。

波形の差し替えは、WAVファイルをC配列化した`.h`を`src/wav/`に置き、`src/main.cpp`の`trackData`テーブル（表示番号・信号名・駆動モード・ゲイン・表示色）を書き換えるだけです。

## ピンアサイン（I2S → hapStak）

hapStakモジュールはATOM裏面ソケットに接続します。AtomS3系はATOM Matrixと**物理位置互換**のGPIOを自動選択します。

| 信号 | ATOM Matrix | AtomS3 / AtomS3R |
|---|---|---|
| BCK | G19 | G6 |
| LRCK | G33 | G8 |
| DATA | G22 | G5 |

## 使用ライブラリ

- [M5Unified](https://github.com/m5stack/M5Unified) — ボード自動判別・LCD・IMU・ボタン
- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) by @earlephilhower — I2S WAV再生（Thanks to @earlephilhower!）
- [FastLED](https://github.com/FastLED/FastLED) — ATOM MatrixのLED表示

## ライセンス

GPLv3 — 詳細は`LICENSE.txt`を参照してください。
Copyright (C) 2021 Foster Electric Co., Ltd.
