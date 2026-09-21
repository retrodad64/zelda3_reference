#!/usr/bin/env python3
"""Decode the dungeon room tables out of zelda3_assets.dat and write a room list.

Everything here comes from the same tables the game reads at runtime:

  asset 6/7   room headers, 14 bytes each
  asset 8     chest list, three bytes each
  asset 10    rooms where falling in a pit hurts
  asset 58/59 per room sprite lists

Run it from the repo root once zelda3_assets.dat exists.
"""

import argparse
import re
import struct
import sys
from pathlib import Path

NUM_ASSETS = 165
SIG_LEN = 48

# Asset slots, matching src/assets.h.
A_ROOM_HEADERS = 6
A_ROOM_HEADER_OFFS = 7
A_ROOM_CHESTS = 8
A_PITS_HURT = 10
A_SPRITES = 58
A_ROOM_DOOR_OFFS = 5
A_ROOM_DATA = 3
A_OW_SPRITES = 160
A_OW_SPRITE_OFFS = 159
A_SPRITE_OFFS = 59
A_SPRITE_HEALTH = None  # health and bump damage live in the C source, not the asset file


def load_assets(path):
    data = path.read_bytes()
    count = struct.unpack_from("<I", data, 80)[0]

    if count != NUM_ASSETS:
        raise SystemExit(f"{path}: expected {NUM_ASSETS} assets, found {count}")

    extra = struct.unpack_from("<I", data, 84)[0]
    sizes = struct.unpack_from(f"<{NUM_ASSETS}I", data, 88)
    off = 88 + NUM_ASSETS * 4 + extra
    blobs = []

    for size in sizes:
        off = (off + 3) & ~3
        blobs.append(data[off:off + size])
        off += size

    return blobs


def sprite_tables(root):
    """kSpriteInit_Health and kSpriteInit_BumpDamage, read out of src/sprite.c."""
    text = (root / "src" / "sprite.c").read_text()
    out = {}

    for name in ("kSpriteInit_Health", "kSpriteInit_BumpDamage"):
        start = text.index(name)
        body = text[text.index("{", start) + 1:text.index("};", start)]
        out[name] = [int(v.strip(), 0) for v in body.replace("\n", " ").split(",") if v.strip()]

    return out["kSpriteInit_Health"], out["kSpriteInit_BumpDamage"]


def category(type_id, health, bump):
    if type_id >= len(bump):
        return "?"
    if bump[type_id] == 0:
        return "npc"
    if health[type_id] >= 32 and health[type_id] != 255:
        return "boss"
    return "enemy"


def room_sprites(blobs, room):
    """Mirrors Dungeon_LoadSprites and Dungeon_LoadSingleSprite in src/sprite.c."""
    offs = struct.unpack_from("<H", blobs[A_SPRITE_OFFS], room * 2)[0]
    blob = blobs[A_SPRITES]
    i = offs + 1  # first byte is the sort setting
    sprites, overlords = [], []

    while i + 2 < len(blob) and blob[i] != 0xff:
        y, x, kind = blob[i], blob[i + 1], blob[i + 2]
        i += 3

        if kind == 0xe4 and y in (0xfe, 0xfd):
            continue  # modifies the sprite before it, not a sprite of its own

        wy = ((y << 4) & 0x1ff) + ((room >> 3 & 0xfe) << 8)
        wx = ((x << 4) & 0x1ff) + (((room & 0xf) << 1) << 8)
        floor = y >> 7

        if x >= 0xe0:
            overlords.append((kind, wx, wy, floor))
        else:
            sprites.append((kind, wx, wy, floor))

    return sprites, overlords


def overworld_sprites(blobs, screen, stage=2):
    """Mirrors Overworld_LoadSprites plus the position decode in
    Sprite_Overworld_ProximityMotivatedLoad. Returns (type, x, y) per entry.

    The offset table holds three sets, picked by sram_progress_indicator, so `stage` is
    0 for the start of the game, 1 once the quest has moved on and 2 after Agahnim."""
    offs = struct.unpack_from("<H", blobs[A_OW_SPRITE_OFFS], (screen + stage * 144) * 2)[0]
    blob = blobs[A_OW_SPRITES]
    x_base = (screen & 7) << 9
    y_base = ((((screen & 0x3f) >> 2) & 0xe)) << 8
    out = []
    i = offs

    while i + 2 < len(blob) and blob[i] != 0xff:
        y, x, kind = blob[i], blob[i + 1], blob[i + 2]
        i += 3

        if kind >= 0xf4:
            continue

        low = (x & 0xf) | ((y << 4) & 0xff)
        high = (x >> 4) + ((y >> 4) << 2)
        out.append((kind, x_base + ((high & 3) << 8) + ((low & 0xf) << 4),
                    y_base + ((high >> 2) << 8) + (low & 0xf0)))

    return out


def room_doors(blobs, room):
    """The room's door table: tilemap addresses ending in FFFF, the list the room load
    copies straight into dung_door_tilemap_address."""
    off = struct.unpack_from("<H", blobs[A_ROOM_DOOR_OFFS], room * 2)[0]
    out, i = [], off

    while i + 1 < len(blobs[A_ROOM_DATA]):
        v = struct.unpack_from("<H", blobs[A_ROOM_DATA], i)[0]

        if v == 0xffff or len(out) >= 24:
            break

        out.append(v)
        i += 2

    return out


def room_chests(blobs):
    blob = blobs[A_ROOM_CHESTS]
    out = {}

    for i in range(0, len(blob) - 2, 3):
        raw = struct.unpack_from("<H", blob, i)[0]
        out.setdefault(raw & 0x7fff, []).append((blob[i + 2], bool(raw & 0x8000)))

    return out


def load_hazards(path):
    """Reads the ROOMHAZ lines that `zelda3 --dump-rooms` writes."""
    out = {}

    for line in Path(path).read_text().splitlines():
        if not line.startswith("ROOMHAZ "):
            continue

        parts = line.split()
        out[int(parts[1], 16)] = {k: int(v) for k, v in
                                  (p.split("=") for p in parts[2:])}

    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--hazards", help="output of: zelda3 --dump-rooms")
    args = ap.parse_args()
    tiles = load_hazards(args.hazards) if args.hazards else {}
    root = Path(__file__).resolve().parent.parent
    blobs = load_assets(root / "zelda3_assets.dat")
    health, bump = sprite_tables(root)
    chests = room_chests(blobs)
    pits_hurt = set(struct.unpack_from(
        f"<{len(blobs[A_PITS_HURT]) // 2}H", blobs[A_PITS_HURT], 0))
    header_offs = blobs[A_ROOM_HEADER_OFFS]
    headers = blobs[A_ROOM_HEADERS]
    rooms = len(blobs[A_SPRITE_OFFS]) // 2

    print(f"# Dungeon rooms\n")
    print(f"{rooms} rooms, decoded from the tables in `zelda3_assets.dat` by")
    print("`tools/dungeon_rooms.py`. Sprite types are the same ids the warp")
    print("parameters take. See the end for what is and is not covered.\n")
    print("| Room | Enemies | NPCs | High HP | Overlords | Chests | Doors | Hazards |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- |")

    totals = {"enemy": 0, "npc": 0, "boss": 0}

    for room in range(rooms):
        sprites, overlords = room_sprites(blobs, room)
        by_cat = {"enemy": [], "npc": [], "boss": [], "?": []}

        for kind, _, _, _ in sprites:
            by_cat[category(kind, health, bump)].append(kind)

        for key in totals:
            totals[key] += len(by_cat[key])

        hazards = []
        tile = tiles.get(room, {})

        for name, label in (("spike", "spikes"), ("pit", "pits"), ("water", "water"),
                            ("ice", "ice"), ("conveyor", "conveyors")):
            if tile.get(name):
                hazards.append(f"{label} {tile[name]}")

        if room in pits_hurt:
            hazards.append("pits hurt")

        hoff = struct.unpack_from("<H", header_offs, room * 2)[0]

        if hoff + 13 < len(headers):
            hdr = headers[hoff:hoff + 14]

            if hdr[0] & 1:
                hazards.append("lights out")

            for tag in (hdr[5], hdr[6]):
                if tag:
                    hazards.append(f"tag {tag:02X}")

        def fmt(vals):
            return " ".join(f"{v:02X}" for v in sorted(set(vals))) or "-"

        chest_text = " ".join(
            f"{item:02X}{'*' if big else ''}" for item, big in chests.get(room, [])) or "-"

        doors = room_doors(blobs, room)
        door_text = " ".join(f"{d:04X}" for d in doors) or "-"
        print(f"| `{room:03X}` | {fmt(by_cat['enemy'])} | {fmt(by_cat['npc'])} | "
              f"{fmt(by_cat['boss'])} | {fmt([o[0] for o in overlords])} | {chest_text} | "
              f"{door_text} | {', '.join(hazards) or '-'} |")

    print()
    print(f"Totals: {totals['enemy']} enemies, {totals['npc']} npcs, {totals['boss']} high health, "
          f"{sum(len(v) for v in chests.values())} chests, {len(pits_hurt)} rooms where pits hurt.")
    print()
    print("## Reading the table\n")
    print("Sprite ids are hex, the same ones `--enemy-warp` and friends take. A chest marked")
    print("with `*` is a big chest. Doors are tilemap addresses from the room's door table,")
    print("the list the room load walks into `dung_door_tilemap_address`, ending at `FFFF`.")
    print("Overlords are the invisible spawners that sit in the same")
    print("list as the sprites, with an x of `E0` or more.\n")
    print("The High HP column is not a boss list. `dungeon_bosses.md` is that, built from the")
    print("boss handlers rather than from a threshold. This column is the same guess the warp")
    print(f"parameters make, a damaging sprite with 32 or more health, which catches {totals['boss']}")
    print("sprites across the game. The sprite data has no boss flag, so there is nothing")
    print("better to key on.\n")
    print("## Where the hazards come from\n")
    print("The counts are tiles, not objects, and they come from the game itself. Room")
    print("attributes are not in the room data: they are produced by drawing the room's")
    print("objects into a tilemap and then converting that tilemap through the current")
    print("tileset. `zelda3 --dump-rooms` does exactly that for all 320 rooms, in the same")
    print("order the game does when you walk in:\n")
    print("```c")
    print("        Dungeon_LoadRoom();")
    print("        Dungeon_LoadCustomTileAttr();")
    print("        Dungeon_LoadAttributeTable();")
    print("```\n")
    print("Feed its output back in to refresh this file:\n")
    print("```")
    print("./zelda3 --load-save <any save> --dump-rooms --quit-after 400 > /tmp/haz.txt")
    print("tools/dungeon_rooms.py --hazards /tmp/haz.txt > dungeon_rooms.md")
    print("```\n")
    print("`pits hurt` is separate. It comes from the table the game checks when Link lands,")
    print("so a room can have pit tiles that cost nothing. `lights out` and the tag bytes")
    print("come from the room header.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
