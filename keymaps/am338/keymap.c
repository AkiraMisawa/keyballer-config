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
//
// UK mode (UK_TOGG on L3): for a computer set to the UK keyboard layout
// (Windows "United Kingdom", Linux "gb"). The keymap is written for the US
// layout; in UK mode the six symbols that sit elsewhere on UK, @ " # ~ \ |,
// are sent as the UK keys that type them. Off at power-up.
//
// JIS mode (JIS_TOGG on L3): for a computer that reads this keyboard as a
// Japanese (JIS) one. Every symbol that sits elsewhere on JIS is sent as the
// JIS key that types it, so the keys type what the US legends say. On by
// itself on a Mac, off elsewhere; JIS_TOGG switches it until unplugged. UK and
// JIS mode are never on together.

#include QMK_KEYBOARD_H
#include "quantum.h"
#include "os_detection.h"
#include "usb_device_state.h"

enum custom_keycodes
{
  SCRSHOT_AREA = KEYBALL_SAFE_RANGE, // Win: Win+Shift+S, Mac: Shift+Cmd+4
  SCRSHOT_FULL,                      // Win: nothing,     Mac: Shift+Cmd+3
  UK_TOGG,                           // UK mode on / off
  JIS_TOGG,                          // JIS mode on / off
};

// Left thumb row, keys 2 and 3. Mac: tap Eisu / Kana, hold Option.
// Windows: plain Ctrl / Alt (see process_record_user and get_tapping_term).
#define TH_EISU LALT_T(KC_LNG2)
#define TH_KANA LALT_T(KC_LNG1)

// Layers where the trackball scrolls instead of moving the cursor, as bits
// (1 << layer). Keyballer's keymap editor reads and changes this line.
#define KEYBALLER_SCROLL_LAYERS (1 << 3)

// The host OS the keys follow: Windows behaviour unless a Mac is found.
// Set by os_watch below once QMK's guess has settled.
static os_variant_t host_os = OS_UNSURE;
static bool is_mac = false;

static bool uk_mode = false;

// What JIS mode sends for \ on a Mac: Option+Yen by default. A terminal that
// takes Option as Meta (iTerm2, VS Code…) gets Esc and Yen from that instead;
// set the Japanese input source's "¥キーで入力する文字" to \ and define
// JIS_MAC_BACKSLASH KC_INT3 in config.h to send the Yen key alone.
#ifndef JIS_MAC_BACKSLASH
#  define JIS_MAC_BACKSLASH A(KC_INT3)
#endif

// JIS mode: -1 follows the host (on for a Mac), 0 off, 1 on (set by JIS_TOGG).
static int8_t jis_choice = -1;
static bool jis_mode(void)
{
  return jis_choice < 0 ? is_mac : jis_choice == 1;
}

// The JIS key, with Shift or Option, that types the character a US key types
// (`key`, with Shift when `shift`); 0 when it is the same key on JIS.
static uint16_t jis_key_for(uint8_t key, bool shift)
{
  switch (key)
  {
  case KC_2:
    return shift ? KC_LBRC : 0; // @
  case KC_6:
    return shift ? KC_EQL : 0; // ^
  case KC_7:
    return shift ? S(KC_6) : 0; // &
  case KC_8:
    return shift ? S(KC_QUOT) : 0; // *
  case KC_9:
    return shift ? S(KC_8) : 0; // (
  case KC_0:
    return shift ? S(KC_9) : 0; // )
  case KC_MINS:
    return shift ? S(KC_INT1) : 0; // _
  case KC_EQL:
    return shift ? S(KC_SCLN) : S(KC_MINS); // + =
  case KC_LBRC:
    return shift ? S(KC_RBRC) : KC_RBRC; // { [
  case KC_RBRC:
    return shift ? S(KC_NUHS) : KC_NUHS; // } ]
  case KC_BSLS:
    // | is Shift+Yen. \ is Option+Yen on a Mac (Yen alone types the yen
    // sign there), and the Yen key itself on Windows.
    return shift ? S(KC_INT3) : (is_mac ? JIS_MAC_BACKSLASH : KC_INT3);
  case KC_SCLN:
    return shift ? KC_QUOT : 0; // :
  case KC_QUOT:
    return shift ? S(KC_2) : S(KC_7); // " '
  case KC_GRV:
    return shift ? S(KC_EQL) : S(KC_LBRC); // ~ `
  default:
    return 0;
  }
}

// Types a symbol the JIS way while JIS mode is on: on the press, the JIS key
// with exactly the Shift it needs (whatever Shift is held), and nothing on the
// release. Ctrl and GUI stay held, as hosts read shortcuts such as Cmd+[ and
// Ctrl+[ through the JIS layout too. With Alt / Option held the key is sent as
// it is: Option makes other characters on a Mac. Translated keys do not repeat
// while held.
static bool process_jis_symbol(uint16_t keycode, keyrecord_t *record)
{
  static uint16_t sent_as_jis[4];
  uint8_t key;
  bool shift_in_key = false;

  if (IS_QK_BASIC(keycode))
  {
    key = keycode;
  }
  else if (IS_QK_MODS(keycode) && QK_MODS_GET_MODS(keycode) == MOD_LSFT)
  {
    key = QK_MODS_GET_BASIC_KEYCODE(keycode);
    shift_in_key = true;
  }
  else if ((IS_QK_LAYER_TAP(keycode) || IS_QK_MOD_TAP(keycode)) && record->tap.count > 0)
  {
    key = keycode & 0xFF; // the tap of a layer-tap or mod-tap
  }
  else
  {
    return true;
  }

  if (!record->event.pressed)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      if (sent_as_jis[i] == keycode)
      {
        sent_as_jis[i] = KC_NO;
        return false; // typed on the press
      }
    }
    return true;
  }

  const uint8_t mods = get_mods();
  const uint8_t oneshot = get_oneshot_mods();
  if (!jis_mode() || ((mods | oneshot) & MOD_MASK_ALT))
  {
    return true;
  }
  const bool shift = shift_in_key || ((mods | oneshot) & MOD_MASK_SHIFT);
  const uint16_t jis = jis_key_for(key, shift);
  if (jis == 0)
  {
    return true;
  }
  for (uint8_t i = 0; i < 4; i++)
  {
    if (sent_as_jis[i] == KC_NO)
    {
      sent_as_jis[i] = keycode;
      break;
    }
  }
  del_mods(MOD_MASK_SHIFT);
  if (oneshot & MOD_MASK_SHIFT)
  {
    del_oneshot_mods(MOD_MASK_SHIFT);
  }
  tap_code16(jis);
  // Shift held goes back on at once: the host saw it released for this key.
  set_mods(mods);
  send_keyboard_report();
  return false;
}

// The UK layout key that types a US symbol, or KC_NO. Every other symbol in
// this keymap is on the same key in both layouts.
static uint16_t uk_key_for(uint16_t keycode)
{
  switch (keycode)
  {
  case KC_AT:
    return S(KC_QUOT);
  case KC_DQUO:
    return S(KC_2);
  case KC_HASH:
    return KC_NUHS;
  case KC_TILD:
    return S(KC_NUHS);
  case KC_BSLS: // also the backslash combo on the right hand
    return KC_NUBS;
  case KC_PIPE:
    return S(KC_NUBS);
  default:
    return KC_NO;
  }
}

// Sends a symbol the UK way while UK mode is on. Remembers which presses went
// out that way, so each release matches its press even if the mode changes
// in between.
static bool process_uk_symbol(uint16_t keycode, keyrecord_t *record)
{
  static uint16_t sent_as_uk[4];
  uint16_t uk = uk_key_for(keycode);

  if (uk == KC_NO)
  {
    return true;
  }
  if (record->event.pressed)
  {
    if (!uk_mode)
    {
      return true;
    }
    for (uint8_t i = 0; i < 4; i++)
    {
      if (sent_as_uk[i] == KC_NO)
      {
        sent_as_uk[i] = keycode;
        register_code16(uk);
        return false;
      }
    }
    tap_code16(uk); // more than four held at once: type it once
    return false;
  }
  for (uint8_t i = 0; i < 4; i++)
  {
    if (sent_as_uk[i] == keycode)
    {
      sent_as_uk[i] = KC_NO;
      unregister_code16(uk);
      return false;
    }
  }
  return true;
}

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
    _______     , _______ , LCA(KC_DEL) , _______ , _______ , KC_ESC  ,   KC_DEL , KC_ENT    , _______  , _______ , JIS_TOGG , UK_TOGG
  ),

  [4] = LAYOUT_universal(
    _______     , _______ , _______     , _______ , _______ ,                     _______   , _______  , _______ , _______  , _______  ,
    _______     , _______ , _______     , _______ , _______ ,                     _______   , _______  , RGB_TOG , _______  , _______  ,
    _______     , _______ , _______     , _______ , _______ ,                  _______      , _______  , _______ , _______  , _______  ,
    _______     , _______ , _______     , _______ , _______ , _______ ,   _______ , _______   , _______  , _______ , _______  , _______
  ),

  [5] = LAYOUT_universal(
    _______     , _______ , _______     , _______ , _______ ,                     _______   , UK_TOGG  , _______ , _______  , _______  ,
    _______     , _______ , _______     , _______ , _______ ,                     _______   , JIS_TOGG , _______ , _______  , _______  ,
    _______     , _______ , _______     , _______ , _______ ,                  _______      , _______  , _______ , _______  , _______  ,
    _______     , _______ , _______     , _______ , _______ , _______ ,   _______ , _______   , _______  , _______ , _______  , _______
  ),
};
// clang-format on

#ifdef COMBO_ENABLE
// roBa's right-hand combos. COMBO_ONLY_FROM_LAYER makes them position based.
const uint16_t PROGMEM combo_comma[] = {KC_M, KC_BTN1, COMBO_END};
const uint16_t PROGMEM combo_dot[] = {KC_BTN1, KC_BTN2, COMBO_END};
const uint16_t PROGMEM combo_bsls[] = {KC_BTN2, RSFT_T(KC_SLSH), COMBO_END};
const uint16_t PROGMEM combo_4[] = {KC_D, KC_F, COMBO_END};
const uint16_t PROGMEM combo_5[] = {KC_F, KC_G, COMBO_END};

combo_t key_combos[] = {
    COMBO(combo_comma, KC_COMM),
    COMBO(combo_dot, KC_DOT),
    COMBO(combo_bsls, KC_BSLS),
    COMBO(combo_4, MO(4)),
    COMBO(combo_5, MO(5)),
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

  if (!process_jis_symbol(keycode, record) || !process_uk_symbol(keycode, record))
  {
    return false;
  }

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

  case UK_TOGG:
  {
    if (record->event.pressed)
    {
      uk_mode = !uk_mode;
      // JIS mode goes off with UK mode on, and follows the host again after.
      jis_choice = uk_mode ? 0 : -1;
    }
    return false;
  }

  case JIS_TOGG:
  {
    if (record->event.pressed)
    {
      jis_choice = jis_mode() ? 0 : 1;
      if (jis_choice == 1)
      {
        uk_mode = false;
      }
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
  keyball_set_scroll_mode((KEYBALLER_SCROLL_LAYERS & (1UL << get_highest_layer(state))) != 0);

  return state;
}

#if defined(OS_DETECTION_ENABLE) && defined(DEFERRED_EXEC_ENABLE)
// QMK 0.22 rewrites its OS guess with every USB setup packet, so a guess read
// once, early, can be half-made. Like newer QMK (quantum/os_detection.c,
// OS_DETECTION_DEBOUNCE), follow the guess only while the host has the
// keyboard configured and once neither the guess nor the USB state has
// changed for OS_STABLE_MS. Keep watching, so a later, better guess is taken.
#  define OS_POLL_MS 50
#  define OS_STABLE_MS 250

static bool usb_configured = false;
static uint16_t usb_changed_at;

void notify_usb_device_state_change_user(enum usb_device_state usb_device_state)
{
  usb_configured = (usb_device_state == USB_DEVICE_STATE_CONFIGURED);
  usb_changed_at = timer_read();
}

static uint32_t os_watch(uint32_t trigger_time, void *cb_arg)
{
  static os_variant_t seen = OS_UNSURE;
  static uint16_t seen_since;
  os_variant_t os = detected_host_os();

  if (os != seen)
  {
    seen = os;
    seen_since = timer_read();
  }
  else if (usb_configured && os != OS_UNSURE && os != host_os &&
           timer_elapsed(seen_since) >= OS_STABLE_MS && timer_elapsed(usb_changed_at) >= OS_STABLE_MS)
  {
    host_os = os;
    is_mac = (os == OS_MACOS || os == OS_IOS);
  }
  return OS_POLL_MS;
}

#endif

// Settings made in Keyballer win after flashing: the trackball speeds
// (KEYBALL_CPI_DEFAULT, KEYBALL_SCROLL_DIV_DEFAULT) and the LED settings
// (RGBLIGHT_DEFAULT_*, KEYBALLER_LEDS_OFF) in config.h. Keyball and QMK keep
// values changed on the keyboard in EEPROM (KBC_SAVE, the LED keys), and
// those would win over new defaults. So a stamp of each group of defaults is
// kept in the user EEPROM data block (EECONFIG_USER_DATA_SIZE in config.h):
// when a build brings different defaults, that group's saved values are reset
// to them once. Values changed on the keyboard afterwards stay until the next
// build with different defaults. A board that has never had a stamp, or whose
// stamp was in the user EEPROM word (before the LED settings), is reset once.
typedef struct
{
  uint32_t trackball;
  uint32_t leds;
  // Which effects are compiled in: RGB Lighting numbers its modes by them, so
  // the same number means another effect, or a mode saved on the keyboard
  // another variant, when they change.
  uint32_t led_effects;
} keyballer_stamps_t;

_Static_assert(sizeof(keyballer_stamps_t) == EECONFIG_USER_DATA_SIZE, "EECONFIG_USER_DATA_SIZE must hold the stamps");

#define KEYBALLER_DEFAULTS_STAMP \
  (0x4B420000UL | ((uint32_t)(KEYBALL_CPI_DEFAULT / 100) << 8) | (uint32_t)KEYBALL_SCROLL_DIV_DEFAULT)

#ifdef RGBLIGHT_ENABLE
// QMK's own defaults (quantum/rgblight/rgblight.c) where config.h sets none.
#  ifndef RGBLIGHT_DEFAULT_MODE
#    define RGBLIGHT_DEFAULT_MODE RGBLIGHT_MODE_STATIC_LIGHT
#  endif
#  ifndef RGBLIGHT_DEFAULT_HUE
#    define RGBLIGHT_DEFAULT_HUE 0
#  endif
#  ifndef RGBLIGHT_DEFAULT_SAT
#    define RGBLIGHT_DEFAULT_SAT UINT8_MAX
#  endif
#  ifndef RGBLIGHT_DEFAULT_VAL
#    define RGBLIGHT_DEFAULT_VAL RGBLIGHT_LIMIT_VAL
#  endif
#  ifdef KEYBALLER_LEDS_OFF
#    define KEYBALLER_LEDS_ON_AT_START 0UL
#  else
#    define KEYBALLER_LEDS_ON_AT_START 1UL
#  endif
// Mode numbers stay below 64, so bit 7 of the first byte says whether the
// LEDs start on.
#  define KEYBALLER_LED_STAMP                                                         \
  ((uint32_t)(RGBLIGHT_DEFAULT_MODE) | (KEYBALLER_LEDS_ON_AT_START << 7) |            \
   ((uint32_t)(RGBLIGHT_DEFAULT_HUE) << 8) | ((uint32_t)(RGBLIGHT_DEFAULT_SAT) << 16) | \
   ((uint32_t)(RGBLIGHT_DEFAULT_VAL) << 24))
// The effects that number RGB Lighting's modes (quantum/rgblight/rgblight_modes.h).
#  ifdef RGBLIGHT_EFFECT_BREATHING
#    define KEYBALLER_FX_0 (1UL << 0)
#  else
#    define KEYBALLER_FX_0 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_RAINBOW_MOOD
#    define KEYBALLER_FX_1 (1UL << 1)
#  else
#    define KEYBALLER_FX_1 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_RAINBOW_SWIRL
#    define KEYBALLER_FX_2 (1UL << 2)
#  else
#    define KEYBALLER_FX_2 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_SNAKE
#    define KEYBALLER_FX_3 (1UL << 3)
#  else
#    define KEYBALLER_FX_3 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_KNIGHT
#    define KEYBALLER_FX_4 (1UL << 4)
#  else
#    define KEYBALLER_FX_4 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_CHRISTMAS
#    define KEYBALLER_FX_5 (1UL << 5)
#  else
#    define KEYBALLER_FX_5 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_STATIC_GRADIENT
#    define KEYBALLER_FX_6 (1UL << 6)
#  else
#    define KEYBALLER_FX_6 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_RGB_TEST
#    define KEYBALLER_FX_7 (1UL << 7)
#  else
#    define KEYBALLER_FX_7 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_ALTERNATING
#    define KEYBALLER_FX_8 (1UL << 8)
#  else
#    define KEYBALLER_FX_8 0UL
#  endif
#  ifdef RGBLIGHT_EFFECT_TWINKLE
#    define KEYBALLER_FX_9 (1UL << 9)
#  else
#    define KEYBALLER_FX_9 0UL
#  endif
// Never 0, so a zeroed block always differs.
#  define KEYBALLER_LED_EFFECTS_STAMP                                                  \
  (0x4C450000UL | KEYBALLER_FX_0 | KEYBALLER_FX_1 | KEYBALLER_FX_2 | KEYBALLER_FX_3 | \
   KEYBALLER_FX_4 | KEYBALLER_FX_5 | KEYBALLER_FX_6 | KEYBALLER_FX_7 | KEYBALLER_FX_8 | KEYBALLER_FX_9)
#endif

static void apply_new_defaults(void)
{
  keyballer_stamps_t stamps;
  eeconfig_read_user_datablock(&stamps);
  bool changed = false;

  if (stamps.trackball != KEYBALLER_DEFAULTS_STAMP)
  {
    keyball_config_t c = {.raw = eeconfig_read_kb()};
    c.cpi = 0; // 0 = KEYBALL_CPI_DEFAULT
    c.sdiv = 0; // 0 = KEYBALL_SCROLL_DIV_DEFAULT
    eeconfig_update_kb(c.raw);
    keyball_set_cpi(0);
    keyball_set_scroll_div(0);
    stamps.trackball = KEYBALLER_DEFAULTS_STAMP;
    changed = true;
  }

#ifdef RGBLIGHT_ENABLE
  if (stamps.leds != KEYBALLER_LED_STAMP || stamps.led_effects != KEYBALLER_LED_EFFECTS_STAMP)
  {
    // Writes the defaults to EEPROM (and to RAM), then shows them or turns
    // the LEDs off.
    eeconfig_update_rgblight_default();
#  ifdef KEYBALLER_LEDS_OFF
    rgblight_disable();
#  else
    rgblight_reload_from_eeprom();
#  endif
    stamps.leds = KEYBALLER_LED_STAMP;
    stamps.led_effects = KEYBALLER_LED_EFFECTS_STAMP;
    changed = true;
  }
#endif

  if (changed)
  {
    eeconfig_update_user_datablock(&stamps);
  }
}

void keyboard_post_init_user(void)
{
  // Keyball has read its saved speeds, and QMK its LED settings, just before
  // this runs.
  apply_new_defaults();
#if defined(OS_DETECTION_ENABLE) && defined(DEFERRED_EXEC_ENABLE)
  defer_exec(OS_POLL_MS, os_watch, NULL);
#endif
}

#ifdef OLED_ENABLE

#include "lib/oledkit/oledkit.h"

void oledkit_render_info_user(void)
{
  keyball_oled_render_keyinfo();
  keyball_oled_render_ballinfo();

  // The OS the keys follow now, not QMK's raw guess.
  oled_write_P(PSTR("OS:"), false);
  switch (host_os)
  {
  case OS_WINDOWS:
    oled_write_P(PSTR(" Win"), false);
    break;
  case OS_MACOS:
    oled_write_P(PSTR(" Mac"), false);
    break;
  case OS_IOS:
    oled_write_P(PSTR(" iOS"), false);
    break;
  case OS_LINUX:
    oled_write_P(PSTR(" Lnx"), false);
    break;
  default:
    oled_write_P(PSTR(" ?  "), false);
    break;
  }
  oled_write_P(uk_mode ? PSTR(" UK  ") : jis_mode() ? PSTR(" JIS ") : PSTR("     "), false);
  oled_write_P(PSTR("Layer:"), false);
  oled_write(get_u8_str(get_highest_layer(layer_state), ' '), false);
}

#endif
