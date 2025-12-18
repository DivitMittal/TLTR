#pragma once

// Bootloader configuration
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 1000U

// Split keyboard communication
#define SERIAL_USART_FULL_DUPLEX
#define SERIAL_USART_TX_PIN GP0
#define SERIAL_USART_RX_PIN GP1

// Caps Word configuration
#define CAPS_WORD_IDLE_TIMEOUT 3000 // 3 seconds timeout
#define DOUBLE_TAP_SHIFT_TURNS_ON_CAPS_WORD

// One-shot modifier configuration (matches Kanata os_timeout)
#define ONESHOT_TIMEOUT 300 // 300ms timeout

// Mouse key configuration - Constant mode with three speed levels
// These override the defaults to provide precise control
#define MOUSEKEY_DELAY 0             // No delay before movement starts
#define MOUSEKEY_INTERVAL 16         // Time between mouse reports (ms)
#define MOUSEKEY_MOVE_DELTA 8        // Default: modestly fast
#define MOUSEKEY_WHEEL_DELTA 1       // Default: modestly fast scroll
#define MOUSEKEY_MAX_SPEED 8         // Same as MOVE_DELTA for constant speed
#define MOUSEKEY_TIME_TO_MAX 0       // No acceleration - constant mode
#define MOUSEKEY_WHEEL_MAX_SPEED 8   // Same as initial for constant
#define MOUSEKEY_WHEEL_TIME_TO_MAX 0 // No scroll acceleration

// Tap-hold configuration matching Kanata timeouts
#define TAPPING_TERM 200
#define TAPPING_TERM_PER_KEY
#define PERMISSIVE_HOLD
#define HOLD_ON_OTHER_KEY_PRESS

// Debounce configuration
#define DEBOUNCE 5

// USB polling rate optimization
#define USB_POLLING_INTERVAL_MS 1

// Memory optimizations
#define LAYER_STATE_8BIT // We only have 4 layers

// Disable unused features to save memory and improve performance
#define NO_MUSIC_MODE
#define NO_ACTION_MACRO    // Don't use legacy macros
#define NO_ACTION_FUNCTION // Don't use legacy functions

// Reduce EEPROM usage if not needed
#define EECONFIG_MAGIC_NUMBER 0x0001
#define EECONFIG_MAGIC_NUMBER_OFF offsetof(eeconfig_t, magic)

// Optimize matrix scanning
#define MATRIX_IO_DELAY 30 // Default is good for RP2040

// Reduce debounce algorithm overhead (sym_defer_g is fastest)
#define DEBOUNCE_TYPE sym_defer_g

// Custom feature flags for TLTR implementation
#define TLTR_CUSTOM_MOUSE_KEYS
#define TLTR_CUSTOM_ONESHOTS
#define TLTR_FN_MODIFIER
