#!/usr/bin/env python3
"""
Single-file nonsmooth multilevel VLSI global placer.

This corrected variant focuses on coarse-to-fine consistency:
  * every fine pin is preserved on coarse nets;
  * cluster-internal nets are retained as translation-invariant HPWL terms;
  * deterministic compact child offsets are shared by coarse pin geometry and
    declustering, so HPWL is continuous across a level transition;
  * declustering translates each child group as a whole instead of clamping
    children independently at the boundary;
  * inter-level HPWL diagnostics are written to interlevel_hpwl.json;
  * macro detection, singleton-macro geometry, best-state bookkeeping,
    per-level step decay, nmax stopping, and adaptive bin counts are corrected.

The output is a GLOBAL placement and may still require an external legalizer.
Macro Shifting is retained. Whitespace Allocation is intentionally disabled.

Dependencies: Python 3.9+ standard library only.
"""

from __future__ import annotations

import argparse
import csv
import gzip
import heapq
import json
import math
import random
import statistics
import sys
import time
from collections import defaultdict
from dataclasses import asdict, dataclass, field
from pathlib import Path
from typing import DefaultDict, Dict, Iterable, Iterator, List, Optional, Sequence, Set, Tuple

EPS = 1.0e-12


# ---------------------------------------------------------------------------
# Bookshelf database
# ---------------------------------------------------------------------------


@dataclass
class Cell:
    name: str
    width: float
    height: float
    x: float = 0.0
    y: float = 0.0
    fixed: bool = False
    terminal: bool = False
    orientation: str = "N"

    @property
    def area(self) -> float:
        return self.width * self.height

    @property
    def cx(self) -> float:
        return self.x + 0.5 * self.width

    @property
    def cy(self) -> float:
        return self.y + 0.5 * self.height


@dataclass
class Pin:
    cell_id: int
    net_id: int
    offset_x: float
    offset_y: float
    direction: str = ""


@dataclass
class Net:
    name: str
    pin_ids: List[int] = field(default_factory=list)


@dataclass
class Row:
    y: float
    height: float
    x_start: float
    x_end: float
    site_width: float
    site_spacing: float
    num_sites: int


@dataclass
class PlacementDB:
    cells: List[Cell] = field(default_factory=list)
    pins: List[Pin] = field(default_factory=list)
    nets: List[Net] = field(default_factory=list)
    rows: List[Row] = field(default_factory=list)
    cell_name_to_id: Dict[str, int] = field(default_factory=dict)

    def add_cell(self, name: str, width: float, height: float, terminal: bool) -> int:
        if name in self.cell_name_to_id:
            raise ValueError(f"duplicate cell: {name}")
        cid = len(self.cells)
        self.cell_name_to_id[name] = cid
        self.cells.append(Cell(name=name, width=width, height=height,
                               terminal=terminal, fixed=terminal))
        return cid

    def add_net(self, name: str) -> int:
        nid = len(self.nets)
        self.nets.append(Net(name=name))
        return nid

    def add_pin(self, cell_id: int, net_id: int, ox: float, oy: float,
                direction: str = "") -> int:
        pid = len(self.pins)
        self.pins.append(Pin(cell_id, net_id, ox, oy, direction))
        self.nets[net_id].pin_ids.append(pid)
        return pid

    def region(self) -> Tuple[float, float, float, float]:
        if self.rows:
            xl = min(r.x_start for r in self.rows)
            xh = max(r.x_end for r in self.rows)
            yl = min(r.y for r in self.rows)
            yh = max(r.y + r.height for r in self.rows)
            if xh > xl and yh > yl:
                return xl, xh, yl, yh
        if not self.cells:
            return 0.0, 1.0, 0.0, 1.0
        xl = min(c.x for c in self.cells)
        xh = max(c.x + c.width for c in self.cells)
        yl = min(c.y for c in self.cells)
        yh = max(c.y + c.height for c in self.cells)
        return xl, max(xh, xl + 1.0), yl, max(yh, yl + 1.0)


def _open_text(path: Path):
    if path.suffix.lower() == ".gz":
        return gzip.open(path, "rt", encoding="utf-8", errors="ignore")
    return path.open("r", encoding="utf-8", errors="ignore")


def _strip_comment(line: str) -> str:
    return line.split("#", 1)[0].strip()


def _tokens(line: str) -> List[str]:
    return _strip_comment(line).replace(":", " : ").split()


def _resolve_aux_reference(aux_path: Path, token: str) -> Path:
    token = token.strip().strip('"').replace("\\", "/")
    candidate = (aux_path.parent / token).resolve()
    if candidate.exists():
        return candidate
    # Permit compressed Bookshelf placement files.
    if candidate.suffix.lower() == ".pl" and candidate.with_suffix(".pl.gz").exists():
        return candidate.with_suffix(".pl.gz")
    return candidate


def parse_aux(aux_path: Path) -> Dict[str, Path]:
    text = aux_path.read_text(encoding="utf-8", errors="ignore")
    result: Dict[str, Path] = {}
    for raw in text.replace("\n", " ").split():
        token = raw.strip().strip('"')
        lower = token.lower()
        for ext in (".nodes", ".nets", ".pl", ".scl"):
            if lower.endswith(ext) or lower.endswith(ext + ".gz"):
                result[ext] = _resolve_aux_reference(aux_path, token)
    missing = [ext for ext in (".nodes", ".nets", ".pl", ".scl") if ext not in result]
    if missing:
        raise ValueError(f"{aux_path}: missing references: {', '.join(missing)}")
    for ext, path in result.items():
        if not path.exists():
            raise FileNotFoundError(f"{ext} file not found: {path}")
    return result


def parse_nodes(path: Path, db: PlacementDB) -> None:
    with _open_text(path) as f:
        for line in f:
            t = _tokens(line)
            if len(t) < 3 or t[0] in {"UCLA", "NumNodes", "NumTerminals"}:
                continue
            try:
                w = float(t[1])
                h = float(t[2])
            except ValueError:
                continue
            terminal = any("terminal" in s.lower() for s in t[3:])
            db.add_cell(t[0], w, h, terminal)


def parse_pl(path: Path, db: PlacementDB) -> None:
    with _open_text(path) as f:
        for line in f:
            t = _tokens(line)
            if len(t) < 3 or t[0] == "UCLA":
                continue
            cid = db.cell_name_to_id.get(t[0])
            if cid is None:
                continue
            try:
                x = float(t[1])
                y = float(t[2])
            except ValueError:
                continue
            c = db.cells[cid]
            c.x, c.y = x, y
            if ":" in t:
                k = t.index(":")
                if k + 1 < len(t) and not t[k + 1].startswith("/"):
                    c.orientation = t[k + 1]
            c.fixed = c.terminal or any("/FIXED" in s.upper() for s in t)


def parse_scl(path: Path, db: PlacementDB) -> None:
    current: Dict[str, float] = {}
    in_row = False

    def flush() -> None:
        nonlocal current, in_row
        if not in_row:
            return
        y = current.get("Coordinate", 0.0)
        h = current.get("Height", 1.0)
        x0 = current.get("SubrowOrigin", 0.0)
        n = int(current.get("NumSites", 0.0))
        sw = current.get("Sitewidth", 1.0)
        ss = current.get("Sitespacing", sw)
        db.rows.append(Row(y, h, x0, x0 + n * ss, sw, ss, n))
        current = {}
        in_row = False

    with _open_text(path) as f:
        for line in f:
            raw = _strip_comment(line)
            if not raw:
                continue
            if raw.startswith("CoreRow"):
                flush()
                in_row = True
                continue
            if raw.startswith("End"):
                flush()
                continue
            if not in_row:
                continue
            p = raw.replace(":", " ").split()
            if len(p) >= 2:
                try:
                    current[p[0]] = float(p[1])
                except ValueError:
                    pass
            if "SubrowOrigin" in p and "NumSites" in p:
                try:
                    current["SubrowOrigin"] = float(p[p.index("SubrowOrigin") + 1])
                    current["NumSites"] = float(p[p.index("NumSites") + 1])
                except (ValueError, IndexError):
                    pass
    flush()


def parse_nets(path: Path, db: PlacementDB) -> None:
    with _open_text(path) as f:
        lines = iter(f)
        for line in lines:
            t = _tokens(line)
            if not t or t[0] != "NetDegree":
                continue
            try:
                k = t.index(":")
                degree = int(t[k + 1])
                name = t[k + 2] if k + 2 < len(t) else f"net_{len(db.nets)}"
            except (ValueError, IndexError):
                continue
            nid = db.add_net(name)
            read = 0
            while read < degree:
                try:
                    p = _tokens(next(lines))
                except StopIteration as exc:
                    raise ValueError(f"premature EOF in net {name}") from exc
                if not p:
                    continue
                cid = db.cell_name_to_id.get(p[0])
                if cid is None:
                    raise ValueError(f"unknown cell {p[0]} in net {name}")
                direction = p[1] if len(p) > 1 else ""
                ox = oy = 0.0
                if ":" in p:
                    q = p.index(":")
                    if q + 2 < len(p):
                        try:
                            ox, oy = float(p[q + 1]), float(p[q + 2])
                        except ValueError:
                            pass
                db.add_pin(cid, nid, ox, oy, direction)
                read += 1


def load_bookshelf(aux_path: Path) -> PlacementDB:
    files = parse_aux(aux_path)
    db = PlacementDB()
    parse_nodes(files[".nodes"], db)
    parse_pl(files[".pl"], db)
    parse_scl(files[".scl"], db)
    parse_nets(files[".nets"], db)
    return db


# ---------------------------------------------------------------------------
# Multilevel representation
# ---------------------------------------------------------------------------


@dataclass
class LObject:
    name: str
    width: float
    height: float
    x: float
    y: float
    fixed: bool
    is_macro: bool
    members: List[int]
    children: List[int] = field(default_factory=list)
    child_offsets: Dict[int, Tuple[float, float]] = field(default_factory=dict)
    # Relative child-envelope edges (left, right, bottom, top).  Projection
    # uses this envelope as well as the coarse density rectangle, preventing a
    # parent from being optimized into a position that its children cannot fit.
    projection_bbox: Optional[Tuple[float, float, float, float]] = None

    @property
    def area(self) -> float:
        return self.width * self.height

    @property
    def cx(self) -> float:
        return self.x + 0.5 * self.width

    @property
    def cy(self) -> float:
        return self.y + 0.5 * self.height

    def set_center(self, cx: float, cy: float) -> None:
        self.x = cx - 0.5 * self.width
        self.y = cy - 0.5 * self.height


@dataclass
class LPin:
    object_id: int
    offset_x: float
    offset_y: float


@dataclass
class LNet:
    name: str
    pins: List[LPin]


@dataclass
class Level:
    index: int
    objects: List[LObject]
    nets: List[LNet]
    fine_to_coarse: Optional[List[int]] = None

    @property
    def movable_ids(self) -> List[int]:
        return [i for i, o in enumerate(self.objects) if not o.fixed]

    @property
    def standard_movable_ids(self) -> List[int]:
        return [i for i, o in enumerate(self.objects) if not o.fixed and not o.is_macro]

    @property
    def macro_ids(self) -> List[int]:
        return [i for i, o in enumerate(self.objects) if not o.fixed and o.is_macro]


def _median_or(values: Sequence[float], default: float) -> float:
    return statistics.median(values) if values else default


def build_level0(db: PlacementDB) -> Level:
    row_h = _median_or([r.height for r in db.rows if r.height > 0], 1.0)
    site_w = _median_or([r.site_width for r in db.rows if r.site_width > 0], 1.0)
    objects: List[LObject] = []
    for cid, c in enumerate(db.cells):
        # Movable macros are multi-row objects.  Width alone is not a reliable
        # macro signal on ISPD Bookshelf benchmarks.
        is_macro = (not c.fixed and c.height > 1.5 * row_h + EPS)
        objects.append(LObject(
            name=c.name, width=c.width, height=c.height, x=c.x, y=c.y,
            fixed=c.fixed, is_macro=is_macro, members=[cid],
        ))
    nets: List[LNet] = []
    for net in db.nets:
        pins = [LPin(db.pins[pid].cell_id, db.pins[pid].offset_x,
                     db.pins[pid].offset_y) for pid in net.pin_ids]
        if len(pins) >= 2:
            nets.append(LNet(net.name, pins))
    return Level(0, objects, nets)


def _object_projection(o: LObject, region: Tuple[float, float, float, float]) -> None:
    xl, xh, yl, yh = region
    left = -0.5 * o.width
    right = 0.5 * o.width
    bottom = -0.5 * o.height
    top = 0.5 * o.height
    if o.projection_bbox is not None:
        pl, pr, pb, pt = o.projection_bbox
        left = min(left, pl)
        right = max(right, pr)
        bottom = min(bottom, pb)
        top = max(top, pt)
    lo_cx, hi_cx = xl - left, xh - right
    lo_cy, hi_cy = yl - bottom, yh - top
    if lo_cx > hi_cx + EPS or lo_cy > hi_cy + EPS:
        raise ValueError(f"object/envelope {o.name} does not fit in placement region")
    o.set_center(min(max(o.cx, lo_cx), hi_cx), min(max(o.cy, lo_cy), hi_cy))


def project_level(level: Level, region: Tuple[float, float, float, float]) -> None:
    for o in level.objects:
        if not o.fixed:
            _object_projection(o, region)


def _needs_nullspace_seed(level: Level) -> bool:
    ids = level.movable_ids
    if len(ids) < 2:
        return False
    # Bookshelf files often place all movable lower-left corners at (0, 0).
    # Different widths/heights then create different centers, so checking
    # centers would incorrectly claim that the placement already has spread.
    xs = [level.objects[i].x for i in ids]
    ys = [level.objects[i].y for i in ids]
    span = (max(xs) - min(xs)) + (max(ys) - min(ys))
    return span <= 1.0e-9


def seed_grid(level: Level, region: Tuple[float, float, float, float], seed: int) -> None:
    """Deterministic null-space seed, used only when all movable coordinates coincide."""
    ids = level.movable_ids
    if not ids:
        return
    xl, xh, yl, yh = region
    nx = max(1, int(math.ceil(math.sqrt(len(ids) * max(xh - xl, EPS) / max(yh - yl, EPS)))))
    ny = max(1, int(math.ceil(len(ids) / nx)))
    rng = random.Random(seed)
    order = ids[:]
    rng.shuffle(order)
    for k, oid in enumerate(order):
        ix, iy = k % nx, k // nx
        cx = xl + (ix + 0.5) * (xh - xl) / nx
        cy = yl + (iy + 0.5) * (yh - yl) / ny
        level.objects[oid].set_center(cx, cy)
        _object_projection(level.objects[oid], region)


# ---------------------------------------------------------------------------
# Modified best-choice clustering
# ---------------------------------------------------------------------------


def _group_connectivity(
    level: Level,
    group_of: Dict[int, int],
    degree_cap: int,
) -> Tuple[Dict[Tuple[int, int], float], Dict[int, float]]:
    conn: DefaultDict[Tuple[int, int], float] = defaultdict(float)
    ext: DefaultDict[int, float] = defaultdict(float)
    for net in level.nets:
        groups: List[int] = []
        seen: Set[int] = set()
        for p in net.pins:
            o = level.objects[p.object_id]
            if o.fixed or o.is_macro:
                continue
            g = group_of.get(p.object_id)
            if g is not None and g not in seen:
                seen.add(g)
                groups.append(g)
        d = len(groups)
        if d == 0:
            continue
        for g in groups:
            ext[g] += 1.0
        if d < 2 or d > degree_cap:
            continue
        w = 1.0 / max(1, d - 1)
        if d <= 16:
            for a in range(d):
                ga = groups[a]
                for b in range(a + 1, d):
                    gb = groups[b]
                    key = (ga, gb) if ga < gb else (gb, ga)
                    conn[key] += w
        else:
            # Linear-size sparse surrogate for medium/high-degree nets.
            groups.sort()
            hub = groups[0]
            for gb in groups[1:]:
                conn[(hub, gb)] += w
            for ga, gb in zip(groups, groups[1:]):
                conn[(ga, gb)] += 0.5 * w
    return dict(conn), dict(ext)


def _fallback_pairing(groups: List[List[int]], level: Level, needed: int) -> List[Tuple[int, int]]:
    if needed <= 0:
        return []
    def key(gid: int) -> Tuple[float, float, float, str]:
        members = groups[gid]
        area = sum(level.objects[i].area for i in members)
        cx = sum(level.objects[i].area * level.objects[i].cx for i in members) / max(area, EPS)
        cy = sum(level.objects[i].area * level.objects[i].cy for i in members) / max(area, EPS)
        return (cx, cy, area, level.objects[members[0]].name)
    ordered = sorted(range(len(groups)), key=key)
    result: List[Tuple[int, int]] = []
    for k in range(0, len(ordered) - 1, 2):
        result.append((ordered[k], ordered[k + 1]))
        if len(result) >= needed:
            break
    return result



def _shelf_pack(
    level: Level,
    children: Sequence[int],
    target_width: float,
) -> Tuple[Dict[int, Tuple[float, float]], float, float]:
    """Pack child rectangles without overlap and return center offsets."""
    ordered = sorted(
        children,
        key=lambda i: (-level.objects[i].height, -level.objects[i].width,
                       level.objects[i].name),
    )
    x = y = row_h = 0.0
    placed: Dict[int, Tuple[float, float]] = {}
    bbox_w = bbox_h = 0.0
    for child in ordered:
        o = level.objects[child]
        if x > EPS and x + o.width > target_width + EPS:
            y += row_h
            x = 0.0
            row_h = 0.0
        placed[child] = (x + 0.5 * o.width, y + 0.5 * o.height)
        x += o.width
        row_h = max(row_h, o.height)
        bbox_w = max(bbox_w, x)
        bbox_h = max(bbox_h, y + row_h)
    offsets = {
        child: (cx - 0.5 * bbox_w, cy - 0.5 * bbox_h)
        for child, (cx, cy) in placed.items()
    }
    return offsets, max(bbox_w, EPS), max(bbox_h, EPS)


def _compact_child_offsets(
    level: Level,
    children: Sequence[int],
) -> Tuple[Dict[int, Tuple[float, float]], float, float]:
    """Choose a deterministic compact shelf packing for one cluster.

    The returned offsets are the *single source of truth* used both when coarse
    pin offsets are built and when the cluster is later declustered.
    """
    if not children:
        return {}, EPS, EPS
    if len(children) == 1:
        child = children[0]
        o = level.objects[child]
        return {child: (0.0, 0.0)}, o.width, o.height

    total_area = sum(level.objects[i].width * level.objects[i].height for i in children)
    max_w = max(level.objects[i].width for i in children)
    sum_w = sum(level.objects[i].width for i in children)
    base = max(max_w, math.sqrt(max(total_area, EPS)))
    candidates = sorted({
        max_w,
        min(sum_w, max_w + 0.70 * (base - max_w)),
        min(sum_w, base),
        min(sum_w, 1.35 * base),
        min(sum_w, 1.80 * base),
    })
    best: Optional[Tuple[float, Dict[int, Tuple[float, float]], float, float]] = None
    for width in candidates:
        offsets, bbox_w, bbox_h = _shelf_pack(level, children, max(width, max_w))
        aspect_penalty = abs(math.log(max(bbox_w, EPS) / max(bbox_h, EPS)))
        whitespace = max(0.0, bbox_w * bbox_h - total_area) / max(total_area, EPS)
        score = whitespace + 0.05 * aspect_penalty
        if best is None or score < best[0]:
            best = (score, offsets, bbox_w, bbox_h)
    assert best is not None
    return best[1], best[2], best[3]


def cluster_one_level(
    fine: Level,
    target_movable: int,
    degree_cap: int,
    verbose: bool = True,
) -> Level:
    std_ids = fine.standard_movable_ids
    macro_ids = fine.macro_ids
    desired_std = max(1, target_movable - len(macro_ids))
    groups: List[List[int]] = [[i] for i in std_ids]
    round_id = 0

    while len(groups) > desired_std:
        round_id += 1
        group_of = {oid: gid for gid, members in enumerate(groups) for oid in members}
        conn, ext = _group_connectivity(fine, group_of, degree_cap)
        areas = [sum(fine.objects[i].area for i in members) for members in groups]
        proposals: List[Tuple[float, int, int]] = []
        for (a, b), dij in conn.items():
            da = max(EPS, ext.get(a, 0.0) - dij)
            db = max(EPS, ext.get(b, 0.0) - dij)
            # Paper-style area suppression: large clusters must become less,
            # not more, attractive as merge candidates.
            score = dij / (da * max(areas[a], EPS)) + dij / (db * max(areas[b], EPS))
            # Prefer balanced merges when scores are otherwise similar.
            score /= 1.0 + abs(math.log(max(areas[a], EPS) / max(areas[b], EPS)))
            proposals.append((-score, a, b))
        heapq.heapify(proposals)
        merges_needed = len(groups) - desired_std
        selected: List[Tuple[int, int]] = []
        used: Set[int] = set()
        while proposals and len(selected) < merges_needed:
            _, a, b = heapq.heappop(proposals)
            if a in used or b in used:
                continue
            used.add(a); used.add(b)
            selected.append((a, b))
        if len(selected) < min(merges_needed, len(groups) // 2):
            fallback = _fallback_pairing(groups, fine, merges_needed - len(selected))
            for a, b in fallback:
                if a not in used and b not in used:
                    used.add(a); used.add(b)
                    selected.append((a, b))
                    if len(selected) >= merges_needed:
                        break
        if not selected:
            break
        mate: Dict[int, int] = {}
        for a, b in selected:
            mate[a] = b; mate[b] = a
        new_groups: List[List[int]] = []
        consumed: Set[int] = set()
        for gid, members in enumerate(groups):
            if gid in consumed:
                continue
            if gid in mate:
                other = mate[gid]
                new_groups.append(members + groups[other])
                consumed.add(gid); consumed.add(other)
            else:
                new_groups.append(members)
                consumed.add(gid)
        groups = new_groups
        if verbose:
            print(f"  [cluster round {round_id}] standard groups={len(groups)} target={desired_std}")

    # If a final partial round is required, merge the smallest groups into the nearest groups.
    while len(groups) > desired_std:
        groups.sort(key=lambda g: sum(fine.objects[i].area for i in g))
        a = groups.pop(0)
        acx = sum(fine.objects[i].area * fine.objects[i].cx for i in a) / max(sum(fine.objects[i].area for i in a), EPS)
        acy = sum(fine.objects[i].area * fine.objects[i].cy for i in a) / max(sum(fine.objects[i].area for i in a), EPS)
        j = min(range(len(groups)), key=lambda k: (
            (sum(fine.objects[i].area * fine.objects[i].cx for i in groups[k]) / max(sum(fine.objects[i].area for i in groups[k]), EPS) - acx) ** 2 +
            (sum(fine.objects[i].area * fine.objects[i].cy for i in groups[k]) / max(sum(fine.objects[i].area for i in groups[k]), EPS) - acy) ** 2
        ))
        groups[j].extend(a)

    objects: List[LObject] = []
    mapping = [-1] * len(fine.objects)

    def make_cluster(children: List[int], name: str, is_macro: bool) -> int:
        physical_area = sum(fine.objects[i].width * fine.objects[i].height for i in children)
        cx = sum(fine.objects[i].area * fine.objects[i].cx for i in children) / max(
            sum(fine.objects[i].area for i in children), EPS
        )
        cy = sum(fine.objects[i].area * fine.objects[i].cy for i in children) / max(
            sum(fine.objects[i].area for i in children), EPS
        )

        # A singleton macro must retain its exact physical dimensions.
        if len(children) == 1 and is_macro:
            child = fine.objects[children[0]]
            w, h = child.width, child.height
            child_offsets = {children[0]: (0.0, 0.0)}
        else:
            child_offsets, pack_w, pack_h = _compact_child_offsets(fine, children)
            # The coarse density surrogate keeps exactly the summed child area,
            # while using the compact packing aspect ratio.
            aspect = min(4.0, max(0.25, pack_w / max(pack_h, EPS)))
            w = math.sqrt(max(physical_area, EPS) * aspect)
            h = max(physical_area, EPS) / max(w, EPS)

        projection_bbox = (
            min(child_offsets[i][0] - 0.5 * fine.objects[i].width for i in children),
            max(child_offsets[i][0] + 0.5 * fine.objects[i].width for i in children),
            min(child_offsets[i][1] - 0.5 * fine.objects[i].height for i in children),
            max(child_offsets[i][1] + 0.5 * fine.objects[i].height for i in children),
        )
        members: List[int] = []
        for child in children:
            members.extend(fine.objects[child].members)
        oid = len(objects)
        obj = LObject(
            name, w, h, cx - 0.5 * w, cy - 0.5 * h,
            False, is_macro, members, children=children[:],
            child_offsets=child_offsets, projection_bbox=projection_bbox,
        )
        for child in children:
            mapping[child] = oid
        objects.append(obj)
        return oid

    for k, group in enumerate(groups):
        make_cluster(group, f"cluster_L{fine.index + 1}_{k}", False)
    for old in macro_ids:
        make_cluster([old], fine.objects[old].name, True)
    for old, o in enumerate(fine.objects):
        if not o.fixed:
            continue
        oid = len(objects)
        copied = LObject(
            o.name, o.width, o.height, o.x, o.y, True, o.is_macro,
            o.members[:], children=[old], child_offsets={old: (0.0, 0.0)},
        )
        objects.append(copied)
        mapping[old] = oid

    # Preserve every fine pin.  Multiple pins may map to the same coarse
    # object; their distinct offsets retain pin span.  Nets internal to one
    # cluster are also retained as translation-invariant HPWL terms.  Because
    # these offsets use the same child_offsets restored by declustering, coarse
    # and fine HPWL are continuous at a level transition.
    nets: List[LNet] = []
    for net in fine.nets:
        pins: List[LPin] = []
        for p in net.pins:
            coarse_id = mapping[p.object_id]
            if coarse_id < 0:
                continue
            dx, dy = objects[coarse_id].child_offsets.get(p.object_id, (0.0, 0.0))
            pins.append(LPin(coarse_id, dx + p.offset_x, dy + p.offset_y))
        if len(pins) >= 2:
            nets.append(LNet(net.name, pins))

    return Level(fine.index + 1, objects, nets, fine_to_coarse=mapping)


def build_hierarchy(
    level0: Level,
    current: int,
    ratio: float,
    max_levels: int,
    degree_cap: int,
) -> List[Level]:
    levels = [level0]
    threshold = current * current
    while len(levels[-1].movable_ids) > threshold and len(levels) < max_levels:
        fine = levels[-1]
        n = len(fine.movable_ids)
        target = max(threshold, int(math.ceil(n / ratio)))
        print(f"[cluster L{fine.index}] movable={n} -> target={target}")
        coarse = cluster_one_level(fine, target, degree_cap)
        levels.append(coarse)
        if len(coarse.movable_ids) >= n:
            print("[cluster] no reduction; stopping hierarchy")
            break
    return levels


# ---------------------------------------------------------------------------
# Quadratic initialization (symmetric net-star alternating solve)
# ---------------------------------------------------------------------------


def quadratic_initialize(
    level: Level,
    region: Tuple[float, float, float, float],
    iterations: int,
    damping: float,
    anchor_weight: float,
    tolerance: float,
    seed: int,
) -> Dict[str, float]:
    if _needs_nullspace_seed(level):
        seed_grid(level, region, seed)
    movable = level.movable_ids
    xl, xh, yl, yh = region
    core_cx, core_cy = 0.5 * (xl + xh), 0.5 * (yl + yh)
    incident: List[List[Tuple[int, float, float, float]]] = [[] for _ in level.objects]
    for nid, net in enumerate(level.nets):
        w = 1.0 / max(1, len(net.pins) - 1)
        for p in net.pins:
            incident[p.object_id].append((nid, p.offset_x, p.offset_y, w))
    net_cx = [core_cx] * len(level.nets)
    net_cy = [core_cy] * len(level.nets)
    last_rms = float("inf")
    actual = 0
    for it in range(iterations):
        for nid, net in enumerate(level.nets):
            if not net.pins:
                continue
            sx = sy = sw = 0.0
            w = 1.0 / max(1, len(net.pins) - 1)
            for p in net.pins:
                o = level.objects[p.object_id]
                sx += w * (o.cx + p.offset_x)
                sy += w * (o.cy + p.offset_y)
                sw += w
            net_cx[nid] = sx / max(sw, EPS)
            net_cy[nid] = sy / max(sw, EPS)
        move2 = 0.0
        for oid in movable:
            o = level.objects[oid]
            sx = anchor_weight * core_cx
            sy = anchor_weight * core_cy
            sw = anchor_weight
            for nid, ox, oy, w in incident[oid]:
                sx += w * (net_cx[nid] - ox)
                sy += w * (net_cy[nid] - oy)
                sw += w
            if sw <= EPS:
                continue
            tx, ty = sx / sw, sy / sw
            nx = (1.0 - damping) * o.cx + damping * tx
            ny = (1.0 - damping) * o.cy + damping * ty
            dx, dy = nx - o.cx, ny - o.cy
            o.set_center(nx, ny)
            _object_projection(o, region)
            move2 += dx * dx + dy * dy
        actual = it + 1
        last_rms = math.sqrt(move2 / max(1, 2 * len(movable)))
        if last_rms <= tolerance:
            break
    return {"iterations": actual, "final_move_rms": last_rms}


# ---------------------------------------------------------------------------
# Exact HPWL and exact rectangle-bin density
# ---------------------------------------------------------------------------


@dataclass
class DensityGrid:
    nx: int
    ny: int
    xl: float
    xh: float
    yl: float
    yh: float
    penalty_density: float
    report_density: float
    bin_w: float = field(init=False)
    bin_h: float = field(init=False)
    bin_area: float = field(init=False)
    fixed_area: List[float] = field(init=False)
    penalty_capacity: List[float] = field(init=False)
    report_capacity: List[float] = field(init=False)

    def __post_init__(self) -> None:
        self.bin_w = (self.xh - self.xl) / self.nx
        self.bin_h = (self.yh - self.yl) / self.ny
        self.bin_area = self.bin_w * self.bin_h
        n = self.nx * self.ny
        self.fixed_area = [0.0] * n
        self.penalty_capacity = [self.penalty_density * self.bin_area] * n
        self.report_capacity = [self.report_density * self.bin_area] * n

    def index(self, ix: int, iy: int) -> int:
        return iy * self.nx + ix

    def build_fixed(self, level: Level) -> None:
        self.fixed_area = [0.0] * (self.nx * self.ny)
        for o in level.objects:
            if o.fixed:
                self.deposit(o.x, o.y, o.width, o.height, self.fixed_area)
        self.penalty_capacity = [max(EPS, self.penalty_density * self.bin_area - a)
                                 for a in self.fixed_area]
        self.report_capacity = [max(EPS, self.report_density * self.bin_area - a)
                                for a in self.fixed_area]

    def bounds_for_rect(self, x: float, y: float, w: float, h: float) -> Tuple[int, int, int, int]:
        ix0 = max(0, int(math.floor((x - self.xl) / self.bin_w)))
        ix1 = min(self.nx - 1, int(math.floor((x + w - self.xl - EPS) / self.bin_w)))
        iy0 = max(0, int(math.floor((y - self.yl) / self.bin_h)))
        iy1 = min(self.ny - 1, int(math.floor((y + h - self.yl - EPS) / self.bin_h)))
        return ix0, ix1, iy0, iy1

    def deposit(self, x: float, y: float, w: float, h: float, out: List[float]) -> None:
        ix0, ix1, iy0, iy1 = self.bounds_for_rect(x, y, w, h)
        for iy in range(iy0, iy1 + 1):
            by0 = self.yl + iy * self.bin_h
            by1 = by0 + self.bin_h
            oy = max(0.0, min(y + h, by1) - max(y, by0))
            if oy <= 0.0:
                continue
            for ix in range(ix0, ix1 + 1):
                bx0 = self.xl + ix * self.bin_w
                bx1 = bx0 + self.bin_w
                ox = max(0.0, min(x + w, bx1) - max(x, bx0))
                if ox > 0.0:
                    out[self.index(ix, iy)] += ox * oy


def _edge_derivative(value: float, boundary: float) -> float:
    if value < boundary - 1.0e-10:
        return 1.0
    if value > boundary + 1.0e-10:
        return 0.0
    return 0.5


def _overlap_1d_and_derivative(x: float, w: float, b0: float, b1: float) -> Tuple[float, float]:
    right = min(x + w, b1)
    left = max(x, b0)
    overlap = right - left
    if overlap <= 0.0:
        return 0.0, 0.0
    d_right = _edge_derivative(x + w, b1)
    # derivative of max(x,b0)
    if x > b0 + 1.0e-10:
        d_left = 1.0
    elif x < b0 - 1.0e-10:
        d_left = 0.0
    else:
        d_left = 0.5
    return overlap, d_right - d_left


@dataclass
class DensityEval:
    penalty: float
    ofr_penalty: float
    ofr_report: float
    max_density: float
    overflow_bins_penalty: int
    overflow_bins_report: int
    gx: List[float]
    gy: List[float]


def density_penalty_gradient(level: Level, grid: DensityGrid) -> DensityEval:
    nbin = grid.nx * grid.ny
    movable_density = [0.0] * nbin
    stencils: List[List[Tuple[int, float, float]]] = [[] for _ in level.objects]
    movable_area = 0.0
    for oid, o in enumerate(level.objects):
        if o.fixed:
            continue
        movable_area += o.area
        ix0, ix1, iy0, iy1 = grid.bounds_for_rect(o.x, o.y, o.width, o.height)
        for iy in range(iy0, iy1 + 1):
            by0 = grid.yl + iy * grid.bin_h
            by1 = by0 + grid.bin_h
            oy, doy = _overlap_1d_and_derivative(o.y, o.height, by0, by1)
            if oy <= 0.0:
                continue
            for ix in range(ix0, ix1 + 1):
                bx0 = grid.xl + ix * grid.bin_w
                bx1 = bx0 + grid.bin_w
                ox, dox = _overlap_1d_and_derivative(o.x, o.width, bx0, bx1)
                if ox <= 0.0:
                    continue
                b = grid.index(ix, iy)
                movable_density[b] += ox * oy
                stencils[oid].append((b, dox * oy, doy * ox))

    norm2 = sum(c * c for c in grid.penalty_capacity) + EPS
    coeff = [0.0] * nbin
    raw2 = raw_pen = raw_rep = 0.0
    bins_pen = bins_rep = 0
    max_density = 0.0
    for b, rho in enumerate(movable_density):
        op = max(0.0, rho - grid.penalty_capacity[b])
        orp = max(0.0, rho - grid.report_capacity[b])
        if op > EPS:
            bins_pen += 1
        if orp > EPS:
            bins_rep += 1
        raw2 += op * op
        raw_pen += op
        raw_rep += orp
        coeff[b] = 2.0 * op / norm2
        max_density = max(max_density, (rho + grid.fixed_area[b]) / max(grid.bin_area, EPS))
    gx = [0.0] * len(level.objects)
    gy = [0.0] * len(level.objects)
    for oid in level.movable_ids:
        for b, dax, day in stencils[oid]:
            gx[oid] += coeff[b] * dax
            gy[oid] += coeff[b] * day
    return DensityEval(
        penalty=raw2 / norm2,
        ofr_penalty=raw_pen / max(movable_area, EPS),
        ofr_report=raw_rep / max(movable_area, EPS),
        max_density=max_density,
        overflow_bins_penalty=bins_pen,
        overflow_bins_report=bins_rep,
        gx=gx,
        gy=gy,
    )


@dataclass
class WireEval:
    hpwl: float
    gx: List[float]
    gy: List[float]


def _pair_abs_gradient(a: Tuple[float, int], b: Tuple[float, int], grad: List[float], weight: float) -> float:
    va, ia = a; vb, ib = b
    d = va - vb
    if d > 0.0:
        grad[ia] += weight; grad[ib] -= weight
    elif d < 0.0:
        grad[ia] -= weight; grad[ib] += weight
    return weight * abs(d)


def wirelength_subgradient(level: Level, mode: str) -> WireEval:
    gx = [0.0] * len(level.objects)
    gy = [0.0] * len(level.objects)
    total = 0.0
    for net in level.nets:
        if len(net.pins) < 2:
            continue
        xs = [(level.objects[p.object_id].cx + p.offset_x, p.object_id) for p in net.pins]
        ys = [(level.objects[p.object_id].cy + p.offset_y, p.object_id) for p in net.pins]
        if mode in {"paper_l1", "b2b"} and len(xs) <= 3:
            if len(xs) == 2:
                total += _pair_abs_gradient(xs[0], xs[1], gx, 1.0)
                total += _pair_abs_gradient(ys[0], ys[1], gy, 1.0)
            else:
                for a in range(3):
                    for b in range(a + 1, 3):
                        total += _pair_abs_gradient(xs[a], xs[b], gx, 0.5)
                        total += _pair_abs_gradient(ys[a], ys[b], gy, 0.5)
            continue
        xmin = min(v for v, _ in xs); xmax = max(v for v, _ in xs)
        ymin = min(v for v, _ in ys); ymax = max(v for v, _ in ys)
        total += xmax - xmin + ymax - ymin
        xtol = 1.0e-10 * max(1.0, abs(xmin), abs(xmax))
        ytol = 1.0e-10 * max(1.0, abs(ymin), abs(ymax))
        lo = {i for v, i in xs if abs(v - xmin) <= xtol}
        hi = {i for v, i in xs if abs(v - xmax) <= xtol}
        bot = {i for v, i in ys if abs(v - ymin) <= ytol}
        top = {i for v, i in ys if abs(v - ymax) <= ytol}
        if xmax > xmin + EPS:
            for i in hi:
                if not level.objects[i].fixed: gx[i] += 1.0 / len(hi)
            for i in lo:
                if not level.objects[i].fixed: gx[i] -= 1.0 / len(lo)
        if ymax > ymin + EPS:
            for i in top:
                if not level.objects[i].fixed: gy[i] += 1.0 / len(top)
            for i in bot:
                if not level.objects[i].fixed: gy[i] -= 1.0 / len(bot)
    return WireEval(total, gx, gy)



def _net_hpwl(level: Level, net: LNet) -> float:
    if len(net.pins) < 2:
        return 0.0
    xs = [level.objects[p.object_id].cx + p.offset_x for p in net.pins]
    ys = [level.objects[p.object_id].cy + p.offset_y for p in net.pins]
    return max(xs) - min(xs) + max(ys) - min(ys)


def exact_hpwl(level: Level) -> float:
    return sum(_net_hpwl(level, net) for net in level.nets)


def interlevel_hpwl_consistency(coarse: Level, fine: Level) -> Dict[str, float]:
    coarse_total = exact_hpwl(coarse)
    fine_total = exact_hpwl(fine)
    paired = min(len(coarse.nets), len(fine.nets))
    sum_abs_delta = 0.0
    max_abs_delta = 0.0
    for i in range(paired):
        delta = abs(_net_hpwl(coarse, coarse.nets[i]) - _net_hpwl(fine, fine.nets[i]))
        sum_abs_delta += delta
        max_abs_delta = max(max_abs_delta, delta)
    return {
        "coarse_hpwl": coarse_total,
        "fine_hpwl": fine_total,
        "delta": fine_total - coarse_total,
        "relative_delta": (fine_total - coarse_total) / max(abs(coarse_total), 1.0),
        "ratio": fine_total / max(abs(coarse_total), EPS),
        "coarse_nets": float(len(coarse.nets)),
        "fine_nets": float(len(fine.nets)),
        "paired_nets": float(paired),
        "sum_abs_net_delta": sum_abs_delta,
        "max_abs_net_delta": max_abs_delta,
    }


def _rms(gx: Sequence[float], gy: Sequence[float], ids: Sequence[int]) -> float:
    if not ids:
        return 0.0
    return math.sqrt(sum(gx[i] * gx[i] + gy[i] * gy[i] for i in ids) / (2.0 * len(ids)))


def _dot(ax: Sequence[float], ay: Sequence[float], bx: Sequence[float], by: Sequence[float], ids: Sequence[int]) -> float:
    return sum(ax[i] * bx[i] + ay[i] * by[i] for i in ids)


@dataclass
class OptimizeConfig:
    iterations_per_stage: int
    penalty_stages: int
    wire_mode: str
    density_only: bool
    s0: float
    s_floor: float
    step_decay: float
    lambda0: float
    lambda_growth_high: float
    lambda_growth_mid: float
    lambda_growth_low: float
    density_gradient_ratio: float
    report_every: int
    target_ofr: float
    nmax: int


def optimize_level(
    level: Level,
    grid: DensityGrid,
    region: Tuple[float, float, float, float],
    cfg: OptimizeConfig,
    history_writer: csv.DictWriter,
    global_state: Dict[str, object],
) -> Dict[str, float]:
    ids = level.movable_ids
    if not ids:
        return {"hpwl": 0.0, "ofr_report": 0.0, "density_penalty": 0.0}

    wire0 = wirelength_subgradient(level, cfg.wire_mode)
    den0 = density_penalty_gradient(level, grid)
    wire_scale = max(abs(wire0.hpwl), 1.0)
    density_scale = max(abs(den0.penalty), 1.0e-8)
    wrms = _rms([g / wire_scale for g in wire0.gx],
                 [g / wire_scale for g in wire0.gy], ids)
    drms = _rms([g / density_scale for g in den0.gx],
                 [g / density_scale for g in den0.gy], ids)
    if cfg.density_only:
        lam = 1.0
    elif cfg.lambda0 > 0.0:
        lam = cfg.lambda0
    elif den0.penalty <= EPS or drms <= EPS:
        lam = 1.0
    else:
        lam = cfg.density_gradient_ratio * wrms / drms
        lam = min(1.0e8, max(1.0e-8, lam))

    prev_gx = prev_gy = prev_dx = prev_dy = None
    best_key = (float("inf"), float("inf"))
    best_pos: Optional[List[Tuple[float, float]]] = None
    best_total = float("inf")
    stall_count = 0
    level_iteration = 0
    final_wire, final_den = wire0, den0
    previous_stage_ofr: Optional[float] = None
    stop_all = False

    for stage in range(cfg.penalty_stages):
        if stage > 0:
            current_ofr = final_den.ofr_report
            if current_ofr < 0.04:
                factor = cfg.lambda_growth_low
            elif previous_stage_ofr is not None and current_ofr <= 0.5 * previous_stage_ofr:
                factor = cfg.lambda_growth_mid
            else:
                factor = cfg.lambda_growth_high
            # Stop the outer penalty loop when overflow no longer improves.
            if (previous_stage_ofr is not None and
                    current_ofr >= previous_stage_ofr * (1.0 - 1.0e-4)):
                print(f"[L{level.index}] penalty loop stopped: OFR no longer decreases")
                break
            lam *= factor
            prev_gx = prev_gy = prev_dx = prev_dy = None
            best_total = float("inf")
            stall_count = 0
        previous_stage_ofr = final_den.ofr_report

        for it in range(cfg.iterations_per_stage):
            # Metrics and coordinates below refer to the same current state.
            wire = wirelength_subgradient(level, cfg.wire_mode)
            den = density_penalty_gradient(level, grid)
            final_wire, final_den = wire, den
            total_norm = ((0.0 if cfg.density_only else wire.hpwl / wire_scale) +
                          lam * den.penalty / density_scale)

            key = (den.ofr_report, wire.hpwl)
            if key < best_key:
                best_key = key
                best_pos = [(level.objects[i].x, level.objects[i].y) for i in ids]

            improve_tol = 1.0e-12 * max(1.0, abs(best_total)) if math.isfinite(best_total) else 0.0
            if not math.isfinite(best_total) or total_norm < best_total - improve_tol:
                best_total = total_norm
                stall_count = 0
            else:
                stall_count += 1

            n = len(level.objects)
            gx = [0.0] * n
            gy = [0.0] * n
            for i in ids:
                if cfg.density_only:
                    gx[i] = den.gx[i] / density_scale
                    gy[i] = den.gy[i] / density_scale
                else:
                    gx[i] = wire.gx[i] / wire_scale + lam * den.gx[i] / density_scale
                    gy[i] = wire.gy[i] / wire_scale + lam * den.gy[i] / density_scale

            beta = 0.0
            if prev_gx is not None and prev_dx is not None:
                num = sum(
                    gx[i] * (gx[i] - prev_gx[i]) +
                    gy[i] * (gy[i] - prev_gy[i]) for i in ids
                )
                denbeta = _dot(prev_gx, prev_gy, prev_gx, prev_gy, ids)
                beta = max(0.0, num / max(denbeta, EPS))

            dx = [0.0] * n
            dy = [0.0] * n
            for i in ids:
                dx[i] = -gx[i] + (beta * prev_dx[i] if prev_dx is not None else 0.0)
                dy[i] = -gy[i] + (beta * prev_dy[i] if prev_dy is not None else 0.0)
            if _dot(gx, gy, dx, dy, ids) >= 0.0:
                beta = 0.0
                for i in ids:
                    dx[i], dy[i] = -gx[i], -gy[i]

            direction_rms = _rms(dx, dy, ids)
            global_it = int(global_state["iteration"])
            step = max(
                cfg.s_floor,
                cfg.s0 / (1.0 + level_iteration / max(cfg.step_decay, 1.0)),
            )
            row = {
                "global_iteration": global_it,
                "level": level.index,
                "stage": stage,
                "iteration": it,
                "hpwl": wire.hpwl,
                "density_penalty": den.penalty,
                "ofr_penalty": den.ofr_penalty,
                "ofr_report": den.ofr_report,
                "max_density": den.max_density,
                "overflow_bins_penalty": den.overflow_bins_penalty,
                "overflow_bins_report": den.overflow_bins_report,
                "lambda": lam,
                "beta_pr": beta,
                "step": step,
                "gradient_rms": _rms(gx, gy, ids),
                "total_norm": total_norm,
                "elapsed_sec": time.perf_counter() - float(global_state["start_time"]),
            }
            history_writer.writerow(row)
            if global_it % cfg.report_every == 0:
                history_file = global_state["history_file"]
                history_file.flush()  # type: ignore[union-attr]
                print(
                    f"[L{level.index} S{stage} I{it}] HPWL={wire.hpwl:.6e} "
                    f"Pden={den.penalty:.4e} OFR@{grid.report_density:g}={den.ofr_report:.4%} "
                    f"maxD={den.max_density:.3f} lambda={lam:.3e} step={step:.3e}"
                )

            global_state["iteration"] = global_it + 1
            level_iteration += 1

            if cfg.nmax > 0 and stall_count >= cfg.nmax:
                print(f"[L{level.index} S{stage}] nmax stop after {stall_count} non-improving iterations")
                stop_all = True
                break
            if direction_rms <= EPS:
                stop_all = True
                break
            if den.ofr_report <= cfg.target_ofr and stage == cfg.penalty_stages - 1:
                stop_all = True
                break

            inv = 1.0 / direction_rms
            for i in ids:
                o = level.objects[i]
                o.x += step * dx[i] * inv
                o.y += step * dy[i] * inv
            project_level(level, region)

            prev_gx, prev_gy, prev_dx, prev_dy = gx, gy, dx, dy

        if stop_all:
            break

    if best_pos is not None:
        for i, (x, y) in zip(ids, best_pos):
            level.objects[i].x, level.objects[i].y = x, y
    final_wire = wirelength_subgradient(level, cfg.wire_mode)
    final_den = density_penalty_gradient(level, grid)
    return {
        "hpwl": final_wire.hpwl,
        "density_penalty": final_den.penalty,
        "ofr_penalty": final_den.ofr_penalty,
        "ofr_report": final_den.ofr_report,
        "max_density": final_den.max_density,
        "lambda": lam,
    }


# ---------------------------------------------------------------------------
# Macro Shifting
# ---------------------------------------------------------------------------


def _rects_overlap(a: Tuple[float, float, float, float], b: Tuple[float, float, float, float], gap: float = 0.0) -> bool:
    ax0, ax1, ay0, ay1 = a; bx0, bx1, by0, by1 = b
    return not (ax1 + gap <= bx0 or bx1 + gap <= ax0 or ay1 + gap <= by0 or by1 + gap <= ay0)


class SpatialHash:
    def __init__(self, cell_size: float):
        self.cell_size = max(cell_size, 1.0)
        self.buckets: DefaultDict[Tuple[int, int], List[Tuple[float, float, float, float]]] = defaultdict(list)

    def _keys(self, rect: Tuple[float, float, float, float]) -> Iterator[Tuple[int, int]]:
        x0, x1, y0, y1 = rect
        ix0 = int(math.floor(x0 / self.cell_size)); ix1 = int(math.floor((x1 - EPS) / self.cell_size))
        iy0 = int(math.floor(y0 / self.cell_size)); iy1 = int(math.floor((y1 - EPS) / self.cell_size))
        for iy in range(iy0, iy1 + 1):
            for ix in range(ix0, ix1 + 1):
                yield ix, iy

    def add(self, rect: Tuple[float, float, float, float]) -> None:
        for key in self._keys(rect):
            self.buckets[key].append(rect)

    def collides(self, rect: Tuple[float, float, float, float], gap: float) -> bool:
        seen: Set[int] = set()
        for key in self._keys(rect):
            for other in self.buckets.get(key, []):
                ident = id(other)
                if ident in seen:
                    continue
                seen.add(ident)
                if _rects_overlap(rect, other, gap):
                    return True
        return False


def macro_shifting(
    level: Level,
    region: Tuple[float, float, float, float],
    row_height: float,
    max_rings: int,
    gap: float,
) -> Dict[str, float]:
    macros = level.macro_ids
    if not macros:
        return {"macros": 0, "moved": 0, "failed": 0, "total_displacement": 0.0}
    xl, xh, yl, yh = region
    fixed_rects = [(o.x, o.x + o.width, o.y, o.y + o.height) for o in level.objects if o.fixed]
    sizes = [max(level.objects[i].width, level.objects[i].height) for i in macros]
    index = SpatialHash(max(row_height, _median_or(sizes, row_height)))
    for r in fixed_rects:
        index.add(r)
    order = sorted(macros, key=lambda i: (-level.objects[i].area, -max(level.objects[i].width, level.objects[i].height), level.objects[i].name))
    step = max(row_height, min((xh - xl) / 100.0, (yh - yl) / 100.0))
    moved = failed = 0
    total_disp = 0.0
    for oid in order:
        o = level.objects[oid]
        ox, oy = o.x, o.y
        best: Optional[Tuple[float, float, float]] = None
        for ring in range(max_rings + 1):
            candidates: List[Tuple[int, int]] = []
            if ring == 0:
                candidates.append((0, 0))
            else:
                for dx in range(-ring, ring + 1):
                    dy = ring - abs(dx)
                    candidates.append((dx, dy))
                    if dy:
                        candidates.append((dx, -dy))
            for dx, dy in candidates:
                x = min(max(ox + dx * step, xl), max(xl, xh - o.width))
                y = min(max(oy + dy * step, yl), max(yl, yh - o.height))
                rect = (x, x + o.width, y, y + o.height)
                if index.collides(rect, gap):
                    continue
                score = (x - ox) ** 2 + (y - oy) ** 2
                if best is None or score < best[0]:
                    best = (score, x, y)
            if best is not None:
                break
        if best is None:
            failed += 1
            rect = (o.x, o.x + o.width, o.y, o.y + o.height)
        else:
            _, o.x, o.y = best
            disp = math.hypot(o.x - ox, o.y - oy)
            total_disp += disp
            if disp > EPS:
                moved += 1
            rect = (o.x, o.x + o.width, o.y, o.y + o.height)
        index.add(rect)
    return {"macros": len(macros), "moved": moved, "failed": failed,
            "total_displacement": total_disp}


# ---------------------------------------------------------------------------
# Whitespace Allocation: recursive slicing tree + cut-line shifting
# ---------------------------------------------------------------------------


@dataclass
class SliceNode:
    ids: List[int]
    bounds: Tuple[float, float, float, float]
    axis: Optional[str] = None
    old_cut: float = 0.0
    left: Optional["SliceNode"] = None
    right: Optional["SliceNode"] = None
    demand: float = 0.0


def _rect_overlap_area(rect: Tuple[float, float, float, float], bounds: Tuple[float, float, float, float]) -> float:
    x0, x1, y0, y1 = rect; bx0, bx1, by0, by1 = bounds
    return max(0.0, min(x1, bx1) - max(x0, bx0)) * max(0.0, min(y1, by1) - max(y0, by0))


def _build_slice_tree(level: Level, ids: List[int], bounds: Tuple[float, float, float, float], leaf_size: int) -> SliceNode:
    node = SliceNode(ids=ids, bounds=bounds)
    if len(ids) <= leaf_size:
        return node
    xl, xh, yl, yh = bounds
    axis = "x" if (xh - xl) >= (yh - yl) else "y"
    ordered = sorted(ids, key=lambda i: level.objects[i].cx if axis == "x" else level.objects[i].cy)
    mid = len(ordered) // 2
    left_ids, right_ids = ordered[:mid], ordered[mid:]
    if not left_ids or not right_ids:
        return node
    if axis == "x":
        a = level.objects[left_ids[-1]].cx; b = level.objects[right_ids[0]].cx
        cut = min(xh - EPS, max(xl + EPS, 0.5 * (a + b)))
        lb = (xl, cut, yl, yh); rb = (cut, xh, yl, yh)
    else:
        a = level.objects[left_ids[-1]].cy; b = level.objects[right_ids[0]].cy
        cut = min(yh - EPS, max(yl + EPS, 0.5 * (a + b)))
        lb = (xl, xh, yl, cut); rb = (xl, xh, cut, yh)
    node.axis, node.old_cut = axis, cut
    node.left = _build_slice_tree(level, left_ids, lb, leaf_size)
    node.right = _build_slice_tree(level, right_ids, rb, leaf_size)
    return node


def _compute_slice_demand(
    node: SliceNode,
    level: Level,
    fixed_rects: Sequence[Tuple[float, float, float, float]],
    target_density: float,
) -> float:
    if node.left is None or node.right is None:
        movable = sum(level.objects[i].area for i in node.ids) / max(target_density, EPS)
        fixed = sum(_rect_overlap_area(r, node.bounds) for r in fixed_rects)
        node.demand = movable + fixed
    else:
        node.demand = (
            _compute_slice_demand(node.left, level, fixed_rects, target_density) +
            _compute_slice_demand(node.right, level, fixed_rects, target_density)
        )
    return node.demand


def _affine_map(value: float, old0: float, old1: float, new0: float, new1: float) -> float:
    if old1 <= old0 + EPS:
        return 0.5 * (new0 + new1)
    t = min(1.0, max(0.0, (value - old0) / (old1 - old0)))
    return new0 + t * (new1 - new0)


def _allocate_slice(node: SliceNode, level: Level, new_bounds: Tuple[float, float, float, float], min_fraction: float) -> None:
    if node.left is None or node.right is None or node.axis is None:
        ox0, ox1, oy0, oy1 = node.bounds
        nx0, nx1, ny0, ny1 = new_bounds
        for oid in node.ids:
            o = level.objects[oid]
            cx = _affine_map(o.cx, ox0, ox1, nx0, nx1)
            cy = _affine_map(o.cy, oy0, oy1, ny0, ny1)
            o.set_center(cx, cy)
        return
    xl, xh, yl, yh = new_bounds
    total = max(node.left.demand + node.right.demand, EPS)
    frac = min(1.0 - min_fraction, max(min_fraction, node.left.demand / total))
    if node.axis == "x":
        cut = xl + frac * (xh - xl)
        _allocate_slice(node.left, level, (xl, cut, yl, yh), min_fraction)
        _allocate_slice(node.right, level, (cut, xh, yl, yh), min_fraction)
    else:
        cut = yl + frac * (yh - yl)
        _allocate_slice(node.left, level, (xl, xh, yl, cut), min_fraction)
        _allocate_slice(node.right, level, (xl, xh, cut, yh), min_fraction)


def whitespace_allocation(
    level: Level,
    region: Tuple[float, float, float, float],
    target_density: float,
    leaf_size: int,
    min_fraction: float,
) -> Dict[str, float]:
    locked = set(level.macro_ids)
    ids = [i for i in level.movable_ids if i not in locked]
    if len(ids) < 2:
        return {"objects": len(ids), "leaves": 0, "rms_displacement": 0.0}
    before = {i: (level.objects[i].cx, level.objects[i].cy) for i in ids}
    root = _build_slice_tree(level, ids, region, leaf_size)
    fixed_rects = [(o.x, o.x + o.width, o.y, o.y + o.height)
                   for o in level.objects if o.fixed or (not o.fixed and o.is_macro)]
    _compute_slice_demand(root, level, fixed_rects, target_density)
    _allocate_slice(root, level, region, min_fraction)
    project_level(level, region)
    move2 = sum((level.objects[i].cx - before[i][0]) ** 2 +
                (level.objects[i].cy - before[i][1]) ** 2 for i in ids)
    leaves = 0
    stack = [root]
    while stack:
        n = stack.pop()
        if n.left is None:
            leaves += 1
        else:
            stack.extend([n.left, n.right])
    return {"objects": len(ids), "leaves": leaves,
            "rms_displacement": math.sqrt(move2 / max(1, 2 * len(ids)))}


# ---------------------------------------------------------------------------
# Declustering
# ---------------------------------------------------------------------------


def decluster(
    coarse: Level,
    fine: Level,
    region: Tuple[float, float, float, float],
    seed: int,
) -> Dict[str, float]:
    del seed  # deterministic packing is already stored in child_offsets
    mapping = coarse.fine_to_coarse
    if mapping is None or len(mapping) != len(fine.objects):
        raise ValueError("invalid hierarchy mapping during declustering")

    xl, xh, yl, yh = region
    by_parent: DefaultDict[int, List[int]] = defaultdict(list)
    for child, parent in enumerate(mapping):
        by_parent[parent].append(child)

    shifted_parents = 0
    max_parent_shift = 0.0
    impossible_groups = 0
    for parent_id, children in by_parent.items():
        parent = coarse.objects[parent_id]
        movable_children = [i for i in children if not fine.objects[i].fixed]
        if not movable_children:
            continue

        min_dx = min(
            parent.child_offsets.get(i, (0.0, 0.0))[0] - 0.5 * fine.objects[i].width
            for i in movable_children
        )
        max_dx = max(
            parent.child_offsets.get(i, (0.0, 0.0))[0] + 0.5 * fine.objects[i].width
            for i in movable_children
        )
        min_dy = min(
            parent.child_offsets.get(i, (0.0, 0.0))[1] - 0.5 * fine.objects[i].height
            for i in movable_children
        )
        max_dy = max(
            parent.child_offsets.get(i, (0.0, 0.0))[1] + 0.5 * fine.objects[i].height
            for i in movable_children
        )

        lo_cx, hi_cx = xl - min_dx, xh - max_dx
        lo_cy, hi_cy = yl - min_dy, yh - max_dy
        if lo_cx > hi_cx + EPS or lo_cy > hi_cy + EPS:
            impossible_groups += 1
            raise ValueError(
                f"child packing of {parent.name} does not fit in placement region"
            )

        anchor_cx = min(max(parent.cx, lo_cx), hi_cx)
        anchor_cy = min(max(parent.cy, lo_cy), hi_cy)
        shift = math.hypot(anchor_cx - parent.cx, anchor_cy - parent.cy)
        if shift > EPS:
            shifted_parents += 1
            max_parent_shift = max(max_parent_shift, shift)
            # Shift the coarse object itself so its pin coordinates remain the
            # exact reference for the fine placement created below.
            parent.set_center(anchor_cx, anchor_cy)

        for child in movable_children:
            dx, dy = parent.child_offsets.get(child, (0.0, 0.0))
            fine.objects[child].set_center(anchor_cx + dx, anchor_cy + dy)

    # No independent per-child projection: it would collapse multiple children
    # onto the same boundary coordinate and break HPWL continuity.
    return {
        "parents": float(len(by_parent)),
        "shifted_parents": float(shifted_parents),
        "max_parent_shift": max_parent_shift,
        "impossible_groups": float(impossible_groups),
    }


# ---------------------------------------------------------------------------
# Output and driver
# ---------------------------------------------------------------------------


def write_level_pl(path: Path, level: Level) -> None:
    with path.open("w", encoding="utf-8") as f:
        f.write("UCLA pl 1.0\n")
        for o in level.objects:
            suffix = " /FIXED" if o.fixed else ""
            f.write(f"{o.name}\t{o.x:.6f}\t{o.y:.6f}\t: N{suffix}\n")


def write_final_pl(path: Path, db: PlacementDB, level0: Level) -> None:
    for oid, o in enumerate(level0.objects):
        if len(o.members) == 1:
            cid = o.members[0]
            if not db.cells[cid].fixed:
                db.cells[cid].x, db.cells[cid].y = o.x, o.y
    with path.open("w", encoding="utf-8") as f:
        f.write("UCLA pl 1.0\n")
        for c in db.cells:
            suffix = " /FIXED" if c.fixed else ""
            f.write(f"{c.name}\t{c.x:.6f}\t{c.y:.6f}\t: {c.orientation}{suffix}\n")


def _history_fields() -> List[str]:
    return [
        "global_iteration", "level", "stage", "iteration", "hpwl",
        "density_penalty", "ofr_penalty", "ofr_report", "max_density",
        "overflow_bins_penalty", "overflow_bins_report", "lambda", "beta_pr",
        "step", "gradient_rms", "total_norm", "elapsed_sec",
    ]


def run(args: argparse.Namespace) -> int:
    start = time.perf_counter()
    aux = Path(args.aux).resolve()
    out = Path(args.out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    print(f"[load] {aux}")
    db = load_bookshelf(aux)
    region = db.region()
    level0 = build_level0(db)
    print(f"[load] cells={len(db.cells)} movable={len(level0.movable_ids)} nets={len(level0.nets)}")
    if _needs_nullspace_seed(level0):
        print("[init] movable coordinates coincide; deferring real initialization to coarsest quadratic solve")

    levels = build_hierarchy(level0, args.current, args.coarsen_ratio,
                             args.max_levels, args.cluster_degree_cap)
    hierarchy_info = {
        "levels": [
            {"index": lv.index, "objects": len(lv.objects), "movable": len(lv.movable_ids),
             "macros": len(lv.macro_ids), "nets": len(lv.nets)} for lv in levels
        ]
    }
    (out / "hierarchy.json").write_text(json.dumps(hierarchy_info, indent=2), encoding="utf-8")
    coarsest = levels[-1]
    write_level_pl(out / "coarsest_before_quadratic.pl", coarsest)
    quad_stats = {"enabled": False}
    if args.quadratic_init:
        print(f"[quadratic] L{coarsest.index}")
        q = quadratic_initialize(coarsest, region, args.quadratic_iterations,
                                 args.quadratic_damping, args.quadratic_anchor,
                                 args.quadratic_tolerance, args.seed)
        quad_stats = {"enabled": True, **q}
    elif _needs_nullspace_seed(coarsest):
        seed_grid(coarsest, region, args.seed)
    # Enforce both the coarse density rectangle and the stored child envelope.
    project_level(coarsest, region)
    write_level_pl(out / "coarsest_after_quadratic.pl", coarsest)

    penalty_density = args.penalty_density if args.penalty_density is not None else args.target_density
    ofr_density = args.ofr_density if args.ofr_density is not None else args.target_density
    row_h = _median_or([r.height for r in db.rows if r.height > 0], 1.0)
    history_path = out / "history.csv"
    level_summaries: List[Dict[str, object]] = []
    interlevel_diagnostics: List[Dict[str, object]] = []
    adaptive_current = args.current
    with history_path.open("w", newline="", encoding="utf-8") as hf:
        writer = csv.DictWriter(hf, fieldnames=_history_fields())
        writer.writeheader()
        global_state: Dict[str, object] = {"iteration": 0, "start_time": start, "history_file": hf}
        for idx in range(len(levels) - 1, -1, -1):
            level = levels[idx]
            decluster_info: Optional[Dict[str, object]] = None
            if idx < len(levels) - 1:
                coarse = levels[idx + 1]
                coarse_before_fit = exact_hpwl(coarse)
                transfer_stats = decluster(coarse, level, region, args.seed)
                consistency = interlevel_hpwl_consistency(coarse, level)
                decluster_info = {
                    "from_level": coarse.index,
                    "to_level": level.index,
                    "coarse_hpwl_before_boundary_fit": coarse_before_fit,
                    **transfer_stats,
                    **consistency,
                }
                interlevel_diagnostics.append(decluster_info)
                print(
                    f"[decluster L{coarse.index}->L{level.index}] "
                    f"coarse={consistency['coarse_hpwl']:.6e} "
                    f"fine={consistency['fine_hpwl']:.6e} "
                    f"delta={consistency['delta']:.6e} "
                    f"rel={consistency['relative_delta']:.3e}"
                )
                if abs(float(consistency["relative_delta"])) > args.hpwl_continuity_tol:
                    print(
                        f"[warning] inter-level HPWL relative delta exceeds "
                        f"{args.hpwl_continuity_tol:g}"
                    )
                adaptive_current = min(
                    2 * adaptive_current,
                    max(1, int(math.ceil(math.sqrt(len(level.movable_ids))))),
                )
            if args.bins_x is not None and args.bins_y is not None:
                bins_x, bins_y = args.bins_x, args.bins_y
            else:
                bins_x = bins_y = adaptive_current
            project_level(level, region)
            grid = DensityGrid(bins_x, bins_y, *region, penalty_density, ofr_density)
            grid.build_fixed(level)
            default_s0 = args.s0 if args.s0 > 0.0 else 0.25 * min(grid.bin_w, grid.bin_h)
            default_floor = args.s_floor if args.s_floor > 0.0 else 1.0e-3 * min(grid.bin_w, grid.bin_h)
            cfg = OptimizeConfig(
                iterations_per_stage=args.iterations_per_stage,
                penalty_stages=args.penalty_stages,
                wire_mode=args.wirelength_mode,
                density_only=args.density_only,
                s0=default_s0,
                s_floor=default_floor,
                step_decay=args.step_decay,
                lambda0=args.lambda0,
                lambda_growth_high=args.lambda_growth_high,
                lambda_growth_mid=args.lambda_growth_mid,
                lambda_growth_low=args.lambda_growth_low,
                density_gradient_ratio=args.density_gradient_ratio,
                report_every=args.report_every,
                target_ofr=args.target_ofr,
                nmax=args.nmax,
            )
            print(f"[optimize] L{level.index} movable={len(level.movable_ids)} bins={bins_x}x{bins_y}")
            opt_stats = optimize_level(level, grid, region, cfg, writer, global_state)  # type: ignore[arg-type]
            inter: Dict[str, object] = {}
            if idx > 0 and args.macro_shifting:
                hpwl_before_ms = exact_hpwl(level)
                ms = macro_shifting(level, region, row_h, args.macro_search_rings, args.macro_gap)
                hpwl_after_ms = exact_hpwl(level)
                inter["macro_shifting"] = {
                    **ms,
                    "hpwl_before": hpwl_before_ms,
                    "hpwl_after": hpwl_after_ms,
                    "hpwl_delta": hpwl_after_ms - hpwl_before_ms,
                }
                print(f"[macro shifting] L{level.index} {inter['macro_shifting']}")
            write_level_pl(out / f"level_{level.index}_final.pl", level)
            level_summaries.append({
                "level": level.index,
                "bins": [bins_x, bins_y],
                "optimization": opt_stats,
                "post_interlevel_hpwl": exact_hpwl(level),
                **({"decluster": decluster_info} if decluster_info is not None else {}),
                **inter,
            })

    final_path = out / "final.pl"
    write_final_pl(final_path, db, level0)
    run_info = {
        "aux": str(aux),
        "region": {"xl": region[0], "xh": region[1], "yl": region[2], "yh": region[3]},
        "arguments": vars(args),
        "penalty_density": penalty_density,
        "ofr_density": ofr_density,
        "quadratic": quad_stats,
        "elapsed_sec": time.perf_counter() - start,
        "global_placement_only": True,
        "note": "HPWL-consistent coarse-to-fine global placement; WSA disabled; final legalization is external.",
    }
    (out / "run_info.json").write_text(json.dumps(run_info, indent=2), encoding="utf-8")
    (out / "interlevel_hpwl.json").write_text(
        json.dumps(interlevel_diagnostics, indent=2), encoding="utf-8"
    )
    (out / "summary.json").write_text(
        json.dumps({"levels": level_summaries, "interlevel_hpwl": interlevel_diagnostics}, indent=2),
        encoding="utf-8",
    )
    print(f"[done] {final_path}")
    print(f"[done] elapsed={run_info['elapsed_sec']:.2f}s")
    return 0


def build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Single-file multilevel nonsmooth VLSI global placer")
    p.add_argument("aux", help="Bookshelf .aux file")
    p.add_argument("--out", default="output/nonsmooth_single")
    p.add_argument("--wirelength-mode", choices=("paper_l1", "b2b", "extrema"), default="paper_l1")
    p.add_argument("--target-density", type=float, default=1.0,
                   help="default threshold used by both penalty and OFR")
    p.add_argument("--penalty-density", type=float, default=None,
                   help="density threshold activating the penalty, e.g. 0.8")
    p.add_argument("--ofr-density", type=float, default=None,
                   help="separate OFR reporting threshold, e.g. 1.0")
    p.add_argument("--bins", nargs=2, type=int, metavar=("NX", "NY"), default=None)
    p.add_argument("--current", type=int, default=150,
                   help="coarsest movable-object threshold is current^2")
    p.add_argument("--coarsen-ratio", type=float, default=5.0)
    p.add_argument("--max-levels", type=int, default=16)
    p.add_argument("--cluster-degree-cap", type=int, default=256)

    p.add_argument("--quadratic-init", action=argparse.BooleanOptionalAction, default=True)
    p.add_argument("--quadratic-iterations", type=int, default=200)
    p.add_argument("--quadratic-damping", type=float, default=0.75)
    p.add_argument("--quadratic-anchor", type=float, default=1.0e-4)
    p.add_argument("--quadratic-tolerance", type=float, default=1.0e-3)

    p.add_argument("--iterations", type=int, default=None,
                   help="compatibility alias for --iterations-per-stage")
    p.add_argument("--iterations-per-stage", type=int, default=100)
    p.add_argument("--penalty-stages", type=int, default=4)
    p.add_argument("--density-only", action=argparse.BooleanOptionalAction, default=False)
    p.add_argument("--lambda0", type=float, default=0.0,
                   help="0 means initialize from wire/density gradient ratio")
    p.add_argument("--density-gradient-ratio", type=float, default=1.0)
    p.add_argument("--lambda-growth-high", type=float, default=2.2)
    p.add_argument("--lambda-growth-mid", type=float, default=1.9)
    p.add_argument("--lambda-growth-low", type=float, default=1.6)
    p.add_argument("--s0", type=float, default=0.0,
                   help="initial RMS displacement; 0 selects 0.25*bin size")
    p.add_argument("--s-floor", type=float, default=0.0)
    p.add_argument("--step-decay", type=float, default=200.0)
    p.add_argument("--target-ofr", type=float, default=0.0)
    p.add_argument("--report-every", type=int, default=10)
    p.add_argument("--nmax", type=int, default=1_000_000,
                   help="stop after this many consecutive non-improving objective evaluations; <=0 disables")
    p.add_argument("--hpwl-continuity-tol", type=float, default=1.0e-8,
                   help="warning threshold for relative HPWL delta across declustering")

    p.add_argument("--macro-shifting", action=argparse.BooleanOptionalAction, default=True)
    p.add_argument("--macro-search-rings", type=int, default=30)
    p.add_argument("--macro-gap", type=float, default=0.0)
    p.add_argument("--whitespace-allocation", action=argparse.BooleanOptionalAction, default=False,
                   help="deprecated compatibility flag; WSA is disabled in this corrected version")
    p.add_argument("--wsa-leaf-size", type=int, default=64)
    p.add_argument("--wsa-min-fraction", type=float, default=0.05)
    p.add_argument("--seed", type=int, default=1)
    return p


def main() -> int:
    parser = build_arg_parser()
    args = parser.parse_args()
    if args.iterations is not None:
        args.iterations_per_stage = args.iterations
    if args.bins is None:
        args.bins_x = args.bins_y = None
    else:
        args.bins_x, args.bins_y = args.bins
    delattr(args, "bins")
    if not (0.0 < args.target_density <= 1.0):
        parser.error("--target-density must be in (0,1]")
    for name in ("penalty_density", "ofr_density"):
        value = getattr(args, name)
        if value is not None and not (0.0 < value <= 1.0):
            parser.error(f"--{name.replace('_','-')} must be in (0,1]")
    if args.current <= 0 or args.cluster_degree_cap < 2:
        parser.error("--current must be positive and --cluster-degree-cap >= 2")
    if args.coarsen_ratio <= 1.0:
        parser.error("--coarsen-ratio must be > 1")
    if args.iterations_per_stage < 0 or args.penalty_stages <= 0:
        parser.error("iteration counts must be nonnegative/positive")
    if args.hpwl_continuity_tol < 0.0:
        parser.error("--hpwl-continuity-tol must be nonnegative")
    if min(args.lambda_growth_low, args.lambda_growth_mid, args.lambda_growth_high) <= 0.0:
        parser.error("lambda growth factors must be positive")
    if not (0.0 < args.quadratic_damping <= 1.0):
        parser.error("--quadratic-damping must be in (0,1]")
    if not (0.0 < args.wsa_min_fraction < 0.5):
        parser.error("--wsa-min-fraction must be in (0,0.5)")
    try:
        return run(args)
    except KeyboardInterrupt:
        print("interrupted", file=sys.stderr)
        return 130
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
