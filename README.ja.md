# keyballer-config

[English](README.md) | 日本語

[Keyballer](https://github.com/AkiraMisawa/keyballer) 用のキーボード設定です。

**書き込み：** https://akiramisawa.github.io/keyballer/?repo=AkiraMisawa/keyballer-config&lang=ja

| ファームウェア | 機種 | キーマップ |
| --- | --- | --- |
| Keyball+ | `keyball/keyballplus` | `keymaps/am338` |
| Keyball39 | `keyball/keyball39` | `keymaps/am338`（Keyball+ と共通） |

## キーマップ am338

roBa（ZMK）のキーマップを移植したもので、Mac / Windows を自動で判定して切り替えます。判定が 0.25 秒変わらなかったときに切り替え、OLED にはキーが従っている OS を表示します（決まるまでは `?`。Linux などは Windows と同じ動き）。

- レイヤーは4枚：ベース、記号（Tab 長押し）、数字・カーソル（Space / Enter 長押し）、ファンクション（K・'・Esc 長押し。この間ボールはスクロール）
- ベースの `,` `.` の位置が左右クリックで、`,` `.` `\` はコンボで入力
- Mac：Q 長押しで Cmd、左下2・3番目はタップで英数／かな・長押しで Option、ボールのスクロールは反転
- L3 のスクリーンショットキー：Mac は Shift+Cmd+4 / Shift+Cmd+3、Windows は Win+Shift+S
- UK モード：L3 の右下のキーで切り替え（電源を入れたときはオフ。オンの間は OLED に「UK」と出ます）。キーボード配列が UK（Windows の「英国」、Linux の「gb」）のコンピュータ向けで、`@` `"` `#` `~` `\` `|` を US 配列と同じ文字が出るように送ります

## 変更から書き込みまで

1. `keymaps/am338` を編集して `main` に push する
2. GitHub Actions がビルドし、数分で `firmware` ブランチに反映される
3. 上の書き込みページを開き、「書き込む」を押してキーボードのリセットボタンを押す（左右それぞれ）

## ライセンス

`keymaps/am338` は Keyball のファームウェア（GPL-2.0-or-later）をもとにしています。
