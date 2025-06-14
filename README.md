# ADCHACY-hapStak

![タイトル画像のURLを右のカッコに](https://bit-trade-one.co.jp/wp/wp-content/uploads/2021/09/hapStak-Bn-BIG.png)  
「触感デバイス開発/体感モジュール“hapStak“」は振動アクチュエーターと  
駆動回路がセットになった、振動による触感再現を手軽に扱うためのモジュール。  
フォスター電機が開発した最新のアクチュエーター「FOSTER ACOUSTICHAPTIC®」を採用、  
従来のモジュールにはない「広域振動」「低遅延」「静粛性」の3拍子併せ持った  
万能アクチュエーターとして様々な触感を生み出します。

## [製品の詳細はこちら](https://bit-trade-one.co.jp/hapstak/) 

## [Q&A](https://github.com/bit-trade-one/ADACHACY-hapStak/blob/master/FAQ.md)
　　
# デジタル版　
<img src="https://bit-trade-one.co.jp/wp/wp-content/uploads/2021/10/9f3637a517794bdd1cc20c901913476b.jpg" width="320px"><img src="https://bit-trade-one.co.jp/wp/wp-content/uploads/2021/10/DSC02596_2.jpg" width="320px">  

M5StackAtomや、ArduinoMKRZeroからのI2S信号により直接モジュールを駆動する事ができるため、  
コンパクトなデバイス作成が可能です。木製の組立ボードも付属。  
  

>ご注意！   デジタル版  
>本製品のご使用には別売のマイコンボード及びプログラミング用のPC,更にそれらへの基本的な知識が必要です。
>M5StackAtom端子とArduinoMKRZero端子は排他となり同時に接続できません。

## 端子情報　

<img src="https://bit-trade-one.co.jp/wp/wp-content/uploads/2021/10/d59c8c71317cfba99d89487564502b4f.jpg" width="720px">  

### [M5 atom のセットアップ方法はこちら](https://qiita.com/eeldog/private/4048234a3f134ed11723)(外部サイトへ遷移）

### [M5 atom を用いた音の鳴らし方](https://qiita.com/eeldog/private/99c412695df9b6c6447c)(外部サイトへ遷移）

### [ArduinoMKRZero を用いた音の鳴らし方](https://qiita.com/eeldog/private/286d1e968b8773e05475)(外部サイトへ遷移）

### [組み立て式ケースの簡易ガイド](https://bit-trade-one.co.jp/hapstakmdfbuild/)(外部サイトへ遷移）

### [サンプルコードはこちら](https://github.com/bit-trade-one/ADACHACY-hapStak/tree/master/Samples)  

### [14種のSignal Patternsを試せる作例はこちら](https://github.com/bit-trade-one/ADACHACY-hapStak/tree/master/hapStakDemo)  


### [変換ツールはこちら](https://github.com/bit-trade-one/ADACHACY-hapStak/tree/master/converter)  

### [付属ケースデータ](https://github.com/bit-trade-one/ADACHACY-hapStak/tree/master/svg)　　

### [回路図・基板図](https://github.com/bit-trade-one/ADACHACY-hapStak/tree/master/Schematics/digital)　

### [3Dプリント用ケース配布](https://github.com/bit-trade-one/ADACHACY-hapStak/tree/master/3Dprint_Case)　

### [発展的なデモソース(GNU GPLv3.0)](https://github.com/bit-trade-one/hapstak_3D_DEMO)　


---
　　
# アナログ版　
  
<img src="https://bit-trade-one.co.jp/wp/wp-content/uploads/2022/03/ADTEDCY_SUB1-fix.png" width="420px"> 
  
アンプを搭載しているため、音声信号入力のみでアクチュエータを振動させることが出来ます。　

>ご注意！アナログ版  
>本製品はアクチュエーターと駆動回路がセットになったモジュール製品です。  
>音源を再生する装置はご自身でご用意ください。USB電源端子と電池ボックスからの給電は排他となり同時使用はできません。　

## 端子情報
<img src="https://bit-trade-one.co.jp/wp/wp-content/uploads/2022/05/アナログ各部.jpg" width="720px">  　
  

### [回路図・基板図](https://github.com/bit-trade-one/ADACHACY-hapStak/tree/master/Schematics/analog)　

### [サンプル動画①プラコップ](https://youtu.be/OYqQZCnmybw)(外部サイトへ遷移）

### [サンプル動画②バドミントン](https://youtu.be/dnwXpK6lNK8)(外部サイトへ遷移）

---
## 作例
・[特定の方向に引っ張られているかのような錯覚を起こすサンプル](https://github.com/stake2722/HapStak_Pullusion/?fbclid=IwAR3_AuXXpM6jU5bLQkxlD852dpKlxqUurUNcO8WBT7jsu5F_fSotosPQlS4) (stake2722様作)

[「ｈ.tanaka」様 /](https://x.com/hide63414/status/1664280147862884352?s=61&t=K7_oe2BjRKS2lTFRml1X2A)



---
## 製品仕様
基本仕様
 
【搭載アクチュエーター】フォスター電機 ACOUSTICHAPTIC® 639897  
【搭載アンプ】デジタル版：NS4168  
【対応マイコンボード】M5StackAtom/Lite/Matrix/Echo(Echo付属スピーカー発音有）、ArduinoMKRZero  
【必要電力】約1W   
（デジタル版:マイコンボードより供給、アナログ版:USBもしくは付属電池ボックスより供給）  
【本体重量】デジタル版：約 45g (MDF組立時）  
アナログ版：基板 約5g、アクチュエーター 約30g  
【本体サイズ】デジタル版：約 W72×H20×D30mm (MDF組立時）  
アナログ版：約 W25×H10×D25mm （基板のみ、電池ボックス及びアクチュエーターを除く）  
【付属品】MDF Stacking Board（デジタル版のみ）、電池ボックス（アナログ版のみ）、保証書  
【パッケージサイズ】F-8 チャック袋梱包　約170*120*20mm  
【使用温度】 0 ～ 40℃（結露なきこと）  
【保証期間】1年間  
【生産国】Made in Japan  
