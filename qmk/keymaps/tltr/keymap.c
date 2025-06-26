#include QMK_KEYBOARD_H

// Define layer names
enum layer_names {
  _CUSTOM_COLEMAK = 0,
  _TL,
  _TR,
  _TLTR
};

// Define custom keycodes for Kanata-specific behaviors that don't have direct QMK equivalents.
// These will require custom process_record_user functions for full functionality.
enum custom_keycodes {
  KC_DELF = SAFE_RANGE, // Delete Fork
  KC_SLAF,            // Slash Fork
  KC_COMF,            // Comma Fork
  KC_DOTF,            // Dot Fork
  KC_PGUF,            // Page Up Fork
  KC_PGDF,            // Page Down Fork
  KC_EXCF,            // Exclamation Fork
  KC_ATSF,            // At Fork
  KC_OCTF,            // Octothorp Fork
  KC_DOLF,            // Dollar Fork
  KC_AMPF,            // Ampersand Fork
  KC_CURF,            // Curly Bracket Fork
  KC_PARF,            // Parenthesis Fork
  KC_SQRF,            // Square Bracket Fork
  KC_ASTF,            // Asterisk Fork (for F11)
  KC_PERF,            // Percent Fork (for F12)
  KC_HPNN,            // Hyphen Fork
  KC_ANOO,            // Angle Open Fork
  KC_ANCC,            // Angle Close Fork
  KC_1F, KC_2F, KC_3F, KC_4F, KC_5F, KC_6F, KC_7F, KC_8F, KC_9F, KC_0F, // Function key forks
  // KC_SCRE,            // Screen control (removed, using KC_SLEP directly)
  KC_MUP, KC_MDN, KC_MLFT, KC_MRGT, // Mouse movement
  KC_WHUP, KC_WHDN, KC_WHLFT, KC_WHRGT, // Scroll movement
  KC_OS_LSFT,         // One-shot Left Shift
  KC_OS_LALT,         // One-shot Left Alt
  KC_OS_LCTL,         // One-shot Left Control
  KC_OS_LGUI,         // One-shot Left GUI (Meta)
  KC_OS_HYPR,         // One-shot Hyper (Alt+Ctrl+Shift+Meta)
  KC_OS_WIN,          // One-shot Win (Alt+Ctrl+Meta)
  KC_TL_MO,           // Momentary TL layer
  KC_TR_MO,           // Momentary TR layer
  KC_TLTR_MO          // Momentary TLTR layer
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┬───┬───┬───┐       ┌───┬───┬───┬───┬───┬───┐
     * │ Q │ W │ E │ R │ T │   │       │   │ Y │ U │ I │ O │ P │
     * ├───┼───┼───┼───┼───┼───┤       ├───┼───┼───┼───┼───┼───┤
     * │ A │ S │ D │ F │ G │   │       │   │ H │ J │ K │ L │ ; │
     * ├───┼───┼───┼───┼───┘   └───┐   ┌───┘   ├───┼───┼───┼───┤
     * │ Z │ X │ C │ V │ B           │           N │ M │ , │ . │ / │
     * └───┴───┴───┴───┴───┐       ┌───┘       └───┴───┴───┴───┘
     *           ┌───┐     │       │     ┌───┐
     *           │TL │     │       │     │TR │
     *           └───┘     └───┐   ┌───┘ └───┘
     *                         │   │
     *                         └───┘
     */
    [_CUSTOM_COLEMAK] = LAYOUT_split_2x6_1x5_2(
        // Left hand
        KC_CAPS_WORD, KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,
        KC_DELF, KC_A,    KC_R,    KC_S,    KC_T,    KC_G,
        KC_Z,    KC_X,    KC_C,    KC_D,    KC_V,
                                 KC_TL_MO, KC_OS_LSFT, // Left thumb cluster (2 keys)
        // Right hand
        KC_J,    KC_L,    KC_U,    KC_Y,    KC_QUOT, KC_SCLN,
        KC_M,    KC_N,    KC_E,    KC_I,    KC_O,    KC_TRNS, // _ is transparent
        KC_K,    KC_H,    KC_COMF, KC_DOTF, KC_SLAF,
        KC_SPC, KC_TR_MO // Right thumb cluster (2 keys)
    ),

    [_TL] = LAYOUT_split_2x6_1x5_2(
        // Left hand
        KC_NO,   KC_ESC,  KC_OS_WIN, KC_NO,   KC_NO,   KC_NO, // @sFn is nop0, so KC_NO
        KC_TRNS, KC_OS_LALT, KC_OS_LCTL, KC_OS_LSFT, KC_OS_LGUI, KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_OS_HYPR, KC_NO,
                                 KC_TRNS, KC_TRNS, // Left thumb cluster (2 keys)
        // Right hand
        KC_PGUF, KC_BTAB, KC_UP,   KC_TAB,  KC_NO,   KC_NO,
        KC_PGDF, KC_LEFT, KC_DOWN, KC_RIGHT, KC_NO,   KC_TRNS,
        KC_NO,   KC_BSPC, KC_DEL,  KC_NO,   KC_TRNS,
        KC_TRNS, KC_TLTR_MO // Right thumb cluster (2 keys)
    ),

    [_TR] = LAYOUT_split_2x6_1x5_2(
        // Left hand
        KC_NO,   KC_EXCF, KC_ATSF, KC_OCTF, KC_DOLF, KC_NO,
        KC_TRNS, KC_AMPF, KC_SQRF, KC_CURF, KC_PARF, KC_NO,
        KC_NO,   KC_NO,   KC_ANOO, KC_ANCC, KC_NO,
                                 KC_TLTR_MO, KC_TRNS, // Left thumb cluster (2 keys)
        // Right hand
        KC_PERF, KC_7F,   KC_8F,   KC_9F,   KC_PLUS, KC_EQL,
        KC_ASTF, KC_4F,   KC_5F,   KC_6F,   KC_6F,   KC_TRNS,
        KC_0F,   KC_1F,   KC_2F,   KC_3F,   KC_SLAF,
        KC_TRNS, KC_TRNS // Right thumb cluster (2 keys)
    ),

    [_TLTR] = LAYOUT_split_2x6_1x5_2(
        // Left hand
        KC_NO,   KC_SLEP, KC_BRIU, KC_MPLY, KC_VOLU, KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_BTN1, KC_BTN2,
        KC_NO,   KC_BRID, KC_MPRV, KC_VOLD, KC_NO,
                                 KC_NO, KC_TRNS, // Left thumb cluster (2 keys)
        // Right hand
        KC_NO,   KC_NO,   KC_MUP,  KC_NO,   KC_NO,   KC_TRNS, // lrld is transparent
        KC_NO,   KC_MLFT, KC_MDN,  KC_MRGT, KC_NO,   KC_NO,
        KC_NO,   KC_NO,   KC_NO,   KC_NO,   KC_NO,
        KC_TRNS, KC_NO // Right thumb cluster (2 keys)
    )
};

// One-shot modifier state
static bool os_lsft_active = false;
static bool os_lalt_active = false;
static bool os_lctl_active = false;
static bool os_lgui_active = false;

// Function to handle one-shot modifiers
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_OS_LSFT:
            if (record->event.pressed) {
                if (!os_lsft_active) {
                    register_code(KC_LSFT);
                    os_lsft_active = true;
                } else {
                    unregister_code(KC_LSFT);
                    os_lsft_active = false;
                }
            }
            return false;
        case KC_OS_LALT:
            if (record->event.pressed) {
                if (!os_lalt_active) {
                    register_code(KC_LALT);
                    os_lalt_active = true;
                } else {
                    unregister_code(KC_LALT);
                    os_lalt_active = false;
                }
            }
            return false;
        case KC_OS_LCTL:
            if (record->event.pressed) {
                if (!os_lctl_active) {
                    register_code(KC_LCTL);
                    os_lctl_active = true;
                } else {
                    unregister_code(KC_LCTL);
                    os_lctl_active = false;
                }
            }
            return false;
        case KC_OS_LGUI:
            if (record->event.pressed) {
                if (!os_lgui_active) {
                    register_code(KC_LGUI);
                    os_lgui_active = true;
                } else {
                    unregister_code(KC_LGUI);
                    os_lgui_active = false;
                }
            }
            return false;
        case KC_OS_HYPR: // Hyper key (Alt+Ctrl+Shift+Meta)
            if (record->event.pressed) {
                register_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT) | MOD_BIT(KC_LGUI));
            } else {
                unregister_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LSFT) | MOD_BIT(KC_LGUI));
            }
            return false;
        case KC_OS_WIN: // Win key (Alt+Ctrl+Meta)
            if (record->event.pressed) {
                register_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LGUI));
            } else {
                unregister_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LCTL) | MOD_BIT(KC_LGUI));
            }
            return false;
        case KC_TL_MO:
            if (record->event.pressed) {
                layer_on(_TL);
            } else {
                layer_off(_TL);
            }
            return false;
        case KC_TR_MO:
            if (record->event.pressed) {
                layer_on(_TR);
            } else {
                layer_off(_TR);
            }
            return false;
        case KC_TLTR_MO:
            if (record->event.pressed) {
                layer_on(_TLTR);
            } else {
                layer_off(_TLTR);
            }
            return false;
        // Forks - these are simplified. Full Kanata fork logic is complex.
        case KC_DELF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_DEL); // Unshifted ⌦
                } else {
                    tap_code(KC_BSPC); // ⌫
                }
            }
            return false;
        case KC_SLAF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_BSLS); // Unshifted \
                } else {
                    tap_code(KC_SLSH); // /
                }
            }
            return false;
        case KC_COMF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_UNDS); // @und (S-‐)
                } else {
                    tap_code(KC_COMM); // ,
                }
            }
            return false;
        case KC_DOTF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_QUES); // @que (S-/)
                } else {
                    tap_code(KC_DOT); // .
                }
            }
            return false;
        case KC_PGUF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_HOME); // Unshifted ⤒
                } else {
                    tap_code(KC_PGUP); // ⇞
                }
            }
            return false;
        case KC_PGDF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_END); // Unshifted ⤓
                } else {
                    tap_code(KC_PGDN); // ⇟
                }
            }
            return false;
        case KC_EXCF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_GRAVE); // Unshifted ˋ
                } else {
                    tap_code(KC_EXLM); // @exc (S-1)
                }
            }
            return false;
        case KC_ATSF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_TILD); // @til (S-ˋ)
                } else {
                    tap_code(KC_AT); // @ats (S-2)
                }
            }
            return false;
        case KC_OCTF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_CIRC); // @car (S-6)
                } else {
                    tap_code(KC_HASH); // @oct (S-3)
                }
            }
            return false;
        case KC_DOLF:
            if (record->event.pressed) {
                // Unicode for Rupee symbol (₹) is not directly supported by QMK keycodes.
                // This would require a custom macro or string. For now, just send dollar.
                tap_code(KC_DLR); // @dol (S-4)
            }
            return false;
        case KC_AMPF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_PIPE); // @pip (S-\)
                } else {
                    tap_code(KC_AMPR); // @amp (S-7)
                }
            }
            return false;
        case KC_CURF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_RCBR); // @cuc (S-])
                } else {
                    tap_code(KC_LCBR); // @cuo (S-[)
                }
            }
            return false;
        case KC_PARF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_RPRN); // @pac (S-0)
                } else {
                    tap_code(KC_LPRN); // @pao (S-9)
                }
            }
            return false;
        case KC_SQRF:
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_RBRC); // Unshifted ]
                } else {
                    tap_code(KC_LBRC); // [
                }
            }
            return false;
        case KC_ASTF: // Asterisk fork for F11
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_F11);
                } else {
                    tap_code(KC_ASTR); // @ast (S-8)
                }
            }
            return false;
        case KC_PERF: // Percent fork for F12
            if (record->event.pressed) {
                if (get_mods() & MOD_MASK_SHIFT) { // If shift is held
                    tap_code(KC_F12);
                } else {
                    tap_code(KC_PERC); // @per (S-5)
                }
            }
            return false;
        case KC_HPNN: // Hyphen unshifted
            if (record->event.pressed) {
                tap_code(KC_MINS);
            }
            return false;
        case KC_ANOO: // Angle open unshifted
            if (record->event.pressed) {
                tap_code(KC_LT);
            }
            return false;
        case KC_ANCC: // Angle close unshifted
            if (record->event.pressed) {
                tap_code(KC_GT);
            }
            return false;
        // Function key forks (simplified: just send the function key if pressed)
        case KC_1F: if (record->event.pressed) { tap_code(KC_F1); } return false;
        case KC_2F: if (record->event.pressed) { tap_code(KC_F2); } return false;
        case KC_3F: if (record->event.pressed) { tap_code(KC_F3); } return false;
        case KC_4F: if (record->event.pressed) { tap_code(KC_F4); } return false;
        case KC_5F: if (record->event.pressed) { tap_code(KC_F5); } return false;
        case KC_6F: if (record->event.pressed) { tap_code(KC_F6); } return false;
        case KC_7F: if (record->event.pressed) { tap_code(KC_F7); } return false;
        case KC_8F: if (record->event.pressed) { tap_code(KC_F8); } return false;
        case KC_9F: if (record->event.pressed) { tap_code(KC_F9); } return false;
        case KC_0F: if (record->event.pressed) { tap_code(KC_F10); } return false;

        // Mouse and Scroll movements (simplified to basic movement)
        case KC_MUP:    if (record->event.pressed) { mouse_move_on(0, 1); } else { mouse_move_off(); } return false;
        case KC_MDN:    if (record->event.pressed) { mouse_move_on(0, -1); } else { mouse_move_off(); } return false;
        case KC_MLFT:   if (record->event.pressed) { mouse_move_on(-1, 0); } else { mouse_move_off(); } return false;
        case KC_MRGT:   if (record->event.pressed) { mouse_move_on(1, 0); } else { mouse_move_off(); } return false;
        case KC_WHUP:   if (record->event.pressed) { mouse_scroll_vert_on(1); } else { mouse_scroll_off(); } return false;
        case KC_WHDN:   if (record->event.pressed) { mouse_scroll_vert_on(-1); } else { mouse_scroll_off(); } return false;
        case KC_WHLFT:  if (record->event.pressed) { mouse_scroll_horiz_on(-1); } else { mouse_scroll_off(); } return false;
        case KC_WHRGT:  if (record->event.pressed) { mouse_scroll_horiz_on(1); } else { mouse_scroll_off(); } return false;
    }
    return true;
}
