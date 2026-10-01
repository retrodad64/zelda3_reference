# Spots from the pug hero demo

F2 in the pug hero demo remembers where Link is standing, in `~/.pug_engine/pug_hero_demo_spot.txt`.
The demo reads its own first lines back to start there next time. After them it writes the same
place and his whole kit and story in the cartridge's own terms, and `--spot` reads those, so the
real game can be put in the same place with the same state and the two compared.

```
./zelda3 --spot ~/.pug_engine/pug_hero_demo_spot.txt
```

## How he gets there

It goes a step at a time, each waiting for the game to be in play (module 7 or 9, submodule 0):

1. If the game starts in a room, which a new file does (Link's house), he goes out by that
   room's own way out (`Dung_HandleExitToOverworld`).
2. On the overworld the kit and story go down. The item bytes are cleared and set from the
   file, and so are every room's `save_dung_info` word and the overworld's `save_ow_event_info`,
   so what the file does not name is as a new file has it. Rooms 0x106 and 0x107 keep the four
   open doors every new file gives them.
3. Outdoors, he lands on the spot the way `--goto` does.
4. In a room, he goes in by the entrance the file names, through the falling entrance module
   `--entrance` uses, which sets up the dungeon. With no entrance named, the first entrance
   whose room is his is used. From the entrance's room he falls into his own room the way a pit
   drops him (`Module07_07`), landing on the spot on the floor the file names.

## The format

Plain text, one thing a line, a word and then its values. The demo's own lines come first and
are skipped here. Numbers in hexadecimal are the cartridge's bytes and words; the rest are
decimal. A line this does not know is left out with a note on stderr.

| Line | Says |
| --- | --- |
| `cart 1` | The format. Anything else is refused. |
| `place outdoors` or `place room` | Where he is. |
| `cart_realm light` | The world, or the world outside the room's dungeon. |
| `cart_room 0C9` | The room, when he is in one. |
| `cart_entrance 08` | The way in he took, when there was one. |
| `cart_floor 0` | `link_is_on_lower_level`: which of the room's attribute tables he is on. |
| `cart_link 248 307` | `link_x_coord` and `link_y_coord`, the top left of his sprite: world pixels outdoors, pixels from the room's corner in a room. |
| `cart_face 2` | `link_direction_facing`: 0 up, 2 down, 4 left, 6 right. |
| `cart_item bow 2` | One line a thing in his kit, by the name `--scene` uses. |
| `cart_hearts`, `cart_health`, `cart_magic` | Containers, health in eighths, the meter. |
| `cart_magic_rate` | `link_magic_consumption`: 0 normal, 1 half, 2 quarter. |
| `cart_rupees`, `cart_bombs`, `cart_arrows`, `cart_heart_pieces` | What he carries. |
| `cart_dungeon_keys 0 0 ...` | `link_keys_earned_per_dungeon`, sixteen of them. |
| `cart_big_keys`, `cart_maps`, `cart_compasses` | `link_bigkey`, `link_dungeon_map`, `link_compass`. |
| `cart_story 2` | `sram_progress_indicator`. |
| `cart_progress_flags 11`, `cart_progress3 04` | `sram_progress_flags` and `sram_progress_indicator_3`. |
| `cart_map_icons`, `cart_start_point`, `cart_follower` | `savegame_map_icons_indicator`, `which_starting_point`, `follower_indicator`. |
| `cart_room_word 0C9 800F` | One room's `save_dung_info` word. |
| `cart_ow_event 1B 20` | One area's `save_ow_event_info` byte. |

## What is not the same

- The demo keeps some of a room's word as separate lists and puts them back together for the
  file. A moving wall is always written as the first tag's bit (0x1000) and a creature's key as
  a small key's (0x4000), which is right for nearly every room.
- A follower is only the indicator. Nobody walks in beside him until the game sets one up
  itself, on the next area it loads.
- The camera after the fall into a room is the cartridge's own, so it can frame the room
  differently from the demo's.
- What the game draws goes by its own rules on the whole state: the shield is not drawn before
  `sram_progress_indicator` is past nought, for one, whatever the kit says. The demo's debug
  kit can hold things the story has not given yet.
