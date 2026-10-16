#include QMK_KEYBOARD_H
#include "hardware/watchdog.h"
#include "usb_util.h"

// Speed level definitions
#define MOUSE_MOVE_DEFAULT 16
#define MOUSE_WHEEL_DEFAULT 6

#define MOUSE_MOVE_SLOW 8
#define MOUSE_WHEEL_SLOW 2

#define MOUSE_MOVE_PRECISE 4
#define MOUSE_WHEEL_PRECISE 1

// Cursor movement intervals
#define CURSOR_INTERVAL_DEFAULT 8
#define CURSOR_INTERVAL_SLOW 12
#define CURSOR_INTERVAL_PRECISE 16

// Scroll intervals
#define SCROLL_INTERVAL_DEFAULT 32
#define SCROLL_INTERVAL_SLOW 64
#define SCROLL_INTERVAL_PRECISE 100

// Zoom uses single wheel steps at a higher cadence so Ctrl+scroll feels
// smoother.
#define ZOOM_WHEEL_DEFAULT 1
#define ZOOM_WHEEL_SLOW 1
#define ZOOM_WHEEL_PRECISE 1

#define ZOOM_INTERVAL_DEFAULT 24
#define ZOOM_INTERVAL_SLOW 48
#define ZOOM_INTERVAL_PRECISE 72

// Define layer names
enum layer_names { _COLEMAK = 0, _TL, _TR, _TLTR };

// State variables
static bool mouse_slow_mode = false;
static bool mouse_precise_mode = false;
static bool mouse_scroll_mode = false;
static bool scroll_direction_reversed = false;

static bool mouse_up_pressed = false;
static bool mouse_down_pressed = false;
static bool mouse_left_pressed = false;
static bool mouse_right_pressed = false;
static bool zoom_in_pressed = false;
static bool zoom_out_pressed = false;

static uint8_t mouse_buttons = 0;
static uint16_t mouse_timer = 0;
static uint16_t boot_hold_timer = 0;

static uint8_t mbtn1_oneshot_mods = 0;
static uint8_t mbtn2_oneshot_mods = 0;

static inline int8_t get_mouse_speed(void);
static inline int8_t get_wheel_speed(void);
static inline int8_t get_zoom_wheel_speed(void);
static inline uint16_t get_mouse_interval(void);
static inline uint16_t get_zoom_interval(void);
static inline void update_zoom_ctrl(void);
static void fire_due_holds(void);

// Hardcoded split roles: the left half is always master, the right half always
// slave. SPLIT_USB_DETECT only waited 2s for enumeration at boot, which lost the
// race after macOS woke from Standby and left both halves acting as slave.
// Handedness falls back to is_keyboard_master(), so left/right follow too.
bool is_keyboard_master_impl(void) {
#ifdef TLTR_HALF_LEFT
  return true;
#else
  usb_disconnect();
  return false;
#endif
}

// Runs once the host OS has been fingerprinted. The master no longer waits for
// enumeration before init, so keyboard_post_init_user is too early to ask.
bool process_detected_host_os_user(os_variant_t detected_os) {
  switch (detected_os) {
  case OS_MACOS:
  case OS_IOS:
    set_unicode_input_mode(UNICODE_MODE_MACOS);
    break;
  case OS_WINDOWS:
    set_unicode_input_mode(UNICODE_MODE_WINCOMPOSE);
    break;
  case OS_LINUX:
    set_unicode_input_mode(UNICODE_MODE_LINUX);
    break;
  default:
    set_unicode_input_mode(UNICODE_MODE_LINUX);
    break;
  }
  return true;
}

// A mouse key held across suspend can miss its release, leaving the cursor
// drifting after wake. QMK clears keys and mods on wake but not this state.
void suspend_wakeup_init_user(void) {
  mouse_up_pressed = false;
  mouse_down_pressed = false;
  mouse_left_pressed = false;
  mouse_right_pressed = false;
  zoom_in_pressed = false;
  zoom_out_pressed = false;
  mouse_slow_mode = false;
  mouse_precise_mode = false;
  mouse_scroll_mode = false;
  mouse_buttons = 0;
  mbtn1_oneshot_mods = 0;
  mbtn2_oneshot_mods = 0;
}

void housekeeping_task_user(void) {
  fire_due_holds();

  if (!mouse_up_pressed && !mouse_down_pressed && !mouse_left_pressed &&
      !mouse_right_pressed && !zoom_in_pressed && !zoom_out_pressed) {
    return;
  }

  if (timer_elapsed(mouse_timer) < get_mouse_interval()) {
    return;
  }
  mouse_timer = timer_read();

  report_mouse_t mouse_report = {
      .buttons = mouse_buttons, .x = 0, .y = 0, .v = 0, .h = 0};

  // HID wheel: +v scrolls up, +h scrolls right. The toggle flips every wheel
  // direction, zoom included.
  int8_t wheel_dir = scroll_direction_reversed ? -1 : 1;

  if (zoom_in_pressed || zoom_out_pressed) {
    int8_t wheel_speed = wheel_dir * get_zoom_wheel_speed();
    if (zoom_in_pressed) {
      mouse_report.v = -wheel_speed;
    }
    if (zoom_out_pressed) {
      mouse_report.v = wheel_speed;
    }
  } else if (mouse_scroll_mode) {
    int8_t wheel_speed = wheel_dir * get_wheel_speed();
    if (mouse_up_pressed) {
      mouse_report.v = wheel_speed;
    }
    if (mouse_down_pressed) {
      mouse_report.v = -wheel_speed;
    }
    if (mouse_left_pressed) {
      mouse_report.h = -wheel_speed;
    }
    if (mouse_right_pressed) {
      mouse_report.h = wheel_speed;
    }
  } else {
    int8_t move_speed = get_mouse_speed();
    if (mouse_up_pressed) {
      mouse_report.y = -move_speed;
    }
    if (mouse_down_pressed) {
      mouse_report.y = move_speed;
    }
    if (mouse_left_pressed) {
      mouse_report.x = -move_speed;
    }
    if (mouse_right_pressed) {
      mouse_report.x = move_speed;
    }
  }

  host_mouse_send(&mouse_report);
}

enum custom_keycodes {
  // Fork keys (context-sensitive keys)
  KC_DOLF = SAFE_RANGE,              // Dollar/Rupee Fork
  KC_ASTF,              // Asterisk/F11 Fork
  KC_PERF,              // Percent/F12 Fork
  KC_HPNF,              // Hyphen/En dash Fork
  KC_EQLF,              // Equal/Em dash Fork

  // Function key forks (number/function based on Fn modifier)
  KC_1F,
  KC_2F,
  KC_3F,
  KC_4F,
  KC_5F,
  KC_6F,
  KC_7F,
  KC_8F,
  KC_9F,
  KC_0F,

  // Advanced thumb key combinations
  KC_TL_KEY,     // Left thumb key with TLTR logic
  KC_TR_KEY,     // Right thumb key with TLTR logic
  KC_TLTLTR_KEY, // TL+TLTR activation (used in TR layer)
  KC_TRTLTR_KEY, // TR+TLTR activation (used in TL layer)

  // One-shot modifier combinations
  KC_OS_HYP, // sHyp: Alt+Ctrl+Shift+Meta (hyper)
  KC_OS_FN,  // sFn: Function modifier

  // Individual modifiers with tap-hold behavior (tap=oneshot, hold=regular)
  KC_MOD_ALT,   // Alt modifier (tap for oneshot, hold for regular)
  KC_MOD_CTRL,  // Ctrl modifier (tap for oneshot, hold for regular)
  KC_MOD_SHIFT, // Shift modifier (tap for oneshot, hold for regular)
  KC_MOD_META,  // Meta/GUI modifier (tap for oneshot, hold for regular)

  // Mouse control modifiers
  KC_MSLW, // Mouse slow modifier
  KC_MPRE, // Mouse precise modifier
  KC_MSCR, // Mouse scroll modifier

  // Directional mouse keys with mode switching
  KC_MUP,  // Mouse/scroll up (combines with modifiers)
  KC_MDN,  // Mouse/scroll down
  KC_MLFT, // Mouse/scroll left
  KC_MRGT, // Mouse/scroll right

  // Mouse buttons
  KC_MBTN1, // Mouse button 1 (left click)
  KC_MBTN2, // Mouse button 2 (right click)

  KC_ZMIN,
  KC_ZMOUT,

  // Media/Screen controls
  KC_SCRE, // Screen control (tap=lock screen, hold=display off)
  KC_MEDC, // Media control (tap=play/pause, hold=next track)

  // Boot/Reboot control
  KC_BOOT_HOLD, // Boot control (tap=reboot, hold=bootloader)

  // Scroll direction toggle
  KC_SCRL_REV, // Toggle scroll direction (for natural scrolling compatibility)
};

enum tap_dance_codes { TD_RIGHT_PEDAL, TD_LEFT_PEDAL };

static uint16_t right_pedal_chord = KC_NO;
static uint16_t left_pedal_chord = KC_NO;

static void right_pedal_finished(tap_dance_state_t *state, void *user_data) {
  right_pedal_chord = state->count >= 2 ? C(S(KC_F13)) : C(KC_F13);
  register_code16(right_pedal_chord);
}

static void right_pedal_reset(tap_dance_state_t *state, void *user_data) {
  if (right_pedal_chord != KC_NO) {
    unregister_code16(right_pedal_chord);
    right_pedal_chord = KC_NO;
  }
}

static void left_pedal_finished(tap_dance_state_t *state, void *user_data) {
  left_pedal_chord = state->count >= 2 ? C(KC_F19) : KC_F19;
  register_code16(left_pedal_chord);
}

static void left_pedal_reset(tap_dance_state_t *state, void *user_data) {
  if (left_pedal_chord != KC_NO) {
    unregister_code16(left_pedal_chord);
    left_pedal_chord = KC_NO;
  }
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_RIGHT_PEDAL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, right_pedal_finished,
                                                    right_pedal_reset),
    [TD_LEFT_PEDAL] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, left_pedal_finished,
                                                   left_pedal_reset),
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // COLEMAK MOD-DH + WIDE + ANGLE (38-key TLTR layout)
    [_COLEMAK] = LAYOUT_split_2x6_1x5_2(
        TD(TD_RIGHT_PEDAL), KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,        KC_J,    KC_L,    KC_U,    KC_Y,    KC_QUOT, KC_SCLN,
        KC_BSPC, KC_A,    KC_R,    KC_S,    KC_T,    KC_G,        KC_M,    KC_N,    KC_E,    KC_I,    KC_O,    KC_ENT,
                 KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,        KC_K,    KC_H,    KC_COMM, KC_DOT,  KC_SLSH,
                                   KC_TL_KEY, KC_LSFT,            KC_SPC,  KC_TR_KEY
    ),

    // TL Layer - Modifiers & Navigation
    [_TL] = LAYOUT_split_2x6_1x5_2(
        KC_BOOT_HOLD, KC_ESC,      KC_F16,       KC_F17,       KC_F18,       KC_NO,       KC_PGUP,   S(KC_TAB), KC_UP,   KC_TAB,  KC_NO,   KC_NO,
        KC_TRNS,      KC_MOD_ALT,  KC_MOD_CTRL,  KC_MOD_SHIFT, KC_MOD_META, KC_OS_FN,    KC_PGDN,   KC_LEFT,   KC_DOWN, KC_RGHT, KC_NO,   KC_TRNS,
                      TD(TD_LEFT_PEDAL), KC_NO,        KC_NO,        KC_OS_HYP,   KC_NO,       KC_NO,     KC_BSPC,   KC_DEL,  KC_NO,   KC_TRNS,
                                                 KC_TRNS,      KC_TRNS,                  KC_TRNS,   KC_TRTLTR_KEY
    ),

    // TR Layer - Numbers & Symbols
    [_TR] = LAYOUT_split_2x6_1x5_2(
        KC_NO,       KC_EXLM, KC_AT,   KC_HASH, KC_DOLF, KC_NO,       KC_PERF, KC_7F,   KC_8F,   KC_9F,   KC_PLUS, KC_EQLF,
        KC_TRNS,     KC_AMPR, KC_LBRC, KC_LCBR, KC_LPRN, KC_NO,       KC_ASTF, KC_4F,   KC_5F,   KC_6F,   KC_HPNF, KC_TRNS,
                     KC_NO,   KC_NO,   KC_LT,   KC_GT,   KC_NO,       KC_0F,   KC_1F,   KC_2F,   KC_3F,   KC_SLSH,
                                       KC_TLTLTR_KEY, KC_TRNS,        KC_TRNS, KC_TRNS
    ),

    // TLTR Layer - Mouse, Media & Display Controls
    [_TLTR] = LAYOUT_split_2x6_1x5_2(
        KC_NO,   KC_SCRE, KC_BRIU, KC_MEDC,  KC_VOLU,  KC_NO,       KC_ZMIN, KC_NO,   KC_MUP,  KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_MPRE, KC_MSLW, KC_MSCR,  KC_MBTN1, KC_MBTN2,    KC_ZMOUT, KC_MLFT, KC_MDN,  KC_MRGT, KC_NO,   KC_NO,
                 KC_NO,   KC_BRID, KC_MPRV,  KC_VOLD,  KC_NO,       KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_SCRL_REV,
                                   KC_TRNS,  KC_TRNS,               KC_TRNS, KC_TRNS
    )
};
// clang-format on

// Shift alternates. Decided when the key goes down (pressing or releasing shift
// mid-hold doesn't switch it), and shift stays suppressed while it's held.
#define SHIFT_FORK(trigger, replacement)                                      \
  ko_make_with_layers_negmods_and_options(                                    \
      MOD_BIT(KC_LSFT), trigger, replacement, ~0, 0,                          \
      ko_option_activation_trigger_down | ko_option_no_reregister_trigger)

static const key_override_t delete_override = SHIFT_FORK(KC_BSPC, KC_DEL);
static const key_override_t backslash_override = SHIFT_FORK(KC_SLSH, KC_BSLS);
static const key_override_t underscore_override = SHIFT_FORK(KC_COMM, KC_UNDS);
static const key_override_t question_override = SHIFT_FORK(KC_DOT, KC_QUES);
static const key_override_t home_override = SHIFT_FORK(KC_PGUP, KC_HOME);
static const key_override_t end_override = SHIFT_FORK(KC_PGDN, KC_END);
static const key_override_t grave_override = SHIFT_FORK(KC_EXLM, KC_GRV);
static const key_override_t tilde_override = SHIFT_FORK(KC_AT, KC_TILD);
static const key_override_t caret_override = SHIFT_FORK(KC_HASH, KC_CIRC);
static const key_override_t pipe_override = SHIFT_FORK(KC_AMPR, KC_PIPE);
static const key_override_t rcbr_override = SHIFT_FORK(KC_LCBR, KC_RCBR);
static const key_override_t rprn_override = SHIFT_FORK(KC_LPRN, KC_RPRN);
static const key_override_t rbrc_override = SHIFT_FORK(KC_LBRC, KC_RBRC);

const key_override_t *key_overrides[] = {
    &delete_override,   &backslash_override, &underscore_override,
    &question_override, &home_override,      &end_override,
    &grave_override,    &tilde_override,     &caret_override,
    &pipe_override,     &rcbr_override,      &rprn_override,
    &rbrc_override,
};

// Advanced state tracking
static bool tl_pressed = false;
static bool tr_pressed = false;

static bool fn_modifier_active = false;
static bool fn_oneshot_active = false;


static struct {
  bool active;
  uint16_t timer;
  uint8_t mods;
} oneshot_state = {false, 0, 0};

static struct {
  bool os_hyp_held;
  bool os_fn_held;
  bool os_hyp_used;
  bool os_fn_used;
  uint16_t os_hyp_timer;
  uint16_t os_fn_timer;

  bool mod_alt_held;
  bool mod_ctrl_held;
  bool mod_shift_held;
  bool mod_meta_held;
  bool mod_alt_used;
  bool mod_ctrl_used;
  bool mod_shift_used;
  bool mod_meta_used;
  uint16_t mod_alt_timer;
  uint16_t mod_ctrl_timer;
  uint16_t mod_shift_timer;
  uint16_t mod_meta_timer;
} modifier_hold_state = {false, false, false, false, 0,     0,
                         false, false, false, false, false, false,
                         false, false, 0,     0,     0,     0};

static struct {
  bool held;
  bool used;
  uint16_t timer;
} screen_hold_state = {false, false, 0};

static struct {
  bool held;
  bool used;
  uint16_t timer;
} media_hold_state = {false, false, 0};

#define ONESHOT_TIMEOUT 300
#define TAPHOLD_TIMEOUT 200

static inline bool is_left_shift_active(void) {
  return (get_mods() & MOD_BIT(KC_LSFT)) ||
         (get_oneshot_mods() & MOD_BIT(KC_LSFT));
}

static inline bool is_shift_active(void) {
  return (get_mods() & MOD_MASK_SHIFT) || (get_oneshot_mods() & MOD_MASK_SHIFT);
}

// Modifiers, mouse speed/scroll modifiers, and layer keys. Pressing one doesn't
// count as using a held modifier and doesn't consume a one-shot Fn.
static bool is_modifier_like_key(uint16_t keycode) {
  switch (keycode) {
  case KC_OS_HYP:
  case KC_OS_FN:
  case KC_MOD_ALT:
  case KC_MOD_CTRL:
  case KC_MOD_SHIFT:
  case KC_MOD_META:
  case KC_MSLW:
  case KC_MPRE:
  case KC_MSCR:
  case KC_TL_KEY:
  case KC_TR_KEY:
  case KC_TLTLTR_KEY:
  case KC_TRTLTR_KEY:
    return true;
  default:
    return false;
  }
}

// Mouse, zoom, and the tap-hold screen/media keys: also don't mark a held
// modifier as used.
static bool is_pointer_or_hold_key(uint16_t keycode) {
  switch (keycode) {
  case KC_SCRE:
  case KC_MEDC:
  case KC_MUP:
  case KC_MDN:
  case KC_MLFT:
  case KC_MRGT:
  case KC_MBTN1:
  case KC_MBTN2:
  case KC_ZMIN:
  case KC_ZMOUT:
    return true;
  default:
    return false;
  }
}

static inline void clear_shift_mods(void) {
  uint8_t mods = get_mods();
  if (mods & MOD_MASK_SHIFT) {
    unregister_mods(MOD_MASK_SHIFT);
  }
  uint8_t oneshot_mods = get_oneshot_mods();
  if (oneshot_mods & MOD_MASK_SHIFT) {
    clear_oneshot_mods();
  }
}

// Caps Word sees the raw keymap keycode, so custom keys that type word
// characters, delete, or switch layers must be listed or they end the word.
bool caps_word_press_user(uint16_t keycode) {
  switch (keycode) {
  case KC_A ... KC_Z:
    add_weak_mods(MOD_BIT(KC_LSFT));
    return true;

  case KC_1 ... KC_0:
  case KC_MINS: // hyphen stays a hyphen; underscore is Shift+comma
  case KC_HPNF:
  case KC_BSPC:
  case KC_DEL:
  case KC_UNDS:
  case KC_UP:
  case KC_DOWN:
  case KC_LEFT:
  case KC_RGHT:
  case KC_LSFT:
  case KC_RSFT:
  case KC_TL_KEY:
  case KC_TR_KEY:
  case KC_TLTLTR_KEY:
  case KC_TRTLTR_KEY:
  case KC_MOD_SHIFT:
  case KC_1F ... KC_0F:
    return true;

  // Shifted it types an underscore, which continues the word; a comma ends it.
  case KC_COMM:
    return is_left_shift_active();

  default:
    return false;
  }
}

// Shift fork whose shifted side is a Unicode character. Not a key override:
// those keep shift suppressed in every report while active, which breaks the
// Linux Ctrl+Shift+U input sequence.
static bool handle_unicode_fork(keyrecord_t *record, uint16_t base_key,
                                const char *shifted) {
  if (record->event.pressed) {
    if (is_left_shift_active()) {
      uint8_t saved_mods = get_mods();
      clear_shift_mods();
      send_unicode_string(shifted);
      set_mods(saved_mods);
    } else {
      register_code16(base_key);
    }
  } else {
    unregister_code16(base_key);
  }
  return false;
}

// Tap locks the screen, hold turns the display off. Windows has no display-off
// shortcut and Linux no common one, so hold locks there as well.
static void lock_screen(void) {
  switch (detected_host_os()) {
  case OS_MACOS:
  case OS_IOS:
    tap_code16(LCTL(LGUI(KC_Q)));
    break;
  default: // Windows, and GNOME/KDE on Linux
    tap_code16(LGUI(KC_L));
    break;
  }
}

static void turn_display_off(void) {
  switch (detected_host_os()) {
  case OS_MACOS:
  case OS_IOS:
    tap_code16(LCTL(LSFT(KC_PWR)));
    break;
  default:
    lock_screen();
    break;
  }
}

static bool handle_fn_fork(keyrecord_t *record, uint16_t number_key,
                           uint16_t function_key) {
  if (record->event.pressed) {
    if (fn_modifier_active) {
      tap_code(function_key);
    } else {
      tap_code(number_key);
    }
  }
  return false;
}

static inline int8_t get_mouse_speed(void) {
  if (mouse_precise_mode) {
    return MOUSE_MOVE_PRECISE;
  } else if (mouse_slow_mode) {
    return MOUSE_MOVE_SLOW;
  } else {
    return MOUSE_MOVE_DEFAULT;
  }
}

static inline int8_t get_wheel_speed(void) {
  if (mouse_precise_mode) {
    return MOUSE_WHEEL_PRECISE;
  } else if (mouse_slow_mode) {
    return MOUSE_WHEEL_SLOW;
  } else {
    return MOUSE_WHEEL_DEFAULT;
  }
}

static inline int8_t get_zoom_wheel_speed(void) {
  if (mouse_precise_mode) {
    return ZOOM_WHEEL_PRECISE;
  } else if (mouse_slow_mode) {
    return ZOOM_WHEEL_SLOW;
  } else {
    return ZOOM_WHEEL_DEFAULT;
  }
}

static inline uint16_t get_mouse_interval(void) {
  if (zoom_in_pressed || zoom_out_pressed) {
    return get_zoom_interval();
  }

  if (mouse_scroll_mode) {
    if (mouse_precise_mode) {
      return SCROLL_INTERVAL_PRECISE;
    } else if (mouse_slow_mode) {
      return SCROLL_INTERVAL_SLOW;
    } else {
      return SCROLL_INTERVAL_DEFAULT;
    }
  }

  if (mouse_precise_mode) {
    return CURSOR_INTERVAL_PRECISE;
  } else if (mouse_slow_mode) {
    return CURSOR_INTERVAL_SLOW;
  } else {
    return CURSOR_INTERVAL_DEFAULT;
  }
}

static inline uint16_t get_zoom_interval(void) {
  if (mouse_precise_mode) {
    return ZOOM_INTERVAL_PRECISE;
  } else if (mouse_slow_mode) {
    return ZOOM_INTERVAL_SLOW;
  } else {
    return ZOOM_INTERVAL_DEFAULT;
  }
}

static inline void update_zoom_ctrl(void) {
  if (zoom_in_pressed || zoom_out_pressed) {
    register_code(KC_LCTL);
  } else {
    unregister_code(KC_LCTL);
  }
}

// Fire the screen/media hold actions once TAPHOLD_TIMEOUT passes, like kanata's
// tap-hold, instead of waiting for the release. Marking the key used stops the
// release from firing anything else.
static void fire_due_holds(void) {
  if (screen_hold_state.held && !screen_hold_state.used &&
      timer_elapsed(screen_hold_state.timer) >= TAPHOLD_TIMEOUT) {
    screen_hold_state.used = true;
    turn_display_off();
  }
  if (media_hold_state.held && !media_hold_state.used &&
      timer_elapsed(media_hold_state.timer) >= TAPHOLD_TIMEOUT) {
    media_hold_state.used = true;
    tap_code(KC_MNXT);
  }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  if (oneshot_state.active) {
    if (timer_elapsed(oneshot_state.timer) > ONESHOT_TIMEOUT) {
      unregister_mods(oneshot_state.mods);
      oneshot_state.active = false;
    }
  }

  if (record->event.pressed) {
    if (modifier_hold_state.os_hyp_held || modifier_hold_state.os_fn_held ||
        modifier_hold_state.mod_alt_held || modifier_hold_state.mod_ctrl_held ||
        modifier_hold_state.mod_shift_held ||
        modifier_hold_state.mod_meta_held || screen_hold_state.held ||
        media_hold_state.held) {

      if (!is_modifier_like_key(keycode) && !is_pointer_or_hold_key(keycode)) {

        if (modifier_hold_state.os_hyp_held)
          modifier_hold_state.os_hyp_used = true;
        if (modifier_hold_state.os_fn_held)
          modifier_hold_state.os_fn_used = true;
        if (modifier_hold_state.mod_alt_held)
          modifier_hold_state.mod_alt_used = true;
        if (modifier_hold_state.mod_ctrl_held)
          modifier_hold_state.mod_ctrl_used = true;
        if (modifier_hold_state.mod_shift_held)
          modifier_hold_state.mod_shift_used = true;
        if (modifier_hold_state.mod_meta_held)
          modifier_hold_state.mod_meta_used = true;
        if (screen_hold_state.held)
          screen_hold_state.used = true;
        if (media_hold_state.held)
          media_hold_state.used = true;
      }
    }
  }

  switch (keycode) {
  case KC_TL_KEY:
    if (record->event.pressed) {
      tl_pressed = true;
      layer_on(_TL);
      if (tr_pressed) {
        layer_on(_TLTR);
      }
    } else {
      tl_pressed = false;
      layer_off(_TLTR);
      layer_off(_TL);
    }
    return false;

  case KC_TR_KEY:
    if (record->event.pressed) {
      tr_pressed = true;
      layer_on(_TR);
      if (tl_pressed) {
        layer_on(_TLTR);
      }
    } else {
      tr_pressed = false;
      layer_off(_TLTR);
      layer_off(_TR);
    }
    return false;

  case KC_TLTLTR_KEY:
    if (record->event.pressed) {
      tl_pressed = true;
      layer_on(_TL);
      layer_on(_TLTR);
    } else {
      tl_pressed = false;
      layer_off(_TLTR);
      layer_off(_TL);
    }
    return false;

  case KC_TRTLTR_KEY:
    if (record->event.pressed) {
      tr_pressed = true;
      layer_on(_TR);
      layer_on(_TLTR);
    } else {
      tr_pressed = false;
      layer_off(_TLTR);
      layer_off(_TR);
    }
    return false;

  case KC_OS_HYP:
    if (record->event.pressed) {
      modifier_hold_state.os_hyp_held = true;
      modifier_hold_state.os_hyp_used = false;
      modifier_hold_state.os_hyp_timer = timer_read();
      register_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT) |
                    MOD_BIT(KC_LGUI));
    } else {
      modifier_hold_state.os_hyp_held = false;
      unregister_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT) |
                      MOD_BIT(KC_LGUI));

      if (!modifier_hold_state.os_hyp_used &&
          timer_elapsed(modifier_hold_state.os_hyp_timer) < TAPHOLD_TIMEOUT) {
        add_oneshot_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) |
                         MOD_BIT(KC_LSFT) | MOD_BIT(KC_LGUI));
      }
    }
    return false;

  case KC_OS_FN:
    if (record->event.pressed) {
      fn_modifier_active = true;
      fn_oneshot_active = false;
      modifier_hold_state.os_fn_held = true;
      modifier_hold_state.os_fn_used = false;
      modifier_hold_state.os_fn_timer = timer_read();
    } else {
      modifier_hold_state.os_fn_held = false;

      if (!modifier_hold_state.os_fn_used &&
          timer_elapsed(modifier_hold_state.os_fn_timer) < TAPHOLD_TIMEOUT) {
        fn_oneshot_active = true;
      } else {
        fn_modifier_active = false;
        fn_oneshot_active = false;
      }
    }
    return false;

  case KC_MOD_ALT:
    if (record->event.pressed) {
      modifier_hold_state.mod_alt_held = true;
      modifier_hold_state.mod_alt_used = false;
      modifier_hold_state.mod_alt_timer = timer_read();
      register_mods(MOD_BIT(KC_LALT));
    } else {
      modifier_hold_state.mod_alt_held = false;
      unregister_mods(MOD_BIT(KC_LALT));
      if (!modifier_hold_state.mod_alt_used &&
          timer_elapsed(modifier_hold_state.mod_alt_timer) < TAPHOLD_TIMEOUT) {
        add_oneshot_mods(MOD_BIT(KC_LALT));
      }
    }
    return false;

  case KC_MOD_CTRL:
    if (record->event.pressed) {
      modifier_hold_state.mod_ctrl_held = true;
      modifier_hold_state.mod_ctrl_used = false;
      modifier_hold_state.mod_ctrl_timer = timer_read();
      register_mods(MOD_BIT(KC_LCTL));
    } else {
      modifier_hold_state.mod_ctrl_held = false;
      unregister_mods(MOD_BIT(KC_LCTL));
      if (!modifier_hold_state.mod_ctrl_used &&
          timer_elapsed(modifier_hold_state.mod_ctrl_timer) < TAPHOLD_TIMEOUT) {
        add_oneshot_mods(MOD_BIT(KC_LCTL));
      }
    }
    return false;

  case KC_MOD_SHIFT:
    if (record->event.pressed) {
      modifier_hold_state.mod_shift_held = true;
      modifier_hold_state.mod_shift_used = false;
      modifier_hold_state.mod_shift_timer = timer_read();
      register_mods(MOD_BIT(KC_LSFT));
    } else {
      modifier_hold_state.mod_shift_held = false;
      unregister_mods(MOD_BIT(KC_LSFT));
      if (!modifier_hold_state.mod_shift_used &&
          timer_elapsed(modifier_hold_state.mod_shift_timer) <
              TAPHOLD_TIMEOUT) {
        add_oneshot_mods(MOD_BIT(KC_LSFT));
      }
    }
    return false;

  case KC_MOD_META:
    if (record->event.pressed) {
      modifier_hold_state.mod_meta_held = true;
      modifier_hold_state.mod_meta_used = false;
      modifier_hold_state.mod_meta_timer = timer_read();
      register_mods(MOD_BIT(KC_LGUI));
    } else {
      modifier_hold_state.mod_meta_held = false;
      unregister_mods(MOD_BIT(KC_LGUI));
      if (!modifier_hold_state.mod_meta_used &&
          timer_elapsed(modifier_hold_state.mod_meta_timer) < TAPHOLD_TIMEOUT) {
        add_oneshot_mods(MOD_BIT(KC_LGUI));
      }
    }
    return false;

  case KC_MSLW:
    mouse_slow_mode = record->event.pressed;
    return false;

  case KC_MPRE:
    mouse_precise_mode = record->event.pressed;
    return false;

  case KC_MSCR:
    mouse_scroll_mode = record->event.pressed;
    return false;

  case KC_MUP:
    mouse_up_pressed = record->event.pressed;
    return false;

  case KC_MDN:
    mouse_down_pressed = record->event.pressed;
    return false;

  case KC_MLFT:
    mouse_left_pressed = record->event.pressed;
    return false;

  case KC_MRGT:
    mouse_right_pressed = record->event.pressed;
    return false;

  case KC_ZMIN:
    zoom_in_pressed = record->event.pressed;
    update_zoom_ctrl();
    return false;

  case KC_ZMOUT:
    zoom_out_pressed = record->event.pressed;
    update_zoom_ctrl();
    return false;

  case KC_MBTN1:
    if (record->event.pressed) {
      mbtn1_oneshot_mods = get_oneshot_mods();
      if (mbtn1_oneshot_mods) {
        register_mods(mbtn1_oneshot_mods);
        clear_oneshot_mods();
      }
      mouse_buttons |= MOUSE_BTN1;
    } else {
      mouse_buttons &= ~MOUSE_BTN1;
      if (mbtn1_oneshot_mods) {
        unregister_mods(mbtn1_oneshot_mods);
        mbtn1_oneshot_mods = 0;
      }
    }
    {
      report_mouse_t mouse_report = {
          .buttons = mouse_buttons, .x = 0, .y = 0, .v = 0, .h = 0};
      host_mouse_send(&mouse_report);
    }
    return false;

  case KC_MBTN2:
    if (record->event.pressed) {
      mbtn2_oneshot_mods = get_oneshot_mods();
      if (mbtn2_oneshot_mods) {
        register_mods(mbtn2_oneshot_mods);
        clear_oneshot_mods();
      }
      mouse_buttons |= MOUSE_BTN2;
    } else {
      mouse_buttons &= ~MOUSE_BTN2;
      if (mbtn2_oneshot_mods) {
        unregister_mods(mbtn2_oneshot_mods);
        mbtn2_oneshot_mods = 0;
      }
    }
    {
      report_mouse_t mouse_report = {
          .buttons = mouse_buttons, .x = 0, .y = 0, .v = 0, .h = 0};
      host_mouse_send(&mouse_report);
    }
    return false;

  case KC_DOLF:
    return handle_unicode_fork(record, KC_DLR, "₹");
  case KC_HPNF:
    return handle_unicode_fork(record, KC_MINS, "–");
  case KC_EQLF:
    return handle_unicode_fork(record, KC_EQL, "—");

  case KC_1F:
    return handle_fn_fork(record, KC_1, KC_F1);
  case KC_2F:
    return handle_fn_fork(record, KC_2, KC_F2);
  case KC_3F:
    return handle_fn_fork(record, KC_3, KC_F3);
  case KC_4F:
    return handle_fn_fork(record, KC_4, KC_F4);
  case KC_5F:
    return handle_fn_fork(record, KC_5, KC_F5);
  case KC_6F:
    return handle_fn_fork(record, KC_6, KC_F6);
  case KC_7F:
    return handle_fn_fork(record, KC_7, KC_F7);
  case KC_8F:
    return handle_fn_fork(record, KC_8, KC_F8);
  case KC_9F:
    return handle_fn_fork(record, KC_9, KC_F9);
  case KC_0F:
    return handle_fn_fork(record, KC_0, KC_F10);

  case KC_ASTF:
    if (record->event.pressed) {
      if (fn_modifier_active) {
        tap_code(KC_F11);
      } else {
        tap_code16(KC_ASTR);
      }
    }
    return false;

  case KC_PERF:
    if (record->event.pressed) {
      if (fn_modifier_active) {
        tap_code(KC_F12);
      } else {
        tap_code16(KC_PERC);
      }
    }
    return false;

  case KC_SCRE:
    if (record->event.pressed) {
      screen_hold_state.held = true;
      screen_hold_state.used = false;
      screen_hold_state.timer = timer_read();
    } else {
      screen_hold_state.held = false;
      uint16_t elapsed = timer_elapsed(screen_hold_state.timer);

      if (!screen_hold_state.used) {
        if (elapsed < TAPHOLD_TIMEOUT) {
          lock_screen();
        } else {
          turn_display_off();
        }
      }
    }
    return false;

  case KC_MEDC:
    if (record->event.pressed) {
      media_hold_state.held = true;
      media_hold_state.used = false;
      media_hold_state.timer = timer_read();
    } else {
      media_hold_state.held = false;
      uint16_t elapsed = timer_elapsed(media_hold_state.timer);

      if (!media_hold_state.used) {
        if (elapsed < TAPHOLD_TIMEOUT) {
          tap_code(KC_MPLY);
        } else {
          tap_code(KC_MNXT);
        }
      }
    }
    return false;

  case KC_BOOT_HOLD:
    if (record->event.pressed) {
      boot_hold_timer = timer_read();
    } else {
      uint16_t elapsed = timer_elapsed(boot_hold_timer);
      if (elapsed < TAPHOLD_TIMEOUT) {
        watchdog_reboot(0, 0, 0);
      } else {
        reset_keyboard();
      }
    }
    return false;

  case KC_SCRL_REV:
    if (record->event.pressed) {
      scroll_direction_reversed = !scroll_direction_reversed;
    }
    return false;
  }

  if (fn_oneshot_active && record->event.pressed) {
    if (!is_modifier_like_key(keycode) && keycode != KC_LSFT &&
        keycode != KC_RSFT) {
      fn_modifier_active = false;
      fn_oneshot_active = false;
    }
  }

  return true;
}
