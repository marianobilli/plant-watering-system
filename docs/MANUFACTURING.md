# Sending the PCB for manufacturing

## Generate the files

```bash
python hardware/fab/export_fab.py
```

The script copies the board to a temp folder, refills the copper pours, runs DRC (it stops if DRC is not clean) and writes `hardware/fab/plant_watering-gerbers.zip`.

The zip holds the Gerbers (F/B copper, F/B mask, F/B silkscreen with mask subtracted, Edge.Cuts), the Gerber job file and two Excellon drill files in mm (PTH and NPTH). No paste layers: the board is all through-hole. The KiCad project itself is never modified; re-run the script after any board change.

## Plated hole size

KiCad drill sizes are **finished** hole sizes, after plating. PCBWay, JLCPCB and most large makers read them that way: they drill slightly larger on their side so the plated hole comes out at the drawn size (typical tolerance ±0.08 mm).

| Hole | Count | Size (mm) |
|------|------:|----------:|
| Vias | 22 | 0.30 |
| Resistors, electrolytic caps | 28 | 0.80 |
| JST-XH 4-pin (sensors) | 24 | 0.95 |
| ESP and UI sockets, JST-XH 2-pin | 69 | 1.00 |
| MOSFETs (TO-220), diodes (DO-41) | 30 | 1.10 |
| MT3608 corner pins | 4 | 1.50 |
| M3 mounting (NPTH) | 11 | 3.20 |

If you use a maker whose drill sizes mean drilled (not finished) holes, enlarge the plated holes by their plating allowance (typically +0.1 mm) first; otherwise the 1.0 mm header holes finish too tight for 0.64 mm square pins.

## Order settings

| Setting | Value |
|---------|-------|
| Layers | 2 |
| Size | 143 × 112 mm, rounded corners (3 mm radius); standard shape, no milling, no V-cut |
| Thickness | 1.6 mm |
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
