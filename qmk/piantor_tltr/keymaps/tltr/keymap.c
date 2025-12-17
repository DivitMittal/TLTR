#include QMK_KEYBOARD_H

// Define layer names
enum layer_names { _COLEMAK = 0, _TL, _TR, _TLTR };

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
  KC_SCRE, // Screen saver/sleep control (tap-hold)
  KC_MEDC, // Media control (play/forward tap-hold)
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * COLEMAK MOD-DH + WIDE + ANGLE (38-key TLTR layout)
     * Matches Kanata CUSTOM_COLEMAK layer
     *
     * ┌─────┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬─────┐
     * │ CW  │ Q │ W │ F │ P │ B │       │ J │ L │ U │ Y │ ' │  ;  │
     * ├─────┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼─────┤
     * │DELF │ A │ R │ S │ T │ G │       │ M │ N │ E │ I │ O │  _  │
     * └─────┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼─────┘
     *       │ Z │ X │ C │ D │ V │       │ K │ H │,/_ │./? │//\ │
     *       └───┴───┴───┼───┼───┤       ├───┼───┼───┴───┴───┘
     *                   │TL │LSF│       │SPC│ TR│
     *                   └───┴───┘       └───┴───┘
     */
    [_COLEMAK] = LAYOUT_split_2x6_1x5_2(
        // Left side
        CW_TOGG, KC_Q, KC_W, KC_F, KC_P, KC_B, KC_DELF, KC_A, KC_R, KC_S, KC_T,
        KC_G, KC_Z, KC_X, KC_C, KC_D, KC_V, KC_TL_KEY, OSM(MOD_LSFT),
        // Right side
        KC_J, KC_L, KC_U, KC_Y, KC_QUOT, KC_SCLN, KC_M, KC_N, KC_E, KC_I, KC_O,
        KC_UNDS, KC_K, KC_H, KC_COMF, KC_DOTF, KC_SLAF, KC_SPC, KC_TR_KEY),

    /*
     * TL Layer - Modifiers & Navigation
     * Matches Kanata TL layer
     *
     * Key mappings:
     * - Esc, sWin, sFn in top left
     * - One-shot modifiers (Alt, Ctrl, Shift, Meta) on home row
     * - Navigation keys (arrows, tab, page up/down) on right
     * - sHyp (Hyper) in bottom left
     */
    [_TL] = LAYOUT_split_2x6_1x5_2(
        // Left side
        KC_NO, KC_ESC, KC_OS_WIN, KC_OS_FN, KC_NO, KC_NO, KC_TRNS,
        OSM(MOD_LALT), OSM(MOD_LCTL), OSM(MOD_LSFT), OSM(MOD_LGUI), KC_NO,
        KC_NO, KC_NO, KC_NO, KC_OS_HYP, KC_NO, KC_TRNS, KC_TRNS,
        // Right side
        KC_PGUF, S(KC_TAB), KC_UP, KC_TAB, KC_NO, KC_NO, KC_PGDF, KC_LEFT,
        KC_DOWN, KC_RGHT, KC_NO, KC_TRNS, KC_NO, KC_BSPC, KC_DEL, KC_NO,
        KC_TRNS, KC_TRNS, KC_TRTLTR_KEY),

    /*
     * TR Layer - Numbers & Symbols
     * Matches Kanata TR layer
     *
     * Layout:
     * - Symbol forks in top left (!@#$)
     * - Function keys with number forks (7f-0f, *, %) on right top
     * - Bracket forks (&, [], {}, ()) on left home
     * - Number forks (4f-6f) on right home
     * - Angle brackets on left bottom
     * - Number forks (0f-3f) on right bottom
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
     * TLTR Layer - Mouse, Media & Display Controls
     * Matches Kanata TLTR layer
     *
     * Features:
     * - Screen/brightness/media controls on left
     * - Mouse mode modifiers (precise, slow, scroll) on left home
     * - Mouse buttons on left home
     * - Directional mouse/scroll on right (with mode switching)
     * - QK_BOOT for reflashing firmware
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

// Mouse control states (matches Kanata nop1/nop2/nop3)
static bool mouse_slow_mode = false;    // nop1
static bool mouse_precise_mode = false; // nop2
static bool mouse_scroll_mode = false;  // nop3

// One-shot state tracking for custom combinations
static struct {
  bool active;
  uint16_t timer;
  uint8_t mods;
} oneshot_state = {false, 0, 0};

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

// ============================================================================
// Mouse control with mode switching (matches Kanata switch logic)
// ============================================================================

// Mouse movement speeds (matches Kanata variables)
#define MOUSE_MOVE_DELTA 8          // $mouse_move
#define SLOW_MOUSE_MOVE_DELTA 4     // $slow_mouse_move
#define PRECISE_MOUSE_MOVE_DELTA 1  // $precise_mouse_move
#define SCROLL_MOVE_DELTA 8         // $scroll_move
#define SLOW_SCROLL_MOVE_DELTA 4    // $slow_scroll_move
#define PRECISE_SCROLL_MOVE_DELTA 1 // $precise_scroll_move

static void handle_mouse_movement(int16_t x, int16_t y) {
  // Match Kanata switch logic:
  // (and nop1 nop2 nop3) → slow precise scroll
  // (and nop1 nop3) → slow scroll
  // (and nop1 nop2) → precise mouse
  // nop1 → slow mouse
  // nop3 → scroll
  // default → mouse

  // Note: QMK mouse keys use tap_code16 for directional movement
  // The speed is controlled by MOUSEKEY config settings
  // For now, this is a simplified implementation that maps to standard QMK
  // mouse keys
  // TODO: Implement variable speed mouse movement using custom mouse report or
  // acceleration keys

  if (mouse_scroll_mode) {
    // Scroll mode: use wheel keys
    if (y < 0)
      tap_code(MS_WHLU); // Scroll up
    else if (y > 0)
      tap_code(MS_WHLD); // Scroll down
    if (x < 0)
      tap_code(MS_WHLL); // Scroll left
    else if (x > 0)
      tap_code(MS_WHLR); // Scroll right
  } else {
    // Mouse mode: use movement keys
    // Note: Speed variations (slow/precise) would need acceleration keys or
    // custom implementation
    if (y < 0)
      tap_code(MS_UP);
    else if (y > 0)
      tap_code(MS_DOWN);
    if (x < 0)
      tap_code(MS_LEFT);
    else if (x > 0)
      tap_code(MS_RGHT);
  }
}

// ============================================================================
// Main key processing
// ============================================================================

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  // Handle one-shot timeout
  if (oneshot_state.active &&
      timer_elapsed(oneshot_state.timer) > ONESHOT_TIMEOUT) {
    unregister_mods(oneshot_state.mods);
    oneshot_state.active = false;
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
      oneshot_state.mods =
          MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LGUI);
      oneshot_state.timer = timer_read();
      oneshot_state.active = true;
      set_oneshot_mods(oneshot_state.mods);
    }
    return false;

  case KC_OS_HYP: // sHyp: Alt+Ctrl+Shift+Meta
    if (record->event.pressed) {
      oneshot_state.mods = MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) |
                           MOD_BIT(KC_LSFT) | MOD_BIT(KC_LGUI);
      oneshot_state.timer = timer_read();
      oneshot_state.active = true;
      set_oneshot_mods(oneshot_state.mods);
    }
    return false;

  case KC_OS_FN: // sFn: Function modifier (nop0 in Kanata)
    if (record->event.pressed) {
      fn_modifier_active = true;
    } else {
      fn_modifier_active = false;
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
    if (record->event.pressed) {
      handle_mouse_movement(0, -1);
    }
    return false;

  case KC_MDN:
    if (record->event.pressed) {
      handle_mouse_movement(0, 1);
    }
    return false;

  case KC_MLFT:
    if (record->event.pressed) {
      handle_mouse_movement(-1, 0);
    }
    return false;

  case KC_MRGT:
    if (record->event.pressed) {
      handle_mouse_movement(1, 0);
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
        tap_code(KC_DEL);
        set_mods(saved_mods);
      } else {
        tap_code(KC_BSPC);
      }
    }
    return false;

  case KC_SLAF: // Slash/Backslash
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code(KC_BSLS);
        set_mods(saved_mods);
      } else {
        tap_code(KC_SLSH);
      }
    }
    return false;

  case KC_COMF: // Comma/Underscore
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Already shifted, output underscore
        tap_code(KC_MINS);
      } else {
        tap_code(KC_COMM);
      }
    }
    return false;

  case KC_DOTF: // Dot/Question
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        // Already shifted, output question mark
        tap_code(KC_SLSH);
      } else {
        tap_code(KC_DOT);
      }
    }
    return false;

  case KC_PGUF: // Page Up/Home
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code(KC_HOME);
        set_mods(saved_mods);
      } else {
        tap_code(KC_PGUP);
      }
    }
    return false;

  case KC_PGDF: // Page Down/End
    if (record->event.pressed) {
      if (is_left_shift_active()) {
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code(KC_END);
        set_mods(saved_mods);
      } else {
        tap_code(KC_PGDN);
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
      // Dollar normally, Rupee when shifted (Unicode)
      // For now, just output dollar - Rupee needs Unicode support
      if (is_left_shift_active()) {
        // TODO: Implement Unicode for Rupee symbol
        uint8_t saved_mods = get_mods();
        clear_shift_mods();
        tap_code16(KC_DLR);
        set_mods(saved_mods);
      } else {
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
      // TODO: Implement tap-hold for screensaver/sleep
      // Tap: screensaver, Hold: sleep
      // This needs platform-specific implementation
    }
    return false;

  case KC_MEDC:
    if (record->event.pressed) {
      // TODO: Implement tap-hold for play/next
      // Tap: play/pause, Hold: next track
      // Simplified version: just play/pause for now
      tap_code(KC_MPLY);
    }
    return false;
  }

  return true;
}
