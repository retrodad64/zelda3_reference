# Command line parameters

Everything is optional. Running `./zelda3` with no arguments starts the game
normally.

| Parameter | Takes | What it does |
| --- | --- | --- |
| `--config <file>` | a path | Reads settings from this file instead of searching for `zelda3.ini` |
| `--skip-intro` | nothing | Starts a new file straight away |
| `--load-save <file>` | a path | Loads a savestate once the game is running |
| `--screen-id <hex>` | 00 to 7F | Jumps to an overworld screen once the game reaches one |
| `--goto [d:]<x>,<y>` | world pixels | Lands on an exact spot, `d:` for the dark world |
| `--list-liftables` | nothing | Prints every liftable cell on the loaded map, then carries on |
| `--invincible` | nothing | Turns the damage cheat on before the first frame |
| `--give-flippers` | nothing | Puts the flippers in Link's kit once the save has loaded |
| `--no-music` | nothing | Starts with the music off |
| `--keys <file>` | a path | Runs a key script, so the game plays itself |
| `--quit-after <n>` | a frame count | Exits cleanly once that many frames have run |
| `--list-sprites` | nothing | Prints every sprite the overworld data places, on all 128 screens |
| `--dump-rooms` | nothing | Prints the hazard tile counts for all 320 dungeon rooms |
| `--no-music` | nothing | Starts with the music off |
| `--enemy-warp <hex>` | a sprite type | Puts Link on a safe tile beside the first sprite of that type |
| `--npc-warp <hex>` | a sprite type | The same, named for a harmless sprite |
| `--boss-warp <hex>` | a sprite type | The same, named for a high health sprite |
| `--scene <file>` | a path | Rebuilds a scene saved by the pug hero demo's entity sandbox |
| `--spot <file>` | a path | Goes where F2 in the pug hero demo left Link, with his kit and story |
| `--help` | nothing | Prints the option list and exits |
| a bare path | a path | ROM for the reference emulator |

Keys are not set here. They live in the `[KeyMap]` section of `zelda3.ini`.

## Options that conflict

Anything that decides where Link ends up claims that job, and a second one fails
before the window opens:

```
$ ./zelda3 --screen-id 5A --boss-warp 53
--boss-warp cannot be used with --screen-id, they both decide where Link ends up
```

That covers `--screen-id`, `--goto`, `--enemy-warp`, `--npc-warp` and
`--boss-warp`, including two warps against each other. It exits with status 1.
New options join the group by calling `ClaimPositionOption` as they are accepted.

`--skip-intro` and `--load-save` are deliberately not in that group. They have a
defined precedence instead, with the savestate winning, because loading a save
over a fresh file is a reasonable thing to ask for.

## Ordering

`--config` only counts as the first argument. This is the test:

<sub><code>src/main.c</code></sub>
```c
  if (argc >= 2 && strcmp(argv[0], "--config") == 0) {
    config_file = argv[1];
    argc -= 2, argv += 2;
  } else {
    SwitchDirectory();
  }
```

Put it anywhere else and it's ignored. Two things follow from that `else`. When
you pass `--config`, the game skips the search that walks up to three directories
looking for `zelda3.ini`, so it stays in the directory you launched from and
every relative path is relative to that. When you leave it off, the game moves to
wherever it found `zelda3.ini`, which is where `zelda3_debug.log` and the PNG
dumps land.

The rest can go in any order. Each one is matched once, so passing the same
parameter twice only picks up the first.

## --skip-intro

Starts file one on a brand new save and drops you in Link's house, past the
title, the attract loop, the file select and the opening. Useful for getting
into the game quickly, not for resuming anything.

## --load-save

Loads a savestate, the same files the F1 to F10 keys use, so any path works
including the 13 reference saves under `saves/ref`. Both spellings are accepted:

```
./zelda3 --load-save saves/save3.sav
./zelda3 --load-save="saves/ref/Chapter 5 - After Hyrule Castle Tower.sav"
```

The file is opened and closed once during parsing, purely to check it exists. A
bad path stops the program before the window appears:

```
$ ./zelda3 --load-save /nope/missing.sav
Unable to open save file '/nope/missing.sav'
```

That exits with status 1. The load itself happens after the first frame, because
the game only initialises itself on that frame.

A savestate carries the whole machine, so it overrides `--skip-intro` if you pass
both. The new file that flag would have set up is discarded.

## --screen-id

Jumps to an overworld screen, the same jump Shift+G performs. The value is hex,
00 to 7F, with 40 and up being the dark world. A bad value stops the program:

```
$ ./zelda3 --screen-id 90
--screen-id wants an overworld screen in hex, 00 to 7F, got '90'
```

It waits for the overworld rather than firing on a fixed frame, since a save load
and an intro skip take different amounts of time to settle:

<sub><code>src/main.c</code></sub>
```c
    if (g_screen_id >= 0) {
      if (main_module_index == 9 && !player_is_indoors) {
        DebugGoto_JumpTo((uint8)g_screen_id);
        g_screen_id = -1;
      } else if (++g_screen_id_waited >= 600) {
        fprintf(stderr, "--screen-id: never reached the overworld, ignoring\n");
        g_screen_id = -1;
      }
    }
```

Module 9 is the overworld. If the game isn't there within 600 frames, roughly ten
seconds, it gives up and writes the message to stderr. So the parameter needs
something that puts you outdoors. On its own it has nothing to act on, because
the game sits on the title screen.

Pair it with a save that's outdoors:

```
./zelda3 --load-save "saves/ref/Chapter 2 - After Eastern Palace.sav" --screen-id 5B
```

That prints the load and the jump, in that order:

```
*** Loading 'saves/ref/Chapter 2 - After Eastern Palace.sav'
goto: screen 5B at 0700,0700
```

The jump lands Link in the middle of the screen. It writes game variables
directly instead of going through the state recorder, so it won't appear in a
replay, and using it during one will throw the recording out of step.

`--goto` takes a coordinate instead of a screen number, and the coordinate is
where his feet go. That's the same position the tile log prints as `feet=`, and
the same one another program comparing itself against this one will be working
in. `link_x_coord` and `link_y_coord` hold the top left of his sprite, eight
pixels left and sixteen up from that, and the jump takes both off for you.

A coordinate that falls inside one of the overworld's big areas works the same
as any other. A big area is two screens square and the game only ever loads it
by its top left corner, so the jump finds that corner and loads the area from
there while leaving Link on the spot you asked for. Without that the map comes
up as a quadrant the game never draws, which looks like the wrong place
entirely.

## The bare path

A path with no parameter in front of it is read as a ROM and handed to the
reference emulator:

<sub><code>src/main.c</code></sub>
```c
  if (argc >= 1 && !g_run_without_emu)
    LoadRom(argv[0]);
```

The game itself runs from `zelda3_assets.dat` and doesn't need it, so leave it
off unless you're doing a comparison run.

## Key scripts

`--keys` reads a text file of timed key presses and feeds them in as the game
runs. Each line is one action:

```
wait 90
hold down 30
tap logtiles
hold loganimation 40
```

`wait` passes frames with nothing held. `tap` presses and releases over two
frames. `hold` presses, waits the given number of frames, then releases. A `#`
starts a comment and blank lines are skipped.

`press` and `release` take no time at all, so a pair of them can bracket other
actions. That's the only way to capture an animation that a button starts, since
the logging key has to already be down when the button goes in:

```
press loganimation
wait 2
tap a
wait 45
release loganimation
```

A name is either a joypad button or a command from `[KeyMap]`. The joypad names
are `up`, `down`, `left`, `right`, `select`, `start`, `a`, `b`, `x`, `y`, `l`
and `r`. The commands are `logtiles`, `loganimation`, `gotoscreen`, `cheatlife`,
`cheatequipment`, `walkthroughwalls`, `invincible`, `nomusic`, `listsprites`,
`pause`, `turbo`, `load1` to `load10` and
`save1` to `save10`.

Events go through the same handler a real key press does, so a command that
cares about being held, like `loganimation`, behaves the way it does under your
finger. The script prints its event count on startup.

Once the script runs out the game keeps going normally, which is why
`--quit-after` is usually worth pairing with it.

The script clock is held at zero until everything else asked for on the command
line has happened, so a save load, an intro skip and a jump all finish before the
first action fires. That means frame 1 of a script is the same moment every run,
however long the loading took. `--quit-after` counts real frames, not script
frames, so it still has to cover the loading as well.

## Finding sprites

`--list-sprites` reads the overworld sprite tables and prints what the data
places, without loading a single screen. One line per sprite:

```
  screen 5A type 12 at 04F0,0680  enemy hp=4   bump=5
  screen 5A type 25 at 0520,0680  npc   hp=0   bump=0
  screen 5A type 11 at 04B0,06F0  enemy hp=20  bump=8
```

There are 556 of them across the 128 overworld screens. Shift+E does the same
for the screen you're on, and also lists what has actually spawned.

The category is worked out from the sprite's own data. A sprite whose bump byte is
zero is called an npc, anything else an enemy, and an enemy with 32 or more health
is called a boss. Both of those are guesses. The sprite data has no boss flag, so
the threshold is the best signal available, and the npc test is a proxy too: a
zero bump byte is damage class 0, worth 2/1/1, not nothing. What makes an npc
harmless is that it never runs the contact damage path.

## Dumping the dungeon rooms

`--dump-rooms` walks all 320 dungeon rooms and counts the hazardous tiles each
one draws, writing a `ROOMHAZ` line per room:

```
ROOMHAZ 00E spike=0 pit=0 water=0 ice=168 conveyor=0
```

Room attributes are not stored anywhere. They come out of drawing the room's
objects into a tilemap and converting that through the current tileset, so the
dump does the same three steps the game does when you walk in, per room.

Drawing a room overwrites the overworld map, because `dung_bg2` and
`overworld_tileattr` are the same memory. The dump rebuilds the screen when it
finishes, so the session stays usable.

Feed the output to the room list generator:

```
./zelda3 --no-music --load-save <any save> --dump-rooms --quit-after 400 > /tmp/haz.txt
tools/dungeon_rooms.py --hazards /tmp/haz.txt > dungeon_rooms.md
```

## Quiet runs

`--no-music` starts with the music already off, the same state Shift+M leaves it
in. Worth having on any unattended or scripted run, since a key script can only
mute once the game is up and a run without a script cannot mute at all.

## Warping to a sprite

The three warp parameters do the same search and differ only in the category
they expect:

```
./zelda3 --load-save "saves/ref/Chapter 5 - After Hyrule Castle Tower.sav" --enemy-warp 12
```

```
warp: type 12 (enemy) on screen 42 at 04E0,0190, Link at 04E0,0178
```

Naming the wrong category is a warning, not an error, so a misfiled sprite
still gets you there:

```
warp: type 25 looks like a npc, not a enemy, going anyway
```

Link lands on a safe tile rather than on top of the sprite. Safe means the
game's own tile behaviour says nothing happens when he stands there, so water,
spikes, pits, slopes, ice, ledges and walls are all rejected, and all four
corners of his box have to be clear. Candidates are tried in a ring out from the
sprite, 24 pixels first and 56 at the furthest, in eight directions. If none of
them pass, the warp says so and leaves Link where the search started.

Being near an enemy still means the enemy attacks. Safe is about the ground
under Link, not about being left alone. Pair it with Shift+I if you want to
stand there and watch.

Only overworld sprites are searched. Bosses live in dungeon rooms, which use
different sprite data and a different loader, so `--boss-warp` will usually
report nothing found. It works for a high health sprite that is out on the
overworld.

## --scene

Rebuilds a place built in the pug hero demo's entity sandbox, so a thing can be watched doing
what it does there and here side by side. The sandbox writes a `.scene` file beside every map
it saves. `scenes.md` has the format and what is and is not rebuilt.

```
./zelda3 --load-save "saves/ref/Chapter 2 - After Eastern Palace.sav" --scene my_place.scene
```

It is one of the options that decide where Link ends up. Like the others it waits for the
overworld, so start it from a save that is outdoors; `--skip-intro` starts in Link's house and
never gets there. The file is read before the window opens, so a scene that will not do stops
the run with a line saying why.

## --spot

Puts Link where F2 in the pug hero demo remembered him, as he was: the same place outdoors or
in a room, the same kit, and the same story, rooms and overworld events. The demo writes the
file to `~/.pug_engine/pug_hero_demo_spot.txt`. `spots.md` has the format and how he gets there.

```
./zelda3 --spot ~/.pug_engine/pug_hero_demo_spot.txt
```

The file carries the whole save, so without `--load-save` it starts from a new file on its own,
the way `--skip-intro` does. With one, the savestate is the machine it starts from and the
spot's kit and story go over it. It is one of the options that decide where Link ends up, and
it does not wait for the overworld: from a room it walks out through that room's way out
first. The file is read before the window opens, so one that will not do stops the run with a
line saying why.

## Related

`debug_dumps.md` covers the PNG files that Shift+T and Shift+A write.

`zdrive.md` covers the wrapper that writes the key script for you and gathers up
what a run produced.

`ground_types.md` lists all 256 tile attribute values and which ones Link can
stand on, which is what the warp placement uses.

`dungeon_rooms.md` lists all 320 dungeon rooms with their sprites, chests and
hazards. Regenerate it with `tools/dungeon_rooms.py`.

`dungeon_bosses.md` lists the twelve bosses with their dungeon, room, health and
contact damage. Regenerate it with `tools/dungeon_bosses.py`.

`boss_combat.md` breaks each boss fight down into attacks, movement, sounds,
music and damage.

`enemy_combat.md` does the same for the 110 non-boss enemies, generated by
`tools/enemy_combat.py`.

`objects.md` covers the ancillae, overlords, prize packs and liftable cells,
generated by `tools/object_reference.py`.

`npcs.md` covers the 55 non-enemy sprites, including their dialogue decoded from
the ROM, generated by `tools/npc_reference.py`.

`houses_caverns.md` covers the 103 non-dungeon entrances, their rooms, doors,
music and contents, generated by `tools/houses_caverns.py`.
