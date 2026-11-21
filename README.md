# PAPI-SYNTH 606

**NetBSD + NEC PC-6001 (PC6001VX) + Real YM2149 PSG + Roland TR-606 Hybrid Retro-Synth System**

> **Status:** Prototype / Work In Progress (WIP)

---

## Overview (概要)

**PAPI-SYNTH 606** is a hybrid retro-synthesizer and rhythm synchronization system centered around **NetBSD** (`earmv7hf` / `amd64`).
Using a custom **Raspberry Pi Pico** USB composite interface hub (`USB-MIDI` + `USB-CDC Serial`), it bridges the NEC PC-6001 series emulator **PC6001VX** (running Hashi's FM sound driver `p6fmdrv`) with a real **Yamaha YM2149** PSG sound chip and a vintage **Roland TR-606** analog drum machine.

**PAPI-SYNTH 606** は、**NetBSD**（`earmv7hf` / `amd64`）を司令塔とし、自作の **Raspberry Pi Pico** 複合USBハブ（`USB-MIDI` ＋ `USBシリアル`）を介して、PC-6001エミュレータ **PC6001VX**（はっしー氏FM音源ドライバ `p6fmdrv` 稼働）、実機の **YM2149** PSG音源チップ、およびビンテージアナログドラムマシン **Roland TR-606** を完全同期・統合演奏させるハイブリッド・レトロシンセシステムです。

### Key Features (主な特徴)
* **Real Hardware PSG Output:** Hooks PSG I/O port writes (`A0H`/`A1H`) from PC6001VX and drives a real 5V **YM2149** chip in real time with a hardware PWM 2.000 MHz clock.
* **Sample-Accurate TR-606 Sync:** Converts `USB-MIDI` clock (`0xF8`) and `Z$90,n` MML port triggers into 5V **DIN Sync (24 PPQN + Run/Stop)** signals for the Roland TR-606.
* **3 Performance Modes:** Switch seamlessly via `midish` between PC6001VX auto-play, 3-voice polyphonic USB-MIDI keyboard play, and hybrid live sessions.

---

## System Architecture (システム構成図)

```text
[USB MIDI Keyboard] ──(USB)──┐
                             ▼
[Host: NetBSD (earmv7hf / amd64)]
  ├── midish (Master Clock & MIDI Router) ──▶ /dev/rmidi0 ──┐
  └── PC6001VX (p6fmdrv FM Sound Driver)                    │
        ├── FM Voice / TALK ──▶ [3.5mm Audio Out] ──────────┼───────┐
        └── PSG / Z-Cmd ──────▶ /dev/ttyU0 ─────────────────┤       │
                                                            ▼       │
                                         (Single USB Cable: Composite Device)
                                                            ▼       │
                                         [P6-YM2149-606 SYNC HUB]   │
                                           (Raspberry Pi Pico)      │
                                           ├── DIN Sync ──▶ [TR-606]┼──┐
                                           └── 8-bit Bus ─▶ [YM2149]┼──┼──┐
                                                                    ▼  ▼  ▼
                                                             [Analog Mixer]
```

---

## Repository Structure (ディレクトリ構成)

```text
papi-synth-606/
├── README.md                                        # Project overview (EN/JP)
├── LICENSE                                          # BSD 2-Clause License
├── .gitignore                                       # Git ignore rules
├── hardware/
│   ├── gerber/
│   │   └── P6-YM2149-606_SYNC_HUB_Gerber.zip        # PCB Gerber files (95x75mm)
│   ├── P6-YM2149-606_PCB_Assembly_Guide.pdf         # PCB assembly & BOM guide
│   └── Takachi_SW125_Drawing_and_Firmware_Guide.pdf # Enclosure drilling drawing
├── firmware/
│   └── P6_YM2149_606_SyncHub/
│       └── P6_YM2149_606_SyncHub.ino                # Raspberry Pi Pico firmware
├── software/
│   ├── patch/
│   │   └── pc6001vx_ym2149_tr606.patch              # Patch for PC6001VX
│   ├── scripts/
│   │   ├── midishrc_synchub                         # ~/.midishrc configuration
│   │   └── start_synchub.sh                         # NetBSD startup script
│   └── NetBSD_Software_Integration_Guide.pdf        # NetBSD setup manual
└── docs/
    ├── P6-YM2149-606_User_Manual.pdf                # Daily operation manual
    ├── P6-YM2149-606_Test_Specification.pdf         # Hardware/Software test spec
    └── P6-YM2149-606_Verification_Checklist.xlsx    # Verification checklist
```

---

## Quick Start (導入手順)

1. **Hardware & Firmware:**
   * Order the 2-layer PCB (`95mm x 75mm`) using `hardware/gerber/P6-YM2149-606_SYNC_HUB_Gerber.zip`.
   * Flash `firmware/P6_YM2149_606_SyncHub/P6_YM2149_606_SyncHub.ino` to a Raspberry Pi Pico using Arduino IDE (`earlephilhower/arduino-pico` core with **Adafruit TinyUSB** stack enabled).
2. **NetBSD Host Setup:**
   * Install `audio/midish` via `pkgsrc`.
   * Apply `software/patch/pc6001vx_ym2149_tr606.patch` to the PC6001VX source tree and build it.
   * Copy `software/scripts/midishrc_synchub` to `~/.midishrc` and run `./start_synchub.sh`.

---

## Credits & Third-Party Software (権利表記)

This repository contains original hardware designs, firmware, integration scripts, and differential patches only. Please obtain the following third-party software from their respective official sources:
* **PC6001VX:** PC-6001 series emulator by Eighttails.
* **p6fmdrv:** PC-6001 series FM sound driver by Hashi (はっしー).

## License

Licensed under the **BSD 2-Clause License**. See `LICENSE` for details.
