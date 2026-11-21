# NetBSD/earmv7hf ソフトウェア導入・連携ガイド

## 1. デバイス権限の設定
一般ユーザーで USBシリアル (`/dev/ttyU0`) と USB-MIDI (`/dev/rmidi0`) を扱うため、ユーザーを `wheel` および `dialer` グループに追加します。
```sh
usermod -G wheel,dialer あなたのユーザー名
```

## 2. 必要パッケージのインストール (pkgsrc)
```sh
cd /usr/pkgsrc/audio/midish && make install clean
```

## 3. PC6001VX へのパッチ適用とビルド
```sh
cd /usr/pkgsrc/emulators/pc6001vx
make extract
cd work/PC6001VX_*
patch -p1 < /path/to/software/patch/pc6001vx_ym2149_tr606.patch
cd /usr/pkgsrc/emulators/pc6001vx
make install clean
```

## 4. midish 運用コマンド一覧
- `go` : TR-606 同期スタート
- `halt` : TR-606 同期ストップ
- `t 120` : テンポ（BPM）を 120 に設定
- `mode_p6` : PC6001VX 自動演奏モード
- `mode_kbd` : USB MIDIキーボード手弾きシンセモード
- `mode_session` : パピコン伴奏 ＋ キーボード手弾きセッションモード
