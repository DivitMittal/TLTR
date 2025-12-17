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

// Mouse key configuration with kinetic speed
#define MK_KINETIC_SPEED
#define MOUSEKEY_DELAY 0
#define MOUSEKEY_INTERVAL 16
#define MOUSEKEY_MOVE_DELTA 5
#define MOUSEKEY_INITIAL_SPEED 100
#define MOUSEKEY_BASE_SPEED 1000
#define MOUSEKEY_DECELERATED_SPEED 400
#define MOUSEKEY_ACCELERATED_SPEED 3000

// Scroll configuration (matches Kanata $scroll_move)
#define MOUSEKEY_WHEEL_DELAY 0
#define MOUSEKEY_WHEEL_INTERVAL 80
#define MOUSEKEY_WHEEL_MAX_SPEED 8
#define MOUSEKEY_WHEEL_TIME_TO_MAX 40

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

// Disable unused features to save memory
#define NO_MUSIC_MODE

// Custom feature flags for TLTR implementation
#define TLTR_CUSTOM_MOUSE_KEYS
#define TLTR_CUSTOM_ONESHOTS
#define TLTR_FN_MODIFIER
