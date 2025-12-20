#include QMK_KEYBOARD_H
#include "hardware/watchdog.h"

// Speed level definitions
// Default: fast
#define MOUSE_MOVE_DEFAULT 16
#define MOUSE_WHEEL_DEFAULT 4

// Slow mode: slower movement
#define MOUSE_MOVE_SLOW 8
#define MOUSE_WHEEL_SLOW 2

// Precise mode: very slow, precise movement
#define MOUSE_MOVE_PRECISE 4
#define MOUSE_WHEEL_PRECISE 1

// Cursor movement intervals - keep fast for smooth movement
#define CURSOR_INTERVAL_DEFAULT 8  // 125Hz - very smooth
#define CURSOR_INTERVAL_SLOW 12    // 83Hz - smooth
#define CURSOR_INTERVAL_PRECISE 16 // 60Hz - smooth

// Scroll intervals - slower for controlled scrolling
#define SCROLL_INTERVAL_DEFAULT 32  // 30Hz - moderate speed
#define SCROLL_INTERVAL_SLOW 64     // 15Hz - slow, controlled
#define SCROLL_INTERVAL_PRECISE 100 // 10Hz - very slow, precise

// Define layer names
enum layer_names { _COLEMAK = 0, _TL, _TR, _TLTR };

// ============================================================================
// State variables (must be declared before use in functions)
// ============================================================================

// Mouse control states
static bool mouse_slow_mode = false;    // nop1
static bool mouse_precise_mode = false; // nop2
static bool mouse_scroll_mode = false;  // nop3

// Mouse direction state tracking for continuous movement/scrolling
static bool mouse_up_pressed = false;
static bool mouse_down_pressed = false;
static bool mouse_left_pressed = false;
static bool mouse_right_pressed = false;

// Mouse button state tracking
static uint8_t mouse_buttons = 0;

// Mouse report throttling timer
static uint16_t mouse_timer = 0;

// Boot/Reboot key timer
static uint16_t boot_hold_timer = 0;

// Forward declarations for helper functions
static inline int8_t get_mouse_speed(void);
static inline int8_t get_wheel_speed(void);
static inline uint16_t get_mouse_interval(void);

// Unicode setup - automatically detect OS and set input mode
void keyboard_post_init_user(void) {
  // Use OS detection to automatically set Unicode input mode
  // This detects the OS when the keyboard is plugged in
  os_variant_t detected_os = detected_host_os();

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
    // Fallback to Linux mode if OS cannot be detected
    set_unicode_input_mode(UNICODE_MODE_LINUX);
    break;
  }
}

// Custom mouse movement handling with dynamic speed adjustment
// This hook is called regularly to handle ongoing mouse movement
void matrix_scan_user(void) {
  // Only process if any direction key is pressed
  if (!mouse_up_pressed && !mouse_down_pressed && !mouse_left_pressed &&
      !mouse_right_pressed) {
    return;
  }

  // Throttle mouse reports based on current speed mode
  if (timer_elapsed(mouse_timer) < get_mouse_interval()) {
    return;
  }
  mouse_timer = timer_read();

  // Create a mouse report with current button state
  report_mouse_t mouse_report = {
      .buttons = mouse_buttons, .x = 0, .y = 0, .v = 0, .h = 0};

  if (mouse_scroll_mode) {
    // Scroll mode - use wheel deltas (reversed for natural scrolling)
    int8_t wheel_speed = get_wheel_speed();
    if (mouse_up_pressed) {
      mouse_report.v = -wheel_speed; // Reversed: up scrolls down
    }
    if (mouse_down_pressed) {
      mouse_report.v = wheel_speed; // Reversed: down scrolls up
    }
    if (mouse_left_pressed) {
      mouse_report.h = wheel_speed; // Reversed: left scrolls right
    }
    if (mouse_right_pressed) {
      mouse_report.h = -wheel_speed; // Reversed: right scrolls left
    }
  } else {
    // Cursor mode - use x/y deltas
    int8_t move_speed = get_mouse_speed();
    if (mouse_up_pressed) {
      mouse_report.y = -move_speed; // Up is negative Y
    }
    if (mouse_down_pressed) {
      mouse_report.y = move_speed; // Down is positive Y
    }
    if (mouse_left_pressed) {
      mouse_report.x = -move_speed; // Left is negative X
    }
    if (mouse_right_pressed) {
      mouse_report.x = move_speed; // Right is positive X
    }
  }

  // Send the mouse report
  host_mouse_send(&mouse_report);
}

// Caps Word configuration
bool caps_word_press_user(uint16_t keycode) {
  switch (keycode) {
  // Keycodes that continue Caps Word, with shift applied
  case KC_A ... KC_Z:
  case KC_MINS:
    add_weak_mods(MOD_BIT(KC_LSFT)); // Apply shift to next key
    return true;

  // Keycodes that continue Caps Word, without shifting
  case KC_1 ... KC_0:
  case KC_BSPC:
  case KC_DEL:
  case KC_UNDS:
  case KC_UP:
  case KC_DOWN:
  case KC_LEFT:
  case KC_RGHT:
  case KC_LSFT:
  case KC_RSFT:
    return true;

  default:
    return false; // Deactivate Caps Word
  }
}

// Define custom keycodes
enum custom_keycodes {
  // Fork keys (context-sensitive keys)
  KC_DELF = SAFE_RANGE, // Delete/Backspace Fork
  KC_SLAF,              // Slash/Backslash Fork
  KC_COMF,              // Comma/Underscore Fork
  KC_DOTF,              // Dot/Question Fork
  KC_PGUF,              // Page Up/Home Fork
  KC_PGDF,              // Page Down/End Fork
  KC_EXCF,              // Exclamation/Grave Fork
  KC_ATSF,              // At/Tilde Fork
  KC_OCTF,              // Hash/Caret Fork
  KC_DOLF,              // Dollar/Rupee Fork
  KC_AMPF,              // Ampersand/Pipe Fork
  KC_CURF,              // Curly Brackets Fork
  KC_PARF,              // Parentheses Fork
  KC_SQRF,              // Square Brackets Fork
  KC_ASTF,              // Asterisk/F11 Fork
  KC_PERF,              // Percent/F12 Fork
  KC_HPNF,              // Hyphen (unshifted minus)
  KC_EQUF,              // Equals (unshifted)
  KC_PLUF,              // Plus (shifted equals)
  KC_ANOF,              // Angle Open (unshifted <)
  KC_ANCF,              // Angle Close (unshifted >)

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
  KC_OS_WIN, // sWin: Alt+Ctrl+Meta (window manager)
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

  // Media/Screen controls
  KC_SCRE, // Screen control (tap=lock screen, hold=sleep)
  KC_MEDC, // Media control (tap=play/pause, hold=next track)

  // Boot/Reboot control
  KC_BOOT_HOLD, // Boot control (tap=reboot, hold=bootloader)
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // COLEMAK MOD-DH + WIDE + ANGLE (38-key TLTR layout)
    [_COLEMAK] = LAYOUT_split_2x6_1x5_2(
        QK_REP,  KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,        KC_J,    KC_L,    KC_U,    KC_Y,    KC_QUOT, KC_SCLN,
        KC_DELF, KC_A,    KC_R,    KC_S,    KC_T,    KC_G,        KC_M,    KC_N,    KC_E,    KC_I,    KC_O,    KC_ENT,
                 KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,        KC_K,    KC_H,    KC_COMF, KC_DOTF, KC_SLAF,
                                   KC_TL_KEY, KC_LSFT,            KC_SPC,  KC_TR_KEY
    ),

    // TL Layer - Modifiers & Navigation
    [_TL] = LAYOUT_split_2x6_1x5_2(
        KC_BOOT_HOLD, KC_ESC,      KC_OS_WIN,    KC_OS_FN,     KC_NO,       KC_NO,       KC_PGUF,   S(KC_TAB), KC_UP,   KC_TAB,  KC_NO,   KC_NO,
        KC_TRNS,      KC_MOD_ALT,  KC_MOD_CTRL,  KC_MOD_SHIFT, KC_MOD_META, KC_NO,       KC_PGDF,   KC_LEFT,   KC_DOWN, KC_RGHT, KC_NO,   KC_TRNS,
                      KC_NO,       KC_NO,        KC_NO,        KC_OS_HYP,   KC_NO,       KC_NO,     KC_BSPC,   KC_DEL,  KC_NO,   KC_TRNS,
                                                 KC_TRNS,      KC_TRNS,                  KC_TRNS,   KC_TRTLTR_KEY
    ),

    // TR Layer - Numbers & Symbols
    [_TR] = LAYOUT_split_2x6_1x5_2(
        KC_NO,       KC_EXCF, KC_ATSF, KC_OCTF, KC_DOLF, KC_NO,       KC_PERF, KC_7F,   KC_8F,   KC_9F,   KC_PLUF, KC_EQUF,
        KC_TRNS,     KC_AMPF, KC_SQRF, KC_CURF, KC_PARF, KC_NO,       KC_ASTF, KC_4F,   KC_5F,   KC_6F,   KC_HPNF, KC_TRNS,
                     KC_NO,   KC_NO,   KC_ANOF, KC_ANCF, KC_NO,       KC_0F,   KC_1F,   KC_2F,   KC_3F,   KC_SLAF,
                                       KC_TLTLTR_KEY, KC_TRNS,        KC_TRNS, KC_TRNS
    ),

    // TLTR Layer - Mouse, Media & Display Controls
    [_TLTR] = LAYOUT_split_2x6_1x5_2(
        KC_NO,   KC_SCRE, KC_BRIU, KC_MEDC,  KC_VOLU,  KC_NO,       KC_NO,   KC_NO,   KC_MUP,  KC_NO,   KC_NO,   KC_NO,
        KC_NO,   KC_MPRE, KC_MSLW, KC_MSCR,  KC_MBTN1, KC_MBTN2,    KC_NO,   KC_MLFT, KC_MDN,  KC_MRGT, KC_NO,   KC_NO,
                 KC_NO,   KC_BRID, KC_MPRV,  KC_VOLD,  KC_NO,       KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
                                   KC_TRNS,  KC_TRNS,               KC_TRNS, KC_TRNS
    )
};
// clang-format on

// ============================================================================
// Advanced state tracking
// ============================================================================

// Thumb key state for TL/TR/TLTR layer management
static bool tl_pressed = false;
static bool tr_pressed = false;

// Fn modifier state
static bool fn_modifier_active = false;
static bool fn_oneshot_active = false;

// Fork key state tracking for key repeat
static uint16_t delf_registered_key = KC_NO;
static uint16_t pguf_registered_key = KC_NO;
static uint16_t pgdf_registered_key = KC_NO;
static uint16_t slaf_registered_key = KC_NO;
static uint16_t comf_registered_key = KC_NO;
static uint16_t dotf_registered_key = KC_NO;

// One-shot state tracking for custom combinations
static struct {
  bool active;
  uint16_t timer;
  uint8_t mods;
} oneshot_state = {false, 0, 0};

// Tap-hold state for custom modifier keys
static struct {
  bool os_win_held;
  bool os_hyp_held;
  bool os_fn_held;
  bool os_win_used; // Was another key pressed while held?
  bool os_hyp_used; // Was another key pressed while held?
  bool os_fn_used;  // Was another key pressed while held?
  uint16_t os_win_timer;
  uint16_t os_hyp_timer;
  uint16_t os_fn_timer;

  // Individual modifiers
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
} modifier_hold_state = {false, false, false, false, false, false, 0,
                         0,     0,     false, false, false, false, false,
                         false, false, false, 0,     0,     0,     0};

// Screen control tap-hold state
static struct {
  bool held;
  bool used;
  uint16_t timer;
} screen_hold_state = {false, false, 0};

// Media control tap-hold state
static struct {
  bool held;
  bool used;
  uint16_t timer;
} media_hold_state = {false, false, 0};

// Timeouts (in milliseconds)
#define ONESHOT_TIMEOUT 300
#define TAPHOLD_TIMEOUT 200

// ============================================================================
// Helper functions
// ============================================================================

// Check if left shift is active
static inline bool is_left_shift_active(void) {
  return (get_mods() & MOD_BIT(KC_LSFT)) ||
         (get_oneshot_mods() & MOD_BIT(KC_LSFT));
}

// Check if any shift is active (for compatibility)
static inline bool is_shift_active(void) {
  return (get_mods() & MOD_MASK_SHIFT) || (get_oneshot_mods() & MOD_MASK_SHIFT);
}

// Clear shift mods temporarily for unshifted output
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

// Handle function key forks (number vs function based on Fn modifier)
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

// Get current mouse movement speed based on mode flags
// Precise takes priority over slow
static inline int8_t get_mouse_speed(void) {
  if (mouse_precise_mode) {
    return MOUSE_MOVE_PRECISE; // Very slow, precise
  } else if (mouse_slow_mode) {
    return MOUSE_MOVE_SLOW; // Slow
  } else {
    return MOUSE_MOVE_DEFAULT; // Modestly fast (default)
  }
}

// Get current mouse wheel speed based on mode flags
static inline int8_t get_wheel_speed(void) {
  if (mouse_precise_mode) {
    return MOUSE_WHEEL_PRECISE; // Very slow, precise
  } else if (mouse_slow_mode) {
    return MOUSE_WHEEL_SLOW; // Slow
  } else {
    return MOUSE_WHEEL_DEFAULT; // Modestly fast (default)
  }
}

// Get current mouse report interval based on mode flags
// Uses different intervals for cursor movement (fast) vs scrolling (slow)
static inline uint16_t get_mouse_interval(void) {
  if (mouse_scroll_mode) {
    // Scrolling mode - use slower intervals for controlled scrolling
    if (mouse_precise_mode) {
      return SCROLL_INTERVAL_PRECISE; // 100ms - very slow
    } else if (mouse_slow_mode) {
      return SCROLL_INTERVAL_SLOW; // 64ms - slow
    } else {
      return SCROLL_INTERVAL_DEFAULT; // 32ms - moderate
    }
  } else {
    // Cursor movement mode - use fast intervals for smooth movement
    if (mouse_precise_mode) {
      return CURSOR_INTERVAL_PRECISE; // 16ms - smooth
    } else if (mouse_slow_mode) {
      return CURSOR_INTERVAL_SLOW; // 12ms - smooth
    } else {
      return CURSOR_INTERVAL_DEFAULT; // 8ms - very smooth
    }
  }
}

// ============================================================================
// Main key processing
// ============================================================================

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  // Handle one-shot timeout - only check if oneshot is actually active
  if (oneshot_state.active) {
    if (timer_elapsed(oneshot_state.timer) > ONESHOT_TIMEOUT) {
      unregister_mods(oneshot_state.mods);
      oneshot_state.active = false;
    }
  }

  // Detect when other keys are pressed while custom modifiers are held
  // Only check on key press, and only check held states (skip if none held)
  if (record->event.pressed) {
    // Fast path: if no modifiers/special keys held, skip all checks
    if (modifier_hold_state.os_win_held || modifier_hold_state.os_hyp_held ||
        modifier_hold_state.os_fn_held || modifier_hold_state.mod_alt_held ||
        modifier_hold_state.mod_ctrl_held ||
        modifier_hold_state.mod_shift_held ||
        modifier_hold_state.mod_meta_held || screen_hold_state.held ||
        media_hold_state.held) {

      // Exclude modifier keys themselves, mouse control keys, and thumb layer
      // keys
      if (keycode != KC_OS_WIN && keycode != KC_OS_HYP && keycode != KC_OS_FN &&
          keycode != KC_MOD_ALT && keycode != KC_MOD_CTRL &&
          keycode != KC_MOD_SHIFT && keycode != KC_MOD_META &&
          keycode != KC_SCRE && keycode != KC_MEDC && keycode != KC_MSLW &&
          keycode != KC_MPRE && keycode != KC_MSCR && keycode != KC_MUP &&
          keycode != KC_MDN && keycode != KC_MLFT && keycode != KC_MRGT &&
          keycode != KC_MBTN1 && keycode != KC_MBTN2 && keycode != KC_TL_KEY &&
          keycode != KC_TR_KEY && keycode != KC_TLTLTR_KEY &&
          keycode != KC_TRTLTR_KEY) {

        // Mark held modifiers as used
        if (modifier_hold_state.os_win_held)
          modifier_hold_state.os_win_used = true;
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
  // ========================================================================
  // Thumb key layer activation
  // ========================================================================
  case KC_TL_KEY:
    if (record->event.pressed) {
      tl_pressed = true;
      // Always activate TL layer
      layer_on(_TL);
      // Also activate TLTR if TR is pressed
      if (tr_pressed) {
        layer_on(_TLTR);
      }
    } else {
      tl_pressed = false;
      // Always turn off TLTR and TL
      layer_off(_TLTR);
      layer_off(_TL);
    }
    return false;

  case KC_TR_KEY:
    if (record->event.pressed) {
      tr_pressed = true;
      // Always activate TR layer
      layer_on(_TR);
      // Also activate TLTR if TL is pressed
      if (tl_pressed) {
        layer_on(_TLTR);
      }
    } else {
      tr_pressed = false;
      // Always turn off TLTR and TR
      layer_off(_TLTR);
      layer_off(_TR);
    }
    return false;

  // Special keys for activating TLTR from within TL/TR layers
  case KC_TLTLTR_KEY:
    // This is the TL key when pressed from TR layer
    if (record->event.pressed) {
      tl_pressed = true;
      // Activate TL layer (TR should already be on)
      layer_on(_TL);
      // Activate TLTR on top
      layer_on(_TLTR);
    } else {
      tl_pressed = false;
      // Turn off TLTR and TL
      layer_off(_TLTR);
      layer_off(_TL);
    }
    return false;

  case KC_TRTLTR_KEY:
    // This is the TR key when pressed from TL layer
    if (record->event.pressed) {
      tr_pressed = true;
      // Activate TR layer (TL should already be on)
      layer_on(_TR);
      // Activate TLTR on top
      layer_on(_TLTR);
    } else {
      tr_pressed = false;
      // Turn off TLTR and TR
      layer_off(_TLTR);
      layer_off(_TR);
    }
    return false;

  // ========================================================================
  // One-shot modifier combinations
  // ========================================================================
  case KC_OS_WIN: // sWin: Alt+Ctrl+Meta
    if (record->event.pressed) {
      modifier_hold_state.os_win_held = true;
      modifier_hold_state.os_win_used = false;
      modifier_hold_state.os_win_timer = timer_read();
      // Register as regular modifier for immediate effect when held
      register_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LGUI));
    } else {
      modifier_hold_state.os_win_held = false;
      // Unregister the regular modifier
      unregister_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LGUI));

      // If it was a quick tap and wasn't used, make it one-shot for next key
      if (!modifier_hold_state.os_win_used &&
          timer_elapsed(modifier_hold_state.os_win_timer) < TAPHOLD_TIMEOUT) {
        add_oneshot_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) |
                         MOD_BIT(KC_LGUI));
      }
    }
    return false;

  case KC_OS_HYP: // sHyp: Alt+Ctrl+Shift+Meta
    if (record->event.pressed) {
      modifier_hold_state.os_hyp_held = true;
      modifier_hold_state.os_hyp_used = false;
      modifier_hold_state.os_hyp_timer = timer_read();
      // Register as regular modifier for immediate effect when held
      register_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT) |
                    MOD_BIT(KC_LGUI));
    } else {
      modifier_hold_state.os_hyp_held = false;
      // Unregister the regular modifier
      unregister_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT) |
                      MOD_BIT(KC_LGUI));

      // If it was a quick tap and wasn't used, make it one-shot for next key
      if (!modifier_hold_state.os_hyp_used &&
          timer_elapsed(modifier_hold_state.os_hyp_timer) < TAPHOLD_TIMEOUT) {
        add_oneshot_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) |
                         MOD_BIT(KC_LSFT) | MOD_BIT(KC_LGUI));
      }
    }
    return false;

  case KC_OS_FN: // sFn: Function modifier
    if (record->event.pressed) {
      fn_modifier_active = true;
      fn_oneshot_active =
          false; // Cancel any existing one-shot when pressed again
      modifier_hold_state.os_fn_held = true;
      modifier_hold_state.os_fn_used = false;
      modifier_hold_state.os_fn_timer = timer_read();
    } else {
      modifier_hold_state.os_fn_held = false;

      // If it was a quick tap and wasn't used, make it one-shot for next key
      if (!modifier_hold_state.os_fn_used &&
          timer_elapsed(modifier_hold_state.os_fn_timer) < TAPHOLD_TIMEOUT) {
        // Keep fn_modifier_active true and mark as one-shot
        fn_oneshot_active = true;
      } else {
        // Was held and used, or held too long - just turn off
        fn_modifier_active = false;
        fn_oneshot_active = false;
      }
    }
    return false;

  // ========================================================================
  // Individual modifiers with tap-hold behavior
  // ========================================================================
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

  // ========================================================================
  // Mouse control modifiers
  // ========================================================================
  case KC_MSLW: // slow mode
    mouse_slow_mode = record->event.pressed;
    return false;

  case KC_MPRE: // precise mode
    mouse_precise_mode = record->event.pressed;
    return false;

  case KC_MSCR: // scroll mode
    mouse_scroll_mode = record->event.pressed;
    return false;

  // ========================================================================
  // Directional mouse/scroll keys
  // ========================================================================
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

  // ========================================================================
  // Mouse buttons (track state for drag operations)
  // ========================================================================
  case KC_MBTN1:
    if (record->event.pressed) {
      mouse_buttons |= MOUSE_BTN1;
    } else {
      mouse_buttons &= ~MOUSE_BTN1;
    }
    // Send immediate button report
    {
      report_mouse_t mouse_report = {
          .buttons = mouse_buttons, .x = 0, .y = 0, .v = 0, .h = 0};
      host_mouse_send(&mouse_report);
    }
    return false;

  case KC_MBTN2:
    if (record->event.pressed) {
      mouse_buttons |= MOUSE_BTN2;
    } else {
      mouse_buttons &= ~MOUSE_BTN2;
    }
    // Send immediate button report
    {
      report_mouse_t mouse_report = {
          .buttons = mouse_buttons, .x = 0, .y = 0, .v = 0, .h = 0};
      host_mouse_send(&mouse_report);
    }
    return false;

  // ========================================================================
  // Fork keys - basic (check left shift)
  // ========================================================================
  case KC_DELF: // Delete/Backspace
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        register_code(KC_DEL);
        set_mods(saved_mods);
        delf_registered_key = KC_DEL;
      } else {
        register_code(KC_BSPC);
        delf_registered_key = KC_BSPC;
      }
    } else {
      // Release the key that was registered
      if (delf_registered_key != KC_NO) {
        unregister_code(delf_registered_key);
        delf_registered_key = KC_NO;
      }
    }
    return false;

  case KC_SLAF: // Slash/Backslash
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        register_code(KC_BSLS);
        set_mods(saved_mods);
        slaf_registered_key = KC_BSLS;
      } else {
        register_code(KC_SLSH);
        slaf_registered_key = KC_SLSH;
      }
    } else {
      if (slaf_registered_key != KC_NO) {
        unregister_code(slaf_registered_key);
        slaf_registered_key = KC_NO;
      }
    }
    return false;

  case KC_COMF: // Comma/Underscore
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Already shifted, output underscore
        register_code(KC_MINS);
        comf_registered_key = KC_MINS;
      } else {
        register_code(KC_COMM);
        comf_registered_key = KC_COMM;
      }
    } else {
      if (comf_registered_key != KC_NO) {
        unregister_code(comf_registered_key);
        comf_registered_key = KC_NO;
      }
    }
    return false;

  case KC_DOTF: // Dot/Question
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Already shifted, output question mark
        register_code(KC_SLSH);
        dotf_registered_key = KC_SLSH;
      } else {
        register_code(KC_DOT);
        dotf_registered_key = KC_DOT;
      }
    } else {
      if (dotf_registered_key != KC_NO) {
        unregister_code(dotf_registered_key);
        dotf_registered_key = KC_NO;
      }
    }
    return false;

  case KC_PGUF: // Page Up/Home
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        register_code(KC_HOME);
        set_mods(saved_mods);
        pguf_registered_key = KC_HOME;
      } else {
        register_code(KC_PGUP);
        pguf_registered_key = KC_PGUP;
      }
    } else {
      if (pguf_registered_key != KC_NO) {
        unregister_code(pguf_registered_key);
        pguf_registered_key = KC_NO;
      }
    }
    return false;

  case KC_PGDF: // Page Down/End
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        register_code(KC_END);
        set_mods(saved_mods);
        pgdf_registered_key = KC_END;
      } else {
        register_code(KC_PGDN);
        pgdf_registered_key = KC_PGDN;
      }
    } else {
      if (pgdf_registered_key != KC_NO) {
        unregister_code(pgdf_registered_key);
        pgdf_registered_key = KC_NO;
      }
    }
    return false;

  // ========================================================================
  // Symbol forks (check left shift, produce shifted symbols)
  // ========================================================================
  case KC_EXCF: // !/`
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Shifted: grave
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code(KC_GRV);
        set_mods(saved_mods);
      } else {
        // Normal: exclamation (S-1)
        tap_code16(KC_EXLM);
      }
    }
    return false;

  case KC_ATSF: // @/~
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Shifted: tilde (S-`)
        tap_code16(KC_TILD);
      } else {
        // Normal: at (S-2)
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code16(KC_AT);
        set_mods(saved_mods);
      }
    }
    return false;

  case KC_OCTF: // #/^
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Shifted: caret (S-6)
        tap_code16(KC_CIRC);
      } else {
        // Normal: hash (S-3)
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code16(KC_HASH);
        set_mods(saved_mods);
      }
    }
    return false;

  case KC_DOLF: // $/₹
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Shifted: Rupee symbol (₹) via Unicode
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        send_unicode_string("₹");
        set_mods(saved_mods);
      } else {
        // Normal: Dollar ($)
        tap_code16(KC_DLR);
      }
    }
    return false;

  case KC_AMPF: // &/|
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Shifted: pipe (S-\)
        tap_code16(KC_PIPE);
      } else {
        // Normal: ampersand (S-7)
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code16(KC_AMPR);
        set_mods(saved_mods);
      }
    }
    return false;

  // ========================================================================
  // Bracket forks
  // ========================================================================
  case KC_CURF: // {/}
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Shifted: right curly
        tap_code16(KC_RCBR);
      } else {
        // Normal: left curly
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code16(KC_LCBR);
        set_mods(saved_mods);
      }
    }
    return false;

  case KC_PARF: // (/)
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Shifted: right paren
        tap_code16(KC_RPRN);
      } else {
        // Normal: left paren
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code16(KC_LPRN);
        set_mods(saved_mods);
      }
    }
    return false;

  case KC_SQRF: // [/]
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Shifted: right square bracket
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code(KC_RBRC);
        set_mods(saved_mods);
      } else {
        // Normal: left square bracket
        tap_code(KC_LBRC);
      }
    }
    return false;

  // ========================================================================
  // Function key forks (number vs function based on Fn modifier)
  // ========================================================================
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

  case KC_ASTF: // */F11
    if (record->event.pressed) {
      if (fn_modifier_active) {
        tap_code(KC_F11);
      } else {
        // Asterisk (S-8)
        tap_code16(KC_ASTR);
      }
    }
    return false;

  case KC_PERF: // %/F12
    if (record->event.pressed) {
      if (fn_modifier_active) {
        tap_code(KC_F12);
      } else {
        // Percent (S-5)
        tap_code16(KC_PERC);
      }
    }
    return false;

  // ========================================================================
  // Unshifted symbols
  // ========================================================================
  case KC_HPNF: // Hyphen (unshifted -)
    if (record->event.pressed) {
      tap_code(KC_MINS);
    }
    return false;

  case KC_EQUF: // Equals (unshifted =)
    if (record->event.pressed) {
      uint8_t saved_mods = get_mods();
      clear_shift_mods();
      tap_code(KC_EQL);
      set_mods(saved_mods);
    }
    return false;

  case KC_PLUF: // Plus (S-=)
    if (record->event.pressed) {
      tap_code16(KC_PLUS);
    }
    return false;

  case KC_ANOF: // < (unshifted)
    if (record->event.pressed) {
      uint8_t saved_mods = get_mods();
      clear_shift_mods();
      tap_code16(KC_LT);
      set_mods(saved_mods);
    }
    return false;

  case KC_ANCF: // > (unshifted)
    if (record->event.pressed) {
      uint8_t saved_mods = get_mods();
      clear_shift_mods();
      tap_code16(KC_GT);
      set_mods(saved_mods);
    }
    return false;

  // ========================================================================
  // Media and screen controls
  // ========================================================================
  case KC_SCRE:
    if (record->event.pressed) {
      screen_hold_state.held = true;
      screen_hold_state.used = false;
      screen_hold_state.timer = timer_read();
    } else {
      screen_hold_state.held = false;
      uint16_t elapsed = timer_elapsed(screen_hold_state.timer);

      // Only trigger action if no other keys were pressed while held
      if (!screen_hold_state.used) {
        if (elapsed < TAPHOLD_TIMEOUT) {
          // Tap: Lock screen (customize for your OS)
          // macOS (default): Ctrl+Cmd+Q
          // For Linux: Replace with Ctrl+Alt+L
          // For Windows: Replace with Win+L
          register_code(KC_LCTL);
          register_code(KC_LGUI);
          tap_code(KC_Q);
          unregister_code(KC_LGUI);
          unregister_code(KC_LCTL);
        } else {
          // Hold: Display sleep
          // macOS: Ctrl+Shift+Power (or Eject)
          // For Windows: Use KC_SYSTEM_SLEEP
          // For Linux: May need xset dpms force off via macro
          register_code(KC_LCTL);
          register_code(KC_LSFT);
          tap_code(KC_PWR);
          unregister_code(KC_LSFT);
          unregister_code(KC_LCTL);
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

      // Only trigger action if no other keys were pressed while held
      if (!media_hold_state.used) {
        if (elapsed < TAPHOLD_TIMEOUT) {
          // Tap: Play/pause
          tap_code(KC_MPLY);
        } else {
          // Hold: Next track
          tap_code(KC_MNXT);
        }
      }
    }
    return false;

  // ========================================================================
  // Boot/Reboot control (tap-hold)
  // ========================================================================
  case KC_BOOT_HOLD:
    if (record->event.pressed) {
      boot_hold_timer = timer_read();
    } else {
      uint16_t elapsed = timer_elapsed(boot_hold_timer);
      if (elapsed < TAPHOLD_TIMEOUT) {
        // Tap: Reboot keyboard using RP2040 watchdog
        watchdog_reboot(0, 0, 0);
      } else {
        // Hold: Enter bootloader mode for flashing
        reset_keyboard();
      }
    }
    return false;
  }

  // Clear Fn one-shot after a key is pressed (excluding modifiers and layer
  // keys)
  if (fn_oneshot_active && record->event.pressed) {
    // Exclude keys that shouldn't consume the one-shot
    if (keycode != KC_OS_WIN && keycode != KC_OS_HYP && keycode != KC_OS_FN &&
        keycode != KC_MOD_ALT && keycode != KC_MOD_CTRL &&
        keycode != KC_MOD_SHIFT && keycode != KC_MOD_META &&
        keycode != KC_MSLW && keycode != KC_MPRE && keycode != KC_MSCR &&
        keycode != KC_TL_KEY && keycode != KC_TR_KEY &&
        keycode != KC_TLTLTR_KEY && keycode != KC_TRTLTR_KEY &&
        keycode != KC_LSFT && keycode != KC_RSFT) {
      // A key that consumes the one-shot was pressed
      // Clear the one-shot after this key is processed
      fn_modifier_active = false;
      fn_oneshot_active = false;
    }
  }

  return true;
}
