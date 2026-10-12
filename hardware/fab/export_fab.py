"""Export fabrication files (Gerbers + Excellon drill) for the plant_watering PCB.

Produces two zips in this folder:
  plant_watering-gerbers.zip            drill sizes = finished hole sizes (PCBWay, JLCPCB, ...)
  plant_watering-gerbers-prototypy.zip  for prototypy.com (Satland Prototype), see docs/MANUFACTURING.md:
                                        plated pad holes +0.1 mm rounded up to a drill they stock,
                                        plain RS-274X Gerbers (no X2 attributes, no aperture macros),
                                        Excellon in inches plus a drill report (tool list)

The board is copied to a temp folder first, so the KiCad project is never modified
(kicad-cli rewrites the .kicad_pro it finds next to the board).

Run: python hardware/fab/export_fab.py   (set KICAD_CLI if kicad-cli is not in the default place)
"""
import os
import re
import shutil
import subprocess
import sys
import tempfile
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
KICAD_DIR = os.path.join(HERE, "..", "kicad")
NAME = "plant_watering"
KICAD_CLI = os.environ.get("KICAD_CLI", r"C:\Program Files\KiCad\10.0\bin\kicad-cli.exe")
LAYERS = "F.Cu,B.Cu,F.Mask,B.Mask,F.Silkscreen,B.Silkscreen,Edge.Cuts"

# prototypy.com: plating makes plated holes ~0.1 mm smaller than drilled, and they only
# stock these drills (https://www.prototypy.com/t/69,Bore); larger holes are milled.
PROTOTYPY_EXTRA_MM = 0.1
PROTOTYPY_DRILLS = (0.3, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95,
                    1.0, 1.1, 1.2, 1.25, 1.3, 1.35, 1.4, 1.5, 1.55, 1.6, 1.7, 1.75, 1.8, 1.85,
                    1.9, 2.0, 2.25, 2.3, 2.5, 2.7, 3.0, 3.2, 3.3, 3.55, 3.7, 3.8, 4.0, 4.2,
                    4.3, 4.5, 4.7, 5.0)
PROTOTYPY_MIN_RING_MM = 0.44  # their minimum: pad diameter = hole + 0.44

VARIANTS = {
    "standard": dict(zip=f"{NAME}-gerbers.zip", extra=0.0, drills=None,
                     gerber_opts=(), drill_opts=("--excellon-units", "mm")),
    "prototypy": dict(zip=f"{NAME}-gerbers-prototypy.zip", extra=PROTOTYPY_EXTRA_MM,
                      drills=PROTOTYPY_DRILLS,
                      gerber_opts=("--no-x2", "--disable-aperture-macros"),
                      drill_opts=("--excellon-units", "in", "--generate-report")),
}


def cli(*args):
    r = subprocess.run([KICAD_CLI, *args], capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(f"kicad-cli {' '.join(args)} failed:\n{r.stdout}{r.stderr}")
    return r.stdout


def enlarge_plated_holes(board, extra, drills):
    """Grow the drill of every plated pad (thru_hole) by `extra`, rounded up to the next
    available drill. Vias and NPTH stay as they are. Returns {old: new} and the smallest
    copper ring (pad minus hole, mm)."""
    s = open(board, newline="", encoding="utf-8").read()
    changes, rings = {}, []

    def grow(m):
        old = float(m.group(3))
        new = next(d for d in drills if d >= round(old + extra, 3))
        changes[old] = new
        rings.append(min(float(v) for v in m.group(2).split()) - new)
        return f"{m.group(1)}{new:g})"

    s = re.sub(r'(\(pad "[^"]*" thru_hole [\s\S]*?\(size ([\d. ]+)\)[\s\S]*?\(drill )([\d.]+)\)', grow, s)
    open(board, "w", newline="", encoding="utf-8").write(s)
    return changes, len(rings), min(rings)


def export(variant, zip, extra, drills, gerber_opts, drill_opts):
    with tempfile.TemporaryDirectory() as tmp:
        for f in (NAME + ".kicad_pcb", NAME + ".kicad_pro", "fp-lib-table"):
            shutil.copy(os.path.join(KICAD_DIR, f), tmp)
        shutil.copytree(os.path.join(KICAD_DIR, "plant_watering_parts.pretty"),
                        os.path.join(tmp, "plant_watering_parts.pretty"))
        board = os.path.join(tmp, NAME + ".kicad_pcb")
        if extra:
            changes, n, ring = enlarge_plated_holes(board, extra, drills)
            print(f"[{variant}] {n} plated pad holes: " +
                  ", ".join(f"{o:g}->{v:g}" for o, v in sorted(changes.items())) + " mm")
            if ring < PROTOTYPY_MIN_RING_MM - 1e-9:
                sys.exit(f"[{variant}] smallest pad ring {ring:.2f} mm < {PROTOTYPY_MIN_RING_MM} mm")
            print(f"[{variant}] smallest pad minus hole: {ring:.2f} mm (min {PROTOTYPY_MIN_RING_MM})")

        drc = os.path.join(tmp, "drc.rpt")
        cli("pcb", "drc", "--refill-zones", "--save-board", "-o", drc, board)
        report = open(drc, encoding="utf-8").read()
        # Enlarged holes make every footprint differ from its library copy; that one is expected.
        allowed = {"lib_footprint_mismatch"} if extra else set()
        violations = [v for v in re.findall(r"^\[(\w+)\]", report, re.M) if v not in allowed]
        if violations or "** Found 0 unconnected pads **" not in report:
            sys.exit(f"[{variant}] DRC not clean, see report:\n{report}")
        print(f"[{variant}] DRC clean")

        gerbers = os.path.join(tmp, "gerbers")
        os.makedirs(gerbers)
        cli("pcb", "export", "gerbers", "--check-zones", "--subtract-soldermask", *gerber_opts,
            "-l", LAYERS, "-o", gerbers + os.sep, board)
        cli("pcb", "export", "drill", "--format", "excellon", "--excellon-separate-th", *drill_opts,
            "--report-path", os.path.join(gerbers, NAME + "-drill_report.txt"),
            "-o", gerbers + os.sep, board)

        zip_path = os.path.join(HERE, zip)
        with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
            for f in sorted(os.listdir(gerbers)):
                z.write(os.path.join(gerbers, f), f)
        holes = {}
        for f in sorted(os.listdir(gerbers)):
            if f.endswith(".drl"):
                tools, cur = {}, None
                text = open(os.path.join(gerbers, f)).read()
                scale = 25.4 if re.search(r"^INCH", text, re.M) else 1.0
                for line in text.splitlines():
                    if m := re.match(r"(T\d+)C([\d.]+)", line):
                        tools[m.group(1)] = (f"{float(m.group(2)) * scale:.3f}", 0)
                    elif re.fullmatch(r"T\d+\s*", line):
                        cur = line.strip()
                    elif line.startswith("X") and cur in tools:
                        d, n = tools[cur]
                        tools[cur] = (d, n + 1)
                holes[f] = sorted(tools.values(), key=lambda t: float(t[0]))
        print(f"[{variant}] wrote {zip_path}")
        for f, tools in holes.items():
            print(f"    {f}: " + ", ".join(f"{n}x {d} mm" for d, n in tools))


if __name__ == "__main__":
    for name, opts in VARIANTS.items():
        export(name, **opts)
