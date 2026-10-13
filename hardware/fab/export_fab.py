"""Export fabrication files (Gerbers + Excellon drill) for the plant_watering PCB.

Writes plant_watering-gerbers.zip to this folder (drill sizes = finished hole sizes, as
PCBWay, JLCPCB and most makers expect); see docs/MANUFACTURING.md.

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

ZIP = f"{NAME}-gerbers.zip"


def cli(*args):
    r = subprocess.run([KICAD_CLI, *args], capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit(f"kicad-cli {' '.join(args)} failed:\n{r.stdout}{r.stderr}")
    return r.stdout


def export():
    with tempfile.TemporaryDirectory() as tmp:
        for f in (NAME + ".kicad_pcb", NAME + ".kicad_pro", "fp-lib-table"):
            shutil.copy(os.path.join(KICAD_DIR, f), tmp)
        shutil.copytree(os.path.join(KICAD_DIR, "plant_watering_parts.pretty"),
                        os.path.join(tmp, "plant_watering_parts.pretty"))
        board = os.path.join(tmp, NAME + ".kicad_pcb")
        drc = os.path.join(tmp, "drc.rpt")
        cli("pcb", "drc", "--refill-zones", "--save-board", "-o", drc, board)
        report = open(drc, encoding="utf-8").read()
        violations = re.findall(r"^\[(\w+)\]", report, re.M)
        if violations or "** Found 0 unconnected pads **" not in report:
            sys.exit(f"DRC not clean, see report:\n{report}")
        print("DRC clean")

        gerbers = os.path.join(tmp, "gerbers")
        os.makedirs(gerbers)
        cli("pcb", "export", "gerbers", "--check-zones", "--subtract-soldermask",
            "-l", LAYERS, "-o", gerbers + os.sep, board)
        cli("pcb", "export", "drill", "--format", "excellon", "--excellon-separate-th",
            "--excellon-units", "mm",
            "--report-path", os.path.join(gerbers, NAME + "-drill_report.txt"),
            "-o", gerbers + os.sep, board)

        zip_path = os.path.join(HERE, ZIP)
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
        print(f"wrote {zip_path}")
        for f, tools in holes.items():
            print(f"    {f}: " + ", ".join(f"{n}x {d} mm" for d, n in tools))


if __name__ == "__main__":
    export()
