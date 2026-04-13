"""
GI (Game Image) file parser for MoF.

Binary format reverse-engineered from:
  - inc/Image/ImageResource.h
  - src/Image/ImageResource.cpp  (LoadGI / LoadGIInPack)
  - src/Image/Comp.cpp           (run_length_decomp)

Header layout::

    +0  uint32  version          (10 = uncompressed, 20 = RLE compressed)
    +4  uint16  width
    +6  uint16  height
    +8  uint32  imageDataSize    (v10: raw size; v20: decompressed size)
    +12 uint32  d3dFormat        (D3DFORMAT enum)
    +16 uint16  animationFrameCount
    +18 AnimationFrameData[animationFrameCount]   (52 bytes each)
    -- v20: pixel stream follows directly (RLE compressed)
    -- v10: 1 byte unknownFlag, then pixel stream

AnimationFrameData (52 bytes)::

    +0  uint32 unknown[3]
    +12 int32  legacy_offsetX
    +16 int32  legacy_offsetY
    +20 int32  width
    +24 int32  height
    +28 int32  offsetX           (used for sprite drawing offset)
    +32 int32  offsetY
    +36 float  u1
    +40 float  v1
    +44 float  u2
    +48 float  v2

Public API::

    from gi_parser import parse_gi, GIFile, render_frame_to_pil

    gi = parse_gi("MoFData/Character/02000001_Dot_Character.gi")
    print(gi.width, gi.height, gi.frame_count, gi.format_name)
    pil_atlas = gi.to_pil_image()                  # the whole atlas
    pil_subframe = gi.get_frame_image(frame_index) # one subframe (with offset)

"""

import os
import struct
from io import BytesIO


# ── D3DFORMAT constants ───────────────────────────────────────────────────────
# Only the subset that appears in MoF assets

D3DFMT_UNKNOWN     = 0
D3DFMT_R8G8B8      = 20
D3DFMT_A8R8G8B8    = 21
D3DFMT_X8R8G8B8    = 22
D3DFMT_R5G6B5      = 23
D3DFMT_X1R5G5B5    = 24
D3DFMT_A1R5G5B5    = 25
D3DFMT_A4R4G4B4    = 26
D3DFMT_A8          = 28
D3DFMT_X4R4G4B4    = 30
D3DFMT_A8R3G3B2    = 29
D3DFMT_A8L8        = 51

# DXT formats use FourCC encoded as little-endian DWORD
def _fourcc(s):
    return (ord(s[3]) << 24) | (ord(s[2]) << 16) | (ord(s[1]) << 8) | ord(s[0])

D3DFMT_DXT1 = _fourcc('DXT1')
D3DFMT_DXT2 = _fourcc('DXT2')
D3DFMT_DXT3 = _fourcc('DXT3')
D3DFMT_DXT4 = _fourcc('DXT4')
D3DFMT_DXT5 = _fourcc('DXT5')

_FORMAT_NAMES = {
    D3DFMT_A8R8G8B8: 'A8R8G8B8',
    D3DFMT_X8R8G8B8: 'X8R8G8B8',
    D3DFMT_R5G6B5:   'R5G6B5',
    D3DFMT_A1R5G5B5: 'A1R5G5B5',
    D3DFMT_A4R4G4B4: 'A4R4G4B4',
    D3DFMT_R8G8B8:   'R8G8B8',
    D3DFMT_A8:       'A8',
    D3DFMT_DXT1:     'DXT1',
    D3DFMT_DXT2:     'DXT2',
    D3DFMT_DXT3:     'DXT3',
    D3DFMT_DXT4:     'DXT4',
    D3DFMT_DXT5:     'DXT5',
}


def get_pixel_depth(fmt):
    """Bytes per pixel (or 1 for DXT block formats — same as the C++ helper)."""
    if fmt in (D3DFMT_A8R8G8B8, D3DFMT_X8R8G8B8, D3DFMT_R8G8B8):
        return 4
    if fmt in (D3DFMT_R5G6B5, D3DFMT_A1R5G5B5, D3DFMT_X1R5G5B5,
               D3DFMT_A4R4G4B4, D3DFMT_X4R4G4B4, D3DFMT_A8R3G3B2,
               D3DFMT_A8L8):
        return 2
    if fmt in (D3DFMT_A8,):
        return 1
    if fmt in (D3DFMT_DXT1, D3DFMT_DXT2, D3DFMT_DXT3, D3DFMT_DXT4, D3DFMT_DXT5):
        return 1  # marker — actual block decode happens elsewhere
    return 0


# ── RLE decompression (matches Comp.cpp::decomp_byte/word/dword) ─────────────

_MARKER_BYTE  = 0xB4
_MARKER_WORD  = 0xB4B4
_MARKER_DWORD = 0xB4B4B4B4


def decomp_byte(data, out_size):
    out = bytearray(out_size)
    rd = 0
    wr = 0
    n = len(data)
    while rd < n and wr < out_size:
        b = data[rd]; rd += 1
        if b == _MARKER_BYTE:
            nxt = data[rd]; rd += 1
            if nxt == _MARKER_BYTE:
                out[wr] = _MARKER_BYTE
                wr += 1
            else:
                count = data[rd]; rd += 1
                end = wr + count
                if end > out_size:
                    end = out_size
                for k in range(wr, end):
                    out[k] = nxt
                wr = end
        else:
            out[wr] = b
            wr += 1
    return bytes(out)


def decomp_word(data, out_size):
    out = bytearray(out_size)
    total_words = len(data) // 2
    rd = 0  # word index
    wr = 0  # byte offset
    while rd < total_words and wr < out_size:
        cur = struct.unpack_from('<H', data, rd * 2)[0]
        rd += 1
        if cur == _MARKER_WORD:
            value = struct.unpack_from('<H', data, rd * 2)[0]
            rd += 1
            if value == _MARKER_WORD:
                struct.pack_into('<H', out, wr, _MARKER_WORD)
                wr += 2
            else:
                count = struct.unpack_from('<H', data, rd * 2)[0]
                rd += 1
                for _ in range(count):
                    if wr + 2 > out_size:
                        break
                    struct.pack_into('<H', out, wr, value)
                    wr += 2
        else:
            if wr + 2 <= out_size:
                struct.pack_into('<H', out, wr, cur)
            wr += 2
    return bytes(out)


def decomp_dword(data, out_size):
    out = bytearray(out_size)
    total_dwords = len(data) // 4
    rd = 0
    wr = 0
    while rd < total_dwords and wr < out_size:
        cur = struct.unpack_from('<I', data, rd * 4)[0]
        rd += 1
        if cur == _MARKER_DWORD:
            value = struct.unpack_from('<I', data, rd * 4)[0]
            rd += 1
            if value == _MARKER_DWORD:
                struct.pack_into('<I', out, wr, _MARKER_DWORD)
                wr += 4
            else:
                count = struct.unpack_from('<I', data, rd * 4)[0]
                rd += 1
                for _ in range(count):
                    if wr + 4 > out_size:
                        break
                    struct.pack_into('<I', out, wr, value)
                    wr += 4
        else:
            if wr + 4 <= out_size:
                struct.pack_into('<I', out, wr, cur)
            wr += 4
    return bytes(out)


def run_length_decomp(data, out_size, unit):
    if unit == 1:
        return decomp_byte(data, out_size)
    if unit == 2:
        return decomp_word(data, out_size)
    if unit == 4:
        return decomp_dword(data, out_size)
    raise ValueError('Unsupported RLE unit: %d' % unit)


# ── Pixel format → RGBA conversion ────────────────────────────────────────────

def _convert_a8r8g8b8(data, w, h):
    """Direct3D A8R8G8B8 = little-endian BGRA in memory."""
    rgba = bytearray(w * h * 4)
    end = w * h * 4
    src = memoryview(data)
    for i in range(0, end, 4):
        if i + 4 > len(src):
            break
        b = src[i]
        g = src[i + 1]
        r = src[i + 2]
        a = src[i + 3]
        rgba[i]     = r
        rgba[i + 1] = g
        rgba[i + 2] = b
        rgba[i + 3] = a
    return bytes(rgba)


def _convert_x8r8g8b8(data, w, h):
    """Same layout as A8R8G8B8, but alpha is ignored — force 255."""
    rgba = bytearray(w * h * 4)
    end = w * h * 4
    src = memoryview(data)
    for i in range(0, end, 4):
        if i + 4 > len(src):
            break
        rgba[i]     = src[i + 2]
        rgba[i + 1] = src[i + 1]
        rgba[i + 2] = src[i]
        rgba[i + 3] = 255
    return bytes(rgba)


def _convert_a1r5g5b5(data, w, h):
    rgba = bytearray(w * h * 4)
    for i in range(w * h):
        if i * 2 + 2 > len(data):
            break
        v = struct.unpack_from('<H', data, i * 2)[0]
        a = 255 if (v & 0x8000) else 0
        r = ((v >> 10) & 0x1F) * 255 // 31
        g = ((v >> 5)  & 0x1F) * 255 // 31
        b = (v         & 0x1F) * 255 // 31
        rgba[i*4 + 0] = r
        rgba[i*4 + 1] = g
        rgba[i*4 + 2] = b
        rgba[i*4 + 3] = a
    return bytes(rgba)


def _convert_a4r4g4b4(data, w, h):
    rgba = bytearray(w * h * 4)
    for i in range(w * h):
        if i * 2 + 2 > len(data):
            break
        v = struct.unpack_from('<H', data, i * 2)[0]
        a = ((v >> 12) & 0xF) * 17
        r = ((v >> 8)  & 0xF) * 17
        g = ((v >> 4)  & 0xF) * 17
        b = (v         & 0xF) * 17
        rgba[i*4 + 0] = r
        rgba[i*4 + 1] = g
        rgba[i*4 + 2] = b
        rgba[i*4 + 3] = a
    return bytes(rgba)


def _convert_r5g6b5(data, w, h):
    rgba = bytearray(w * h * 4)
    for i in range(w * h):
        if i * 2 + 2 > len(data):
            break
        v = struct.unpack_from('<H', data, i * 2)[0]
        r = ((v >> 11) & 0x1F) * 255 // 31
        g = ((v >> 5)  & 0x3F) * 255 // 63
        b = (v         & 0x1F) * 255 // 31
        rgba[i*4 + 0] = r
        rgba[i*4 + 1] = g
        rgba[i*4 + 2] = b
        rgba[i*4 + 3] = 255
    return bytes(rgba)


# ── DXT decompression (DXT1 / DXT3 / DXT5) ────────────────────────────────────

def _unpack_565(c):
    r = ((c >> 11) & 0x1F) * 255 // 31
    g = ((c >> 5)  & 0x3F) * 255 // 63
    b = (c         & 0x1F) * 255 // 31
    return r, g, b


def _decode_dxt1_block(block):
    """Decode one 8-byte DXT1 block to a 4×4 RGBA buffer (64 bytes)."""
    c0, c1 = struct.unpack_from('<HH', block, 0)
    bits = struct.unpack_from('<I', block, 4)[0]
    r0, g0, b0 = _unpack_565(c0)
    r1, g1, b1 = _unpack_565(c1)
    if c0 > c1:
        r2 = (2 * r0 + r1) // 3
        g2 = (2 * g0 + g1) // 3
        b2 = (2 * b0 + b1) // 3
        r3 = (r0 + 2 * r1) // 3
        g3 = (g0 + 2 * g1) // 3
        b3 = (b0 + 2 * b1) // 3
        a0 = a1 = a2 = a3 = 255
    else:
        r2 = (r0 + r1) // 2
        g2 = (g0 + g1) // 2
        b2 = (b0 + b1) // 2
        r3 = g3 = b3 = 0
        a0 = a1 = a2 = 255
        a3 = 0
    palette = [
        (r0, g0, b0, a0),
        (r1, g1, b1, a1),
        (r2, g2, b2, a2),
        (r3, g3, b3, a3),
    ]
    out = bytearray(64)
    for y in range(4):
        for x in range(4):
            idx = (bits >> (2 * (4 * y + x))) & 0x3
            r, g, b, a = palette[idx]
            o = (y * 4 + x) * 4
            out[o]     = r
            out[o + 1] = g
            out[o + 2] = b
            out[o + 3] = a
    return out


def _decode_dxt3_block(block):
    alpha_data = struct.unpack_from('<Q', block, 0)[0]
    out = _decode_dxt1_block(block[8:])
    for y in range(4):
        for x in range(4):
            a4 = (alpha_data >> (4 * (4 * y + x))) & 0xF
            o = (y * 4 + x) * 4 + 3
            out[o] = a4 * 17
    return out


def _decode_dxt5_block(block):
    a0 = block[0]
    a1 = block[1]
    abits = (block[2] | (block[3] << 8) | (block[4] << 16) |
             (block[5] << 24) | (block[6] << 32) | (block[7] << 40))
    if a0 > a1:
        alphas = [a0, a1,
                  (6 * a0 + 1 * a1) // 7,
                  (5 * a0 + 2 * a1) // 7,
                  (4 * a0 + 3 * a1) // 7,
                  (3 * a0 + 4 * a1) // 7,
                  (2 * a0 + 5 * a1) // 7,
                  (1 * a0 + 6 * a1) // 7]
    else:
        alphas = [a0, a1,
                  (4 * a0 + 1 * a1) // 5,
                  (3 * a0 + 2 * a1) // 5,
                  (2 * a0 + 3 * a1) // 5,
                  (1 * a0 + 4 * a1) // 5,
                  0, 255]
    out = _decode_dxt1_block(block[8:])
    for y in range(4):
        for x in range(4):
            ai = (abits >> (3 * (4 * y + x))) & 0x7
            o = (y * 4 + x) * 4 + 3
            out[o] = alphas[ai]
    return out


def _decode_dxt(data, w, h, decoder, block_size):
    """Common DXT block decoder. Each block is 4×4 pixels."""
    rgba = bytearray(w * h * 4)
    bw = max(1, (w + 3) // 4)
    bh = max(1, (h + 3) // 4)
    pos = 0
    for by in range(bh):
        for bx in range(bw):
            if pos + block_size > len(data):
                return bytes(rgba)
            block = data[pos:pos + block_size]
            pos += block_size
            decoded = decoder(block)
            for y in range(4):
                py = by * 4 + y
                if py >= h:
                    continue
                for x in range(4):
                    px = bx * 4 + x
                    if px >= w:
                        continue
                    src = (y * 4 + x) * 4
                    dst = (py * w + px) * 4
                    rgba[dst]     = decoded[src]
                    rgba[dst + 1] = decoded[src + 1]
                    rgba[dst + 2] = decoded[src + 2]
                    rgba[dst + 3] = decoded[src + 3]
    return bytes(rgba)


def to_rgba(data, w, h, fmt):
    """Convert raw pixel data (already decompressed) to an RGBA byte buffer."""
    if fmt == D3DFMT_A8R8G8B8:
        return _convert_a8r8g8b8(data, w, h)
    if fmt == D3DFMT_X8R8G8B8:
        return _convert_x8r8g8b8(data, w, h)
    if fmt == D3DFMT_R5G6B5:
        return _convert_r5g6b5(data, w, h)
    if fmt == D3DFMT_A1R5G5B5:
        return _convert_a1r5g5b5(data, w, h)
    if fmt == D3DFMT_A4R4G4B4:
        return _convert_a4r4g4b4(data, w, h)
    if fmt == D3DFMT_DXT1:
        return _decode_dxt(data, w, h, _decode_dxt1_block, 8)
    if fmt == D3DFMT_DXT3:
        return _decode_dxt(data, w, h, _decode_dxt3_block, 16)
    if fmt == D3DFMT_DXT5:
        return _decode_dxt(data, w, h, _decode_dxt5_block, 16)
    raise ValueError('Unsupported D3DFORMAT: 0x%X (%s)' %
                     (fmt, _FORMAT_NAMES.get(fmt, '?')))


# ── Data classes ──────────────────────────────────────────────────────────────

ANIMATION_FRAME_FMT = '<3i' '2i' '2i' '2i' '4f'
ANIMATION_FRAME_SIZE = 52
assert struct.calcsize(ANIMATION_FRAME_FMT) == ANIMATION_FRAME_SIZE


class AnimationFrame:
    __slots__ = ('unknown0', 'unknown1', 'unknown2',
                 'legacy_offset_x', 'legacy_offset_y',
                 'width', 'height',
                 'offset_x', 'offset_y',
                 'u1', 'v1', 'u2', 'v2')

    def __init__(self, data, off):
        (self.unknown0, self.unknown1, self.unknown2,
         self.legacy_offset_x, self.legacy_offset_y,
         self.width, self.height,
         self.offset_x, self.offset_y,
         self.u1, self.v1, self.u2, self.v2) = struct.unpack_from(
            ANIMATION_FRAME_FMT, data, off)

    def __repr__(self):
        return ('AnimationFrame(size=%dx%d, off=(%d,%d), uv=(%.3f,%.3f)-(%.3f,%.3f))'
                % (self.width, self.height, self.offset_x, self.offset_y,
                   self.u1, self.v1, self.u2, self.v2))


class GIFile:
    """Parsed GI image."""

    def __init__(self):
        self.version = 0
        self.width = 0
        self.height = 0
        self.image_data_size = 0
        self.decompressed_size = 0
        self.d3d_format = 0
        self.frame_count = 0
        self.frames = []        # list[AnimationFrame]
        self.unknown_flag = 0
        self.pixel_data = b''   # raw decompressed pixel bytes
        self._rgba_cache = None
        self._pil_cache = None

    @property
    def format_name(self):
        return _FORMAT_NAMES.get(self.d3d_format, '0x%X' % self.d3d_format)

    def to_rgba(self):
        if self._rgba_cache is None:
            self._rgba_cache = to_rgba(self.pixel_data,
                                       self.width, self.height,
                                       self.d3d_format)
        return self._rgba_cache

    def to_pil_image(self):
        """Return a PIL Image of the entire texture atlas."""
        if self._pil_cache is None:
            from PIL import Image  # local import — Pillow is optional
            self._pil_cache = Image.frombytes('RGBA',
                                              (self.width, self.height),
                                              self.to_rgba())
        return self._pil_cache

    def get_frame_image(self, idx):
        """Crop one animation frame from the atlas using its UV coordinates.

        Returns a PIL Image of size (frame.width, frame.height). If UV is zero
        or invalid, returns None.
        """
        if idx < 0 or idx >= len(self.frames):
            return None
        f = self.frames[idx]
        if f.width <= 0 or f.height <= 0:
            return None
        # u2/v2 store the sub-rect *size* in UV units (width/atlas_w), not the
        # right/bottom edge. GameImage::Process at mofclient.c:272557 builds the
        # right-edge vertex U as `u2 + u1`, and the bottom-edge V as `v1 + v2`.
        u_right = f.u1 + f.u2
        v_bottom = f.v1 + f.v2
        x1 = max(0, min(self.width,  int(round(f.u1    * self.width))))
        y1 = max(0, min(self.height, int(round(f.v1    * self.height))))
        x2 = max(0, min(self.width,  int(round(u_right  * self.width))))
        y2 = max(0, min(self.height, int(round(v_bottom * self.height))))
        if x2 <= x1 or y2 <= y1:
            return None
        atlas = self.to_pil_image()
        return atlas.crop((x1, y1, x2, y2))


# ── Parser ────────────────────────────────────────────────────────────────────

def parse_gi_bytes(data):
    """Parse a GI file from a raw byte buffer.

    Mirrors ImageResource::LoadGI in src/Image/ImageResource.cpp.
    """
    gi = GIFile()
    off = 0
    gi.version          = struct.unpack_from('<I', data, off)[0]; off += 4
    if gi.version not in (10, 20):
        raise ValueError('Bad GI version: %d' % gi.version)
    gi.width            = struct.unpack_from('<H', data, off)[0]; off += 2
    gi.height           = struct.unpack_from('<H', data, off)[0]; off += 2
    gi.image_data_size  = struct.unpack_from('<I', data, off)[0]; off += 4
    gi.d3d_format       = struct.unpack_from('<I', data, off)[0]; off += 4
    gi.frame_count      = struct.unpack_from('<H', data, off)[0]; off += 2

    if gi.frame_count > 10000:
        raise ValueError('Implausible animationFrameCount: %d' % gi.frame_count)

    for i in range(gi.frame_count):
        gi.frames.append(AnimationFrame(data, off))
        off += ANIMATION_FRAME_SIZE

    is_compressed = (gi.version == 20)

    if is_compressed:
        # v20: raw compressed stream follows directly. m_imageDataSize is the
        # decompressed size; the compressed payload runs to EOF.
        gi.decompressed_size = gi.image_data_size
        compressed = data[off:]
        unit = max(1, get_pixel_depth(gi.d3d_format))
        # DXT formats use byte-stream RLE (unit=1).
        if gi.d3d_format in (D3DFMT_DXT1, D3DFMT_DXT2, D3DFMT_DXT3,
                             D3DFMT_DXT4, D3DFMT_DXT5):
            unit = 1
        gi.pixel_data = run_length_decomp(compressed, gi.decompressed_size, unit)
    else:
        # v10: 1 byte unknownFlag, then raw pixel data
        gi.unknown_flag = data[off]; off += 1
        if gi.image_data_size > 0:
            gi.pixel_data = data[off:off + gi.image_data_size]
        else:
            gi.pixel_data = b''

    return gi


def parse_gi(filepath):
    with open(filepath, 'rb') as f:
        data = f.read()
    return parse_gi_bytes(data)


# ── CLI ───────────────────────────────────────────────────────────────────────

def main():
    import argparse
    parser = argparse.ArgumentParser(description='Parse a MoF .gi image file.')
    parser.add_argument('files', nargs='+')
    parser.add_argument('--save-png', action='store_true',
                        help='Write the decoded atlas next to the .gi file')
    parser.add_argument('--dump-frames', action='store_true',
                        help='Print every AnimationFrameData entry')
    args = parser.parse_args()

    for fp in args.files:
        print('=' * 70)
        print(' ', fp)
        print('=' * 70)
        try:
            gi = parse_gi(fp)
        except Exception as e:
            print('  ERROR:', e)
            continue
        print('  Version : %d  (%s)' % (gi.version,
              'compressed' if gi.version == 20 else 'raw'))
        print('  Size    : %dx%d' % (gi.width, gi.height))
        print('  Format  : %s (0x%X)' % (gi.format_name, gi.d3d_format))
        print('  Frames  : %d' % gi.frame_count)
        print('  Pixels  : %d bytes' % len(gi.pixel_data))
        if args.dump_frames:
            for i, f in enumerate(gi.frames):
                print('    [%3d] %r' % (i, f))
        if args.save_png:
            try:
                img = gi.to_pil_image()
                out_path = os.path.splitext(fp)[0] + '.png'
                img.save(out_path)
                print('  Saved   :', out_path)
            except Exception as e:
                print('  PNG save failed:', e)


if __name__ == '__main__':
    main()
