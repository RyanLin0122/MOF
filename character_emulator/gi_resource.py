"""
GI image cache + lookup (analogue of ``cltGIResource`` and ``cltImageManager``).

The dot character Process loop in mofclient.c:241121 calls::

    cltImageManager::GetGameImage(0, *(_DWORD *)v20, 0, 0)

where ``*(DWORD*)v20`` is the ``image_id`` field stored in each ``DOTINFO``.
The loaded resource is then accessed for its frame metadata (52-byte
``AnimationFrameData`` array) and its texture data.

In the engine, group 0 maps to ``MoFData/Character`` (mofclient.c:269275),
and the on-disk filename is ``%08X_*.gi``. We replicate that here, with
case-insensitive matching to be friendly to Linux file systems.
"""

import os

from gi_parser import parse_gi


class GIResource:
    """Loads .gi files lazily on demand from a single base directory.

    Mirrors ``cltGIResource::LoadResource`` (src/Image/cltGIResource.cpp:85).
    """

    def __init__(self, base_dir):
        self.base_dir = base_dir
        self._index = None
        self._cache = {}      # id (int) → GIFile
        self._missing = set() # id (int) — failed lookups, never retried

    # — directory index ——
    def _build_index(self):
        """Build a dict ``id → absolute path`` by scanning ``base_dir``."""
        index = {}
        if not os.path.isdir(self.base_dir):
            self._index = index
            return
        try:
            for name in os.listdir(self.base_dir):
                if not name.lower().endswith('.gi'):
                    continue
                # filename is "%08X_<rest>.gi". Try to parse the leading hex.
                head = name.split('_', 1)[0]
                if len(head) < 8:
                    continue
                try:
                    res_id = int(head[:8], 16)
                except ValueError:
                    continue
                index.setdefault(res_id, os.path.join(self.base_dir, name))
        except OSError:
            pass
        self._index = index

    def index(self):
        if self._index is None:
            self._build_index()
        return self._index

    def list_ids(self):
        return sorted(self.index().keys())

    # — lookup ——
    def find_path(self, image_id):
        return self.index().get(image_id)

    def get(self, image_id):
        """Return a parsed :class:`gi_parser.GIFile`, or None if not found."""
        if image_id in self._cache:
            return self._cache[image_id]
        if image_id in self._missing:
            return None
        path = self.find_path(image_id)
        if path is None:
            self._missing.add(image_id)
            return None
        try:
            gi = parse_gi(path)
        except Exception as e:
            print('  [gi_resource] failed to parse %08X (%s): %s'
                  % (image_id, path, e))
            self._missing.add(image_id)
            return None
        self._cache[image_id] = gi
        return gi

    def clear_cache(self):
        self._cache.clear()
        self._missing.clear()
