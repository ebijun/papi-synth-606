/*
 * PAPI-SYNTH 606 (P6-YM2149-606 SYNC HUB) Firmware
 * Target: Raspberry Pi Pico (RP2040)
 * Core: earlephilhower/arduino-pico (USB Stack: Adafruit TinyUSB)
 */

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include "hardware/pwm.h"

// Pin Definitions (via 3.3V -> 5V Level Shifter)
const uint8_t PIN_DATA[8]   = {2, 3, 4, 5, 6, 7, 8, 9}; // DA0 - DA7
const uint8_t PIN_BC1       = 10;
const uint8_t PIN_BDIR      = 11;
const uint8_t PIN_YM_CLK    = 12; // 2.000 MHz PWM
const uint8_t PIN_YM_RESET  = 13;
const uint8_t PIN_DIN_RUN   = 14; // DIN Sync Pin 1 (Start/Stop)
const uint8_t PIN_DIN_CLK   = 15; // DIN Sync Pin 3 (24 PPQN Clock)

Adafruit_USBD_MIDI usb_midi;

unsigned long dinPulseStart = 0;
bool dinPulseHigh = false;
uint8_t serialState = 0;
uint8_t cmdType = 0;
uint8_t regAddr = 0;

// 3-voice polyphonic MIDI fallback state
int8_t voiceNote[3] = {-1, -1, -1};
bool kbdModeActive = false;

void ymWriteBus(uint8_t val) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(PIN_DATA[i], (val >> i) & 0x01);
  }
}

void ymWriteReg(uint8_t reg, uint8_t val) {
  // Latch Address (BDIR=1, BC1=1)
  ymWriteBus(reg & 0x0F);
  digitalWrite(PIN_BDIR, HIGH);
  digitalWrite(PIN_BC1, HIGH);
  delayMicroseconds(1);
  digitalWrite(PIN_BDIR, LOW);
  digitalWrite(PIN_BC1, LOW);
  delayMicroseconds(1);

  // Write Data (BDIR=1, BC1=0)
  ymWriteBus(val);
  digitalWrite(PIN_BDIR, HIGH);
  digitalWrite(PIN_BC1, LOW);
  delayMicroseconds(1);
  digitalWrite(PIN_BDIR, LOW);
  digitalWrite(PIN_BC1, LOW);
  delayMicroseconds(1);
}

void ymMuteAll() {
  ymWriteReg(7, 0x3F); // Disable all tone & noise
  ymWriteReg(8, 0x00);
  ymWriteReg(9, 0x00);
  ymWriteReg(10, 0x00);
}

void setupClock2MHz() {
  gpio_set_function(PIN_YM_CLK, GPIO_FUNC_PWM);
  uint slice = pwm_gpio_to_slice_num(PIN_YM_CLK);
  // Assuming 125MHz system clock -> wrap 61 -> ~2.016MHz (or 120MHz -> wrap 59 -> exact 2.000MHz)
  pwm_set_wrap(slice, 61);
  pwm_set_chan_level(slice, pwm_gpio_to_channel(PIN_YM_CLK), 31);
  pwm_set_enabled(slice, true);
}

uint16_t midiNoteToPeriod(uint8_t note) {
  double freq = 440.0 * pow(2.0, ((int)note - 69) / 12.0);
  uint32_t period = (uint32_t)(2000000.0 / (16.0 * freq) + 0.5);
  if (period > 4095) period = 4095;
  return (uint16_t)period;
}

void handleMidiPacket(uint8_t packet[4]) {
  uint8_t status = packet[1];
  if (status == 0xFA || status == 0xFB) {
    digitalWrite(PIN_DIN_RUN, HIGH);
    return;
  }
  if (status == 0xFC) {
    digitalWrite(PIN_DIN_RUN, LOW);
    return;
  }
  if (status == 0xF8) {
    digitalWrite(PIN_DIN_CLK, HIGH);
    dinPulseStart = micros();
    dinPulseHigh = true;
    return;
  }

  uint8_t msg = status & 0xF0;
  uint8_t note = packet[2];
  uint8_t vel  = packet[3];

  if (msg == 0x90 && vel > 0) {
    if (!kbdModeActive) {
      ymMuteAll();
      ymWriteReg(7, 0x38); // Enable Tone A, B, C
      kbdModeActive = true;
    }
    for (int v = 0; v < 3; v++) {
      if (voiceNote[v] == -1) {
        voiceNote[v] = note;
        uint16_t p = midiNoteToPeriod(note);
        ymWriteReg(v * 2, p & 0xFF);
        ymWriteReg(v * 2 + 1, (p >> 8) & 0x0F);
        ymWriteReg(8 + v, 14);
        break;
      }
    }
  } else if (msg == 0x80 || (msg == 0x90 && vel == 0)) {
    for (int v = 0; v < 3; v++) {
      if (voiceNote[v] == note) {
        voiceNote[v] = -1;
        ymWriteReg(8 + v, 0);
      }
    }
  }
}

void setup() {
  for (int i = 0; i < 8; i++) pinMode(PIN_DATA[i], OUTPUT);
  pinMode(PIN_BC1, OUTPUT);
  pinMode(PIN_BDIR, OUTPUT);
  pinMode(PIN_YM_RESET, OUTPUT);
  pinMode(PIN_DIN_RUN, OUTPUT);
  pinMode(PIN_DIN_CLK, OUTPUT);

  digitalWrite(PIN_BC1, LOW);
  digitalWrite(PIN_BDIR, LOW);
  digitalWrite(PIN_DIN_RUN, LOW);
  digitalWrite(PIN_DIN_CLK, LOW);

  setupClock2MHz();

  digitalWrite(PIN_YM_RESET, LOW);
  delay(10);
  digitalWrite(PIN_YM_RESET, HIGH);
  delay(10);
  ymMuteAll();

  usb_midi.begin();
  Serial.begin(115200);
}

void loop() {
  // 1. Process USB-MIDI
  uint8_t packet[4];
  while (usb_midi.readPacket(packet)) {
    handleMidiPacket(packet);
  }

  // 2. Process USB-CDC Serial (from PC6001VX)
  while (Serial.available() > 0) {
    uint8_t b = Serial.read();
    if (serialState == 0) {
      if (b == 0xAA || b == 0xBB) {
        cmdType = b;
        serialState = 1;
      }
    } else if (serialState == 1) {
      if (cmdType == 0xAA) {
        regAddr = b;
        serialState = 2;
      } else if (cmdType == 0xBB) {
        digitalWrite(PIN_DIN_RUN, (b != 0) ? HIGH : LOW);
        serialState = 0;
      }
    } else if (serialState == 2) {
      kbdModeActive = false;
      ymWriteReg(regAddr, b);
      serialState = 0;
    }
  }

  // 3. End 2ms DIN Sync clock pulse
  if (dinPulseHigh && (micros() - dinPulseStart >= 2000)) {
    digitalWrite(PIN_DIN_CLK, LOW);
    dinPulseHigh = false;
  }
}
