"""
CA (Character Animation) file parser for MoF.

Binary format reverse-engineered from CCA / CAManager in mofclient.c.
Parses both Dot (sprite) and Illust CA files.

API usage::

    from parse_ca import parse_ca, parse_ca_bytes

    ca = parse_ca("mofdata/character/dot_character.ca")
    ca = parse_ca_bytes(raw_bytes)

    print(ca.name, ca.num_layers, ca.fps)
    for layer in ca.layers:
        for frame in layer.frames:
            for dot in frame.dots:
                print(dot.image_id, dot.offset_x, dot.offset_y)
    for key in ca.keys:
        print(key.name, key.start_frame, key.end_frame)

CLI usage::

    python parse_ca.py <file.ca> [--verbose] [--dump-dots] [--dump-keys]
"""

import struct
import sys
import os
import argparse


# ── Constants ─────────────────────────────────────────────────────────────────

HEADER_SIZE = 279          # 4 + 255 + 5*4
LAYER_HEADER_SIZE = 50     # 0x32
DOTINFO_SIZE = 32          # 0x20
IMAGEINFO_SIZE = 102       # 0x66
KEYINFO_SIZE = 28          # 0x1C

DOTINFO_FMT = '<IHHffIII2s'
KEYINFO_FMT = '<20sII'


# ── Helper ────────────────────────────────────────────────────────────────────

def decode_name(raw_bytes, encoding='cp949'):
    """Decode a null-terminated name from a raw byte buffer."""
    end = raw_bytes.find(b'\x00')
    if end != -1:
        raw_bytes = raw_bytes[:end]
    try:
        return raw_bytes.decode(encoding)
    except (UnicodeDecodeError, LookupError):
        try:
            return raw_bytes.decode('ascii', errors='replace')
        except Exception:
            return raw_bytes.hex()


# ── Data classes ──────────────────────────────────────────────────────────────

class DotInfo:
    """A single sprite reference within a frame (32 bytes).

    Attributes:
        image_id    (int):   Resource ID for GetGameImage.
        frame_index (int):   Sub-frame index in the sprite sheet.
        flags       (int):   Unknown flags.
        offset_x    (float): X position offset.
        offset_y    (float): Y position offset.
        alpha       (int):   Opacity 0-255 (default 255).
        field_20    (int):   Unknown (default 0).
        scale_pct   (int):   Scale percentage (default 100).
        tail_bytes  (bytes): Last 2 bytes (padding).
    """
    __slots__ = ('image_id', 'frame_index', 'flags', 'offset_x', 'offset_y',
                 'alpha', 'field_20', 'scale_pct', 'tail_bytes')

    def __init__(self, data, offset):
        (self.image_id, self.frame_index, self.flags,
         self.offset_x, self.offset_y,
         self.alpha, self.field_20, self.scale_pct,
         self.tail_bytes) = struct.unpack_from(DOTINFO_FMT, data, offset)

    def __repr__(self):
        return (f'DotInfo(img=0x{self.image_id:08X}, frame={self.frame_index}, '
                f'flags=0x{self.flags:04X}, pos=({self.offset_x:.1f}, {self.offset_y:.1f}), '
                f'alpha={self.alpha}, scale={self.scale_pct}%)')


class ImageInfo:
    """Image info entry (102 bytes, rarely used in dot files).

    Attributes:
        raw (bytes): The 102 raw bytes.
    """
    __slots__ = ('raw',)

    def __init__(self, data, offset):
        self.raw = data[offset:offset + IMAGEINFO_SIZE]


class FrameInfo:
    """One animation frame within a layer.

    Attributes:
        dots        (list[DotInfo]):  Sprite entries for this frame.
        image_infos (list[ImageInfo]): Image info entries (usually empty).
        extra       (int):            Per-frame property.
    """
    __slots__ = ('dots', 'image_infos', 'extra')

    def __init__(self):
        self.dots = []
        self.image_infos = []
        self.extra = 0


class LayerInfo:
    """One layer containing a sequence of frames.

    Attributes:
        name       (str):   Decoded layer name.
        name_raw   (bytes): Raw 50-byte layer header (includes name).
        header_raw (bytes): Alias for name_raw.
        frames     (list[FrameInfo]): Frames in this layer.
    """
    __slots__ = ('name_raw', 'name', 'header_raw', 'frames')

    def __init__(self):
        self.name_raw = b''
        self.name = ''
        self.header_raw = b''
        self.frames = []


class KeyInfo:
    """Animation key defining a named frame range.

    Attributes:
        name        (str):   Decoded animation name (e.g. "걷기").
        name_raw    (bytes): Raw 20-byte name field.
        start_frame (int):   First frame index (inclusive).
        end_frame   (int):   Last frame index (inclusive).
    """
    __slots__ = ('name_raw', 'name', 'start_frame', 'end_frame')

    def __init__(self, data, offset):
        raw_name, self.start_frame, self.end_frame = struct.unpack_from(KEYINFO_FMT, data, offset)
        self.name_raw = raw_name
        self.name = decode_name(raw_name)

    def __repr__(self):
        return f'KeyInfo("{self.name}", frames={self.start_frame}-{self.end_frame})'


class CAFile:
    """Parsed CA file.

    Attributes:
        version    (int):   File version (typically 13).
        name       (str):   Embedded file name.
        name_raw   (bytes): Raw 255-byte name buffer.
        field_260  (int):   Unknown header field (struct offset 260).
        max_frames (int):   Frames per layer (struct offset 272).
        fps        (int):   Frame rate (struct offset 276).
        field_280  (int):   Unknown header field (struct offset 280).
        num_layers (int):   Number of layers (struct offset 264).
        layers     (list[LayerInfo]):  Layer data.
        anim_fps   (int):   Animation timing from CANIMATIONINFO footer.
        keys       (list[KeyInfo]):    Animation key definitions.
        trailing   (bytes): Any unread bytes at end of file.
    """

    def __init__(self):
        self.version = 0
        self.name = ''
        self.name_raw = b''
        self.field_260 = 0
        self.max_frames = 0
        self.fps = 0
        self.field_280 = 0
        self.num_layers = 0
        self.layers = []
        self.anim_fps = 0
        self.keys = []
        self.trailing = b''


# ── Parser ────────────────────────────────────────────────────────────────────

def parse_ca_bytes(data):
    """Parse a CA file from a ``bytes`` buffer.

    Args:
        data (bytes): Raw file content.

    Returns:
        CAFile: Parsed result.
    """
    ca = CAFile()
    off = 0

    # ── TIMELINEINFO header (279 bytes) ──
    ca.version = struct.unpack_from('<I', data, off)[0]; off += 4
    ca.name_raw = data[off:off + 255]; off += 255
    ca.name = decode_name(ca.name_raw, 'ascii')
    ca.field_260 = struct.unpack_from('<I', data, off)[0]; off += 4
    ca.max_frames = struct.unpack_from('<I', data, off)[0]; off += 4
    ca.fps = struct.unpack_from('<I', data, off)[0]; off += 4
    ca.field_280 = struct.unpack_from('<I', data, off)[0]; off += 4
    ca.num_layers = struct.unpack_from('<I', data, off)[0]; off += 4

    # ── Layers ──
    for _ in range(ca.num_layers):
        layer = LayerInfo()
        layer.header_raw = data[off:off + LAYER_HEADER_SIZE]
        layer.name_raw = layer.header_raw
        layer.name = decode_name(layer.header_raw)
        off += LAYER_HEADER_SIZE

        nFrames = struct.unpack_from('<I', data, off)[0]; off += 4

        for _ in range(nFrames):
            frame = FrameInfo()

            nDots = struct.unpack_from('<I', data, off)[0]; off += 4
            for _ in range(nDots):
                frame.dots.append(DotInfo(data, off))
                off += DOTINFO_SIZE

            nImgs = struct.unpack_from('<I', data, off)[0]; off += 4
            for _ in range(nImgs):
                frame.image_infos.append(ImageInfo(data, off))
                off += IMAGEINFO_SIZE

            frame.extra = struct.unpack_from('<I', data, off)[0]; off += 4
            layer.frames.append(frame)

        ca.layers.append(layer)

    # ── CANIMATIONINFO footer ──
    if off + 8 <= len(data):
        ca.anim_fps = struct.unpack_from('<I', data, off)[0]; off += 4
        nKeys = struct.unpack_from('<I', data, off)[0]; off += 4
        for _ in range(nKeys):
            if off + KEYINFO_SIZE <= len(data):
                ca.keys.append(KeyInfo(data, off))
                off += KEYINFO_SIZE

    if off < len(data):
        ca.trailing = data[off:]

    return ca


def parse_ca(filepath):
    """Parse a ``.ca`` file from disk.

    Args:
        filepath (str): Path to the .ca file.

    Returns:
        CAFile: Parsed result.
    """
    with open(filepath, 'rb') as f:
        data = f.read()
    return parse_ca_bytes(data)


# ── Display ───────────────────────────────────────────────────────────────────

def print_summary(ca, verbose=False, dump_dots=False, dump_keys=False):
    """Print a human-readable summary of a parsed CAFile."""
    print('=' * 70)
    print(f'  CA File: {ca.name}')
    print('=' * 70)
    print(f'  Version     : {ca.version}')
    print(f'  MaxFrames   : {ca.max_frames}')
    print(f'  FPS         : {ca.fps}')
    print(f'  NumLayers   : {ca.num_layers}')
    print(f'  Field_260   : {ca.field_260}')
    print(f'  Field_280   : {ca.field_280}')
    print()

    print(f'  {"Layer":<5} {"Name":<30} {"Frames":>6} {"TotalDots":>10} {"TotalImgs":>10}')
    print(f'  {"-"*5} {"-"*30} {"-"*6} {"-"*10} {"-"*10}')
    for i, layer in enumerate(ca.layers):
        total_dots = sum(len(f.dots) for f in layer.frames)
        total_imgs = sum(len(f.image_infos) for f in layer.frames)
        print(f'  {i:<5} {layer.name:<30} {len(layer.frames):>6} {total_dots:>10} {total_imgs:>10}')

    if verbose:
        print()
        for i, layer in enumerate(ca.layers):
            total_dots = sum(len(f.dots) for f in layer.frames)
            if total_dots == 0:
                continue
            print(f'  --- Layer[{i}] "{layer.name}" ---')
            for fi, frame in enumerate(layer.frames):
                if frame.dots or frame.image_infos:
                    print(f'    Frame[{fi}]: {len(frame.dots)} dots, '
                          f'{len(frame.image_infos)} imgs, extra={frame.extra}')
                    if dump_dots:
                        for dot in frame.dots:
                            print(f'      {dot}')

    if ca.keys:
        print()
        print(f'  Animation Keys (anim_fps={ca.anim_fps}, count={len(ca.keys)}):')
        print(f'  {"Key":<5} {"Name":<20} {"Start":>6} {"End":>6} {"Frames":>7}')
        print(f'  {"-"*5} {"-"*20} {"-"*6} {"-"*6} {"-"*7}')
        for i, key in enumerate(ca.keys):
            nf = key.end_frame - key.start_frame + 1
            print(f'  {i:<5} {key.name:<20} {key.start_frame:>6} {key.end_frame:>6} {nf:>7}')
        if dump_keys:
            print()
            for i, key in enumerate(ca.keys):
                print(f'  Key[{i}] raw: {key.name_raw.hex()}')

    if ca.trailing:
        print()
        print(f'  Trailing bytes ({len(ca.trailing)}): {ca.trailing.hex()}')

    print()


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description='Parse MoF .ca (Character Animation) files')
    parser.add_argument('files', nargs='+', help='.ca file(s) to parse')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='Show per-frame details for non-empty layers')
    parser.add_argument('--dump-dots', '-d', action='store_true',
                        help='Dump individual DotInfo entries (implies --verbose)')
    parser.add_argument('--dump-keys', '-k', action='store_true',
                        help='Dump raw KeyInfo hex data')
    args = parser.parse_args()

    if args.dump_dots:
        args.verbose = True

    for filepath in args.files:
        if not os.path.isfile(filepath):
            print(f'Error: file not found: {filepath}', file=sys.stderr)
            continue
        ca = parse_ca(filepath)
        print_summary(ca, verbose=args.verbose, dump_dots=args.dump_dots,
                      dump_keys=args.dump_keys)


if __name__ == '__main__':
    main()
