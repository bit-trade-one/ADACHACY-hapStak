# ADCHACY - hapStakDemo インストール方法


## はじめに
本作例では、触感デバイス開発モジュールとM5Atom Matrixの構成で、14種のSignal Patternsを試すことができます。

## ファイルリスト
- README.md [本ドキュメント]
- LICENSE.txt[ライセンス条項]
- hapStakDemo Instruction.pdf［使用方法］
- hapStakDemo.ino[サンプルプログラム]
- AudioGeneratorWAVRepeatable.cpp[サンプルプログラム]
- AudioGeneratorWAVRepeatable.h[サンプルプログラム]
- wav/*.h[サンプル音源]

## 前提条件
本作例の動作を確認済みの環境は以下になります。
- Arduino IDE 1.8.19（Windows版）
- ESP32ボードライブラリ 1.0.6
- M5Atomライブラリ 0.0.9
- [ESP8266Audio 1.9.5](https://github.com/earlephilhower/ESP8266Audio/tree/1.9.5) [@earlephilhower氏の
ESP8266Audioライブラリを使用させていただきました。]<BR>
Thanks to  @earlephilhower, author of the ESP8266Audio library.

## 注意事項
　2022年6月現在、最新のArduinoのライブラリでは本サンプルソフトの正常なビルドおよび動作が確認できておりません。指定のバージョンのライブラリをインストールした環境を推奨します。

## インストール方法　
- ボードマネージャのインストール<BR>
「ESP32 by Espressif Systems バージョン1.0.6」をインストールします。
![picture 1](../images/89a5ef96f92f29bdf87d66efbaf3586e0cef3293b27472ae8659c5794c735002.png)  

- M5Atomライブラリのインストール<BR>
「ツール」「ライブラリを管理」からライブラリマネージャを開き、「M5Atom by M5Stack バージョン0.0.9」をインストールします。
![picture 2](../images/5a81abc69ab3942a0fdfbdb9199789f7da432a962d089819ad68d4e7167577f9.png)  

- ESP8266Audioライブラリ<BR>
GitHubからzip形式でダウンロード後、Arduino IDEの「スケッチ」「ライブラリをインクルード」「.ZIP形式のライブラリをインストール」を選択し、ダウンロードしたzipファイルを指定します。
![picture 3](../images/f4b3f09730bf3eaf9dd44b32c0ec8538b107a0d9cf88dc32d05b6caa9cd0955c.png)  

- hapStakDemo<BR>
Arduino IDEの「ファイル」「開く」から予めダウンロード済みの「hapStakDemo.ino」を指定します。
![picture 5](../images/347cd4c3ffe8ea5035ef91f7e852d83a41b2a45d2cb4a5c47637e7ec163d6fba.png)  

- Arduino IDEの設定<BR>
「ツール」メニューから「ボード」は「M5Stack-ATOM」、「シリアルポート」は接続したCOM番号を指定します。
![picture 2](../images/e57135b2a9183bd8201bfc908707f5ecdba7b1e4cdb3e1b5d3fca59896fa6499.png)  

- ビルドとインストール<BR>
「スケッチ」「マイコンボードに書き込む」からビルドとインストールを実施します。以降は、[HapStackDemo Instruction](https://github.com/bit-trade-one/ADACHACY-hapStak/tree/master/hapStakDemo/hapStakDemo_Instruction.pdf)に従ってください。