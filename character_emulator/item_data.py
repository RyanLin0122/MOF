"""
Parser for the per-item CA mapping tables.

Two source files are loaded by ``CAManager`` in mofclient.c:

* ``CA_Character_Dot.txt``         (parsed by ``CAManager::LoadCADataDot``,
  mofclient.c:245180) — text columns ``%s %s %s %d %s %d``
  → ``code  type1  name  intval1  type2  intval2``

* ``CA_Character_illustration.txt`` (parsed by ``CAManager::LoadCADataIllust``,
  mofclient.c:245394) — text columns ``%s %d %d %s``
  → ``code  byteval  intval  type``

Each row binds a 5-character item code (e.g. ``H0001``) to:
  • a fashion *slot type* (HAIR / FACE / COAT / …)
  • a *layer index* inside that slot's ``.ca`` file

The slot type → integer mapping comes from
``CAManager::ParsingFashionItemType`` (mofclient.c:245488)::

    HAIR=0  FACE=1  COAT=2  TRIUSERS=3  SHOES=4  HAND=5  CLOCK=6
    RWEAPON=7  LWEAPON=8  ACC1=9  ACC2=10  ACC3=11  MASKHAIR=12
    SUIT=13  EMOTICON=14  DUALWEAPON=15

The 16-bit item ID stored in the live game comes from
``CAManager::ItemtoWORD`` (mofclient.c:246219)::

    ((toupper(letter) + 31) << 11) | atoi(digits)

Public API::

    from item_data import (load_dot_items, load_illust_items,
                           item_code_to_id, item_id_to_code,
                           SLOT_NAMES)

    items = load_dot_items("MoFData/CA_Character_Dot.txt")
    print(items["H0001"])   # ItemCAInfoDot(...)
    print(item_code_to_id("H0001"))
"""

import os


# ── Constants matching CAManager::ParsingFashionItemType ─────────────────────

SLOT_HAIR        = 0
SLOT_FACE        = 1
SLOT_COAT        = 2
SLOT_TRIUSERS    = 3
SLOT_SHOES       = 4
SLOT_HAND        = 5
SLOT_CLOCK       = 6
SLOT_RWEAPON     = 7
SLOT_LWEAPON     = 8
SLOT_ACC1        = 9
SLOT_ACC2        = 10
SLOT_ACC3        = 11
SLOT_MASKHAIR    = 12
SLOT_SUIT        = 13
SLOT_EMOTICON    = 14
SLOT_DUALWEAPON  = 15

SLOT_NAMES = {
    SLOT_HAIR:       'HAIR',
    SLOT_FACE:       'FACE',
    SLOT_COAT:       'COAT',
    SLOT_TRIUSERS:   'TRIUSERS',
    SLOT_SHOES:      'SHOES',
    SLOT_HAND:       'HAND',
    SLOT_CLOCK:      'CLOCK',
    SLOT_RWEAPON:    'RWEAPON',
    SLOT_LWEAPON:    'LWEAPON',
    SLOT_ACC1:       'ACC1',
    SLOT_ACC2:       'ACC2',
    SLOT_ACC3:       'ACC3',
    SLOT_MASKHAIR:   'MASKHAIR',
    SLOT_SUIT:       'SUIT',
    SLOT_EMOTICON:   'EMOTICON',
    SLOT_DUALWEAPON: 'DUALWEAPON',
}

# Reverse: name → slot id (case-insensitive)
SLOT_BY_NAME = {v.upper(): k for k, v in SLOT_NAMES.items()}
# Aliases sometimes seen in the data files
SLOT_BY_NAME['NONE'] = -1
SLOT_BY_NAME['NULL'] = -1


def parse_slot_type(name):
    """Return the slot id for a slot keyword, or -1 if not recognised.

    Mirrors ``CAManager::ParsingFashionItemType`` (mofclient.c:245488).
    """
    if not name:
        return -1
    return SLOT_BY_NAME.get(name.upper(), -1)


# ── Item code ⇄ 16-bit id (CAManager::ItemtoWORD, mofclient.c:246219) ────────

def item_code_to_id(code):
    """Convert a 5-character item code (e.g. ``H0001``) to a 16-bit id.

    Returns 0 on failure, matching ``ItemtoWORD``'s convention.
    """
    if not code or len(code) != 5:
        return 0
    try:
        upper = (ord(code[0].upper()) + 31) << 11
        digits = int(code[1:])
    except (ValueError, IndexError):
        return 0
    if digits >= 0x800:
        return 0
    return (upper | digits) & 0xFFFF


def item_id_to_code(item_id):
    """Inverse of :func:`item_code_to_id`.

    The forward mapping is ``((toupper(c) + 31) << 11) | digits`` and the
    result is truncated to 16 bits, so only the low 5 bits of ``c + 31``
    survive. For ``'A'..'Z'`` (65..90), ``c + 31`` is ``96..121``, whose low
    5 bits are ``0..25`` — therefore the inverse is simply ``chr(bits + 65)``.
    """
    if item_id <= 0:
        return ''
    bits = (item_id >> 11) & 0x1F
    digits = item_id & 0x7FF
    return '%c%04d' % (chr(bits + ord('A')), digits)


# ── Data classes ──────────────────────────────────────────────────────────────

class ItemCAInfoDot:
    """An entry of CA_Character_Dot.txt (per-item slot bindings).

    Mirrors the 0x78-byte ``ITEMCAINFO_DOT`` allocated in
    ``CAManager::LoadCADataDot`` (mofclient.c:245209).
    """
    __slots__ = ('code', 'item_id', 'name',
                 'type1', 'layer_index1',
                 'type2', 'layer_index2')

    def __init__(self, code='', name='',
                 type1=-1, layer_index1=0,
                 type2=-1, layer_index2=0):
        self.code = code
        self.item_id = item_code_to_id(code)
        self.name = name
        self.type1 = type1
        self.layer_index1 = layer_index1
        self.type2 = type2
        self.layer_index2 = layer_index2

    @property
    def primary_slot_name(self):
        return SLOT_NAMES.get(self.type1, '?')

    @property
    def secondary_slot_name(self):
        return SLOT_NAMES.get(self.type2, '?')

    def __repr__(self):
        return ('ItemCAInfoDot(%s id=0x%04X "%s" %s/%d + %s/%d)'
                % (self.code, self.item_id, self.name,
                   self.primary_slot_name, self.layer_index1,
                   self.secondary_slot_name, self.layer_index2))


class ItemCAInfoIllust:
    """An entry of CA_Character_illustration.txt.

    Mirrors the 0x0C-byte ``ITEMCAINFO_ILLUST`` allocated in
    ``CAManager::LoadCADataIllust`` (mofclient.c:245420).
    """
    __slots__ = ('code', 'item_id', 'byte_val', 'frame_index', 'type')

    def __init__(self, code='', byte_val=0, frame_index=0, type=-1):
        self.code = code
        self.item_id = item_code_to_id(code)
        self.byte_val = byte_val
        self.frame_index = frame_index
        self.type = type

    @property
    def slot_name(self):
        return SLOT_NAMES.get(self.type, '?')

    def __repr__(self):
        return ('ItemCAInfoIllust(%s id=0x%04X %s frame=%d byte=%d)'
                % (self.code, self.item_id, self.slot_name,
                   self.frame_index, self.byte_val))


# ── Loaders ───────────────────────────────────────────────────────────────────

def _open_text(path):
    """Open a text file, trying common Korean / Asian encodings."""
    encodings = ('utf-8-sig', 'cp949', 'cp950', 'big5', 'utf-8', 'latin-1')
    last_err = None
    for enc in encodings:
        try:
            with open(path, 'r', encoding=enc) as f:
                return f.read().splitlines(), enc
        except UnicodeDecodeError as e:
            last_err = e
    raise last_err


def load_dot_items(path):
    """Parse ``CA_Character_Dot.txt`` and return ``dict[str, ItemCAInfoDot]``.

    The first line is a header (skipped), then each subsequent line is::

        code  type1  name  intval1  type2  intval2

    Tab- or whitespace-separated; mirrors the ``%s %s %s %d %s %d`` sscanf in
    ``CAManager::LoadCADataDot`` (mofclient.c:245204).
    """
    if not os.path.isfile(path):
        return {}
    lines, _enc = _open_text(path)
    out = {}
    started = False
    for raw_line in lines:
        line = raw_line.strip()
        if not line or line.startswith('//') or line.startswith('#'):
            continue
        # Skip the first non-empty/comment line — it is the header.
        if not started:
            started = True
            continue
        parts = line.split()
        if len(parts) < 6:
            continue
        code, type1_name, name, intval1, type2_name, intval2 = parts[:6]
        try:
            iv1 = int(intval1)
            iv2 = int(intval2)
        except ValueError:
            continue
        info = ItemCAInfoDot(
            code=code,
            name=name,
            type1=parse_slot_type(type1_name),
            layer_index1=iv1,
            type2=parse_slot_type(type2_name),
            layer_index2=iv2,
        )
        if info.item_id:
            out[code.upper()] = info
    return out


def load_illust_items(path):
    """Parse ``CA_Character_illustration.txt`` → ``dict[str, ItemCAInfoIllust]``.

    Each line is ``code byteval intval type`` (``%s %d %d %s``), matching
    ``CAManager::LoadCADataIllust`` (mofclient.c:245418).
    """
    if not os.path.isfile(path):
        return {}
    lines, _enc = _open_text(path)
    out = {}
    started = False
    for raw_line in lines:
        line = raw_line.strip()
        if not line or line.startswith('//') or line.startswith('#'):
            continue
        if not started:
            started = True
            continue
        parts = line.split()
        if len(parts) < 4:
            continue
        code, byte_val, frame_index, type_name = parts[:4]
        try:
            bv = int(byte_val)
            fi = int(frame_index)
        except ValueError:
            continue
        info = ItemCAInfoIllust(
            code=code,
            byte_val=bv,
            frame_index=fi,
            type=parse_slot_type(type_name),
        )
        if info.item_id:
            out[code.upper()] = info
    return out


# ── CLI ───────────────────────────────────────────────────────────────────────

def main():
    import argparse
    parser = argparse.ArgumentParser(
        description='Dump CA item-binding tables (DOT/ILLUST).')
    parser.add_argument('--dot',    help='CA_Character_Dot.txt')
    parser.add_argument('--illust', help='CA_Character_illustration.txt')
    parser.add_argument('--filter', help='Only show entries whose code starts with this prefix')
    args = parser.parse_args()

    if args.dot:
        items = load_dot_items(args.dot)
        print('DOT  items:', len(items))
        for code, info in sorted(items.items()):
            if args.filter and not code.startswith(args.filter.upper()):
                continue
            print(' ', info)
    if args.illust:
        items = load_illust_items(args.illust)
        print('ILLUST items:', len(items))
        for code, info in sorted(items.items()):
            if args.filter and not code.startswith(args.filter.upper()):
                continue
            print(' ', info)


if __name__ == '__main__':
    main()
