# タカチ SW-125 ケース加工寸法図 ＆ ファームウェア導入手順

## 1. タカチ電機工業 SW-125 穴あけ追加工寸法
- ケース外寸: 85mm × 125mm × 40mm (ABS樹脂)
- **底面（基板固定穴 × 4箇所）:** φ3.2mm（M3×6mm スペーサー用）
  - 中心基準ピッチ: 横 87.0mm × 縦 67.0mm（四隅から各 4.0mm 内側）
- **左側面（USB Micro-B ケーブル用長穴）:** 幅 11.5mm × 高さ 7.0mm（角R2.0）、底面内側から高さ 14.5mm
- **右側面（パネル取付用コネクタ穴）:**
  - 5ピン DIN メスコネクタ主穴: φ15.5mm（中心高さ 18.0mm）＋ 左右固定ネジ穴 φ3.2mm（ピッチ 22.2mm）
  - 3.5mm オーディオジャック穴: φ6.5mm（DIN穴中心から横へ 28.0mm、高さ 18.0mm）

## 2. Raspberry Pi Pico ファームウェア書き込み手順
1. Arduino IDE にボードパッケージ `earlephilhower/arduino-pico` を追加します。
2. ツールメニューから以下を設定します:
   - **Board:** `Raspberry Pi Pico`
   - **USB Stack:** `Adafruit TinyUSB`（※必須：USB-MIDIとUSBシリアルの複合動作に必要）
3. `firmware/P6_YM2149_606_SyncHub/P6_YM2149_606_SyncHub.ino` を開き、BOOTSEL ボタンを押しながら接続した Pico へ書き込みます。
