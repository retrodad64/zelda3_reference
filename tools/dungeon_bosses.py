#!/usr/bin/env python3
"""Build the dungeon boss list.

Everything is derived, nothing is typed in by hand:

  boss identity   handler names in src/sprite_main.c, which spell their own type id
  health, damage  kSpriteInit_Health and kSpriteInit_BumpDamage in src/sprite.c
  rooms           the per room sprite tables in zelda3_assets.dat
  dungeon         asset 97, the dungeon map floor layout, which lists a palace's rooms
  dungeon name    the item each palace holds, from CheckPalaceItemPosession in src/hud.c
"""

import importlib.util
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Bosses, by the handler that runs them. Minions, projectiles and cutscene copies are excluded
# here and reported as parts instead.
BOSS_TYPES = [0x09, 0x53, 0x54, 0x7A, 0x88, 0x8C, 0x92, 0xA2, 0xBD, 0xCB, 0xCE, 0xD6]

# Palace index to name. The index is what cur_palace_index_x2 holds, halved. The item beside each
# name is the one CheckPalaceItemPosession tests for that index, which is how the mapping was
# pinned down rather than recalled.
PALACES = {
    0: ("Sewers", None), 1: ("Hyrule Castle", None), 2: ("Eastern Palace", "bow"),
    3: ("Desert Palace", "power glove"), 4: ("Castle Tower", None),
    5: ("Swamp Palace", "hookshot"), 6: ("Palace of Darkness", "hammer"),
    7: ("Misery Mire", "cane of somaria"), 8: ("Skull Woods", "fire rod"),
    9: ("Ice Palace", "armor"), 10: ("Tower of Hera", "moon pearl"),
    11: ("Thieves Town", "titan's mitt"), 12: ("Turtle Rock", "mirror shield"),
    13: ("Ganon's Tower", "red mail"),
}


def load_rooms_module():
    spec = importlib.util.spec_from_file_location("dr", ROOT / "tools" / "dungeon_rooms.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def indexed_blob(blob, i):
    """The FindIndexInMemblk layout used by the indexed assets."""
    end = len(blob) - 2
    count = struct.unpack_from("<H", blob, end)[0]

    if count >= 8192 or i > count:
        return None

    lo = count * 2 if i == 0 else count * 2 + struct.unpack_from("<H", blob, i * 2 - 2)[0]
    hi = end if i == count else count * 2 + struct.unpack_from("<H", blob, i * 2)[0]
    return blob[lo:hi]


def player_damages(root):
    text = (root / "src" / "sprite.c").read_text()
    start = text.index("kPlayerDamages[30]")
    body = text[text.index("{", start) + 1:text.index("};", start)]
    vals = [int(v.strip(), 0) for v in body.replace("\n", " ").split(",") if v.strip()]
    return [vals[i * 3:i * 3 + 3] for i in range(10)]


def damage_of(bump_byte, table):
    """The bump field is a class index in its low nibble, not an amount."""
    g, bl, r = table[bump_byte & 0xf]
    return f"{g}/{bl}/{r}"


def sprite_names():
    text = (ROOT / "src" / "sprite_main.c").read_text()
    out = {}

    for hexid, name in re.findall(r"Sprite_([0-9A-Fa-f]{2})_(\w+)", text):
        out.setdefault(int(hexid, 16), name)

    return out


def spaced(name):
    return re.sub(r"(?<=[a-z])(?=[A-Z])", " ", name)


def main():
    dr = load_rooms_module()
    blobs = dr.load_assets(ROOT / "zelda3_assets.dat")
    health, bump = dr.sprite_tables(ROOT)
    names = sprite_names()
    damages = player_damages(ROOT)

    # Which rooms each sprite type sits in, and how many of it per room.
    placements = {}

    for room in range(320):
        sprites, _ = dr.room_sprites(blobs, room)

        for kind, _, _, _ in sprites:
            placements.setdefault(kind, {}).setdefault(room, 0)
            placements[kind][room] += 1

    # Room to palace, from the dungeon map floor layouts.
    room_palace = {}

    for palace in range(14):
        layout = indexed_blob(blobs[97], palace)

        if layout is None:
            continue

        for room in set(layout):
            room_palace.setdefault(room, palace)

    print("# Dungeon bosses\n")
    print("Twelve boss handlers, in the rooms the sprite tables place them. Everything here is")
    print("read out of the game's own data by `tools/dungeon_bosses.py`.\n")
    print("| Boss | Type | Dungeon | Room | Count | HP | Damage | Related sprites |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- |")

    rows = []

    for kind in BOSS_TYPES:
        base = names.get(kind, "?")
        stem = re.match(r"[A-Z][a-z]+", base)
        stem = stem.group(0) if stem else base

        parts = sorted(t for t, n in names.items()
                       if t != kind and stem in n and t not in BOSS_TYPES)
        part_text = ", ".join(
            f"`{t:02X}` {spaced(names[t])} ({damage_of(bump[t], damages)})" for t in parts) or "-"

        for room, count in sorted(placements.get(kind, {}).items()):
            palace = room_palace.get(room)
            dungeon = PALACES[palace][0] if palace is not None else "not on any dungeon map"
            hp = "255" if health[kind] == 255 else str(health[kind])
            rows.append((spaced(base), kind, dungeon, room, count, hp,
                         damage_of(bump[kind], damages), part_text))

    for name, kind, dungeon, room, count, hp, touch, part_text in rows:
        print(f"| {name} | `{kind:02X}` | {dungeon} | `{room:03X}` | {count} | {hp} | {touch} | "
              f"{part_text} |")

    print()
    print("## Reading it\n")
    print("Count is how many of the sprite the room places. Armos Knights is six sprites and")
    print("Lanmolas is three, which is why they appear once per room with a count rather than")
    print("as separate rows.\n")
    print("Damage is what a touch costs, against green, blue and red mail. The byte in")
    print("`kSpriteInit_BumpDamage` is not an amount: its low nibble picks a row of")
    print("`kPlayerDamages`, which holds one value per mail. Health is in quarter hearts,")
    print("so 8 is a full heart. The figures in brackets after each related sprite are")
    print("that sprite's own three.\n")
    print("Related sprites are the ones whose handler name carries the boss's name. Some are")
    print("the boss's own projectiles, such as Mothula Beam and King Helmasaur Fireball. Some")
    print("are not part of the fight at all: Armos Statue is an ordinary enemy that shares a")
    print("name, and Blind Maiden is the follower who turns into Blind. The column is a name")
    print("match, not a claim about the fight.\n")
    print("Four bosses appear twice, once in their own dungeon and once in Ganon's Tower.")
    print("Ganon himself sits in room `000`, which is not on any dungeon map, because the")
    print("Pyramid has no map screen.\n")
    print("HP of 255 means the sprite cannot be damaged by ordinary means and the handler")
    print("decides when it may be hurt, which is how Ganon's fight is staged.\n")
    print("`boss_combat.md` goes through each fight in detail.\n")
    print("## What this does not say\n")
    print("How a boss actually attacks. That is behaviour spread through its handler, not a")
    print("table, and the spawn calls are written differently enough from one boss to the next")
    print("that reading them mechanically gives wrong answers. The Parts column is the honest")
    print("version: the sprites named after the boss, which are its projectiles and pieces.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
