"""Generate MT3608_module.step: a simplified 3D model of the HW-183B MT3608
boost module (30 x 17 mm, USB-C input) soldered on four corner header pins.

Built from boxes and cylinders (see step_writer.py). Footprint coordinates:
origin = module centre, VIN side (and USB-C) at -x, pads 26 x 13 mm apart.
Part positions are approximate. Run: python make_mt3608_module.py
"""
import os

from step_writer import Model

L, W = 30.0, 17.0
PADS = ((-13, -6.5), (-13, 6.5), (13, -6.5), (13, 6.5))

SPACER_H = 2.5          # header pin plastic spacer under the module
PCB_T = 1.6
TOP = SPACER_H + PCB_T

COLORS = {
    "pcb":    (0.05, 0.25, 0.70),
    "black":  (0.06, 0.06, 0.06),
    "gold":   (0.86, 0.70, 0.30),
    "silver": (0.60, 0.61, 0.63),
    "pot":    (0.15, 0.35, 0.80),
    "brass":  (0.80, 0.65, 0.30),
    "cap":    (0.65, 0.55, 0.40),
    "inductor": (0.20, 0.20, 0.22),
}

m = Model("MT3608_module", COLORS)

# Module PCB on four header pins with plastic spacers.
m.box("pcb", "pcb", -L / 2, -W / 2, SPACER_H, L / 2, W / 2, TOP)
for k, (px, py) in enumerate(PADS):
    m.cbox(f"spacer{k}", "black", px, py, 2.54, 2.54, 0, SPACER_H)
    m.cbox(f"tail{k}", "gold", px, py, 0.64, 0.64, -1.6 - 3.0, 3.0)
    m.cbox(f"pin{k}", "gold", px, py, 0.64, 0.64, TOP, 1.0)

# USB-C input overhanging the VIN edge.
m.box("usbc", "silver", -L / 2 - 0.5, -4.47, TOP, -L / 2 + 6.85, 4.47, TOP + 3.26)

# 3296W multi-turn trimmer with its brass adjustment screw on top.
m.cbox("pot", "pot", 3.0, -5.4, 9.5, 4.8, TOP, 10.0)
m.cyl("pot_screw", "brass", 6.5, -5.4, 1.1, TOP + 10.0, 0.8)

# CD54 inductor, MT3608 (SOT-23-6), SS34 diode, input/output caps.
m.cbox("inductor", "inductor", -3.0, 3.5, 5.8, 5.2, TOP, 4.5)
m.cbox("mt3608", "black", 2.5, 2.0, 3.0, 1.6, TOP, 1.1)
m.cbox("diode", "black", 7.8, 4.0, 4.3, 2.6, TOP, 2.2)
m.cbox("c_in", "cap", -6.5, -5.5, 1.6, 3.2, TOP, 1.2)
m.cbox("c_out", "cap", 10.5, 0.0, 1.6, 3.2, TOP, 1.2)


if __name__ == "__main__":
    m.write(os.path.join(os.path.dirname(os.path.abspath(__file__)), "MT3608_module.step"),
            "make_mt3608_module.py")
