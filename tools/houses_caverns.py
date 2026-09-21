#!/usr/bin/env python3
"""Build the houses and caverns reference.

A house or cavern is an entrance whose palace byte is -1, meaning it belongs to no
dungeon. Everything below is read out of the game's own tables:

  entrance    kEntranceData_* in zelda3_assets.dat, 133 entrances
  screen      kOverworld_Entrance_Area, _Pos and _Id, which say where the door is
  doors       kDungeonRoomDoorOffs into kDungeonRoom, a tilemap address per door
  contents    the per room sprite and chest tables
  darkness    bit 0 of the room header, the same bit the dungeon list calls lights out
"""

import importlib.util
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Asset slots, from src/assets.h.
A_ROOM_DOOR_OFFS = 5
A_ROOM = 3
A_HEADERS, A_HEADER_OFFS = 6, 7
A_ENT_ROOMS, A_ENT_PLAYER_X, A_ENT_PLAYER_Y = 11, 15, 16
A_ENT_BLOCKSET, A_ENT_FLOOR, A_ENT_PALACE = 19, 20, 21
A_ENT_DOORWAY, A_ENT_STARTBG, A_ENT_QUAD1, A_ENT_QUAD2 = 22, 23, 24, 25
A_ENT_DOORSETTINGS, A_ENT_MUSIC = 26, 27
A_OW_ENT_AREA, A_OW_ENT_POS, A_OW_ENT_ID = 124, 125, 126


def load_module(path, name):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def u16s(blob):
    return list(struct.unpack_from("<%dH" % (len(blob) // 2), blob, 0))


def room_doors(blobs, room):
    off = struct.unpack_from("<H", blobs[A_ROOM_DOOR_OFFS], room * 2)[0]
    out, i = [], off

    while i + 1 < len(blobs[A_ROOM]):
        v = struct.unpack_from("<H", blobs[A_ROOM], i)[0]

        if v == 0xffff or len(out) >= 24:
            break

        out.append(v)
        i += 2

    return out


def spaced(name):
    return re.sub(r"(?<=[a-z0-9])(?=[A-Z])", " ", name)


def main():
    dr = load_module(ROOT / "tools" / "dungeon_rooms.py", "dr")
    blobs = dr.load_assets(ROOT / "zelda3_assets.dat")
    health, bump = dr.sprite_tables(ROOT)

    names = {}
    for hexid, nm in re.findall(r"Sprite_([0-9A-Fa-f]{2})_(\w+)",
                                (ROOT / "src" / "sprite_main.c").read_text()):
        names.setdefault(int(hexid, 16), nm)

    ent_rooms = u16s(blobs[A_ENT_ROOMS])
    ent_px, ent_py = u16s(blobs[A_ENT_PLAYER_X]), u16s(blobs[A_ENT_PLAYER_Y])
    ent_doorset = u16s(blobs[A_ENT_DOORSETTINGS])
    palace = [v if v < 128 else v - 256 for v in blobs[A_ENT_PALACE]]
    floor = [v if v < 128 else v - 256 for v in blobs[A_ENT_FLOOR]]
    blockset, music = blobs[A_ENT_BLOCKSET], blobs[A_ENT_MUSIC]
    startbg, quad1, quad2 = blobs[A_ENT_STARTBG], blobs[A_ENT_QUAD1], blobs[A_ENT_QUAD2]
    doorway = blobs[A_ENT_DOORWAY]

    # Where each entrance sits on the overworld.
    ow_area, ow_pos, ow_id = u16s(blobs[A_OW_ENT_AREA]), u16s(blobs[A_OW_ENT_POS]), blobs[A_OW_ENT_ID]
    on_screen = {}

    for i, eid in enumerate(ow_id):
        if i < len(ow_area):
            on_screen.setdefault(eid, []).append((ow_area[i], ow_pos[i]))

    chests = dr.room_chests(blobs)
    houses = [i for i in range(len(palace)) if palace[i] == -1]

    print("# Houses and caverns\n")
    print(f"{len(houses)} of the {len(palace)} entrances lead somewhere that is not a dungeon.")
    print("That is the test used here: the entrance's palace byte is -1. Dungeon rooms are")
    print("in `dungeon_rooms.md`.\n")
    print("An entrance is the door itself. The room is what is behind it, and several")
    print("entrances can share a room.\n")
    print("## Summary\n")
    print("| Entrance | Room | Screen | Music | Tileset | Doors | Sprites | Chests | Dark |")
    print("| --- | --- | --- | --- | --- | --- | --- | --- | --- |")

    detail = []

    for e in houses:
        room = ent_rooms[e]
        doors = room_doors(blobs, room)
        sprites, overlords = dr.room_sprites(blobs, room)
        screens = on_screen.get(e, [])
        hoff = struct.unpack_from("<H", blobs[A_HEADER_OFFS], room * 2)[0]
        hdr = blobs[A_HEADERS][hoff:hoff + 14]
        dark = bool(hdr[0] & 1) if len(hdr) == 14 else False
        scr = " ".join(f"`{a:02X}`" for a, _ in screens) or "-"
        print(f"| `{e:02X}` | `{room:03X}` | {scr} | `{music[e]:02X}` | `{blockset[e]:02X}` | "
              f"{len(doors) or '-'} | {len(sprites) or '-'} | "
              f"{len(chests.get(room, [])) or '-'} | {'yes' if dark else '-'} |")
        detail.append((e, room, doors, sprites, overlords, screens, hdr, dark))

    print("\n## Each entrance\n")

    for e, room, doors, sprites, overlords, screens, hdr, dark in detail:
        print(f"### Entrance `{e:02X}`, room `{room:03X}`\n")

        if screens:
            print("Door on overworld " + ", ".join(
                f"screen `{a:02X}` at map position `{p:04X}`" for a, p in screens) + ".\n")
        else:
            print("No overworld door points at this entrance, so it is reached another way.\n")

        bits = [f"music `{music[e]:02X}`", f"tileset `{blockset[e]:02X}`",
                f"background `{startbg[e]:02X}`", f"quadrants `{quad1[e]:02X}`/`{quad2[e]:02X}`",
                f"floor {floor[e]}", f"doorway `{doorway[e]:02X}`",
                f"door setting `{ent_doorset[e]:04X}`"]
        print("Entrance data: " + ", ".join(bits) + ".\n")
        print(f"Link starts at `{ent_px[e]:04X}`,`{ent_py[e]:04X}`.\n")

        if dark:
            print("Dark. The room header has bit 0 set, so it needs the lantern.\n")

        if doors:
            print("Doors: " + ", ".join(f"`{d:04X}`" for d in doors) + ".\n")
        else:
            print("No doors in the room's door table.\n")

        if sprites:
            print("Sprites:\n")
            for kind, x, y, fl in sprites:
                nm = spaced(names[kind]) if kind in names else "unnamed"
                role = "npc" if bump[kind] == 0 else "enemy"
                print(f"- `{kind:02X}` {nm} ({role}) at `{x:04X}`,`{y:04X}`, floor {fl}")
            print()

        if overlords:
            print("Overlords: " + ", ".join(f"`{o[0]:02X}`" for o in overlords) + ".\n")

        if chests.get(room):
            print("Chests: " + ", ".join(
                f"`{item:02X}`" + (" big" if big else "") for item, big in chests[room]) + ".\n")

    print("## Reading it\n")
    print("Music, tileset, background and quadrant values are ids into the game's own")
    print("tables, not names. The tileset is what `kEntranceData_blockset` calls the")
    print("blockset, which decides the graphics the room is built from.\n")
    print("Door values are tilemap addresses rather than door numbers. A room's door table")
    print("is a list of them ending in `FFFF`, and the room load walks it straight into")
    print("`dung_door_tilemap_address`.\n")
    print("Sprite positions are room pixels. The floor column separates the two layers a")
    print("room can have.\n")
    print("## What is not here\n")
    print("Which door leads where. The door table says where a door is drawn, not what is")
    print("on the other side, and the connection is made from Link's position when he walks")
    print("through rather than from a table.\n")
    print("Furniture and scenery. Those are room objects, and reading them needs a decoder")
    print("for the object format that does not exist yet.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
