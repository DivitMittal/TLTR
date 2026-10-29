// Wiring diagram for the RP2040 IR + ultrasonic HID sketch, rendered with
// tscircuit (https://tscircuit.com). This is a schematic of the wiring in
// README.md, not a fabricatable board: `MCU`/`IR`/`US` are generic footprints
// standing in for the Waveshare RP2040-Zero and the two breakout modules.
//
// Regenerate wiring.schematic.svg after editing this file:
//   cd pedals/rp2040-ir-ultrasonic-hid/circuit
//   npx -y tsci export index.circuit.tsx -f schematic-svg -o wiring.schematic.svg

export default () => (
  <board name="rp2040_ir_ultrasonic_wiring">
    <chip
      name="MCU"
      footprint="soic16"
      pinLabels={{
        pin1: "3V3",
        pin2: "GND",
        pin3: "GP27",
        pin4: "GP28",
        pin5: "GP29",
      }}
      schPinArrangement={{
        leftSide: { direction: "top-to-bottom", pins: ["3V3", "GND"] },
        rightSide: {
          direction: "top-to-bottom",
          pins: ["GP27", "GP28", "GP29"],
        },
      }}
    />

    {/* IR obstacle sensor: 3-pin digital out, LM393 comparator + pull-up. */}
    <chip
      name="IR"
      footprint="soic8"
      pinLabels={{ pin1: "VCC", pin2: "GND", pin3: "OUT" }}
      schPinArrangement={{
        leftSide: { direction: "top-to-bottom", pins: ["VCC", "GND"] },
        rightSide: { direction: "top-to-bottom", pins: ["OUT"] },
      }}
    />

    {/* HC-SR04-style ultrasonic breakout, powered at 3.3V (no ECHO level
        shifting needed; see README's "Echo voltage" note). */}
    <chip
      name="US"
      footprint="soic8"
      pinLabels={{ pin1: "VCC", pin2: "TRIG", pin3: "ECHO", pin4: "GND" }}
      schPinArrangement={{
        leftSide: { direction: "top-to-bottom", pins: ["VCC", "GND"] },
        rightSide: { direction: "top-to-bottom", pins: ["TRIG", "ECHO"] },
      }}
    />

    <trace from=".MCU > .3V3" to=".IR > .VCC" />
    <trace from=".MCU > .GND" to=".IR > .GND" />
    <trace from=".MCU > .GP28" to=".IR > .OUT" />

    <trace from=".MCU > .3V3" to=".US > .VCC" />
    <trace from=".MCU > .GND" to=".US > .GND" />
    <trace from=".MCU > .GP29" to=".US > .TRIG" />
    <trace from=".MCU > .GP27" to=".US > .ECHO" />
  </board>
)
