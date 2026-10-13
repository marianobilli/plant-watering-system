"""Generate goat_head_logo.svg: a detailed front-view billy goat head, built with shapely and
written as hole-free filled polygons (the KiCad SVG importer fills every path and cannot cut holes,
so shapes with cut-outs are split into pieces).

Run: python goat_head_logo.py goat_head_logo.svg preview.png   (needs: pip install shapely pillow)
"""
import math
import sys

from PIL import Image, ImageDraw
from shapely import affinity
from shapely.geometry import LineString, MultiPolygon, Point, Polygon, box
from shapely.ops import split, unary_union

OUT_SVG, OUT_PNG = sys.argv[1], sys.argv[2]


def bez(p0, p1, p2, p3, t):
    u = 1 - t
    return tuple(u**3 * a + 3 * u * u * t * b + 3 * u * t * t * c + t**3 * d
                 for a, b, c, d in zip(p0, p1, p2, p3))


def ellipse(cx, cy, rx, ry, rot=0):
    e = affinity.scale(Point(0, 0).buffer(1, 64), rx, ry)
    return affinity.translate(affinity.rotate(e, rot, origin=(0, 0)), cx, cy)


def mirror(g):
    return affinity.scale(g, -1, 1, origin=(50, 0))


def thin_curve(pts, w):
    return LineString(pts).buffer(w / 2, cap_style="round")


# ---- horns: tapered curved band with ridge notches on the outer edge ----
def horn():
    P = ((44, 30), (41, 8), (22, -2), (4, 8))
    n = 80
    left, right, frames = [], [], []
    for i in range(n + 1):
        t = i / n
        x, y = bez(*P, t)
        x2, y2 = bez(*P, min(t + 1e-3, 1))
        x1, y1 = bez(*P, max(t - 1e-3, 0))
        dx, dy = x2 - x1, y2 - y1
        L = math.hypot(dx, dy)
        nx, ny = -dy / L, dx / L
        w = 5.0 * (1 - t) ** 0.85
        left.append((x + nx * w, y + ny * w))
        right.append((x - nx * w, y - ny * w))
        frames.append((x, y, nx, ny, w))
    band = Polygon(left + right[::-1]).buffer(0)
    cuts = []
    for t in (0.18, 0.29, 0.40, 0.51, 0.62, 0.73):
        x, y, nx, ny, w = frames[round(t * n)]
        # notch from the outer edge (+normal side) 60 % across the horn
        a = (x + nx * (w + 1), y + ny * (w + 1))
        b = (x + nx * (w - 1.2 * w), y + ny * (w - 1.2 * w))
        cuts.append(LineString([a, b]).buffer(0.7, cap_style="flat"))
    return band.difference(unary_union(cuts))


horns = unary_union([horn(), mirror(horn())])

# ---- head: forehead + muzzle hull, plus forehead tuft and cheek tufts ----
head = unary_union([ellipse(50, 40, 15, 12), ellipse(50, 64, 9, 13)]).convex_hull
tuft = Polygon([(38, 32), (41, 22), (44, 28), (47, 19), (50, 26), (53, 19), (56, 28), (59, 22), (62, 32)])
cheek = Polygon([(37, 50), (31, 56), (37, 55), (34, 61), (40, 59), (38, 65), (43, 62)])
head = unary_union([head, tuft, cheek, mirror(cheek)])

# ---- ears: pointed leaves drooping outward, with a centre vein ----
lens = Point(0, -9).buffer(14, 64).intersection(Point(0, 9).buffer(14, 64))  # ~22 x 10, pointed ends
lens = affinity.scale(lens, 1.1, 0.95)
ear = affinity.translate(affinity.rotate(lens, -22, origin=(0, 0)), 25, 47)
vein = thin_curve([(35, 43), (25, 47), (17, 50.2)], 1.1)
ear = ear.difference(vein).difference(head.buffer(1.4))
ears = unary_union([ear, mirror(ear)])

# ---- beard: strands from the chin, splayed, with gaps toward the tips ----
strands = []
for x0, tip, lean in ((44, 90, -4), (47, 97, -2), (50, 101, 0), (53, 97, 2), (56, 90, 4)):
    strands.append(Polygon([(x0 - 2.3, 72), (x0 + 2.3, 72), (x0 + lean, tip)]))
beard = unary_union(strands + [box(42, 70, 58, 76)])

face = unary_union([head, beard])

# ---- cut-outs: eyes, brows, nose bridge, nostrils, mouth ----
eye = ellipse(42, 43, 4.6, 2.3, rot=12)
brow = thin_curve([(36.5, 38.5), (41, 37), (46.5, 38.8)], 1.3)
bridge = thin_curve([(47.0, 55), (46.4, 62.5)], 1.1)
nostril = ellipse(46.3, 69.5, 1.9, 1.15, rot=-35)
mouth = thin_curve([(46.5, 75.2), (50, 76.3), (53.5, 75.2)], 1.2)
beard_parts = [thin_curve([(x, 80), (x + l * 0.5, 87)], 1.0) for x, l in ((45.5, -1), (54.5, 1))]
cuts = unary_union([eye, brow, bridge, nostril, mouth] + [mirror(g) for g in (eye, brow, bridge, nostril)]
                   + beard_parts)
face = face.difference(cuts)

# goat pupils are horizontal bars
pupil = affinity.rotate(box(39.6, 42.3, 44.4, 43.7), 12, origin=(42, 43))
pupils = unary_union([pupil, mirror(pupil)])

shape = unary_union([horns, ears, face])
shape = unary_union([shape, pupils])


# ---- make hole-free: split each polygon through its holes until none remain ----
def no_holes(geom):
    todo = list(geom.geoms) if isinstance(geom, MultiPolygon) else [geom]
    done = []
    while todo:
        p = todo.pop()
        if not p.interiors:
            done.append(p)
            continue
        cx = p.interiors[0].centroid.x
        minx, miny, maxx, maxy = p.bounds
        parts = split(p, LineString([(cx, miny - 1), (cx, maxy + 1)]))
        todo.extend(g for g in parts.geoms if g.area > 1e-6)
    return done


# grow each piece slightly so split seams overlap instead of showing as hairlines,
# then clip back to the true outline so the cut-outs keep their full width
polys = []
for piece in no_holes(shape):
    g = piece.buffer(0.2, join_style="mitre").intersection(shape)
    polys.extend(g.geoms if hasattr(g, "geoms") else [g])
polys = [p for p in polys if isinstance(p, Polygon) and p.area > 1e-3]
assert all(not p.interiors for p in polys)
minx, miny, maxx, maxy = unary_union(polys).bounds
pad = 1
W, H = maxx - minx + 2 * pad, maxy - miny + 2 * pad


def tr(x, y):
    return x - minx + pad, y - miny + pad


paths = []
for p in polys:
    c = list(p.exterior.coords)[:-1]
    d = "M" + " L".join(f"{tr(x, y)[0]:.2f} {tr(x, y)[1]:.2f}" for x, y in c) + " Z"
    paths.append(f'    <path d="{d}"/>')
svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W:.2f} {H:.2f}" '
       f'width="{W:.2f}" height="{H:.2f}">\n'
       '  <!-- Billy goat head, front view. Hole-free filled polygons for KiCad silkscreen import\n'
       '       (eye, nostril and ear gaps are made by splitting shapes, since the importer cannot cut holes).\n'
       '       Designed for ~12 mm wide or more; smallest gaps are ~1 unit. -->\n'
       '  <g fill="#000">\n' + "\n".join(paths) + "\n  </g>\n</svg>\n")
open(OUT_SVG, "w", encoding="utf-8").write(svg)

S = 10
img = Image.new("RGB", (int(W * S), int(H * S)), "white")
dr = ImageDraw.Draw(img)
for p in polys:
    dr.polygon([tuple(v * S for v in tr(x, y)) for x, y in p.exterior.coords], fill="black")
img.save(OUT_PNG)
print(len(polys), "polygons, size", round(W, 1), "x", round(H, 1))
