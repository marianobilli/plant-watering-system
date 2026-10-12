# Sending the PCB for manufacturing

## Generate the files

```bash
python hardware/fab/export_fab.py
```

The script copies the board to a temp folder, refills the copper pours, runs DRC (it stops if DRC is not clean) and writes two zips to `hardware/fab/`:

| File | Use for | Plated holes |
|------|---------|--------------|
| `plant_watering-gerbers.zip` | PCBWay, JLCPCB and most other makers | As drawn in KiCad (= finished size) |
| `plant_watering-gerbers-prototypy.zip` | [prototypy.com](https://www.prototypy.com) (Satland Prototype, Gdańsk) | Every plated component hole +0.1 mm, rounded up to a drill they stock; formats per their instructions (below) |

Each zip holds the Gerbers (F/B copper, F/B mask, F/B silkscreen with mask subtracted, Edge.Cuts), the Gerber job file and two Excellon drill files (PTH and NPTH). No paste layers: the board is all through-hole. The KiCad project itself is never modified; re-run the script after any board change.

## Why two versions: plated hole size

KiCad drill sizes are **finished** hole sizes, after plating.

- **PCBWay, JLCPCB** and most large makers read them that way: they drill slightly larger on their side so the plated hole comes out at the drawn size (typical tolerance ±0.08 mm). Send the standard zip unchanged.
- **prototypy.com** drills the drawn size and the plating then makes the hole about 0.1 mm smaller. Their order form says: *"Plated holes have smaller diameters, so please increase their sizes in project by 0.1mm."* Without that, the 1.0 mm header holes would finish at ~0.9 mm, too tight for 0.64 mm square pins (0.9 mm across the corners). The prototypy zip grows all 155 plated pad holes by 0.1 mm and rounds up to the next drill in [their stock list](https://www.prototypy.com/t/69,Bore) (0.95 + 0.1 = 1.05 is not stocked, so 1.1). Vias stay at 0.3 mm (no pins go through them) and the non-plated M3 holes stay at 3.2 mm. DRC is still clean with the larger holes, and every pad is at least 0.60 mm bigger than its hole (their minimum is hole + 0.44 mm).

| Hole | Count | Standard | prototypy |
|------|------:|---------:|----------:|
| Vias | 22 | 0.30 | 0.30 |
| Resistors, electrolytic caps | 28 | 0.80 | 0.90 |
| JST-XH 4-pin (sensors) | 24 | 0.95 | 1.10 |
| ESP and UI sockets, JST-XH 2-pin | 69 | 1.00 | 1.10 |
| MOSFETs (TO-220), diodes (DO-41) | 30 | 1.10 | 1.20 |
| MT3608 corner pins | 4 | 1.50 | 1.60 |
| M3 mounting (NPTH) | 11 | 3.20 | 3.20 |

If you use another maker, check whether their drill sizes mean finished or drilled holes. If drilled, use the prototypy zip (or change `PROTOTYPY_EXTRA_MM` in the script).

## prototypy.com file format

Their instructions: [Gerber page](https://www.prototypy.com/t/67,Gerbers) and the [KiCad guide (PDF, 2007)](https://www.prototypy.com/uploads/KiCad-27.08.2007_en.pdf).

| They ask for | prototypy zip | |
|---|---|---|
| Layers: top/bottom copper, top/bottom solder mask, top/bottom silkscreen, board outline | F.Cu, B.Cu, F.Mask, B.Mask, F.Silkscreen, B.Silkscreen, Edge.Cuts | ✓ |
| (Extended) Gerber, absolute coordinates, ASCII | RS-274X, absolute, ASCII. Plain format for older CAM software: no X2 attributes (`--no-x2`), no aperture macros (`--disable-aperture-macros`) | ✓ |
| Gerber in inches, 2:3, leading zeros omitted | **mm, 4.6, leading zeros omitted.** KiCad cannot write inch Gerbers. The format and units are declared in every file (`%FSLAX46Y46*%`, `%MOMM*%`), which all RS-274X readers use. Say so in the order comments if unsure | ≈ |
| Excellon drill: inches, decimal format, absolute origin, no drill sheet (KiCad guide) | Inches, decimal, absolute, no map | ✓ |
| Tools file | Tool table in each `.drl` header, plus `plant_watering-drill_report.txt` listing every tool in mm and inches with hole counts | ✓ |
| One drill file (`Wiertla.drl` in their guide) | Two files: `plant_watering-PTH.drl` (plated) and `plant_watering-NPTH.drl` (11 non-plated M3 holes). Their 2007 guide predates KiCad's NPTH support; separate files tell them which holes must **not** be plated. Mention it in the order comments | ≈ |

### Order comment (paste into the prototypy.com order form)

Only the two points where the files differ from their instructions:

```text
Dzień dobry,

dwie uwagi do plików:

1. Gerbery są w milimetrach (RS-274X, format 4.6), nie w calach - KiCad nie zapisuje
   Gerberów w calach. Jednostki i format są zadeklarowane w nagłówku każdego pliku.

2. Wiercenia są w dwóch plikach Excellon (cale, format dziesiętny):
   - plant_watering-PTH.drl  - otwory metalizowane,
   - plant_watering-NPTH.drl - 11 otworów montażowych 3,2 mm, proszę ich NIE metalizować.

Pozdrawiam
```

## Order settings

| Setting | Value |
|---------|-------|
| Layers | 2 |
| Size | 143 × 112 mm, rounded corners (3 mm radius); standard shape, no milling, no V-cut |
| Thickness | 1.6 mm (prototypy.com offers 1.5 mm: choose that; the board has only through-hole parts, so 0.1 mm thinner changes nothing) |
| Copper | 35 µm (1 oz). Enough: under 1 A total; 0.6 mm traces carry ~1.6 A, the thinnest +12 V segment (0.45 mm) ~1.3 A for 0.17 A of load |
| Holes per board | 188 (177 plated: 22 vias + 155 component holes; 11 non-plated) |
| Hole plating (metalization) | Yes |
| Solder mask | Both sides, green (any colour works; black makes traces hard to inspect) |
| Surface finish | HAL (hot-air leveling, leaded or lead-free) |
| Silkscreen | Top only, white |
| Gold plating, edge plating, castellated / milled holes, depth milling | None |

## Design limits vs. maker capability

| | This board | Typical minimum |
|---|---|---|
| Track width | 0.2 mm (7.9 mil) | 6 mil (0.15 mm) |
| Spacing | 0.2 mm (7.9 mil); copper pours 0.3 mm | 6 mil (0.15 mm) |
| Smallest hole | 0.3 mm (vias) | 0.3 mm |
| Via | 0.6 mm pad / 0.3 mm drill (0.15 mm ring) | 0.3 mm drill |
| Copper to board edge | 0.5 mm | 0.3 mm |

## Before ordering

- Look through the Gerbers in the maker's online viewer (or KiCad GerbView): outline, holes, silkscreen.
- Open items in the [README TODO](../README.md#before-ordering-the-pcb) and the [BOM](../hardware/BOM.md#open-items-to-verify-before-ordering).
