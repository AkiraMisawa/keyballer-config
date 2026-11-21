/*
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

// am338: the roBa keymap (AkiraMisawa/zmk-config-roBa, config/roBa.keymap on
// main) ported to Keyball39 / Keyball+.
//
//   L0 base      = roBa default_layer
//   L1 symbol    = roBa SYMBOL
//   L2 number    = roBa NUM
//   L3 function  = roBa FUNCTION (roBa layer 5); the trackball scrolls here
//
// Keyball39 has no inner column, so roBa's inner-column keys are dropped.

#include QMK_KEYBOARD_H
#include "quantum.h"
#include "os_detection.h"

enum custom_keycodes
{
  SCRSHOT_AREA = KEYBALL_SAFE_RANGE, // Win: Win+Shift+S, Mac: Shift+Cmd+4
  SCRSHOT_FULL,                      // Win: nothing,     Mac: Shift+Cmd+3
};

// Left thumb row, keys 2 and 3. Mac: tap Eisu / Kana, hold Option.
// Windows: plain Ctrl / Alt (see process_record_user and get_tapping_term).
#define TH_EISU LALT_T(KC_LNG2)
#define TH_KANA LALT_T(KC_LNG1)

// Set once the host OS is detected. Windows layout unless a Mac is found.
static bool is_mac = false;

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // base
  [0] = LAYOUT_universal(
    ALT_T(KC_Q) , KC_W    , KC_E    , KC_R          , KC_T          ,                                KC_Y    , KC_U          , KC_I        , KC_O    , KC_P            ,
    CTL_T(KC_A) , KC_S    , KC_D    , KC_F          , KC_G          ,                                KC_H    , KC_J          , LT(3, KC_K) , KC_L    , LT(3, KC_QUOT)  ,
    SFT_T(KC_Z) , KC_X    , KC_C    , KC_V          , KC_B          ,                                KC_N    , KC_M          , KC_BTN1     , KC_BTN2 , RSFT_T(KC_SLSH) ,
    KC_LGUI     , TH_EISU , TH_KANA , LT(1, KC_TAB) , LT(2, KC_SPC) , LT(3, KC_ESC) ,      KC_BSPC , LT(2, KC_ENT) , _______ , _______     , _______ , RCTL_T(KC_MINS)
  ),

  // symbol
  [1] = LAYOUT_universal(
    KC_PERC     , KC_DLR  , KC_AMPR , KC_CIRC , KC_SLSH ,                         KC_PIPE , KC_LPRN , KC_RPRN , KC_LT   , KC_GT   ,
    KC_AT       , KC_HASH , KC_ASTR , KC_PLUS , KC_EQL  ,                         KC_UNDS , KC_LCBR , KC_RCBR , KC_SCLN , KC_COLN ,
    KC_GRV      , KC_TILD , KC_HASH , KC_EXLM , KC_QUES ,                         KC_BSLS , KC_LBRC , KC_RBRC , KC_QUOT , KC_DQUO ,
    _______     , _______ , _______ , _______ , _______ , KC_ESC  ,     KC_DEL  , KC_ENT  , _______ , _______ , _______ , KC_ESC
  ),

  // number
  [2] = LAYOUT_universal(
    C(KC_W)     , KC_7    , KC_8    , KC_9    , KC_PLUS ,                         C(KC_PGUP) , KC_HOME , KC_UP   , KC_END  , C(KC_PGDN) ,
    KC_LCTL     , KC_4    , KC_5    , KC_6    , KC_0    ,                         KC_PGUP    , KC_LEFT , KC_DOWN , KC_RGHT , KC_PGDN    ,
    KC_LSFT     , KC_1    , KC_2    , KC_3    , KC_DOT  ,                         KC_INS     , KC_BTN4 , KC_BTN1 , KC_APP  , KC_BTN5    ,
    _______     , _______ , _______ , _______ , _______ , KC_ESC  ,     KC_DEL  , KC_ENT     , _______ , _______ , _______ , KC_ESC
  ),

  // function (the trackball scrolls while this layer is active)
  [3] = LAYOUT_universal(
    KC_LALT     , KC_F7   , KC_F8       , KC_F9   , KC_F10  ,                     KC_F9     , KC_F10   , KC_F11 , KC_F12   , A(KC_F4) ,
    KC_LCTL     , KC_F4   , KC_F5       , KC_F6   , KC_F11  ,                     CPI_D100  , SCRL_DVI , KC_F5  , SCRL_DVD , CPI_I100 ,
    SCRSHOT_FULL, KC_F1   , KC_F2       , KC_F3   , KC_F12  ,                  SCRSHOT_AREA , KC_BTN4  , KC_F2  , KC_APP   , KC_BTN5  ,
    _______     , _______ , LCA(KC_DEL) , _______ , _______ , KC_ESC  ,   KC_DEL , KC_ENT    , _______  , _______ , _______  , _______
  ),
};
// clang-format on

#ifdef COMBO_ENABLE
// roBa's right-hand combos. COMBO_ONLY_FROM_LAYER makes them position based.
const uint16_t PROGMEM combo_comma[] = {KC_M, KC_BTN1, COMBO_END};
const uint16_t PROGMEM combo_dot[] = {KC_BTN1, KC_BTN2, COMBO_END};
const uint16_t PROGMEM combo_bsls[] = {KC_BTN2, RSFT_T(KC_SLSH), COMBO_END};

combo_t key_combos[] = {
    COMBO(combo_comma, KC_COMM),
    COMBO(combo_dot, KC_DOT),
    COMBO(combo_bsls, KC_BSLS),
};
#endif

// roBa's &mt uses flavor "balanced", which is QMK's permissive hold.
bool get_permissive_hold(uint16_t keycode, keyrecord_t *record)
{
  return IS_QK_MOD_TAP(keycode);
}

// roBa's Tab / ' / Esc layer keys are "hold-preferred" hold-taps.
bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record)
{
  switch (keycode)
  {
  case LT(1, KC_TAB):
  case LT(3, KC_QUOT):
  case LT(3, KC_ESC):
    return true;
  default:
    return false;
  }
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record)
{
  // On Windows the Eisu / Kana keys are plain Ctrl / Alt, so make them hold
  // right away. Ctrl + ball scrolling then works without waiting.
  if (!is_mac && (keycode == TH_EISU || keycode == TH_KANA))
  {
    return 0;
  }
  return TAPPING_TERM;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record)
{
  // Remember what was pressed so the release matches even if the OS changes.
  static bool q_hold_is_cmd;

  switch (keycode)
  {
  case ALT_T(KC_Q):
  {
    // Holding Q gives Command on Mac instead of Option. Taps are left to QMK.
    if (record->tap.count == 0)
    {
      if (record->event.pressed)
      {
        q_hold_is_cmd = is_mac;
      }
      if (q_hold_is_cmd)
      {
        if (record->event.pressed)
        {
          register_code(KC_LGUI);
        }
        else
        {
          unregister_code(KC_LGUI);
        }
        return false;
      }
    }
    return true;
  }

  case TH_EISU:
  case TH_KANA:
  {
    if (is_mac)
    {
      return true;
    }
    // Windows: no IME keys on tap, and the Eisu key holds Ctrl instead of Alt.
    if (record->tap.count)
    {
      return false;
    }
    if (keycode == TH_EISU)
    {
      if (record->event.pressed)
      {
        register_code(KC_LCTL);
      }
      else
      {
        unregister_code(KC_LCTL);
      }
      return false;
    }
    return true;
  }

  case SCRSHOT_AREA:
  {
    if (record->event.pressed)
    {
      tap_code16(is_mac ? LSG(KC_4) : LSG(KC_S));
    }
    return false;
  }

  case SCRSHOT_FULL:
  {
    if (record->event.pressed && is_mac)
    {
      tap_code16(LSG(KC_3));
    }
    return false;
  }
  }
  return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report)
{
  // The keyball driver has already turned ball motion into h/v here.
  if (is_mac)
  {
    mouse_report.h = -mouse_report.h;
    mouse_report.v = -mouse_report.v;
  }
  return mouse_report;
}

layer_state_t layer_state_set_user(layer_state_t state)
{
  keyball_set_scroll_mode(get_highest_layer(state) == 3);

  return state;
}

#if defined(OS_DETECTION_ENABLE) && defined(DEFERRED_EXEC_ENABLE)
static uint32_t os_detect_callback(uint32_t trigger_time, void *cb_arg)
{
  os_variant_t os = detected_host_os();
  is_mac = (os == OS_MACOS || os == OS_IOS);
  return 0;
}

void keyboard_post_init_user(void)
{
  defer_exec(100, os_detect_callback, NULL);
}
#endif

#ifdef OLED_ENABLE

#include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void)
{
  keyball_oled_render_keyinfo();
  keyball_oled_render_ballinfo();

  oled_write_P(PSTR("OS:"), false);
  switch (detected_host_os())
  {
  case OS_WINDOWS:
  {
    oled_write_P(PSTR(" Win  "), false);
    break;
  }
  case OS_MACOS:
  {
    oled_write_P(PSTR(" Mac  "), false);
    break;
  }
  default:
    oled_write_P(PSTR("NA  "), false);
    break;
  }
  oled_write_P(PSTR("Layer:"), false);
  oled_write(get_u8_str(get_highest_layer(layer_state), ' '), false);
}

#endif
