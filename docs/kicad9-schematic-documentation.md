# KiCad 9 Schematic File Format Documentation

> Reference documentation for writing KiCad 9 schematic files (.kicad_sch)
> Compiled from official KiCad developer documentation (dev-docs.kicad.org)

---

## Table of Contents

1. [S-Expression Format Overview](#s-expression-format-overview)
2. [Schematic File Structure](#schematic-file-structure)
3. [Header Section](#header-section)
4. [Unique Identifier Section](#unique-identifier-section)
5. [Library Symbol Section](#library-symbol-section)
6. [Schematic Elements](#schematic-elements)
7. [Labels](#labels)
8. [Symbol Section](#symbol-section)
9. [Hierarchical Sheets](#hierarchical-sheets)
10. [Common Syntax Elements](#common-syntax-elements)
11. [Practical Examples](#practical-examples)

---

## S-Expression Format Overview

KiCad uses an s-expression file format for symbol libraries, footprint libraries, schematics, printed circuit boards, and worksheets.

### Syntax Rules

- Syntax is based on the Specctra DSN file format
- Token definitions are delimited by opening `(` and closing `)` parenthesis
- All tokens are lowercase
- Tokens cannot contain whitespace or special characters (except underscore `_`)
- All strings are quoted using double quotes (`"`) and are UTF-8 encoded
- Tokens can have zero or more attributes
- Human readability is a design goal

### Notation Conventions

- Token attributes are UPPER CASE descriptive names (e.g., `(at X Y)`)
- Limited attribute values separated by `|` (e.g., `(visible yes|no)`)
- Optional attributes enclosed in square brackets (e.g., `[portrait]`)

### Coordinates and Sizes

- All values are in **millimeters**
- Exponential floating point values are not used
- All coordinates are relative to the origin of their containing object
- Maximum resolution: 6 decimal places (0.000001 mm)

---

## Schematic File Structure

Schematic files use the `.kicad_sch` extension and include these sections:

1. Header
2. Unique Identifier (UUID)
3. Page Settings
4. Title Block
5. Library Symbol Definitions
6. Junctions
7. No Connects
8. Wires and Buses
9. Images
10. Graphical Lines
11. Graphical Text
12. Local Labels
13. Global Labels
14. Hierarchical Labels
15. Symbols
16. Hierarchical Sheets
17. Root Sheet Instance

---

## Header Section

The `kicad_sch` token indicates a KiCad schematic file. This section is **required**.

```lisp
(kicad_sch
  (version VERSION)                    ; YYYYMMDD date format
  (generator GENERATOR)                ; Program that wrote the file

  ;; contents of the schematic file...
)
```

**Important:** Third party scripts should NOT use `eeschema` as the generator identifier.

**Example:**
```lisp
(kicad_sch
  (version 20231120)
  (generator "my_tool")
  ;; schematic contents...
)
```

---

## Unique Identifier Section

The `uuid` token defines a globally unique identifier for the schematic.

```lisp
(uuid "00000000-0000-0000-0000-000000000000")
```

- Uses Version 4 (random) UUID format
- Generated using mt19937 Mersenne Twister algorithm
- Root schematic UUID is used as the virtual root sheet identifier

---

## Library Symbol Section

The `lib_symbols` token contains all symbols used in the schematic.

```lisp
(lib_symbols
  (symbol "LIBRARY:SYMBOL_NAME"
    ;; symbol definition...
  )
  ;; more symbols...
)
```

---

## Schematic Elements

### Junction Section

Junctions connect crossing wires.

```lisp
(junction
  (at X Y)                             ; Position coordinates
  (diameter DIAMETER)                  ; 0 = default system diameter
  (color R G B A)                      ; 0 0 0 0 = default color
  (uuid "...")                         ; Unique identifier
)
```

### No Connect Section

No connects mark intentionally unconnected pins.

```lisp
(no_connect
  (at X Y)                             ; Position coordinates
  (uuid "...")                         ; Unique identifier
)
```

### Bus Entry Section

Bus entries provide graphical connection points.

```lisp
(bus_entry
  (at X Y)                             ; Position coordinates
  (size X Y)                           ; Distance to end point
  (stroke
    (width WIDTH)
    (type TYPE)                        ; dash|dash_dot|dash_dot_dot|dot|default|solid
    (color R G B A)
  )
  (uuid "...")
)
```

### Wire and Bus Section

Wires and buses define electrical connections.

```lisp
(wire
  (pts
    (xy X1 Y1)
    (xy X2 Y2)
  )
  (stroke
    (width WIDTH)
    (type TYPE)
    (color R G B A)
  )
  (uuid "...")
)

(bus
  (pts
    (xy X1 Y1)
    (xy X2 Y2)
  )
  (stroke ...)
  (uuid "...")
)
```

### Graphical Line (Polyline)

```lisp
(polyline
  (pts
    (xy X1 Y1)
    (xy X2 Y2)
    ;; additional points...
  )
  (stroke
    (width WIDTH)
    (type TYPE)
    (color R G B A)
  )
  (uuid "...")
)
```

### Graphical Text

```lisp
(text
  "TEXT_CONTENT"
  (at X Y ANGLE)                       ; Position and rotation
  (effects
    (font
      (size HEIGHT WIDTH)
      (thickness THICKNESS)
      [bold]
      [italic]
    )
    (justify [left|right] [top|bottom] [mirror])
    [hide]
  )
  (uuid "...")
)
```

---

## Labels

### Local Label

Connects items within the **same sheet only**.

```lisp
(label
  "LABEL_TEXT"
  (at X Y ANGLE)
  (effects
    (font (size H W))
    (justify ...)
  )
  (uuid "...")
)
```

### Global Label

Connects items across **all sheets** in the design.

```lisp
(global_label
  "LABEL_TEXT"
  (shape input|output|bidirectional|tri_state|passive)
  [fields_autoplaced]
  (at X Y ANGLE)
  (effects
    (font (size H W))
    (justify ...)
  )
  (uuid "...")
  (property "Intersheetref" "VALUE"
    (at X Y ANGLE)
    (effects ...)
  )
)
```

**Label Shapes:**

| Token | Description |
|-------|-------------|
| `input` | Input shape |
| `output` | Output shape |
| `bidirectional` | Bidirectional shape |
| `tri_state` | Tri-state shape |
| `passive` | Passive shape |

### Hierarchical Label

Used to create connections between sheets in hierarchical designs.

```lisp
(hierarchical_label
  "LABEL_TEXT"
  (shape input|output|bidirectional|tri_state|passive)
  (at X Y ANGLE)
  (effects
    (font (size H W))
    (justify ...)
  )
  (uuid "...")
)
```

---

## Symbol Section

Defines an instance of a symbol from the library symbol section.

```lisp
(symbol
  "LIBRARY:SYMBOL_NAME"                ; Library identifier
  (at X Y ANGLE)                       ; Position and rotation
  (unit UNIT_NUMBER)                   ; Unit for multi-unit symbols
  (in_bom yes|no)                      ; Include in BOM
  (on_board yes|no)                    ; Export footprint to board
  (uuid "...")

  ;; Properties
  (property "Reference" "R1"
    (at X Y ANGLE)
    (effects (font (size H W)))
  )
  (property "Value" "10k"
    (at X Y ANGLE)
    (effects (font (size H W)))
  )
  (property "Footprint" "Resistor_SMD:R_0603_1608Metric"
    (at X Y ANGLE)
    (effects (font (size H W)) hide)
  )
  (property "Datasheet" "~"
    (at X Y ANGLE)
    (effects (font (size H W)) hide)
  )

  ;; Pin UUIDs
  (pin "1" (uuid "..."))
  (pin "2" (uuid "..."))

  ;; Instances
  (instances
    (project "PROJECT_NAME"
      (path "/ROOT_UUID"
        (reference "R1")
        (unit 1)
      )
    )
  )
)
```

---

## Hierarchical Sheets

### Sheet Definition

```lisp
(sheet
  (at X Y)                             ; Position
  (size WIDTH HEIGHT)                  ; Sheet dimensions
  [fields_autoplaced]
  (stroke
    (width WIDTH)
    (type TYPE)
    (color R G B A)
  )
  (fill (color R G B A))
  (uuid "...")

  ;; Sheet name property (mandatory)
  (property "Sheetname" "SubSheet"
    (at X Y ANGLE)
    (effects (font (size H W)))
  )

  ;; Sheet filename property (mandatory)
  (property "Sheetfile" "subsheet.kicad_sch"
    (at X Y ANGLE)
    (effects (font (size H W)))
  )

  ;; Hierarchical pins
  (pin "PIN_NAME" input|output|bidirectional|tri_state|passive
    (at X Y ANGLE)
    (effects (font (size H W)))
    (uuid "...")
  )

  ;; Instances
  (instances
    (project "PROJECT_NAME"
      (path "/PATH"
        (page "PAGE_NUMBER")
      )
    )
  )
)
```

### Hierarchical Sheet Pin

Maps hierarchical labels from the associated schematic file.

```lisp
(pin
  "PIN_NAME"
  input|output|bidirectional|tri_state|passive
  (at X Y ANGLE)
  (effects
    (font (size H W))
  )
  (uuid "...")
)
```

---

## Common Syntax Elements

### Position Identifier

```lisp
(at X Y [ANGLE])
```
- X: horizontal position (mm)
- Y: vertical position (mm)
- ANGLE: rotation angle (degrees, optional)

**Note:** Symbol text angles are stored in tenths of a degree; all others in degrees.

### Coordinate Point List

```lisp
(pts
  (xy X1 Y1)
  (xy X2 Y2)
  ;; more points...
)
```

### Stroke Definition

```lisp
(stroke
  (width WIDTH)                        ; Line width in mm
  (type TYPE)                          ; Line style
  (color R G B A)                      ; RGBA color values
)
```

**Valid stroke types:**
- `dash`
- `dash_dot`
- `dash_dot_dot` (from version 7)
- `dot`
- `default`
- `solid`

### Text Effects

```lisp
(effects
  (font
    [(face "FONT_NAME")]               ; TrueType font or "KiCad Font"
    (size HEIGHT WIDTH)                ; Font dimensions
    [(thickness THICKNESS)]            ; Line thickness
    [bold]
    [italic]
    [(line_spacing RATIO)]             ; Not yet supported
  )
  [(justify [left|right] [top|bottom] [mirror])]
  [hide]
)
```

### Fill Definition

```lisp
(fill
  (type none|outline|background)       ; Fill type
  (color R G B A)                      ; Fill color
)
```

### Page Settings

```lisp
(paper
  PAPER_SIZE | WIDTH HEIGHT            ; A0-A5, A-E, or custom dimensions
  [portrait]                           ; Optional portrait mode
)
```

**Valid paper sizes:** A0, A1, A2, A3, A4, A5, A, B, C, D, E

### Title Block

```lisp
(title_block
  (title "TITLE")
  (date "YYYY-MM-DD")
  (rev "REVISION")
  (company "COMPANY_NAME")
  (comment 1 "COMMENT_1")
  (comment 2 "COMMENT_2")
  ;; comments 1-9
)
```

### UUID (Universally Unique Identifier)

```lisp
(uuid "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx")
```
- Version 4 random UUID
- Generated using mt19937 Mersenne Twister

### Instance Path

For multi-instance sheets, paths use UUIDs separated by `/`:

```
"/00000000-0000-0000-0000-00004b3a13a4/00000000-0000-0000-0000-00004b617b88"
```

The first identifier is always the root sheet UUID.

---

## Practical Examples

### Minimal Schematic File

```lisp
(kicad_sch
  (version 20231120)
  (generator "kicad_sch")
  (uuid "12345678-1234-1234-1234-123456789abc")

  (paper "A4")

  (title_block
    (title "My Schematic")
    (date "2024-01-01")
    (rev "1.0")
  )

  (lib_symbols)

  (sheet_instances
    (path "/"
      (page "1")
    )
  )
)
```

### Adding a Wire

```lisp
(wire
  (pts
    (xy 100.33 50.8)
    (xy 120.65 50.8)
  )
  (stroke
    (width 0)
    (type default)
    (color 0 0 0 0)
  )
  (uuid "abcd1234-5678-90ab-cdef-1234567890ab")
)
```

### Adding a Resistor Symbol

```lisp
(symbol
  "Device:R"
  (at 110.49 50.8 0)
  (unit 1)
  (in_bom yes)
  (on_board yes)
  (uuid "11111111-2222-3333-4444-555555555555")

  (property "Reference" "R1"
    (at 112.395 49.53 0)
    (effects (font (size 1.27 1.27)) (justify left))
  )
  (property "Value" "10k"
    (at 112.395 52.07 0)
    (effects (font (size 1.27 1.27)) (justify left))
  )
  (property "Footprint" "Resistor_SMD:R_0603_1608Metric"
    (at 108.712 50.8 90)
    (effects (font (size 1.27 1.27)) hide)
  )
  (property "Datasheet" "~"
    (at 110.49 50.8 0)
    (effects (font (size 1.27 1.27)) hide)
  )

  (pin "1" (uuid "pin1-uuid-here"))
  (pin "2" (uuid "pin2-uuid-here"))

  (instances
    (project "MyProject"
      (path "/12345678-1234-1234-1234-123456789abc"
        (reference "R1")
        (unit 1)
      )
    )
  )
)
```

### Adding a Junction

```lisp
(junction
  (at 110.49 50.8)
  (diameter 0)
  (color 0 0 0 0)
  (uuid "junction-uuid-here")
)
```

### Adding a Global Label

```lisp
(global_label
  "VCC"
  (shape input)
  (at 100.33 25.4 0)
  (effects
    (font (size 1.27 1.27))
    (justify left)
  )
  (uuid "global-label-uuid")
  (property "Intersheetref" "${INTERSHEET_REFS}"
    (at 108.2262 25.3206 0)
    (effects (font (size 1.27 1.27)) hide)
  )
)
```

### Adding a Hierarchical Sheet

```lisp
(sheet
  (at 152.4 50.8)
  (size 25.4 20.32)
  (stroke
    (width 0.1524)
    (type solid)
    (color 0 0 0 0)
  )
  (fill (color 0 0 0 0))
  (uuid "sheet-uuid-here")

  (property "Sheetname" "PowerSupply"
    (at 152.4 50.0888 0)
    (effects (font (size 1.27 1.27)) (justify left bottom))
  )
  (property "Sheetfile" "power_supply.kicad_sch"
    (at 152.4 71.6288 0)
    (effects (font (size 1.27 1.27)) (justify left top))
  )

  (pin "VIN" input
    (at 152.4 55.88 180)
    (effects (font (size 1.27 1.27)) (justify right))
    (uuid "sheet-pin-uuid-1")
  )
  (pin "VOUT" output
    (at 177.8 55.88 0)
    (effects (font (size 1.27 1.27)) (justify left))
    (uuid "sheet-pin-uuid-2")
  )

  (instances
    (project "MyProject"
      (path "/root-uuid"
        (page "2")
      )
    )
  )
)
```

---

## Symbol Library Format Reference

For defining symbols in `lib_symbols`, see the Symbol Library format documentation.

Key symbol elements:
- `symbol` - Symbol definition container
- `pin` - Pin definition with electrical type
- `rectangle`, `circle`, `arc`, `polyline` - Graphics primitives
- `property` - Symbol properties (Reference, Value, Footprint, Datasheet)

---

## References

- [KiCad Developer Documentation - Schematic File Format](https://dev-docs.kicad.org/en/file-formats/sexpr-schematic/)
- [KiCad Developer Documentation - S-Expression Format](https://dev-docs.kicad.org/en/file-formats/sexpr-intro/)
- [KiCad 9.0 Schematic Editor Manual](https://docs.kicad.org/9.0/en/eeschema/eeschema.html)
