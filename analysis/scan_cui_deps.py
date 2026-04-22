#!/usr/bin/env python3
"""Scan mofclient.c for CUI* class dependencies on DCTTextManager and clt*Info classes.

Outputs, per class:
  - depends_on_text: "Y" if the class uses DCTTextManager (MoFTexts.txt) or reads from
    any clt*Info / clt*KindInfo static, else "N".
  - text_sources: comma-separated sources. "MoFTexts.txt" is listed if DCTTextManager
    is used; Info class names are listed alphabetically.

Also regenerates cui_restore_order.md and cui_restore_order.tsv with the two new columns.
"""
import re
from pathlib import Path

MOF_C = Path(r"d:\Study\MOF\mofclient.c")
TSV   = Path(r"d:\Study\MOF\analysis\cui_restore_order.tsv")
MD    = Path(r"d:\Study\MOF\analysis\cui_restore_order.md")

SIG_RE = re.compile(
    r'^[A-Za-z_].*?\b(?P<cls>(?:CUI[A-Za-z0-9_]+))::[A-Za-z_~`][^;]*\)\s*$'
)
INFO_CLS_RE = re.compile(r'\bclt[A-Za-z]+Info::')
DCTTXT_RE   = re.compile(r'\bDCTTextManager\b')


def load_rows():
    rows = []
    with TSV.open(encoding='utf-8') as f:
        header = next(f).rstrip('\n').split('\t')
        for line in f:
            line = line.rstrip('\n')
            if not line:
                continue
            parts = line.split('\t')
            # pad missing trailing fields
            while len(parts) < len(header):
                parts.append('')
            rows.append(parts)
    return header, rows


def scan(classes):
    per = {c: {'dct_hits': 0, 'info_refs': {}} for c in classes}
    lines = MOF_C.read_text(encoding='utf-8', errors='replace').splitlines()

    i, n = 0, len(lines)
    while i < n:
        line = lines[i]
        m = SIG_RE.match(line)
        if not m or (i + 1) >= n or lines[i + 1] != '{':
            i += 1
            continue
        cls = m.group('cls')
        if cls not in per:
            i += 1
            continue

        # Walk the function body balancing braces.
        j = i + 2
        depth = 1
        body = []
        while j < n:
            ln = lines[j]
            if ln == '}':
                depth -= 1
                if depth == 0:
                    break
            elif ln == '{':
                depth += 1
            body.append(ln)
            j += 1

        pc = per[cls]
        for bl in body:
            if DCTTXT_RE.search(bl):
                pc['dct_hits'] += 1
            for im in INFO_CLS_RE.finditer(bl):
                name = im.group(0)[:-2]
                pc['info_refs'][name] = pc['info_refs'].get(name, 0) + 1
        i = j + 1

    return per


def summarise(pc):
    sources = []
    if pc['dct_hits'] > 0:
        sources.append('MoFTexts.txt')
    for name in sorted(pc['info_refs']):
        sources.append(name)
    depends = 'Y' if sources else 'N'
    return depends, ', '.join(sources)


def write_tsv(header, rows):
    # New header: class, lines, parents, depends_on_text, text_sources
    new_header = ['class', 'lines', 'parents', 'depends_on_text', 'text_sources']
    with TSV.open('w', encoding='utf-8', newline='\n') as f:
        f.write('\t'.join(new_header) + '\n')
        for r in rows:
            f.write('\t'.join(r) + '\n')


def write_md(rows):
    # Rebuild the markdown, preserving the preamble and the numbered "order" column.
    preamble = [
        '# CUI classes reverse-engineering order (derived from mofclient.c)',
        '',
        '- Source: `mofclient.c` function definition ranges and constructor calls.',
        '- `lines` = sum of decompiled function block lines for each class in `mofclient.c`.',
        '- `parents` = inferred base class from constructor call (`Parent::Parent(...)`).',
        '- `depends_on_text` = `Y` if the class uses `DCTTextManager` (reads `MoFTexts.txt`) or reads from a `clt*Info` / `clt*KindInfo` static.',
        '- `text_sources` = `MoFTexts.txt` when `DCTTextManager` is used, plus any `clt*Info` / `clt*KindInfo` classes the code reads from.',
        '',
        '| order | class | lines | parents | depends_on_text | text_sources |',
        '|---:|---|---:|---|:---:|---|',
    ]
    body = []
    for idx, r in enumerate(rows, start=1):
        cls, lines, parents, dep, srcs = r
        parents_md = parents if parents else '-'
        srcs_md = srcs if srcs else '-'
        body.append(f'| {idx} | {cls} | {lines} | {parents_md} | {dep} | {srcs_md} |')
    MD.write_text('\n'.join(preamble + body) + '\n', encoding='utf-8')


def main():
    header, rows = load_rows()
    classes = [r[0] for r in rows]
    per = scan(classes)

    new_rows = []
    for r in rows:
        cls = r[0]
        lines = r[1]
        parents = r[2] if len(r) >= 3 else ''
        dep, srcs = summarise(per[cls])
        new_rows.append([cls, lines, parents, dep, srcs])

    write_tsv(header, new_rows)
    write_md(new_rows)

    # Print a short summary for visibility.
    yes = sum(1 for r in new_rows if r[3] == 'Y')
    print(f'Classes marked Y: {yes}/{len(new_rows)}')


if __name__ == '__main__':
    main()
