#include QMK_KEYBOARD_H

// Speed level definitions (matching Kanata behavior)
// Default: fast
#define MOUSE_MOVE_DEFAULT 12
#define MOUSE_WHEEL_DEFAULT 4

// Slow mode: slower movement
#define MOUSE_MOVE_SLOW 3
#define MOUSE_WHEEL_SLOW 2

// Precise mode: very slow, precise movement
#define MOUSE_MOVE_PRECISE 1
#define MOUSE_WHEEL_PRECISE 1

// Define layer names
enum layer_names { _COLEMAK = 0, _TL, _TR, _TLTR };

// ============================================================================
// State variables (must be declared before use in functions)
// ============================================================================

// Mouse control states (matches Kanata nop1/nop2/nop3)
static bool mouse_slow_mode = false;    // nop1
static bool mouse_precise_mode = false; // nop2
static bool mouse_scroll_mode = false;  // nop3

// Mouse direction state tracking for continuous movement/scrolling
static bool mouse_up_pressed = false;
static bool mouse_down_pressed = false;
static bool mouse_left_pressed = false;
static bool mouse_right_pressed = false;

// Mouse report throttling timer
static uint16_t mouse_timer = 0;

// Forward declarations for helper functions
static inline int8_t get_mouse_speed(void);
static inline int8_t get_wheel_speed(void);

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

  // Throttle mouse reports to MOUSEKEY_INTERVAL (16ms)
  if (timer_elapsed(mouse_timer) < MOUSEKEY_INTERVAL) {
    return;
  }
  mouse_timer = timer_read();

  // Create a mouse report
  report_mouse_t mouse_report = {0};

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

// Caps Word configuration (matches Kanata: 3000ms timeout)
bool caps_word_press_user(uint16_t keycode) {
  switch (keycode) {
  // Keycodes that continue Caps Word, with shift applied
  case KC_A ... KC_Z:
  case KC_MINS:
    add_weak_mods(MOD_BIT(KC_LSFT)); // Apply shift to next key
    return true;

  // Keycodes that continue Caps Word, without shifting (matches Kanata ignore
  // list)
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

  // Advanced thumb key combinations (matches Kanata implementation 3)
  KC_TL_KEY,     // Left thumb key with TLTR logic
  KC_TR_KEY,     // Right thumb key with TLTR logic
  KC_TLTLTR_KEY, // TL+TLTR activation (used in TR layer)
  KC_TRTLTR_KEY, // TR+TLTR activation (used in TL layer)

  // One-shot modifier combinations (matches Kanata)
  KC_OS_WIN, // sWin: Alt+Ctrl+Meta (window manager)
  KC_OS_HYP, // sHyp: Alt+Ctrl+Shift+Meta (hyper)
  KC_OS_FN,  // sFn: Function modifier

  // Individual modifiers with tap-hold behavior (tap=oneshot, hold=regular)
  KC_MOD_ALT,   // Alt modifier (tap for oneshot, hold for regular)
  KC_MOD_CTRL,  // Ctrl modifier (tap for oneshot, hold for regular)
  KC_MOD_SHIFT, // Shift modifier (tap for oneshot, hold for regular)
  KC_MOD_META,  // Meta/GUI modifier (tap for oneshot, hold for regular)

  // Mouse control modifiers (matches Kanata nop1/nop2/nop3)
  KC_MSLW, // Mouse slow modifier (nop1)
  KC_MPRE, // Mouse precise modifier (nop2)
  KC_MSCR, // Mouse scroll modifier (nop3)

  // Directional mouse keys with mode switching
  KC_MUP,  // Mouse/scroll up (combines with modifiers)
  KC_MDN,  // Mouse/scroll down
  KC_MLFT, // Mouse/scroll left
  KC_MRGT, // Mouse/scroll right

  // Media/Screen controls
  KC_SCRE, // Screen control (tap=lock screen, hold=sleep)
  KC_MEDC, // Media control (tap=play/pause, hold=next track)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * COLEMAK MOD-DH + WIDE + ANGLE (38-key TLTR layout)
     * Matches Kanata CUSTOM_COLEMAK layer
     *
     * ┌─────┬───┬───┬───┬───┬───┐       ┌───┬───┬────┬────┬────┬─────┐
     * │ CW  │ Q │ W │ F │ P │ B │       │ J │ L │ U  │ Y  │ '  │  ;  │
     * ├─────┼───┼───┼───┼───┼───┤       ├───┼───┼────┼────┼────┼─────┤
     * │DELF │ A │ R │ S │ T │ G │       │ M │ N │ E  │ I  │ O  │ ENT │
     * └─────┼───┼───┼───┼───┼───┤       ├───┼───┼────┼────┼────┼─────┘
     *       │ Z │ X │ C │ D │ V │       │ K │ H │,/_ │./? │//\\ │
     *       └───┴───┴───┼───┼───┤       ├───┼───┼────┴────┴────┘
     *                   │TL │LSF│       │SPC│ TR│
     *                   └───┴───┘       └───┴───┘
     */
    [_COLEMAK] = LAYOUT_split_2x6_1x5_2(
        // Left side
        CW_TOGG, KC_Q, KC_W, KC_F, KC_P, KC_B, KC_DELF, KC_A, KC_R, KC_S, KC_T,
        KC_G, KC_Z, KC_X, KC_C, KC_D, KC_V, KC_TL_KEY, KC_LSFT,
        // Right side
        KC_J, KC_L, KC_U, KC_Y, KC_QUOT, KC_SCLN, KC_M, KC_N, KC_E, KC_I, KC_O,
        KC_ENT, KC_K, KC_H, KC_COMF, KC_DOTF, KC_SLAF, KC_SPC, KC_TR_KEY),

    /*
     * TL Layer - Modifiers & Navigation (Matches Kanata TL layer)
     *
     * ┌─────┬─────┬──────┬──────┬──────┬───┐
     * ┌──────┬──────┬──────┬──────┬───┬─────┐ │ --  │ Esc │ sWin │ sFn  │  --
     * │-- │       │ PgU  │S-Tab │  Up  │ Tab  │-- │ --  │
     * ├─────┼─────┼──────┼──────┼──────┼───┤
     * ├──────┼──────┼──────┼──────┼───┼─────┤ │     │ Alt │ Ctrl │Shift │ Meta
     * │-- │       │ PgD  │ Left │ Down │ Rght │-- │     │
     * └─────┼─────┼──────┼──────┼──────┼───┤
     * ├──────┼──────┼──────┼──────┼───┼─────┘ │ --  │  --  │  --  │ sHyp │-- │
     * │  --  │ Bspc │ Del  │  --  │   │ └─────┴──────┴──────┼──────┼───┤
     * ├──────┼──────┼──────┴──────┴───┘ │  TL  │LSF│       │ Spc  │ TLTR │
     *                           └──────┴───┘       └──────┴──────┘
     */
    [_TL] = LAYOUT_split_2x6_1x5_2(
        // Left side
        KC_NO, KC_ESC, KC_OS_WIN, KC_OS_FN, KC_NO, KC_NO, KC_TRNS, KC_MOD_ALT,
        KC_MOD_CTRL, KC_MOD_SHIFT, KC_MOD_META, KC_NO, KC_NO, KC_NO, KC_NO,
        KC_OS_HYP, KC_NO, KC_TRNS, KC_TRNS,
        // Right side
        KC_PGUF, S(KC_TAB), KC_UP, KC_TAB, KC_NO, KC_NO, KC_PGDF, KC_LEFT,
        KC_DOWN, KC_RGHT, KC_NO, KC_TRNS, KC_NO, KC_BSPC, KC_DEL, KC_NO,
        KC_TRNS, KC_TRNS, KC_TRTLTR_KEY),

    /*
     * TR Layer - Numbers & Symbols (Matches Kanata TR layer)
     *
     * ┌─────┬─────┬─────┬─────┬─────┬───┐ ┌──────┬─────┬─────┬─────┬───┬───┐ │
     * --  │ !/` │ @/~ │ #/^ │ $/₹ │-- │       │ %/F12│ 7/F7│ 8/F8│ 9/F9│ + │ =
     * │ ├─────┼─────┼─────┼─────┼─────┼───┤ ├──────┼─────┼─────┼─────┼───┼───┤
     * │     │ &/| │ [/] │ {/} │ (/) │-- │       │  *   │ 4/F4│ 5/F5│ 6/F6│ - │
     * │ └─────┼─────┼─────┼─────┼─────┼───┤ ├──────┼─────┼─────┼─────┼───┼───┘
     *       │ --  │ --  │  <  │  >  │-- │       │ 0/F10│ 1/F1│ 2/F2│ 3/F3│ / │
     *       └─────┴─────┴─────┼─────┼───┤       ├──────┼─────┼─────┴─────┴───┘
     *                         │TLTR │LSF│       │ Spc  │ TR  │
     *                         └─────┴───┘       └──────┴─────┘
     */
    [_TR] = LAYOUT_split_2x6_1x5_2(
        // Left side
        KC_NO, KC_EXCF, KC_ATSF, KC_OCTF, KC_DOLF, KC_NO, KC_TRNS, KC_AMPF,
        KC_SQRF, KC_CURF, KC_PARF, KC_NO, KC_NO, KC_NO, KC_ANOF, KC_ANCF, KC_NO,
        KC_TLTLTR_KEY, KC_TRNS,
        // Right side
        KC_PERF, KC_7F, KC_8F, KC_9F, KC_PLUF, KC_EQUF, KC_ASTF, KC_4F, KC_5F,
        KC_6F, KC_HPNF, KC_TRNS, KC_0F, KC_1F, KC_2F, KC_3F, KC_SLAF, KC_TRNS,
        KC_TRNS),

    /*
     * TLTR Layer - Mouse, Media & Display Controls (Matches Kanata TLTR layer)
     *
     * ┌─────┬──────┬──────┬─────┬──────┬───┐ ┌───┬───┬────┬───┬──────┬──────┐
     * │ --  │ Scrn │ Bri+ │ Med │ Vol+ │-- │       │-- │-- │ M↑ │-- │  --  │
     * Boot │ ├─────┼──────┼──────┼─────┼──────┼───┤
     * ├───┼───┼────┼───┼──────┼──────┤ │ --  │ MPre │ MSlo │MScr │ Btn1 │Bt2│
     * │-- │M← │ M↓ │M→ │  --  │  --  │ └─────┼──────┼──────┼─────┼──────┼───┤
     * ├───┼───┼────┼───┼──────┼──────┘ │  --  │ Bri- │MPrv │ Vol- │-- │ │-- │--
     * │ -- │-- │  --  │ └──────┴──────┴─────┼──────┼───┤
     * ├───┼───┼────┴───┴──────┘ │  TL  │LSF│       │Spc│TR │ └──────┴───┘
     * └───┴───┘
     */
    [_TLTR] = LAYOUT_split_2x6_1x5_2(
        // Left side
        KC_NO, KC_SCRE, KC_BRIU, KC_MEDC, KC_VOLU, KC_NO, KC_NO, KC_MPRE,
        KC_MSLW, KC_MSCR, MS_BTN1, MS_BTN2, KC_NO, KC_BRID, KC_MPRV, KC_VOLD,
        KC_NO, KC_TRNS, KC_TRNS,
        // Right side
        KC_NO, KC_NO, KC_MUP, KC_NO, KC_NO, QK_BOOT, KC_NO, KC_MLFT, KC_MDN,
        KC_MRGT, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_TRNS,
        KC_TRNS)};

// ============================================================================
// Advanced state tracking
// ============================================================================

// Thumb key state for TL/TR/TLTR layer management
static bool tl_pressed = false;
static bool tr_pressed = false;

// Fn modifier state (matches Kanata nop0)
static bool fn_modifier_active = false;

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
  uint16_t os_win_timer;
  uint16_t os_hyp_timer;

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
} modifier_hold_state = {false, false, false, false, false, 0,     0,
                         false, false, false, false, false, false, false,
                         false, 0,     0,     0,     0};

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

// Timeouts (in milliseconds) - match Kanata config
#define ONESHOT_TIMEOUT 300
#define TAPHOLD_TIMEOUT 200

// ============================================================================
// Helper functions
// ============================================================================

// Check if left shift is active (not any shift - matches Kanata (‹⇧))
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
// Precise takes priority over slow (matches Kanata behavior)
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
        modifier_hold_state.mod_alt_held || modifier_hold_state.mod_ctrl_held ||
        modifier_hold_state.mod_shift_held ||
        modifier_hold_state.mod_meta_held || screen_hold_state.held ||
        media_hold_state.held) {

      // Exclude modifier keys themselves and mouse control keys
      if (keycode != KC_OS_WIN && keycode != KC_OS_HYP && keycode != KC_OS_FN &&
          keycode != KC_MOD_ALT && keycode != KC_MOD_CTRL &&
          keycode != KC_MOD_SHIFT && keycode != KC_MOD_META &&
          keycode != KC_SCRE && keycode != KC_MEDC && keycode != KC_MSLW &&
          keycode != KC_MPRE && keycode != KC_MSCR && keycode != KC_MUP &&
          keycode != KC_MDN && keycode != KC_MLFT && keycode != KC_MRGT) {

        // Mark held modifiers as used
        if (modifier_hold_state.os_win_held)
          modifier_hold_state.os_win_used = true;
        if (modifier_hold_state.os_hyp_held)
          modifier_hold_state.os_hyp_used = true;
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
  // Thumb key layer activation (matches Kanata implementation 3)
  // ========================================================================
  case KC_TL_KEY:
    if (record->event.pressed) {
      tl_pressed = true;
      if (tr_pressed) {
        // Both TL and TR pressed = activate TLTR layer
        layer_on(_TLTR);
      } else {
        // Only TL pressed = activate TL layer
        layer_on(_TL);
      }
    } else {
      tl_pressed = false;
      // Release TLTR if it's active
      if (IS_LAYER_ON(_TLTR)) {
        layer_off(_TLTR);
      }
      // Release TL if TR is not pressed
      if (!tr_pressed) {
        layer_off(_TL);
      } else {
        // Ensure TR layer is active if TR still held
        if (!IS_LAYER_ON(_TR)) {
          layer_on(_TR);
        }
      }
    }
    return false;

  case KC_TR_KEY:
    if (record->event.pressed) {
      tr_pressed = true;
      if (tl_pressed) {
        // Both TL and TR pressed = activate TLTR layer
        layer_on(_TLTR);
      } else {
        // Only TR pressed = activate TR layer
        layer_on(_TR);
      }
    } else {
      tr_pressed = false;
      // Release TLTR if it's active
      if (IS_LAYER_ON(_TLTR)) {
        layer_off(_TLTR);
      }
      // Release TR if TL is not pressed
      if (!tl_pressed) {
        layer_off(_TR);
      } else {
        // Ensure TL layer is active if TL still held
        if (!IS_LAYER_ON(_TL)) {
          layer_on(_TL);
        }
      }
    }
    return false;

  // Special keys for activating TLTR from within TL/TR layers
  case KC_TLTLTR_KEY:
    if (record->event.pressed) {
      layer_on(_TLTR);
    } else {
      layer_off(_TLTR);
    }
    return false;

  case KC_TRTLTR_KEY:
    if (record->event.pressed) {
      layer_on(_TLTR);
    } else {
      layer_off(_TLTR);
    }
    return false;

  // ========================================================================
  // One-shot modifier combinations (matches Kanata)
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

  case KC_OS_FN: // sFn: Function modifier (nop0 in Kanata)
    if (record->event.pressed) {
      fn_modifier_active = true;
      modifier_hold_state.os_fn_held = true;
    } else {
      fn_modifier_active = false;
      modifier_hold_state.os_fn_held = false;
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
  // Mouse control modifiers (matches Kanata nop1/nop2/nop3)
  // ========================================================================
  case KC_MSLW: // nop1 - slow mode
    mouse_slow_mode = record->event.pressed;
    return false;

  case KC_MPRE: // nop2 - precise mode
    mouse_precise_mode = record->event.pressed;
    return false;

  case KC_MSCR: // nop3 - scroll mode
    mouse_scroll_mode = record->event.pressed;
    return false;

  // ========================================================================
  // Directional mouse/scroll keys (matches Kanata m△/m▽/m◅/m▻)
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
  // Unshifted symbols (matches Kanata @hpn, @equ, @plu, @ano, @anc)
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
  // Media and screen controls (tap-hold matching Kanata)
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
  }

  return true;
}
