# keyballer-config

English | [日本語](README.ja.md)

Keyboard settings for [Keyballer](https://github.com/AkiraMisawa/keyballer).

**Flash:** https://akiramisawa.github.io/keyballer/?repo=AkiraMisawa/keyballer-config

| Firmware | Keyboard | Keymap |
| --- | --- | --- |
| Keyball+ | `keyball/keyballplus` | `keymaps/am338` |
| Keyball39 | `keyball/keyball39` | `keymaps/am338` (shared with Keyball+) |

## The am338 keymap

A port of the roBa (ZMK) keymap. It detects Mac or Windows and switches on its own.

- Four layers: base, symbols (hold Tab), numbers and cursor (hold Space or Enter), and function (hold K, ' or Esc; the ball scrolls while it is held)
- Left and right click sit where `,` and `.` are on the base layer; `,` `.` `\` are typed with combos
- On Mac: holding Q gives Cmd; the second and third left thumb-row keys type 英数 / かな (Eisu / Kana) on tap and act as Option when held; ball scrolling is reversed
- Screenshot keys on layer 3: Shift+Cmd+4 / Shift+Cmd+3 on Mac, Win+Shift+S on Windows
- UK mode: the bottom-right key on layer 3 turns it on and off (off at power-up; the OLED shows "UK" while it is on). For a computer set to the UK keyboard layout (Windows "United Kingdom", Linux "gb"): `@` `"` `#` `~` `\` `|` are sent so they come out as on the US layout

## From change to flash

1. Edit `keymaps/am338` and push to `main`
2. GitHub Actions builds it; the `firmware` branch updates within a few minutes
3. Open the flash page above, press "Flash", then press the reset button on the keyboard (once for each half)

## License

`keymaps/am338` is based on the Keyball firmware (GPL-2.0-or-later).
