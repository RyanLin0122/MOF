"""
Tkinter UI for the MoF dot/illust character emulator.

Run with::

    python character_emulator.py /path/to/MoFData

Or set ``MOFDATA_DIR`` in the environment and run with no arguments.

Requirements: Python 3.8+, Pillow (``pip install Pillow``), tkinter (stdlib).

Key bindings:
    Space   pause / resume animation
    Right   step one frame forwards (when paused)
    Left    step one frame backwards (when paused)
    1 / 2   switch between Dot and Illust modes (if illust files exist)
"""

import argparse
import os
import sys
import time
import tkinter as tk
from tkinter import ttk, messagebox

try:
    from PIL import Image, ImageTk
except ImportError:
    print('This UI requires Pillow:  pip install Pillow', file=sys.stderr)
    raise

from ca_loader import (
    load_character_dot, load_character_illust,
    DOT_SLOT_FILES, ILLUST_SLOT_FILES, find_character_dir,
)
from character_renderer import (
    NUM_CCA_SLOTS, build_default_slots, apply_equipment, render_frame,
)
from gi_resource import GIResource
from item_data import (
    SLOT_NAMES, load_dot_items, load_illust_items, item_id_to_code,
)


CANVAS_W, CANVAS_H = 480, 560
DEFAULT_FPS_FALLBACK = 12


# ── A single equipment combobox row ──────────────────────────────────────────

class SlotControl:
    def __init__(self, parent, slot_id, slot_name, max_index, on_change):
        self.slot_id = slot_id
        self.slot_name = slot_name
        self.max_index = max_index
        self.var = tk.StringVar(value='(none)')
        ttk.Label(parent, text='%-9s' % slot_name, width=10).grid(
            row=slot_id, column=0, padx=2, pady=1, sticky='w')
        values = ['(none)'] + [str(i) for i in range(max_index)]
        self.combo = ttk.Combobox(parent, textvariable=self.var,
                                  values=values, width=8, state='readonly')
        self.combo.grid(row=slot_id, column=1, padx=2, pady=1, sticky='w')
        self.combo.bind('<<ComboboxSelected>>', lambda e: on_change())

    def get_index(self):
        v = self.var.get()
        if v == '(none)':
            return None
        try:
            return int(v)
        except ValueError:
            return None

    def set_index(self, idx):
        self.var.set('(none)' if idx is None else str(idx))


# ── Main application ─────────────────────────────────────────────────────────

class CharacterEmulator:
    def __init__(self, root, mofdata_dir):
        self.root = root
        self.mofdata_dir = mofdata_dir
        self.character_dir = find_character_dir(mofdata_dir)
        if self.character_dir is None:
            raise FileNotFoundError(
                'Could not find a Character directory under %r' % mofdata_dir)

        self.gi_resource = GIResource(self.character_dir)
        self.gi_resource.index()  # warm the directory listing

        # Dot is the default; Illust loaded lazily on demand.
        self.character = load_character_dot(mofdata_dir, verbose=True)
        self.mode = 'dot'  # 'dot' or 'illust'

        # Optional item-binding tables (display only — emulator works without).
        self.item_table_dot    = self._try_load_items('CA_Character_Dot.txt',
                                                      load_dot_items)
        self.item_table_illust = self._try_load_items(
            'CA_Character_illustration.txt', load_illust_items)

        # Animation state
        self.equipment = {}        # slot_id → layer_index
        self.current_key_index = 0
        self.frame_index = 0       # relative frame index within current key
        self.playing = True
        self.last_time = time.monotonic()
        self.accum_time = 0.0      # accumulated time for frame advancement

        self._build_ui()
        self._refresh_keys()
        self._tick()

    # — initialisation helpers ——
    def _try_load_items(self, name, loader):
        for sub in ('', 'Character', 'character'):
            p = os.path.join(self.mofdata_dir, sub, name) if sub \
                else os.path.join(self.mofdata_dir, name)
            if os.path.isfile(p):
                try:
                    return loader(p)
                except Exception as e:
                    print('  [items] failed to read %s: %s' % (p, e))
        return {}

    # — UI construction ——
    def _build_ui(self):
        self.root.title('MoF Character Emulator')
        self.root.minsize(820, 640)

        # Left: equipment
        left = ttk.LabelFrame(self.root, text='Equipment')
        left.grid(row=0, column=0, padx=8, pady=8, sticky='ns')
        self._build_equipment_pane(left)

        # Centre: canvas + transport
        centre = ttk.Frame(self.root)
        centre.grid(row=0, column=1, padx=8, pady=8, sticky='nsew')
        self.canvas = tk.Canvas(centre, width=CANVAS_W, height=CANVAS_H,
                                bg='#202024', highlightthickness=1,
                                highlightbackground='#444')
        self.canvas.pack()
        self._build_transport(centre)

        # Right: animation key list + status
        right = ttk.LabelFrame(self.root, text='Animation')
        right.grid(row=0, column=2, padx=8, pady=8, sticky='ns')
        self._build_anim_pane(right)

        self.root.columnconfigure(1, weight=1)
        self.root.rowconfigure(0, weight=1)

        self.root.bind('<space>', lambda e: self._toggle_play())
        self.root.bind('<Right>', lambda e: self._step(+1))
        self.root.bind('<Left>',  lambda e: self._step(-1))
        self.root.bind('1',       lambda e: self._switch_mode('dot'))
        self.root.bind('2',       lambda e: self._switch_mode('illust'))
        self.root.bind('s',       lambda e: self._save_debug_frame())

    def _build_equipment_pane(self, parent):
        self.slot_controls = {}
        for slot_id, fname in DOT_SLOT_FILES:
            slot_ca = self.character.get_slot(slot_id)
            n_layers = len(slot_ca.ca.layers) if slot_ca else 0
            ctl = SlotControl(parent, slot_id, SLOT_NAMES[slot_id],
                              n_layers, self._on_equipment_change)
            self.slot_controls[slot_id] = ctl

        ttk.Separator(parent, orient='horizontal').grid(
            row=NUM_CCA_SLOTS + 1, column=0, columnspan=2,
            sticky='ew', pady=4)
        ttk.Button(parent, text='Clear all',
                   command=self._clear_equipment).grid(
            row=NUM_CCA_SLOTS + 2, column=0, columnspan=2, pady=2)
        ttk.Button(parent, text='By item code…',
                   command=self._dialog_item_code).grid(
            row=NUM_CCA_SLOTS + 3, column=0, columnspan=2, pady=2)

    def _build_transport(self, parent):
        bar = ttk.Frame(parent)
        bar.pack(fill='x', pady=4)
        self.play_btn = ttk.Button(bar, text='Pause',
                                   command=self._toggle_play, width=8)
        self.play_btn.pack(side='left', padx=4)
        ttk.Button(bar, text='←', width=3,
                   command=lambda: self._step(-1)).pack(side='left')
        ttk.Button(bar, text='→', width=3,
                   command=lambda: self._step(+1)).pack(side='left')
        ttk.Button(bar, text='Reset', width=6,
                   command=self._reset_tick).pack(side='left', padx=4)

        self.frame_label = ttk.Label(bar, text='frame 0 / 0')
        self.frame_label.pack(side='left', padx=8)

        # Mode toggle
        self.mode_var = tk.StringVar(value='dot')
        ttk.Radiobutton(bar, text='Dot', value='dot', variable=self.mode_var,
                        command=lambda: self._switch_mode('dot')).pack(side='right')
        ttk.Radiobutton(bar, text='Illust', value='illust',
                        variable=self.mode_var,
                        command=lambda: self._switch_mode('illust')
                        ).pack(side='right')

        self.status = tk.StringVar(value='')
        ttk.Label(parent, textvariable=self.status,
                  foreground='#aaa').pack(anchor='w', padx=4, pady=(4, 0))

    def _build_anim_pane(self, parent):
        self.key_list = tk.Listbox(parent, height=22, width=22,
                                   exportselection=False)
        self.key_list.pack(fill='both', expand=True, padx=4, pady=4)
        self.key_list.bind('<<ListboxSelect>>', self._on_key_select)
        self.info_label = ttk.Label(parent, text='', foreground='#666',
                                    justify='left', wraplength=180)
        self.info_label.pack(anchor='w', padx=4, pady=4)

    # — event handlers ——
    def _refresh_keys(self):
        self.key_list.delete(0, 'end')
        keys = self.character.keys or []
        if keys:
            for i, k in enumerate(keys):
                label = '%2d  %-12s [%d-%d]' % (
                    i, (k.name or '').strip() or '(unnamed)',
                    k.start_frame, k.end_frame)
                self.key_list.insert('end', label)
            if self.current_key_index >= len(keys):
                self.current_key_index = 0
            self.key_list.selection_set(self.current_key_index)
        else:
            self.key_list.insert('end', '(no keys)')
            self.current_key_index = 0

        slots_count = len(self.character.slots)
        max_frames = self.character.max_frames or 0
        anim_fps = self.character.anim_fps or 0
        self.info_label.config(text=(
            'mode      : %s\n'
            'slot files: %d\n'
            'max frames: %d\n'
            'anim fps  : %d\n'
            'gi atlas  : %d files'
        ) % (self.mode, slots_count, max_frames, anim_fps,
             len(self.gi_resource.index())))

    def _on_key_select(self, _evt):
        sel = self.key_list.curselection()
        if not sel:
            return
        self.current_key_index = sel[0]
        self.frame_index = 0
        self.accum_time = 0.0

    def _on_equipment_change(self):
        self.equipment = {sid: ctl.get_index()
                          for sid, ctl in self.slot_controls.items()
                          if ctl.get_index() is not None}

    def _clear_equipment(self):
        for ctl in self.slot_controls.values():
            ctl.set_index(None)
        self.equipment = {}

    def _dialog_item_code(self):
        win = tk.Toplevel(self.root)
        win.title('Equip by item code')
        win.transient(self.root)
        win.geometry('320x100')
        ttk.Label(win, text='Item code (e.g. H0001):').pack(pady=(8, 2))
        var = tk.StringVar()
        entry = ttk.Entry(win, textvariable=var, width=12)
        entry.pack(pady=2)
        entry.focus_set()

        def apply():
            code = var.get().strip().upper()
            table = (self.item_table_dot if self.mode == 'dot'
                     else self.item_table_illust)
            info = table.get(code)
            if not info:
                messagebox.showerror('Not found',
                                     'Item code %r not in CA table.' % code)
                return
            if self.mode == 'dot':
                if info.type1 in self.slot_controls:
                    self.slot_controls[info.type1].set_index(info.layer_index1)
                if info.type2 in self.slot_controls:
                    self.slot_controls[info.type2].set_index(info.layer_index2)
            else:
                if info.type in self.slot_controls:
                    self.slot_controls[info.type].set_index(info.frame_index)
            self._on_equipment_change()
            win.destroy()

        ttk.Button(win, text='Apply', command=apply).pack(pady=4)
        win.bind('<Return>', lambda e: apply())

    def _save_debug_frame(self):
        """Press 's' to save the current frame as PNG with debug log."""
        keys = self.character.keys or []
        key = keys[self.current_key_index] if keys else None
        if key is not None:
            span = max(1, key.end_frame - key.start_frame + 1)
            fi = self.frame_index % span
            abs_frame = key.start_frame + fi
        else:
            abs_frame = self.frame_index % max(1, self.character.max_frames)
        print('=== DEBUG FRAME: mode=%s frame=%d ===' % (self.mode, abs_frame))
        slots = build_default_slots(self.character)
        apply_equipment(slots, self.character, self.equipment)
        result = render_frame(slots, abs_frame, self.gi_resource,
                              canvas_size=(CANVAS_W, CANVAS_H), debug=True)
        out = 'debug_%s_frame%d.png' % (self.mode, abs_frame)
        result.image.save(out)
        print('  saved → %s  (%dx%d, dots=%d, miss=%d)'
              % (out, result.image.width, result.image.height,
                 result.dot_count, len(result.missing_ids)))

    def _toggle_play(self):
        self.playing = not self.playing
        self.play_btn.config(text='Play' if not self.playing else 'Pause')
        if self.playing:
            self.last_time = time.monotonic()
            self.accum_time = 0.0

    def _step(self, delta):
        self.playing = False
        self.play_btn.config(text='Play')
        self.frame_index += delta
        self.accum_time = 0.0

    def _reset_tick(self):
        self.frame_index = 0
        self.accum_time = 0.0

    def _switch_mode(self, mode):
        if mode == self.mode:
            return
        try:
            if mode == 'dot':
                self.character = load_character_dot(self.mofdata_dir)
            else:
                self.character = load_character_illust(self.mofdata_dir)
        except Exception as e:
            messagebox.showerror('Switch failed', str(e))
            self.mode_var.set(self.mode)
            return
        self.mode = mode
        self.mode_var.set(mode)
        self._clear_equipment()
        # Rebuild slot combos with new layer counts.
        for slot_id, ctl in self.slot_controls.items():
            slot_ca = self.character.get_slot(slot_id)
            n = len(slot_ca.ca.layers) if slot_ca else 0
            ctl.max_index = n
            ctl.combo.config(values=['(none)'] + [str(i) for i in range(n)])
        self.frame_index = 0
        self.accum_time = 0.0
        self.last_time = time.monotonic()
        self.current_key_index = 0
        self._refresh_keys()

    # — main animation loop ——
    # Mirrors CCA::Process (mofclient.c:240931-240968):
    #   - time-based frame advancement using accumulated wall-clock time
    #   - frame_index is RELATIVE (0, 1, 2, …) within the current key
    #   - absolute frame = start_frame + frame_index
    def _tick(self):
        now = time.monotonic()
        fps = self.character.anim_fps or DEFAULT_FPS_FALLBACK
        frame_duration = 1.0 / max(1, fps)  # seconds per frame

        # Advance frame counter using elapsed time (matches C++ logic).
        # Illust mode is static — no frame animation.
        if self.playing and self.mode == 'dot':
            dt = now - self.last_time
            self.accum_time += dt
            if self.accum_time >= frame_duration:
                frames_to_advance = int(self.accum_time / frame_duration)
                self.accum_time -= frames_to_advance * frame_duration
                self.frame_index += frames_to_advance
        self.last_time = now

        # Compute absolute frame from relative frame_index + key range
        keys = self.character.keys or []
        key = keys[self.current_key_index] if keys else None
        if key is not None:
            span = key.end_frame - key.start_frame + 1
            if span > 0:
                self.frame_index %= span
            abs_frame = key.start_frame + self.frame_index
            # Boundary check (matches mofclient.c:240962-240966)
            if abs_frame > key.end_frame:
                self.frame_index = 0
                abs_frame = key.start_frame
        else:
            max_f = max(1, self.character.max_frames)
            self.frame_index %= max_f
            abs_frame = self.frame_index

        slots = build_default_slots(self.character)
        apply_equipment(slots, self.character, self.equipment)
        result = render_frame(slots, abs_frame, self.gi_resource,
                              canvas_size=(CANVAS_W, CANVAS_H))

        # Draw
        self._photo = ImageTk.PhotoImage(result.image)
        self.canvas.delete('all')
        self.canvas.create_image(0, 0, anchor='nw', image=self._photo)
        # Crosshair at the character origin
        ox, oy = result.origin_x, result.origin_y
        self.canvas.create_line(ox - 8, oy, ox + 8, oy, fill='#666')
        self.canvas.create_line(ox, oy - 8, ox, oy + 8, fill='#666')

        max_frame = key.end_frame if key else self.character.max_frames - 1
        min_frame = key.start_frame if key else 0
        self.frame_label.config(text='frame %d / %d-%d  dots=%d  miss=%d'
                                % (abs_frame, min_frame, max_frame,
                                   result.dot_count, len(result.missing_ids)))
        if result.missing_ids:
            sample = sorted(result.missing_ids)[:3]
            self.status.set('missing GI: ' + ', '.join('%08X' % x for x in sample))
        else:
            self.status.set('')

        # Schedule next tick — use fixed 16ms (~60 UI fps) since frame
        # advancement is now time-based, not tick-based.
        self.root.after(16, self._tick)


# ── entry point ──────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description='MoF dot/illust character emulator.')
    parser.add_argument('mofdata', nargs='?',
                        default=os.environ.get('MOFDATA_DIR', './mofdata'),
                        help='Path to the MoFData directory '
                             '(default: $MOFDATA_DIR or ./mofdata)')
    args = parser.parse_args()

    if not os.path.isdir(args.mofdata):
        sys.stderr.write('MoFData directory not found: %s\n' % args.mofdata)
        sys.exit(1)

    root = tk.Tk()
    try:
        app = CharacterEmulator(root, args.mofdata)
    except Exception as e:
        messagebox.showerror('Failed to initialise', str(e))
        raise
    root.mainloop()


if __name__ == '__main__':
    main()
