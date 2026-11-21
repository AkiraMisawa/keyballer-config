/*
This is the c configuration file for the keymap

Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#define TAP_CODE_DELAY 5

#define KEYBALL_CPI_DEFAULT 700
#define KEYBALL_SCROLL_DIV_DEFAULT 4

// Tap-hold behaviour matching roBa (ZMK): 200ms tapping term, no quick-tap
// auto-repeat, and per-key flavors set in keymap.c.
#define TAPPING_TERM 200
#define TAPPING_TERM_PER_KEY
#define QUICK_TAP_TERM 0
#define PERMISSIVE_HOLD_PER_KEY
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY

// Look up combo keys on the base layer only, so combos work by position
// on every layer like ZMK combos.
#define COMBO_ONLY_FROM_LAYER 0

// Yowkees/keyball's lib/keyball/keyball.h maps PRODUCT_ID to KEYBALL_MODEL
// but has no entry for Keyball+ (0x0500), which stops the build with
// "unknown Keyball model". Keyball+ is derived from Keyball39 and uses the
// same sensor orientation.
#if defined(KEYBOARD_keyball_keyballplus) && !defined(KEYBALL_MODEL)
#    define KEYBALL_MODEL 39
#endif
