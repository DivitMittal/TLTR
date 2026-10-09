# Which half this build is for; the left half is hardcoded as master.
#   qmk compile -kb beekeeb/piantor_tltr -km tltr -e TLTR_HALF=left
ifeq ($(strip $(TLTR_HALF)),left)
    OPT_DEFS += -DTLTR_HALF_LEFT
else ifneq ($(strip $(TLTR_HALF)),right)
    $(error Set TLTR_HALF=left or TLTR_HALF=right, e.g. qmk compile ... -e TLTR_HALF=left)
endif

# Enable OS detection for automatic Unicode input mode
OS_DETECTION_ENABLE = yes

# Enable Unicode support
UNICODE_ENABLE = yes

# Enable caps word
CAPS_WORD_ENABLE = yes

# Enable tap dance for pedal-style double-tap chords
TAP_DANCE_ENABLE = yes

# Enable mouse keys
MOUSEKEY_ENABLE = yes

# Enable media keys
EXTRAKEY_ENABLE = yes

# Performance optimizations
LTO_ENABLE = yes           # Link Time Optimization - smaller/faster code
CONSOLE_ENABLE = no        # Disable console for performance
COMMAND_ENABLE = no        # Disable command feature
