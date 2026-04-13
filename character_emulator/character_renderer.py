"""
Composite renderer for one CCA frame.

This is the Python port of the inner loop of ``CCA::Process(GameImage*)``
(mofclient.c:240976).  For each of the 23 *layer slots* in the CCA, we look up
the currently bound ``LAYERINFO``, fetch its frame at the current animation
position, then draw every ``DOTINFO`` in that frame onto a single PIL canvas.

The pivotal pieces of the original code we mirror::

    v18 = 20 * v4;                       # frame index → byte offset
    v20 = 32 * v19 + *(_DWORD *)(v17 + v18 + 4);    # &dot[i]
    v45 = *(...) + 52 * *(WORD *)(v20 + 4);         # frame.subframe
    v46 = (double)*(int *)(v45 + 28) + *(float *)(v20 + 8);  # x = sub.offX + dot.offX
    v48 = (double)*(int *)(v45 + 32) + *(float *)(v20 + 12); # y = sub.offY + dot.offY

So a single dot's screen position is::

    pos = (subframe.offset_x + dot.offset_x + character_origin_x,
           subframe.offset_y + dot.offset_y + character_origin_y)

The "character origin" is just the centre of our PIL canvas: in the game it is
``CCA::*((float*)this+32) / +33`` (the character's world position).

The 23-slot CCA layer table starts as a copy of ``Dot_Character.ca``'s layers
(mofclient.c:240785-240793). When an item is "equipped", ``CCA::LayerPutOn``
overrides specific slot indices with layers from the per-category .ca file.
We reproduce that mapping in :func:`apply_equipment` below — it is a
straightforward translation of the C++ switch in mofclient.c:242019-242109.
"""

from item_data import (
    SLOT_HAIR, SLOT_FACE, SLOT_COAT, SLOT_TRIUSERS, SLOT_SHOES,
    SLOT_HAND, SLOT_CLOCK, SLOT_RWEAPON, SLOT_LWEAPON,
    SLOT_ACC1, SLOT_ACC2, SLOT_ACC3, SLOT_MASKHAIR, SLOT_SUIT,
    SLOT_EMOTICON, SLOT_DUALWEAPON, SLOT_NAMES,
)


# Total number of CCA layer slots, per mofclient.c:241594 (`while ( v13 < 0x17 );`)
NUM_CCA_SLOTS = 23


# ── Slot-table builder ───────────────────────────────────────────────────────

def build_default_slots(character_ca):
    """Return a 23-element list of base-character LAYERINFO objects.

    Mirrors the loop at mofclient.c:240785-240793, which copies the first 23
    layers of ``Dot_Character.ca`` into the CCA's layer slots.
    """
    layers = character_ca.base.layers if character_ca.base else []
    slots = [None] * NUM_CCA_SLOTS
    for i in range(min(NUM_CCA_SLOTS, len(layers))):
        slots[i] = layers[i]
    return slots


# CCA-slot index → (slot_id, sub-layer offset relative to the item's
# layer_index1).  Pulled directly from CCA::LayerPutOn in
# mofclient.c:242019-242109.  Some categories paint multiple CCA slots from
# consecutive layer indices.
_PUT_ON_RULES = {
    SLOT_HAIR:       [(7,  0), (19, 1)],                              # case 0
    SLOT_FACE:       [(12, 0)],                                       # case 1
    SLOT_COAT:       [(16, 0)],                                       # case 2
    SLOT_TRIUSERS:   [(15, 0)],                                       # case 3
    SLOT_SHOES:      [(14, 0)],                                       # case 4
    SLOT_HAND:       [(10, 0), (25, 1)],                              # case 5
    SLOT_CLOCK:      [(6,  0), (8,  1), (18, 2), (23, 3)],            # case 6
    SLOT_RWEAPON:    [(24, 0), (27, 1)],                              # case 7
    SLOT_LWEAPON:    [(9,  0), (26, 1)],                              # case 8
    SLOT_ACC1:       [(13, 0)],                                       # case 9
    SLOT_ACC2:       [(21, 0)],                                       # case 10
    SLOT_ACC3:       [(22, 0)],                                       # case 11
    SLOT_MASKHAIR:   [(20, 0)],                                       # case 12
    SLOT_SUIT:       [(17, 0), (10, 1), (25, 2)],                     # case 13
    SLOT_DUALWEAPON: [(11, 0), (24, 1), (27, 2)],                     # case 15
}


def apply_equipment(slots, character_ca, equipment):
    """Override ``slots`` in-place with layers from equipped items.

    ``equipment`` is a mapping ``slot_id → layer_index`` (e.g. picked by the
    UI).  Mirrors a sequence of ``CCA::LayerPutOn`` calls.
    """
    for slot_id, layer_index in equipment.items():
        if layer_index is None or layer_index < 0:
            continue
        rules = _PUT_ON_RULES.get(slot_id)
        if not rules:
            continue
        slot_ca = character_ca.get_slot(slot_id)
        if slot_ca is None:
            continue
        for cca_slot_index, sub_offset in rules:
            layer = slot_ca.get_layer(layer_index + sub_offset)
            if layer is not None:
                slots[cca_slot_index] = layer
    return slots


# ── Renderer ──────────────────────────────────────────────────────────────────

class RenderResult:
    __slots__ = ('image', 'origin_x', 'origin_y', 'dot_count', 'missing_ids')

    def __init__(self, image, origin_x, origin_y, dot_count, missing_ids):
        self.image = image
        self.origin_x = origin_x
        self.origin_y = origin_y
        self.dot_count = dot_count
        self.missing_ids = missing_ids


def render_frame(slots, frame_index, gi_resource,
                 canvas_size=(400, 500),
                 origin=None,
                 background=(0, 0, 0, 0),
                 debug=False):
    """Render a single composite frame.

    Args:
        slots:       list[LAYERINFO|None] — one entry per CCA slot.
        frame_index: int — current animation frame (within the layer).
        gi_resource: :class:`gi_resource.GIResource`.
        canvas_size: (w, h) of the output image.
        origin:      (x, y) world origin in canvas pixels. Defaults to the
                     bottom-centre, which roughly matches how the engine
                     anchors a character's feet.
        background:  RGBA fill colour.

    Returns:
        :class:`RenderResult`.
    """
    from PIL import Image

    canvas_w, canvas_h = canvas_size
    if origin is None:
        origin = (canvas_w // 2, canvas_h * 3 // 4)
    ox, oy = origin

    canvas = Image.new('RGBA', canvas_size, background)
    dot_count = 0
    missing_ids = set()

    for slot_idx, layer in enumerate(slots):
        if layer is None or not layer.frames:
            continue
        if frame_index < 0 or frame_index >= len(layer.frames):
            continue
        frame = layer.frames[frame_index]
        if not frame.dots:
            continue
        for dot in frame.dots:
            if dot.image_id == 0:
                continue
            gi = gi_resource.get(dot.image_id)
            if gi is None:
                missing_ids.add(dot.image_id)
                continue
            sub = gi.get_frame_image(dot.frame_index)
            if sub is None:
                continue
            anim = gi.frames[dot.frame_index] if dot.frame_index < len(gi.frames) else None
            if anim is None:
                continue
            # CCA::Process (mofclient.c:241541-241547) computes the sprite
            # top-left position, which ID3DXSprite::Draw uses directly:
            #   x = anim.offset_x + dot.offset_x + character_x
            #   y = anim.offset_y + dot.offset_y + character_y
            x = ox + anim.offset_x + int(round(dot.offset_x))
            y = oy + anim.offset_y + int(round(dot.offset_y))
            if debug:
                print('  slot=%2d dot img=0x%08X frm=%d  '
                      'anim.off=(%d,%d) dot.off=(%.1f,%.1f) '
                      'sub=%dx%d → pos=(%d,%d)'
                      % (slot_idx, dot.image_id, dot.frame_index,
                         anim.offset_x, anim.offset_y,
                         dot.offset_x, dot.offset_y,
                         sub.width, sub.height, x, y))
            # Apply per-dot alpha if non-default
            paste = sub
            if dot.alpha != 255 and dot.alpha is not None:
                a = max(0, min(255, dot.alpha))
                if a == 0:
                    continue
                paste = sub.copy()
                # Pre-multiply alpha channel
                alpha_band = paste.split()[3].point(lambda v, a=a: v * a // 255)
                paste.putalpha(alpha_band)
            canvas.alpha_composite(paste, (x, y))
            dot_count += 1

    return RenderResult(canvas, ox, oy, dot_count, missing_ids)


# ── Convenience: pick frame index from animation key + tick ──────────────────

def frame_index_for_key(character_ca, key, tick):
    """Return the absolute frame index for ``tick`` ticks into a key."""
    if key is None:
        return tick % max(1, character_ca.max_frames)
    span = key.end_frame - key.start_frame + 1
    if span <= 0:
        return key.start_frame
    return key.start_frame + (tick % span)
