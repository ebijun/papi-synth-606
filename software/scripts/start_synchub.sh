#!/bin/sh
# NetBSD startup script for PAPI-SYNTH 606

SERIAL_DEV="/dev/ttyU0"
MIDI_HUB="/dev/rmidi0"

echo "=== Starting PAPI-SYNTH 606 Environment ==="

if [ -c "$SERIAL_DEV" ]; then
    stty -f "$SERIAL_DEV" 115200 raw -echo
    echo "[OK] Serial bridge ready: $SERIAL_DEV"
else
    echo "[WARN] $SERIAL_DEV not found. Check USB connection."
fi

if [ -c "$MIDI_HUB" ]; then
    echo "[OK] MIDI Sync Hub ready: $MIDI_HUB"
else
    echo "[WARN] $MIDI_HUB not found."
fi

pc6001vx &
P6_PID=$!

midish

kill $P6_PID 2>/dev/null
echo "=== PAPI-SYNTH 606 Stopped ==="
