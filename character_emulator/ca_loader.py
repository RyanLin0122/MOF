"""
High-level loader that mirrors ``CAManager::LoadCADataDot`` /
``LoadCADataIllust`` in mofclient.c.

The MoF client keeps **17 ``TIMELINEINFO`` instances** for the dot character
and 7 + 1 instances for the illust character. They are loaded by individual
``CAManager::LoadTimeline`` calls (mofclient.c:245327-245383)::

    Dot_Character_HAIR.ca           → slot 0  HAIR
    Dot_Character_FACE.ca           → slot 1  FACE
    Dot_Character_COAT.ca           → slot 2  COAT
    Dot_Character_TRIUSERS.ca       → slot 3  TRIUSERS
    Dot_Character_SHOES.ca          → slot 4  SHOES
    Dot_Character_HAND.ca           → slot 5  HAND
    Dot_Character_CLOCK.ca          → slot 6  CLOCK
    Dot_Character_RIGHTHANDWEAPON.ca→ slot 7  RWEAPON
    Dot_Character_LEFTHANDWEAPON.ca → slot 8  LWEAPON
    Dot_Character_ACCESSORY1.ca     → slot 9  ACC1
    Dot_Character_ACCESSORY2.ca     → slot 10 ACC2
    Dot_Character_ACCESSORY3.ca     → slot 11 ACC3
    Dot_Character_MASKHAIR.ca       → slot 12 MASKHAIR
    Dot_Character_SUIT.ca           → slot 13 SUIT
    Dot_Character_EMOTICON.ca       → slot 14 EMOTICON
    Dot_Character_DUALWEAPON.ca     → slot 15 DUALWEAPON
    Dot_Character.ca                → BASE (keys + animation timing)

The ``Dot_Character.ca`` is the **base timeline**: it is the only file that is
loaded with a non-NULL ``CANIMATIONINFO`` argument and therefore contains the
animation key list (mofclient.c:245319-245323). Its 23 layers are also used as
the *initial* contents of every CCA layer slot (mofclient.c:240785-240793).
"""

import os

from parse_ca import parse_ca, CAFile
from item_data import (
    SLOT_HAIR, SLOT_FACE, SLOT_COAT, SLOT_TRIUSERS, SLOT_SHOES,
    SLOT_HAND, SLOT_CLOCK, SLOT_RWEAPON, SLOT_LWEAPON,
    SLOT_ACC1, SLOT_ACC2, SLOT_ACC3, SLOT_MASKHAIR, SLOT_SUIT,
    SLOT_EMOTICON, SLOT_DUALWEAPON, SLOT_NAMES,
)


# ── File-name table for the dot character ────────────────────────────────────

DOT_SLOT_FILES = [
    (SLOT_HAIR,       'Dot_Character_HAIR.ca'),
    (SLOT_FACE,       'Dot_Character_FACE.ca'),
    (SLOT_COAT,       'Dot_Character_COAT.ca'),
    (SLOT_TRIUSERS,   'Dot_Character_TRIUSERS.ca'),
    (SLOT_SHOES,      'Dot_Character_SHOES.ca'),
    (SLOT_HAND,       'Dot_Character_HAND.ca'),
    (SLOT_CLOCK,      'Dot_Character_CLOCK.ca'),
    (SLOT_RWEAPON,    'Dot_Character_RIGHTHANDWEAPON.ca'),
    (SLOT_LWEAPON,    'Dot_Character_LEFTHANDWEAPON.ca'),
    (SLOT_ACC1,       'Dot_Character_ACCESSORY1.ca'),
    (SLOT_ACC2,       'Dot_Character_ACCESSORY2.ca'),
    (SLOT_ACC3,       'Dot_Character_ACCESSORY3.ca'),
    (SLOT_MASKHAIR,   'Dot_Character_MASKHAIR.ca'),
    (SLOT_SUIT,       'Dot_Character_SUIT.ca'),
    (SLOT_EMOTICON,   'Dot_Character_EMOTICON.ca'),
    (SLOT_DUALWEAPON, 'Dot_Character_DUALWEAPON.ca'),
]

DOT_BASE_FILE = 'Dot_Character.ca'


ILLUST_SLOT_FILES = [
    (SLOT_HAIR,    'Illust_Character_HAIR.ca'),
    (SLOT_FACE,    'Illust_Character_FACE.ca'),
    (SLOT_COAT,    'Illust_Character_COAT.ca'),
    (SLOT_TRIUSERS,'Illust_Character_TRIUSERS.ca'),
    (SLOT_SHOES,   'Illust_Character_SHOES.ca'),
    (SLOT_HAND,    'Illust_Character_HAND.ca'),
]
ILLUST_BASE_FILE = 'Illust_Character.ca'


# ── Case-insensitive file lookup ──────────────────────────────────────────────

def _find_file_ci(directory, name):
    """Return the actual case-correct path for ``name`` inside ``directory``,
    or None if no such file exists. Linux case-sensitivity workaround."""
    if not os.path.isdir(directory):
        return None
    target = name.lower()
    try:
        for entry in os.listdir(directory):
            if entry.lower() == target:
                return os.path.join(directory, entry)
    except OSError:
        return None
    return None


def find_character_dir(mofdata_root):
    """Locate the ``Character`` sub-directory under a MoFData root."""
    for sub in ('character', 'Character', 'CHARACTER'):
        path = os.path.join(mofdata_root, sub)
        if os.path.isdir(path):
            return path
    if os.path.isdir(mofdata_root):
        return mofdata_root
    return None


# ── Slot collection ───────────────────────────────────────────────────────────

class SlotCA:
    """One CA file backing a single slot category (HAIR, COAT, …)."""
    __slots__ = ('slot_id', 'slot_name', 'filename', 'path', 'ca')

    def __init__(self, slot_id, filename, path, ca):
        self.slot_id = slot_id
        self.slot_name = SLOT_NAMES.get(slot_id, '?')
        self.filename = filename
        self.path = path
        self.ca = ca

    @property
    def num_layers(self):
        return len(self.ca.layers) if self.ca else 0

    def get_layer(self, idx):
        if not self.ca or idx < 0 or idx >= len(self.ca.layers):
            return None
        return self.ca.layers[idx]


class CharacterCA:
    """Loaded set of CA files for the dot or illust character.

    Attributes:
        base   (CAFile):     the base timeline (Dot_Character.ca / Illust_Character.ca)
        slots  (dict[int, SlotCA]): slot id → SlotCA
        keys   (list[KeyInfo]):     animation key list (from base.keys)
        anim_fps (int):            animation timing from base.anim_fps
    """

    def __init__(self, base=None):
        self.base = base
        self.slots = {}
        self.character_dir = None

    # — helpers ——
    @property
    def keys(self):
        return self.base.keys if self.base else []

    @property
    def anim_fps(self):
        return self.base.anim_fps if self.base else 0

    @property
    def max_frames(self):
        return self.base.max_frames if self.base else 0

    @property
    def num_base_layers(self):
        return len(self.base.layers) if self.base else 0

    def get_slot(self, slot_id):
        return self.slots.get(slot_id)

    def get_layer(self, slot_id, layer_index):
        slot = self.slots.get(slot_id)
        if slot is None:
            return None
        return slot.get_layer(layer_index)

    def find_key(self, name):
        """Find an animation key by name (case-insensitive)."""
        if not self.base:
            return None
        for k in self.base.keys:
            if k.name == name or k.name.lower() == name.lower():
                return k
        return None


# ── Loaders ───────────────────────────────────────────────────────────────────

def _safe_parse(path):
    try:
        return parse_ca(path)
    except Exception as e:
        print('  [ca_loader] failed to parse %s: %s' % (path, e))
        return None


def load_character_dot(mofdata_root, verbose=False):
    """Load the full dot character set from a MoFData root.

    Mirrors ``CAManager::LoadCADataDot`` (mofclient.c:245180), minus the
    text-mode ``CA_Character_Dot.txt`` step (use :func:`item_data.load_dot_items`
    for that).
    """
    char_dir = find_character_dir(mofdata_root)
    if char_dir is None:
        raise FileNotFoundError(
            'Could not find a Character directory under %r' % mofdata_root)

    out = CharacterCA()
    out.character_dir = char_dir

    base_path = _find_file_ci(char_dir, DOT_BASE_FILE)
    if base_path is None:
        raise FileNotFoundError('%s not found in %s' % (DOT_BASE_FILE, char_dir))
    out.base = _safe_parse(base_path)
    if verbose:
        print('  base   :', base_path,
              '(layers=%d frames=%d keys=%d fps=%d)' %
              (len(out.base.layers), out.base.max_frames,
               len(out.base.keys), out.base.anim_fps))

    for slot_id, fname in DOT_SLOT_FILES:
        path = _find_file_ci(char_dir, fname)
        if path is None:
            if verbose:
                print('  MISS  : slot %2d %s' % (slot_id, fname))
            continue
        ca = _safe_parse(path)
        if ca is None:
            continue
        out.slots[slot_id] = SlotCA(slot_id, fname, path, ca)
        if verbose:
            print('  slot %2d %-12s : %s  layers=%d' %
                  (slot_id, SLOT_NAMES.get(slot_id, '?'),
                   os.path.basename(path), len(ca.layers)))
    return out


def load_character_illust(mofdata_root, verbose=False):
    """Load the illust character set."""
    char_dir = find_character_dir(mofdata_root)
    if char_dir is None:
        raise FileNotFoundError(
            'Could not find a Character directory under %r' % mofdata_root)

    out = CharacterCA()
    out.character_dir = char_dir

    base_path = _find_file_ci(char_dir, ILLUST_BASE_FILE)
    if base_path is None:
        raise FileNotFoundError(
            '%s not found in %s' % (ILLUST_BASE_FILE, char_dir))
    out.base = _safe_parse(base_path)
    if verbose:
        print('  base   :', base_path)

    for slot_id, fname in ILLUST_SLOT_FILES:
        path = _find_file_ci(char_dir, fname)
        if path is None:
            if verbose:
                print('  MISS  : slot %2d %s' % (slot_id, fname))
            continue
        ca = _safe_parse(path)
        if ca is None:
            continue
        out.slots[slot_id] = SlotCA(slot_id, fname, path, ca)
        if verbose:
            print('  slot %2d %-12s : %s' %
                  (slot_id, SLOT_NAMES.get(slot_id, '?'),
                   os.path.basename(path)))
    return out


# ── CLI ───────────────────────────────────────────────────────────────────────

def main():
    import argparse
    parser = argparse.ArgumentParser(
        description='Load and dump a MoF character CA set.')
    parser.add_argument('mofdata', help='Path to the MoFData root directory')
    parser.add_argument('--illust', action='store_true',
                        help='Load the illust set instead of dot')
    parser.add_argument('--keys', action='store_true',
                        help='Dump animation keys')
    args = parser.parse_args()

    loader = load_character_illust if args.illust else load_character_dot
    char = loader(args.mofdata, verbose=True)

    print()
    print('Total slots loaded :', len(char.slots))
    print('Base max_frames    :', char.max_frames)
    print('Base anim_fps      :', char.anim_fps)
    print('Animation keys     :', len(char.keys))
    if args.keys and char.keys:
        for i, k in enumerate(char.keys):
            print('  [%2d] %-20s frames=%d-%d (%d)'
                  % (i, k.name, k.start_frame, k.end_frame,
                     k.end_frame - k.start_frame + 1))


if __name__ == '__main__':
    main()
