"""Export fabrication files (Gerbers + Excellon drill) for the plant_watering PCB.

Produces two zips in this folder:
  plant_watering-gerbers.zip            drill sizes = finished hole sizes (PCBWay, JLCPCB, ...)
  plant_watering-gerbers-prototypy.zip  plated component holes +0.1 mm (prototypy.com: their
                                        plating eats into the drilled hole, see docs/MANUFACTURING.md)

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
PROTOTYPY_EXTRA_MM = 0.1


def cli(*args):
    r = subprocess.run([KICAD_CLI, *args], capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(f"kicad-cli {' '.join(args)} failed:\n{r.stdout}{r.stderr}")
    return r.stdout


def enlarge_plated_holes(board, extra):
    """Grow the drill of every plated pad (thru_hole). Vias and NPTH stay as they are."""
    s = open(board, newline="", encoding="utf-8").read()
    count = 0

    def grow(m):
        nonlocal count
        count += 1
        return f"{m.group(1)}{float(m.group(2)) + extra:g})"

    s = re.sub(r'(\(pad "[^"]*" thru_hole [\s\S]*?\(drill )([\d.]+)\)', grow, s)
    open(board, "w", newline="", encoding="utf-8").write(s)
    return count


def export(variant, zip_name, extra=0.0):
    with tempfile.TemporaryDirectory() as tmp:
        for f in (NAME + ".kicad_pcb", NAME + ".kicad_pro", "fp-lib-table"):
            shutil.copy(os.path.join(KICAD_DIR, f), tmp)
        shutil.copytree(os.path.join(KICAD_DIR, "plant_watering_parts.pretty"),
                        os.path.join(tmp, "plant_watering_parts.pretty"))
        board = os.path.join(tmp, NAME + ".kicad_pcb")
        if extra:
            print(f"[{variant}] enlarged {enlarge_plated_holes(board, extra)} plated pad holes by {extra} mm")

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
        cli("pcb", "export", "gerbers", "--check-zones", "--subtract-soldermask",
            "-l", LAYERS, "-o", gerbers + os.sep, board)
        cli("pcb", "export", "drill", "--format", "excellon", "--excellon-units", "mm",
            "--excellon-separate-th", "-o", gerbers + os.sep, board)

        zip_path = os.path.join(HERE, zip_name)
        with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED) as z:
            for f in sorted(os.listdir(gerbers)):
                z.write(os.path.join(gerbers, f), f)
        holes = {}
        for f in sorted(os.listdir(gerbers)):
            if f.endswith(".drl"):
                tools, cur = {}, None
                for line in open(os.path.join(gerbers, f)):
                    if m := re.match(r"(T\d+)C([\d.]+)", line):
                        tools[m.group(1)] = (m.group(2), 0)
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
    export("standard", f"{NAME}-gerbers.zip")
    export("prototypy", f"{NAME}-gerbers-prototypy.zip", PROTOTYPY_EXTRA_MM)
