#pragma once

// Ultrasonic volume classifier. Takes a filtered distance reading (cm, or
// DISTANCE_OUT_OF_RANGE) and the current time, and reports which key (if any)
// should be tapped right now. No Arduino dependencies so pedals/tests/ can
// exercise it on the host; the ranging hardware itself (trigger/echo timing)
// lives in the .ino since it needs interrupts and micros().
//
// Vocabulary: near (volume up) and far (volume down). Unlike the IR tap/hold
// classifier, a zone is not a one-shot gesture: holding a hand in a zone
// repeats a brief key tap every pulseIntervalMs, the same way turning a
// rotary encoder keeps emitting volume ticks for as long as it turns.
//
// Zone transitions use hysteresis so a hand resting near a threshold cannot
// flicker between zones: entering NEAR requires distance <= nearCm, but
// leaving it requires distance > nearReleaseCm (nearReleaseCm > nearCm).
// Symmetrically for FAR with farCm/farReleaseCm (farReleaseCm < farCm).
// Readings beyond maxRangeCm, and a missing echo (DISTANCE_OUT_OF_RANGE),
// are always NONE: nothing in comfortable gesture range means no gesture.
//
//   NONE
//     distance <= nearCm ................................... -> NEAR
//     distance >= farCm .................................... -> FAR
//
//   NEAR (repeats volUpKey)
//     out of range or distance > nearReleaseCm ............. -> NONE
//
//   FAR (repeats volDownKey)
//     out of range or distance < farReleaseCm .............. -> NONE
//
// Within NEAR/FAR, a key is tapped (pressed for pulseOnMs) every
// pulseIntervalMs for as long as the zone holds.

#include <stdint.h>

constexpr uint16_t DISTANCE_OUT_OF_RANGE = 0xFFFF;

enum VolumeZone : uint8_t {
  VOL_NONE,
  VOL_NEAR,
  VOL_FAR,
};

struct VolumeGestureConfig {
  uint16_t nearCm;
  uint16_t nearReleaseCm;
  uint16_t farCm;
  uint16_t farReleaseCm;
  uint16_t maxRangeCm;
  uint32_t pulseIntervalMs;
  uint32_t pulseOnMs;
  uint8_t volUpKey;
  uint8_t volDownKey;
};

struct VolumeGesture {
  VolumeGestureConfig cfg;
  VolumeZone zone;
  uint32_t lastPulseAt;
  bool pulsing;
  uint32_t pulseStartedAt;

  void begin(const VolumeGestureConfig &config, uint32_t now) {
    cfg = config;
    zone = VOL_NONE;
    lastPulseAt = now;
    pulsing = false;
  }

  void update(uint16_t distanceCm, uint32_t now) {
    const bool inRange =
        distanceCm != DISTANCE_OUT_OF_RANGE && distanceCm <= cfg.maxRangeCm;

    switch (zone) {
    case VOL_NONE:
      if (inRange && distanceCm <= cfg.nearCm) {
        zone = VOL_NEAR;
      } else if (inRange && distanceCm >= cfg.farCm) {
        zone = VOL_FAR;
      }
      break;

    case VOL_NEAR:
      if (!inRange || distanceCm > cfg.nearReleaseCm) {
        zone = VOL_NONE;
      }
      break;

    case VOL_FAR:
      if (!inRange || distanceCm < cfg.farReleaseCm) {
        zone = VOL_NONE;
      }
      break;
    }

    if (zone == VOL_NONE) {
      pulsing = false;
      return;
    }

    if (pulsing) {
      if (now - pulseStartedAt >= cfg.pulseOnMs) {
        pulsing = false;
      }
    } else if (now - lastPulseAt >= cfg.pulseIntervalMs) {
      pulsing = true;
      pulseStartedAt = now;
      lastPulseAt = now;
    }
  }

  // Key the volume subsystem wants pressed right now (0 = none).
  uint8_t key() const {
    if (!pulsing) {
      return 0;
    }
    return zone == VOL_NEAR ? cfg.volUpKey : cfg.volDownKey;
  }
};
