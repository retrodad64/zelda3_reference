#!/usr/bin/env python3
"""Build the NPC reference.

An NPC here is a sprite with a named handler whose bump byte is zero, which is
what separates the people and props from the enemies in `enemy_combat.md`. That
test is a proxy: a zero byte is damage class 0, and what really makes an NPC
harmless is that it never runs the contact damage path.

Sources, all read rather than typed in:

  names, movement  the handlers in src/sprite_main.c, whose names spell their type
  location         the overworld sprite tables and the per room dungeon tables
  tiles            the draw functions and their graphics writes
  dialogue         decoded from the ROM with assets/text_compression.py
"""

import argparse
import importlib.util
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FPS = 60.0
BOSSES = {0x09, 0x53, 0x54, 0x7A, 0x88, 0x8C, 0x92, 0xA2, 0xBD, 0xCB, 0xCE, 0xD6}


def load_module(path, name):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def dialogue_lines():
    """Every message in the ROM, indexed the way the code indexes them."""
    assets = ROOT / "assets"
    saved = sys.path[:]
    cwd = Path.cwd()

    try:
        sys.path.insert(0, str(assets))
        import os
        os.chdir(assets)
        util = load_module(assets / "util.py", "z3util")
        tc = load_module(assets / "text_compression.py", "z3text")
        rom = util.load_rom(None)
        texts = tc.decode_strings_generic(rom.get_byte, rom.language)

        if len(texts) == 396:
            extra = "[Speed 00]0- [Number 00]. 1- [Number 01][2]2- [Number 02]. 3- [Number 03]"
            texts = texts[:4] + [(extra, None)] + texts[4:]

        return [t[0] for t in texts]
    except Exception as e:
        print(f"note: dialogue not decoded ({e})", file=sys.stderr)
        return []
    finally:
        os.chdir(cwd)
        sys.path[:] = saved


def bodies(text):
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


def spaced(name):
    return re.sub(r"(?<=[a-z0-9])(?=[A-Z])", " ", name)


def one_line(s, limit=150):
    """Dialogue with its control codes turned into something readable on one line."""
    s = re.sub(r"\[(Waitkey|Scroll|Speed \w+|Choose|1|2|3)\]", " ", s)
    s = s.replace("[Name]", "Link").replace("[...]", "...")
    s = re.sub(r"\[([^\]]*)\]", r"<\1>", s)
    s = re.sub(r"\s+", " ", s).strip()
    return s if len(s) <= limit else s[:limit - 3] + "..."


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.parse_args()

    dr = load_module(ROOT / "tools" / "dungeon_rooms.py", "dr")
    blobs = dr.load_assets(ROOT / "zelda3_assets.dat")
    health, bump = dr.sprite_tables(ROOT)
    text = (ROOT / "src" / "sprite_main.c").read_text()
    fns = bodies(text)
    texts = dialogue_lines()

    names = {}
    for hexid, nm in re.findall(r"Sprite_([0-9A-Fa-f]{2})_(\w+)", text):
        names.setdefault(int(hexid, 16), nm)

    dungeon, overworld = {}, {}

    for room in range(320):
        for kind, _, _, _ in dr.room_sprites(blobs, room)[0]:
            dungeon.setdefault(kind, []).append(room)

    for screen in range(0x80):
        for kind, _, _ in dr.overworld_sprites(blobs, screen):
            overworld.setdefault(kind, []).append(screen)

    npcs = [t for t in sorted(names)
            if t not in BOSSES and t < len(bump) and bump[t] == 0]

    print("# NPC reference\n")
    print(f"{len(npcs)} sprites with a named handler and a bump byte of zero: the people, the")
    print("shopkeepers, the followers and the props that share the sprite system with the")
    print("enemies. Enemies are in `enemy_combat.md`.\n")
    print("The zero bump test is a proxy rather than a proof. A zero byte is damage class 0,")
    print("worth 2/1/1 rather than nothing, and what actually keeps an NPC harmless is that")
    print("it never runs the contact damage path.\n")

    if texts:
        print(f"Dialogue is decoded from the ROM, {len(texts)} messages. A message id in the")
        print("code is an index into that list. Control codes are folded out for reading, so")
        print("`[Waitkey]` and `[Scroll]` become spaces and `[Name]` becomes Link.\n")
    else:
        print("Dialogue ids are listed without their text, because the ROM was not readable.\n")

    print("## Summary\n")
    print("| Type | Name | Screens | Rooms | Messages | Moves |")
    print("| --- | --- | --- | --- | --- | --- |")

    detail = []

    for t in npcs:
        stem = names[t]
        stem_re = re.compile(re.escape(stem) + r"(?![a-z])")
        other = re.compile(r"^Sprite_([0-9A-Fa-f]{2})_")
        related = {}

        for n, b in fns.items():
            m = other.match(n)

            if m and int(m.group(1), 16) != t:
                continue

            if n.startswith(f"Sprite_{t:02X}_") or stem_re.search(n):
                related[n] = b

        blob = "\n".join(related.values())
        msgs = sorted({int(v, 0) for v in re.findall(
            r"(?:Sprite_ShowMessageUnconditional|Sprite_ShowSolicitedMessage)\("
            r"(?:\w+,\s*)?(0x[0-9a-fA-F]+|\d+)\)", blob)}
            | {int(v, 0) for v in re.findall(
                r"dialogue_message_index\s*=\s*(0x[0-9a-fA-F]+|\d+)", blob)})
        moves = len(re.findall(r"sprite_[xy]_vel\[", blob))
        screens, rooms = overworld.get(t, []), dungeon.get(t, [])
        print(f"| `{t:02X}` | {spaced(stem)} | {len(set(screens)) or '-'} | "
              f"{len(set(rooms)) or '-'} | {len(msgs) or '-'} | {moves or '-'} |")
        detail.append((t, stem, related, blob, msgs, moves, screens, rooms))

    print("\n## Each NPC\n")

    for t, stem, related, blob, msgs, moves, screens, rooms in detail:
        print(f"### {spaced(stem)} `{t:02X}`\n")
        print(f"{len(related)} function{'' if len(related) == 1 else 's'}: "
              + ", ".join(f"`{n}`" for n in sorted(related)) + ".\n")

        where = []

        if screens:
            shown = " ".join(f"`{s:02X}`" for s in sorted(set(screens))[:12])
            more = "" if len(set(screens)) <= 12 else f" and {len(set(screens)) - 12} more"
            where.append(f"Overworld screens {shown}{more}")

        if rooms:
            shown = " ".join(f"`{r:03X}`" for r in sorted(set(rooms))[:12])
            more = "" if len(set(rooms)) <= 12 else f" and {len(set(rooms)) - 12} more"
            where.append(f"dungeon rooms {shown}{more}")

        print((", ".join(where) + ".\n") if where
              else "Not placed by either table, so something else spawns it.\n")

        draws = sorted(n for n in related if "Draw" in n or "Oam" in n)
        gfx = len(re.findall(r"sprite_graphics\[\w+\]\s*=", blob))
        anim = len(re.findall(r"sprite_anim_clock\[\w+\]", blob))

        if draws or gfx or anim:
            bits = []
            if draws:
                bits.append("drawn by " + ", ".join(f"`{d}`" for d in draws))
            if gfx:
                bits.append(f"{gfx} graphics write{'' if gfx == 1 else 's'}")
            if anim:
                bits.append(f"{anim} animation clock use{'' if anim == 1 else 's'}")
            print("Tiles: " + ", ".join(bits) + ".\n")

        m = re.search(r"switch\s*\(\s*sprite_(ai_state|subtype2|[A-G])\[\w+\]\s*\)", blob)

        if moves or m:
            bits = []
            if m:
                bits.append(f"a state machine on `sprite_{m.group(1)}`")
            bits.append(f"{moves} velocity write{'' if moves == 1 else 's'}")
            print("Movement: " + ", ".join(bits) + ".\n")
        else:
            print("Movement: none, it does not write a velocity.\n")

        if msgs:
            print("Dialogue:\n")
            for msg in msgs:
                if texts and 0 <= msg < len(texts):
                    print(f"- `{msg:03X}` {one_line(texts[msg])}")
                else:
                    print(f"- `{msg:03X}`")
            print()

    print("## What is not here\n")
    print("Which message a branch picks at run time. Many NPCs choose between several")
    print("messages on a flag, an item or a previous answer, and the list above is every")
    print("message the handler can reach, not the order it reaches them in.\n")
    print("The tile numbers themselves. A draw function builds its OAM from a table keyed")
    print("on the frame, so the graphics a sprite uses are a function of its state rather")
    print("than a fixed list.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
