"""Generate UI_module_OLED_EC11.step: a simplified 3D model of the blue 1.3in
OLED + EC11 encoder + BACK/CONFIRM module, plugged into a 1x9 female socket
and held on four M3 brass standoffs.

Built from boxes and cylinders (see step_writer.py). Dimensions come from
docs/oled-rotary-encoder-dimensions.jpg, measured from the module's top-left
corner (u right, v down) and shifted to footprint coordinates (origin = pad 1,
CON). Run: python make_ui_module_oled_ec11.py
"""
import os

from step_writer import Model

W, H = 64.90, 35.03     # module outline
U0, V0 = 11.38, 1.93    # pad 1 position in drawing coordinates
PITCH = 2.54
PINS = 9

SOCKET_H = 8.5
SPACER_H = 2.5
PCB_Z = SOCKET_H + SPACER_H
PCB_T = 1.6
TOP = PCB_Z + PCB_T

COLORS = {
    "pcb":    (0.05, 0.25, 0.70),
    "black":  (0.06, 0.06, 0.06),
    "gold":   (0.86, 0.70, 0.30),
    "brass":  (0.80, 0.65, 0.30),
    "silver": (0.60, 0.61, 0.63),
    "glass":  (0.16, 0.16, 0.18),
    "screen": (0.02, 0.02, 0.03),
    "flex":   (0.70, 0.45, 0.12),
    "white":  (0.92, 0.92, 0.90),
    "knob":   (0.09, 0.09, 0.10),
}

m = Model("UI_module_OLED_EC11", COLORS)


def x(u):
    return u - U0


def y(v):
    return v - V0


def ubox(name, color, u0, v0, u1, v1, z0, z1):
    m.box(name, color, x(u0), y(v0), z0, x(u1), y(v1), z1)


# Female socket on the carrier board, header spacer, pin tips and tails.
x_end = (PINS - 1) * PITCH
m.box("socket", "black", -1.27, -1.27, 0, x_end + 1.27, 1.27, SOCKET_H)
m.box("spacer", "black", -1.27, -1.27, SOCKET_H, x_end + 1.27, 1.27, PCB_Z)
for p in range(PINS):
    m.cbox(f"tail{p}", "gold", p * PITCH, 0, 0.64, 0.64, -1.6 - 3.0, 3.0)
    m.cbox(f"pin{p}", "gold", p * PITCH, 0, 0.64, 0.64, TOP, 1.0)

# Module PCB with the cut-out under the OLED flex at the bottom edge.
NOTCH = (13.0, 30.5, 3.2)  # u0, u1, depth
ubox("pcb", "pcb", 0, 0, W, H - NOTCH[2], PCB_Z, TOP)
ubox("pcb_l", "pcb", 0, H - NOTCH[2], NOTCH[0], H, PCB_Z, TOP)
ubox("pcb_r", "pcb", NOTCH[1], H - NOTCH[2], W, H, PCB_Z, TOP)

# M3 brass standoffs underneath and screw heads on top.
for k, (u, v) in enumerate(((2.87, 2.85), (W - 3.00, 2.90), (2.95, H - 2.97), (W - 2.97, H - 3.15))):
    m.cyl(f"standoff{k}", "brass", x(u), y(v), 2.75, 0, PCB_Z)
    m.cyl(f"screw{k}", "silver", x(u), y(v), 2.75, TOP, 1.8)

# 1.3in OLED glass, active area and the flex folded round the bottom edge.
ubox("oled_glass", "glass", 4.01, 6.05, 39.06, 29.0, TOP, TOP + 1.5)
ubox("oled_screen", "screen", 7.70, 9.98, 35.34, 27.6, TOP + 1.5, TOP + 1.55)
ubox("oled_flex", "flex", 15.5, 29.0, 28.0, H - 0.4, TOP, TOP + 0.2)

# EC11 encoder: body, threaded bushing, knob with a white pointer.
EU, EV = W - 11.61, 17.60
m.cbox("ec11_base", "black", x(EU), y(EV), 12.0, 12.4, TOP, 1.5)
m.cbox("ec11_body", "silver", x(EU), y(EV), 11.7, 12.0, TOP + 1.5, 5.0)
m.cyl("ec11_bushing", "silver", x(EU), y(EV), 3.5, TOP + 6.5, 1.5)
m.cyl("knob", "knob", x(EU), y(EV), 7.5, TOP + 8.0, 14.0)
m.cbox("knob_mark", "white", x(EU), y(EV) - 4.0, 1.0, 5.0, TOP + 22.0, 0.1)

# BACK (top) and CONFIRM (bottom) tactile switches, 6.10 x 3.63.
for name, v0 in (("back", 3.89), ("confirm", H - 7.54)):
    u0 = W - 14.66
    ubox(name, "silver", u0, v0, u0 + 6.10, v0 + 3.63, TOP, TOP + 1.6)
    m.cbox(name + "_btn", "white", x(u0 + 3.05), y(v0 + 1.815), 2.6, 1.4, TOP + 1.6, 0.6)


if __name__ == "__main__":
    m.write(os.path.join(os.path.dirname(os.path.abspath(__file__)), "UI_module_OLED_EC11.step"),
            "make_ui_module_oled_ec11.py")
