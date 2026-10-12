"""Generate YD-ESP32-23.step: a simplified 3D model of the VCC-GND YD-ESP32-S3
DevKitC-1 clone sitting in two 1x22 female sockets.

Every part is a coloured box (see step_writer.py). Coordinates are footprint
coordinates (mm, origin = pad 1, +y towards the USB end, z up from the
carrier PCB top).

Dimensions are estimated from docs/yd_esp32_s3-23.jpg and the pinout image
(pin pitch used as scale). Run: python make_yd_esp32_23.py
"""
import os

from step_writer import Model

PITCH = 2.54
ROW_X = (0.0, 25.4)
PINS = 22

SOCKET_H = 8.5          # female socket body height
SPACER_H = 2.5          # male header plastic spacer
PCB_Z = SOCKET_H + SPACER_H
PCB_T = 1.6
TOP = PCB_Z + PCB_T     # module top surface

COLORS = {
    "black":  (0.06, 0.06, 0.06),
    "pcb":    (0.06, 0.36, 0.16),
    "gold":   (0.86, 0.70, 0.30),
    "silver": (0.60, 0.61, 0.63),
    "module": (0.10, 0.12, 0.16),
    "white":  (0.92, 0.92, 0.90),
    "tant":   (0.80, 0.55, 0.20),
    "grey":   (0.35, 0.35, 0.37),
}

m = Model("YD-ESP32-23", COLORS)

# Female sockets on the carrier board, pins and tails.
y_end = (PINS - 1) * PITCH
for i, x in enumerate(ROW_X):
    m.box(f"socket{i}", "black", x - 1.27, -1.27, 0, x + 1.27, y_end + 1.27, SOCKET_H)
    m.box(f"spacer{i}", "black", x - 1.27, -1.27, SOCKET_H, x + 1.27, y_end + 1.27, PCB_Z)
    for p in range(PINS):
        y = p * PITCH
        m.cbox(f"tail{i}_{p}", "gold", x, y, 0.64, 0.64, -1.6 - 3.0, 3.0)
        m.cbox(f"pin{i}_{p}", "gold", x, y, 0.64, 0.64, TOP, 1.0)

# Module PCB.
m.box("pcb", "pcb", -1.27, -1.5, PCB_Z, 26.67, 55.1, TOP)

# ESP32-S3-WROOM-1: 18 x 25.5 mm, antenna overhangs the pin-1 end.
m.box("wroom_pcb", "module", 12.7 - 9.0, -7.5, TOP, 12.7 + 9.0, 18.0, TOP + 0.8)
m.box("wroom_can", "silver", 12.7 - 8.5, -0.6, TOP + 0.8, 12.7 + 8.5, 17.5, TOP + 3.1)

# Two USB-C receptacles at the far end.
for i, cx in enumerate((6.3, 19.1)):
    m.box(f"usbc{i}", "silver", cx - 4.47, 48.2, TOP, cx + 4.47, 55.6, TOP + 3.26)

# RST / BOOT buttons.
for name, cy in (("rst", 21.6), ("boot", 26.4)):
    m.cbox(name, "silver", 17.4, cy, 4.0, 3.0, TOP, 1.4)
    m.cbox(name + "_cap", "black", 17.4, cy, 1.6, 1.6, TOP + 1.4, 0.5)

# AMS1117 (SOT-223) with tab.
m.box("ldo", "black", 5.0, 23.0, TOP, 8.5, 29.5, TOP + 1.6)
m.box("ldo_tab", "silver", 8.5, 24.5, TOP, 10.2, 28.0, TOP + 0.3)
for k, cy in enumerate((24.0, 26.25, 28.5)):
    m.cbox(f"ldo_lead{k}", "silver", 4.4, cy, 1.2, 0.7, TOP, 0.3)

# Tantalum caps.
m.cbox("c1", "tant", 7.2, 19.7, 3.5, 2.0, TOP, 1.8)
m.cbox("c2", "tant", 7.2, 31.7, 3.5, 2.0, TOP, 1.8)

# WS2812 RGB LED.
m.cbox("rgb", "white", 7.2, 36.5, 5.0, 5.0, TOP, 1.6)

# USB-UART bridge (QFN) and a few SMD diodes.
m.cbox("uart", "black", 20.1, 40.2, 4.0, 4.0, TOP, 0.9)
for k, cx in enumerate((10.5, 12.6, 14.7)):
    m.cbox(f"d{k}", "black", cx, 41.8, 1.6, 3.0, TOP, 1.0)


if __name__ == "__main__":
    m.write(os.path.join(os.path.dirname(os.path.abspath(__file__)), "YD-ESP32-23.step"),
            "make_yd_esp32_23.py")
