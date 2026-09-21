#!/usr/bin/env python3
"""Build the non-boss enemy reference.

Every field is read out of the game rather than written by hand:

  name, states   the handler in src/sprite_main.c, whose name spells its type id
  HP             kSpriteInit_Health in src/sprite.c
  damage         kSpriteInit_BumpDamage, whose low nibble indexes kPlayerDamages
  timers         the sprite delay values each handler writes, in frames
  sounds         the sound effect ids each handler queues
  spawns         Sprite_SpawnDynamically calls
  where found    the overworld sprite tables and the per room dungeon tables

Bosses are excluded, they have their own document. So are sprites whose bump byte
is zero, which are mostly npcs, props and projectiles. That test is a proxy: a
zero byte is damage class 0, which is 2/1/1 rather than nothing, and what really
makes an npc harmless is that it never runs the contact damage path at all.
"""

import argparse
import importlib.util
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FRAMES_PER_SECOND = 60.0

BOSSES = {0x09, 0x53, 0x54, 0x7A, 0x88, 0x8C, 0x92, 0xA2, 0xBD, 0xCB, 0xCE, 0xD6}

# Helper functions are matched by the enemy's name stem. These words appear in names that
# belong to something else, so a stem that matches them is not treated as shared logic.
STEM_NOISE = re.compile(r"^(Sprite|Draw|Prep|Handle|Check|Spawn|Main|Init)$", re.I)


def load_rooms_module():
    spec = importlib.util.spec_from_file_location("dr", ROOT / "tools" / "dungeon_rooms.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def function_bodies(text):
    out = {}

    for m in re.finditer(r"\n(?:static )?void (\w+)\([^)]*\) \{", text):
        i = m.end()
        depth = 1

        while i < len(text) and depth:
            if text[i] == '{':
                depth += 1
            elif text[i] == '}':
                depth -= 1
            i += 1

        out[m.group(1)] = text[m.end():i]

    return out


def player_damages(root):
    """kPlayerDamages in src/sprite.c: ten damage classes, three armor levels each."""
    text = (root / "src" / "sprite.c").read_text()
    start = text.index("kPlayerDamages[30]")
    body = text[text.index("{", start) + 1:text.index("};", start)]
    vals = [int(v.strip(), 0) for v in body.replace("\n", " ").split(",") if v.strip()]
    return [vals[i * 3:i * 3 + 3] for i in range(10)]


def damage_text(bump_byte, table):
    """The bump field is a class index in its low nibble, not an amount."""
    cls = bump_byte & 0xf
    green, blue, red = table[cls]
    return cls, f"{green}/{blue}/{red}"


def spaced(name):
    return re.sub(r"(?<=[a-z0-9])(?=[A-Z])", " ", name)


def state_machine(blob):
    """Case labels and their comments, for the first switch on a state variable."""
    m = re.search(r"switch\s*\(\s*sprite_(ai_state|subtype2|[A-G])\[\w+\]\s*\)\s*\{", blob)

    if not m:
        return None, []

    i = m.end()
    depth = 1

    while i < len(blob) and depth:
        if blob[i] == '{':
            depth += 1
        elif blob[i] == '}':
            depth -= 1
        i += 1

    body = blob[m.end():i]
    cases = []

    for c in re.finditer(r"\n\s*case (\w+):\s*(?:\{)?\s*(?://\s*(.*))?", body):
        label = c.group(1)
        note = (c.group(2) or "").strip()
        # A comment that is really the next line of code is not a name.
        if note.startswith(("if", "sprite_", "}", "/*")) or len(note) > 28:
            note = ""
        cases.append((label, note))

    return "sprite_" + m.group(1), cases


def timers(blob):
    found = {}

    for var, val in re.findall(
            r"sprite_(delay_main|delay_aux\d)\[\w+\]\s*=\s*(0x[0-9a-fA-F]+|\d+)", blob):
        value = int(val, 0)

        if value:
            found.setdefault(var, set()).add(value)

    return found


def gates(blob):
    out = []

    for line in blob.splitlines():
        s = line.strip()

        if not re.match(r"(if|\} else if|else if)\s*\(", s):
            continue

        if re.search(r"sprite_health|sprite_[xy]_vel|sprite_z\[|frame_counter &", s):
            out.append(re.sub(r"\s+", " ", s)[:88])

    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--detail", type=int, default=0,
                    help="only give a full section to enemies placed at least this many times")
    args = ap.parse_args()

    dr = load_rooms_module()
    blobs = dr.load_assets(ROOT / "zelda3_assets.dat")
    health, bump = dr.sprite_tables(ROOT)
    text = (ROOT / "src" / "sprite_main.c").read_text()
    fns = function_bodies(text)

    names = {}
    for hexid, name in re.findall(r"Sprite_([0-9A-Fa-f]{2})_(\w+)", text):
        names.setdefault(int(hexid, 16), name)

    # Where each type is placed. The dungeon decoder lives in the rooms tool; the overworld
    # tables are read the same way Overworld_LoadSprites does.
    dungeon = {}

    for room in range(320):
        for kind, _, _, _ in dr.room_sprites(blobs, room)[0]:
            dungeon.setdefault(kind, []).append(room)

    overworld = {}

    for screen in range(0x80):
        entry = dr.overworld_sprites(blobs, screen)

        for kind, _, _ in entry:
            overworld.setdefault(kind, []).append(screen)

    damages = player_damages(ROOT)

    enemies = [t for t in sorted(names)
               if t not in BOSSES and t < len(bump) and bump[t] > 0]

    print("# Enemy combat\n")
    print(f"{len(enemies)} non-boss sprites that do contact damage, generated by")
    print("`tools/enemy_combat.py`. Bosses are in `boss_combat.md`. Sprites whose bump")
    print("byte is zero are left out, which is mostly npcs, props and projectiles.\n")
    print("That last test is a proxy rather than a proof. A zero byte is damage class 0,")
    print("worth 2/1/1, not nothing. What actually makes an npc harmless is that it never")
    print("runs the contact damage path, which is behaviour and not a table.\n")
    print("Timers are frame counts and each drops by one per frame of normal play, so a")
    print("value of 60 is about a second. A state with no timer ends on a condition")
    print("instead.\n")
    print("## Summary\n")
    print("| Type | Name | HP | Damage | Screens | Rooms | States | Timers |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- |")

    detail = []

    for t in enemies:
        stem = names[t]
        related = {n: b for n, b in fns.items() if stem in n}
        blob = "\n".join(related.values())
        var, cases = state_machine(blob)
        tm = timers(blob)
        rooms = dungeon.get(t, [])
        screens = overworld.get(t, [])
        hp = health[t] if t < len(health) else 0
        cls, dmg = damage_text(bump[t], damages)
        print(f"| `{t:02X}` | {spaced(stem)} | {hp} | {dmg} | {len(screens)} | {len(rooms)} | "
              f"{len(cases) if cases else '-'} | {sum(len(v) for v in tm.values()) or '-'} |")
        detail.append((t, stem, related, blob, var, cases, tm, rooms, screens, cls, dmg))

    print("\n## Enemies\n")

    for t, stem, related, blob, var, cases, tm, rooms, screens, cls, dmg in detail:
        hp = health[t] if t < len(health) else 0
        print(f"### {spaced(stem)} `{t:02X}`\n")
        print(f"HP {hp}, damage class {cls}, costing {dmg} against green, blue and red mail. "
              f"{len(related)} function"
              f"{'' if len(related) == 1 else 's'}: "
              + ", ".join(f"`{n}`" for n in sorted(related)) + ".\n")

        if screens:
            shown = " ".join(f"`{s:02X}`" for s in sorted(set(screens))[:14])
            more = "" if len(set(screens)) <= 14 else f" and {len(set(screens)) - 14} more"
            n = len(set(screens))
            print(f"Overworld screens: {shown}{more}. {len(screens)} placement"
                  f"{'' if len(screens) == 1 else 's'} in {n} screen{'' if n == 1 else 's'}.\n")

        if rooms:
            shown = " ".join(f"`{r:03X}`" for r in sorted(set(rooms))[:14])
            more = "" if len(set(rooms)) <= 14 else f" and {len(set(rooms)) - 14} more"
            n = len(set(rooms))
            print(f"Dungeon rooms: {shown}{more}. {len(rooms)} placement"
                  f"{'' if len(rooms) == 1 else 's'} in {n} room{'' if n == 1 else 's'}.\n")
        elif not screens:
            print("Not placed by either table, so it is spawned by something else.\n")

        if cases:
            labels = ", ".join(f"`{lab}`" + (f" {note}" if note else "") for lab, note in cases)
            print(f"States, on `{var}`: {labels}.\n")

        if tm:
            parts = []
            for key in sorted(tm):
                vals = ", ".join(f"{v} ({v / FRAMES_PER_SECOND:.2f}s)" for v in sorted(tm[key]))
                parts.append(f"`{key}` {vals}")
            print("Timers: " + "; ".join(parts) + ".\n")

        sfx = sorted({v for _, v in re.findall(
            r"(SpriteSfx_QueueSfx\d WithPan|SpriteSfx_QueueSfx\dWithPan)\(\w+,\s*(0x[0-9a-fA-F]+|\d+)",
            blob)})
        if sfx:
            print("Sounds: " + ", ".join(f"`{int(v, 0):02X}`" for v in sfx) + ".\n")

        spawns = sorted({int(x, 16) for x in re.findall(
            r"Sprite_SpawnDynamically\([^,]+,\s*0x([0-9a-fA-F]{2})", blob)})
        if spawns:
            print("Spawns: " + ", ".join(
                f"`{sp:02X}`" + (f" {spaced(names[sp])}" if sp in names else "")
                for sp in spawns) + ".\n")

        g = gates(blob)
        if g:
            print("Gates:\n")
            print("```c")
            for line in g[:3]:
                print("  " + line)
            print("```\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
