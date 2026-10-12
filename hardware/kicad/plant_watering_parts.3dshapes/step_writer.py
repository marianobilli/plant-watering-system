"""Minimal AP214 STEP writer for simple coloured 3D models (boxes and vertical
cylinders), used by the make_*.py generators in this folder. No CAD library.

Parts are given in footprint coordinates (mm, +y down as in the footprint
editor, z up from the carrier PCB top); y is negated on output because
KiCad's 3D frame has +y up.
"""


def _f(v):
    s = f"{v:.6f}".rstrip("0")
    return s + "0" if s.endswith(".") else s


class Model:
    def __init__(self, name, colors):
        self.name = name
        self.colors = colors
        self.parts = []  # (kind, name, color, geometry...)
        self.lines = []
        self.n = 0

    # ------------------------------------------------------------ geometry
    def box(self, name, color, x0, y0, z0, x1, y1, z1):
        self.parts.append(("box", name, color, min(x0, x1), min(y0, y1), min(z0, z1),
                           max(x0, x1), max(y0, y1), max(z0, z1)))

    def cbox(self, name, color, cx, cy, w, l, z0, h):
        self.box(name, color, cx - w / 2, cy - l / 2, z0, cx + w / 2, cy + l / 2, z0 + h)

    def cyl(self, name, color, cx, cy, r, z0, h):
        """Cylinder with its axis along z."""
        self.parts.append(("cyl", name, color, cx, cy, r, z0, z0 + h))

    # ------------------------------------------------------------ STEP output
    def _add(self, text):
        self.n += 1
        self.lines.append(f"#{self.n}={text};")
        return f"#{self.n}"

    def _pt(self, p):
        return self._add(f"CARTESIAN_POINT('',({_f(p[0])},{_f(p[1])},{_f(p[2])}))")

    def _dir(self, d):
        return self._add("DIRECTION('',(" + ",".join(_f(c) for c in d) + "))")

    def _write_box(self, part):
        _, name, _, x0, y0, z0, x1, y1, z1 = part
        y0, y1 = -y1, -y0
        pts = [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
               (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
        verts = []
        for p in pts:
            cp = self._pt(p)
            verts.append((self._add(f"VERTEX_POINT('',{cp})"), p))

        edges = {}

        def edge(a, b):
            key = (min(a, b), max(a, b))
            if key not in edges:
                s, e = key
                ps, pe = verts[s][1], verts[e][1]
                d = [pe[k] - ps[k] for k in range(3)]
                length = max(abs(c) for c in d)
                dirn = self._dir([c / length for c in d])
                vec = self._add(f"VECTOR('',{dirn},{_f(length)})")
                cp = self._pt(ps)
                line = self._add(f"LINE('',{cp},{vec})")
                edges[key] = self._add(f"EDGE_CURVE('',{verts[s][0]},{verts[e][0]},{line},.T.)")
            return edges[key], a < b

        # Each face: vertex loop CCW seen from outside, outward normal, x-ref axis.
        faces = [
            ((0, 3, 2, 1), (0, 0, -1), (1, 0, 0)),
            ((4, 5, 6, 7), (0, 0, 1), (1, 0, 0)),
            ((0, 1, 5, 4), (0, -1, 0), (1, 0, 0)),
            ((2, 3, 7, 6), (0, 1, 0), (-1, 0, 0)),
            ((1, 2, 6, 5), (1, 0, 0), (0, 1, 0)),
            ((3, 0, 4, 7), (-1, 0, 0), (0, -1, 0)),
        ]
        face_ids = []
        for loop, nrm, ref in faces:
            oes = []
            for i in range(4):
                ec, fwd = edge(loop[i], loop[(i + 1) % 4])
                oes.append(self._add(f"ORIENTED_EDGE('',*,*,{ec},{'.T.' if fwd else '.F.'})"))
            el = self._add("EDGE_LOOP('',(" + ",".join(oes) + "))")
            fb = self._add(f"FACE_OUTER_BOUND('',{el},.T.)")
            cp = self._pt(verts[loop[0]][1])
            dn = self._dir(nrm)
            dr = self._dir(ref)
            ax = self._add(f"AXIS2_PLACEMENT_3D('',{cp},{dn},{dr})")
            pl = self._add(f"PLANE('',{ax})")
            face_ids.append(self._add(f"ADVANCED_FACE('',({fb}),{pl},.T.)"))
        shell = self._add("CLOSED_SHELL('',(" + ",".join(face_ids) + "))")
        return self._add(f"MANIFOLD_SOLID_BREP('{name}',{shell})")

    def _write_cyl(self, part):
        _, name, _, cx, cy, r, z0, z1 = part
        cy = -cy
        dz = self._dir((0, 0, 1))
        dx = self._dir((1, 0, 0))
        v, circ = [], []
        for z in (z0, z1):
            v.append(self._add(f"VERTEX_POINT('',{self._pt((cx + r, cy, z))})"))
            ax = self._add(f"AXIS2_PLACEMENT_3D('',{self._pt((cx, cy, z))},{dz},{dx})")
            c = self._add(f"CIRCLE('',{ax},{_f(r)})")
            circ.append(self._add(f"EDGE_CURVE('',{v[-1]},{v[-1]},{c},.T.)"))
        vec = self._add(f"VECTOR('',{dz},{_f(z1 - z0)})")
        seam_line = self._add(f"LINE('',{self._pt((cx + r, cy, z0))},{vec})")
        seam = self._add(f"EDGE_CURVE('',{v[0]},{v[1]},{seam_line},.T.)")

        def face(edges, surface):
            oes = [self._add(f"ORIENTED_EDGE('',*,*,{e},{s})") for e, s in edges]
            el = self._add("EDGE_LOOP('',(" + ",".join(oes) + "))")
            fb = self._add(f"FACE_OUTER_BOUND('',{el},.T.)")
            return self._add(f"ADVANCED_FACE('',({fb}),{surface},.T.)")

        down = self._dir((0, 0, -1))
        ax_b = self._add(f"AXIS2_PLACEMENT_3D('',{self._pt((cx, cy, z0))},{down},{dx})")
        ax_t = self._add(f"AXIS2_PLACEMENT_3D('',{self._pt((cx, cy, z1))},{dz},{dx})")
        ax_s = self._add(f"AXIS2_PLACEMENT_3D('',{self._pt((cx, cy, z0))},{dz},{dx})")
        faces = [
            face([(circ[0], ".F.")], self._add(f"PLANE('',{ax_b})")),
            face([(circ[1], ".T.")], self._add(f"PLANE('',{ax_t})")),
            face([(circ[0], ".T."), (seam, ".T."), (circ[1], ".F."), (seam, ".F.")],
                 self._add(f"CYLINDRICAL_SURFACE('',{ax_s},{_f(r)})")),
        ]
        shell = self._add("CLOSED_SHELL('',(" + ",".join(faces) + "))")
        return self._add(f"MANIFOLD_SOLID_BREP('{name}',{shell})")

    def write(self, path, generator):
        a = self._add
        app = a("APPLICATION_CONTEXT('core data for automotive mechanical design processes')")
        a(f"APPLICATION_PROTOCOL_DEFINITION('international standard','automotive_design',2000,{app})")
        pc = a(f"PRODUCT_CONTEXT('',{app},'mechanical')")
        prod = a(f"PRODUCT('{self.name}','{self.name}','',({pc}))")
        pdf = a(f"PRODUCT_DEFINITION_FORMATION('','',{prod})")
        pdc = a(f"PRODUCT_DEFINITION_CONTEXT('part definition',{app},'design')")
        pd = a(f"PRODUCT_DEFINITION('design','',{pdf},{pdc})")
        pds = a(f"PRODUCT_DEFINITION_SHAPE('','',{pd})")
        lu = a("(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT(.MILLI.,.METRE.))")
        au = a("(NAMED_UNIT(*)PLANE_ANGLE_UNIT()SI_UNIT($,.RADIAN.))")
        su = a("(NAMED_UNIT(*)SI_UNIT($,.STERADIAN.)SOLID_ANGLE_UNIT())")
        unc = a(f"UNCERTAINTY_MEASURE_WITH_UNIT(LENGTH_MEASURE(1.E-07),{lu},'distance_accuracy_value','confusion accuracy')")
        ctx = a(f"(GEOMETRIC_REPRESENTATION_CONTEXT(3)GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT(({unc}))"
                f"GLOBAL_UNIT_ASSIGNED_CONTEXT(({lu},{au},{su}))REPRESENTATION_CONTEXT('',''))")

        styles = {}
        for cname, (r, g, b) in self.colors.items():
            c = a(f"COLOUR_RGB('{cname}',{_f(r)},{_f(g)},{_f(b)})")
            fac = a(f"FILL_AREA_STYLE_COLOUR('',{c})")
            fas = a(f"FILL_AREA_STYLE('',({fac}))")
            ssfa = a(f"SURFACE_STYLE_FILL_AREA({fas})")
            sss = a(f"SURFACE_SIDE_STYLE('',({ssfa}))")
            ssu = a(f"SURFACE_STYLE_USAGE(.BOTH.,{sss})")
            styles[cname] = a(f"PRESENTATION_STYLE_ASSIGNMENT(({ssu}))")

        solids, styled = [], []
        for part in self.parts:
            s = self._write_box(part) if part[0] == "box" else self._write_cyl(part)
            solids.append(s)
            styled.append(a(f"STYLED_ITEM('color',({styles[part[2]]}),{s})"))

        o = a("CARTESIAN_POINT('',(0.,0.,0.))")
        dz = a("DIRECTION('',(0.,0.,1.))")
        dx = a("DIRECTION('',(1.,0.,0.))")
        origin = a(f"AXIS2_PLACEMENT_3D('',{o},{dz},{dx})")
        rep = a(f"ADVANCED_BREP_SHAPE_REPRESENTATION('{self.name}',(" +
                ",".join([origin] + solids) + f"),{ctx})")
        a(f"SHAPE_DEFINITION_REPRESENTATION({pds},{rep})")
        a("MECHANICAL_DESIGN_GEOMETRIC_PRESENTATION_REPRESENTATION('',(" +
          ",".join(styled) + f"),{ctx})")

        fname = path.replace("\\", "/").rsplit("/", 1)[-1]
        with open(path, "w", newline="\n") as fh:
            fh.write("ISO-10303-21;\nHEADER;\n")
            fh.write(f"FILE_DESCRIPTION(('{self.name} simplified model'),'2;1');\n")
            fh.write(f"FILE_NAME('{fname}','2026-10-12T00:00:00',(''),(''),"
                     f"'{generator}','','');\n")
            fh.write("FILE_SCHEMA(('AUTOMOTIVE_DESIGN { 1 0 10303 214 1 1 1 1 }'));\n")
            fh.write("ENDSEC;\nDATA;\n")
            fh.write("\n".join(self.lines))
            fh.write("\nENDSEC;\nEND-ISO-10303-21;\n")
        print(f"wrote {path}: {len(self.parts)} parts, {self.n} entities")
