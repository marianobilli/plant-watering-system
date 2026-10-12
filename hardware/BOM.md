# Bill of Materials – 6-plant board

Source of truth: `hardware/kicad/plant_watering.kicad_sch` (regenerate the on-board list with
`kicad-cli sch export bom hardware/kicad/plant_watering.kicad_sch --group-by Value,Footprint --fields "Reference,Value,Footprint,\${QUANTITY}" -o bom.csv`).
Prices are rough USD estimates, not quotes.

## On the PCB

| Qty | Ref | Part | Footprint | Notes | Est. USD |
|----:|-----|------|-----------|-------|---------:|
| 1 | U1 | YD-ESP32-23 (ESP32-S3, N16R8: 16 MB flash / 8 MB PSRAM) | 2×22 pin header, 25.4 mm row spacing | Plugs into female headers; use the **UART** USB-C port for flashing | 8–12 |
| 1 | PS1 | MT3608 boost module, HW-183B (USB-C input, 4 corner holes) | `plant_watering_parts:MT3608_module` | 5 V → 12 V for the pumps. **Set output to 12 V before connecting pumps** | 2–3 |
| 1 | J1 | JST-PH 6-pin header (B6B-PH-K) | `JST_PH_B6B-PH-K_1x06_P2.00mm_Vertical` | Power in from the panel USB-C socket cable: 1 GND, 2 VBUS, 3/4 n.c. (D−/D+), 5 CC1, 6 CC2 | 0.5 |
| 2 | R13, R14 | 5.1 kΩ, 1/4 W | DIN0207 axial | USB-C CC pull-downs (needed: the panel socket brings CC1/CC2 out on the cable) | <1 |
| 6 | Q1–Q6 | IRLZ44N logic-level N-MOSFET | TO-220-3 vertical | Pump low-side switches. Must be **IRLZ**, not IRF | 3–5 |
| 6 | R1–R6 | 220 Ω, 1/4 W | DIN0207 axial | Gate series resistors | <1 |
| 6 | R7–R12 | 10 kΩ, 1/4 W | DIN0207 axial | Gate pull-downs (pumps stay off during boot) | <1 |
| 6 | D1–D6 | 1N5819 Schottky, 40 V 1 A | DO-41 horizontal | Flyback diodes across each pump | 1 |
| 1 | C1 | 470 µF / 25 V electrolytic | radial D8 mm, P3.5 mm | +12 V bulk | 0.5 |
| 1 | C5 | 470 µF / 10 V electrolytic | radial D8 mm, P3.5 mm | +5 V bulk | 0.5 |
| 6 | J3–J8 | JST-XH 4-pin header (B4B-XH-A) | 2.5 mm vertical | Sensor 1–6: GND, VCC (3V3), DIS, OUT | 1 |
| 6 | J9–J14 | JST-XH 2-pin header (B2B-XH-A) | 2.5 mm vertical | Pump 1–6 (+12 V, switched GND) | 1 |
| 1 | J2 | 1×9 female pin header, 2.54 mm, 8.5 mm tall | 2.54 mm vertical socket + 4 M3 holes (`UI_module_OLED_EC11`) | UI module plugs straight in, face up: CON SDA SCL PSH TRA TRB BAK GND VCC | 0.5 |
| 4 | – | M3 × 11 mm brass standoffs (female–female) + 8 M3 × 5 mm screws | – | Hold the UI module over J2. 11 mm = 8.5 mm socket + 2.5 mm pin-header spacer; measure your parts (10 mm + washer also works) | 1–2 |
| 1 | J15 | JST-XH 2-pin header (B2B-XH-A) | 2.5 mm vertical | Optional float switch (closes to GND when reservoir is low) | 0.5 |
| 2 | – | 1×22 female header, 2.54 mm (or 2×22 sockets) | – | Socket for U1 | 1 |

PCB: 143 × 112 mm, 2 layers (fabrication order, ~5 pcs: 5–15 USD at a typical prototype house).

## Off-board (connect with JST-XH housings + crimp pins)

| Qty | Part | Notes | Est. USD |
|----:|------|-------|---------:|
| 6 | Capacitive soil moisture sensor **with DIS pin** (4-pin: GND, VCC, DIS, OUT) | Powered from 3V3 so OUT is ADC-safe. 3-pin sensors will **not** work. Seal PCB edges with 2 coats of clear nail polish | 3 each |
| 6 | 12 V peristaltic pump, ~100 mL/min, ~0.17 A | Only one pump runs at a time | 8–12 each |
| 1 | UI module: SH1106 128×64 I²C OLED + rotary encoder (push) + 2 push buttons | Plugs into J2 and screws onto the board (64.9 × 35 mm, holes 59 × 29 mm). Its 9-pin male header must point out of the back; 3V3 powered | 6–10 |
| 1 | Panel-mount USB-C socket with 6-pin JST-PH 2.0 cable (white, Letel, EAN 5906379381793) | Mounts in the enclosure wall, plugs into J1. **Check its wire order before plugging in** | 2–3 |
| 1 | 5 V / 2 A USB-C charger and cable | Only source of power for the whole board | 5 |
| 1 | Float switch (optional) | Normally open, closes when water is low; leave J15 empty if unused | 3–5 |
| – | 5–6 mm ID opaque silicone tubing, ~10 m | Opaque prevents algae | 10–15 |
| – | Reservoir (below plant level) | Keep below the plants to avoid siphoning | – |
| 7 | JST-XH 2-pin housing (XHP-2) | Pump cables ×6 + float switch | <1 |
| 6 | JST-XH 4-pin housing (XHP-4) | Sensor cables | <1 |
| ~60 | JST-XH crimp contacts (SXH-001T-P0.6) | 38 needed (7×2 + 6×4) plus spares; or use pre-crimped XH leads instead | 2 |
| 1 | Crimp tool for JST-XH/PH (e.g. Engineer PA-09, IWISS SN-28B) | Only needed if crimping your own cables | 15–30 |

## Rough total

| | USD |
|---|---:|
| On-board parts | ~25–35 |
| PCB (5 pcs, per board ~) | ~3–6 |
| 6 sensors + 6 pumps | ~70–90 |
| UI, power, tubing, connectors | ~30–40 |
| **Total (one system)** | **~130–170** |

## Open items to verify before ordering

- **MT3608 mounting holes:** measured: 30 × 17 mm module, hole centres 2 mm from each edge (26 × 13 mm spacing), 1.5 mm drills.
- **USB-C panel cable pinout:** the seller gives no pinout. J1 expects 1 GND, 2 VBUS, 5 CC1, 6 CC2 (3/4 unused). Beep out each PH position to the socket's VBUS/GND/CC pins; re-pin the PH housing (or change J1's pin order) if it differs. CC1/CC2 and D+/D− are interchangeable; only VBUS/GND matter for safety.
- **ESP header pin 21:** some YD-ESP32-23 units wire it as a second 3V3 output. Measure before feeding 5 V into it.
- **Pump current:** sized for one pump at a time (0.17 A @ 12 V ≈ 0.5 A from the 5 V side). Do not enable concurrent pumping.
