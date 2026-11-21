# keyballer-config

[Keyballer](https://github.com/AkiraMisawa/keyballer) 用のキーボード設定です。

**書き込み：** https://akiramisawa.github.io/keyballer/?repo=AkiraMisawa/keyballer-config

| ファームウェア | 機種 | キーマップ |
| --- | --- | --- |
| Keyball+ | `keyball/keyballplus` | `keymaps/am338` |
| Keyball39 | `keyball/keyball39` | `keymaps/am338`（Keyball+ と共通） |

## キーマップ am338

roBa（ZMK）のキーマップを移植したもので、Mac / Windows を自動で判定して切り替えます。

- レイヤーは4枚：ベース、記号（Tab 長押し）、数字・カーソル（Space / Enter 長押し）、ファンクション（K・'・Esc 長押し。この間ボールはスクロール）
- ベースの `,` `.` の位置が左右クリックで、`,` `.` `\` はコンボで入力
- Mac：Q 長押しで Cmd、左下2・3番目はタップで英数／かな・長押しで Option、ボールのスクロールは反転
- L3 のスクリーンショットキー：Mac は Shift+Cmd+4 / Shift+Cmd+3、Windows は Win+Shift+S

## 変更から書き込みまで

1. `keymaps/am338` を編集して `main` に push する
2. GitHub Actions がビルドし、数分で `firmware` ブランチに反映される
3. 上の書き込みページを開き、「書き込む」を押してキーボードのリセットボタンを押す（左右それぞれ）

## ライセンス

`keymaps/am338` は Keyball のファームウェア（GPL-2.0-or-later）をもとにしています。
