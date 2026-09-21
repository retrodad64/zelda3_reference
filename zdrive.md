# zdrive, driving the game from a script

`tools/zdrive.py` runs the game with nobody at the keyboard. You give it a list
of actions, it writes the key script, launches the game with the right flags,
waits for it to finish, and gathers everything the run produced into one
directory.

It exists because working out which tile is which is slow when you have to play
to the spot yourself, press the debug key, and then go hunting for the PNG it
wrote. This turns that into one command you can run again when you want the same
answer twice.

It needs nothing but Python 3 and a built `zelda3` binary.

## Running it

```
tools/zdrive.py --skip-intro --do "wait 90; tap logtiles" --out runs/house
```

```
tools/zdrive.py --save "saves/ref/Chapter 2 - After Eastern Palace.sav" \
                --screen 2C \
                --do "wait 120; tap logtiles; hold right 90; tap logtiles" \
                --out runs/screen2c
```

| Option | What it does |
| --- | --- |
| `--do "..."` | Actions, separated by semicolons |
| `--script <file>` | Actions from a file, one per line |
| `--out <dir>` | Where to collect the run. Without it nothing is gathered |
| `--skip-intro` | Start a new file in the house |
| `--save <file>` | Load a savestate |
| `--screen <hex>` | Jump to an overworld screen, 00 to 7F |
| `--goto [d:]<x>,<y>` | Land on an exact world pixel. `d:` means the dark world |
| `--list-liftables` | Print every liftable cell on the loaded map before the script runs |
| `--music` | Leave the music on |
| `--mortal` | Leave the damage cheat off |
| `--frames <n>` | Override the frame budget |
| `--timeout <s>` | Give up on the game after this long. Default is 120 |
| `--keep` | Copy the dumps instead of moving them out of the repo root |

`--do` and `--script` are the same thing in two forms, so pass one or the other.
With neither, the game just runs and quits.

## Actions

```
wait N          nothing happens for N frames
tap NAME        press and release
hold NAME N     press, wait N frames, release
press NAME      press and take no time
release NAME    release and take no time
```

A `#` starts a comment. The names are the joypad buttons and the commands, and
`command_line.md` lists them all.

`press` and `release` exist so two of them can bracket other actions. An
animation capture needs that, because the logging key has to already be down when
the button that starts the animation goes in:

```
press loganimation
wait 2
tap a
wait 45
release loganimation
```

Two patterns cover most of what you'll want. `tap logtiles` turns the tile log on
and writes a tile PNG, and a second `tap logtiles` turns it off again, which
keeps the log short. `hold loganimation 45` captures 45 frames of Link and
everything beside him, then writes the sheet on release.

## Quiet and unkillable

A scripted run starts with the music off and the damage cheat on, because nobody
is there to turn the volume down or heal Link, and an enemy wandering into him
moves him off the spot you put him on. That last one is worth knowing: a knockback
looks exactly like a jump landing in the wrong place.

`--music` and `--mortal` put either back. `--music` also brings the audio device back,
because without it the device is dummied out and the sound effects go with it. Combat
work wants `--mortal`.

## The frame budget

Every run ends on its own. The budget is the length of the script plus 60 frames,
so a dump triggered by the last action still gets written. `--frames` overrides
it when you want the game to keep running after the script is done.

The script clock waits for the rest of the command line. A save load, an intro
skip and a jump all finish before the first action fires, so frame 1 of a script
is the same moment every run. The budget is in real frames though, loading
included, so leave room for it.

## What you get

With `--out` set, the directory holds:

| File | What it is |
| --- | --- |
| `stdout.txt` | Everything the game printed |
| `tiles.txt` | The tile log, collapsed |
| `debug.log` | The slice `zelda3_debug.log` grew by during this run |
| `run.keys` | The script that was used, so the run can be repeated |
| `zelda3_tiles_NNNN.png` | Tile dumps |
| `zelda3_anim_NNNN.png` | Animation sheets |

`debug_dumps.md` explains the layout of both image kinds.

The PNGs are moved out of the repo root rather than copied, so a run leaves
nothing behind and the numbering starts at 0001 again next time. `--keep` leaves
them where the game wrote them if you'd rather.

## The collapsed tile log

The game logs a line every frame while tile logging is on, and standing still
gives you the same line a hundred times. `tiles.txt` keeps one line per distinct
state, where the state is everything except Link's pixel position. Walking across
a screen goes from 131 lines to 11:

```
[121] tiles module=09/00 indoors=0 floor=0 screen=2C feet=0908,0B10 attr=48 map16=0034
[130] tiles module=09/00 indoors=0 floor=0 screen=2C feet=0910,0B10 attr=00 map16=01ED
[149] tiles module=09/00 indoors=0 floor=2 screen=2C feet=0911,0B28 attr=00 map16=01ED
[159] tiles module=09/00 indoors=0 floor=2 screen=2C feet=0920,0B28 attr=01 map16=0220
```

That reads as the tiles he crossed, in order, with the frame each one started on.
`map16` is the map16 cell under his feet and `attr` is its tile type. Those two
are the numbers worth writing down, because they're stable across runs.

The first twelve lines print to the terminal as well, and the rest are in the
file.

## Big areas, and a jump that lands wrong

Some overworld areas are four screens drawn from one of them. Only that one, the head, holds
the map. Jumping straight onto one of the other three makes the game load data it never draws
there, so the map looks wrong when it is the jump that is wrong. It prints a warning when that
happens; jump to the head and walk in instead.

The tile log names the head, not the quadrant, so walking east out of screen 18 keeps saying
`screen=18` for another whole screen. That is the game, not a bug.

## Finding something to stand next to

`--list-liftables` scans the loaded map for every cell the game will let Link pick
up, and prints each one with its map16 value and world coordinate:

```
liftables: scanning the loaded area around screen 35
  liftable 020F at 0AC0,0C50  2752,3152  stone, pale
  liftable 0036 at 0AE0,0C60  2784,3168  bush
  liftable 0101 at 0AE0,0C80  2784,3200  stone, heavy
```

Feed a coordinate from that straight back into `--goto`, offset so Link stands
beside the cell rather than on it, and the tile log's `front=` field says whether
he's actually lined up. `front=` is the cell the game would act on if A went in
that frame, and it gains a `LIFTABLE` marker when the lift would take it. Getting
that marker before pressing A saves a lot of guessing.

A cell can be liftable and still refuse to come up. `stone, heavy` wants gloves,
and without them the same A press makes Link grab it instead.

## Related

`command_line.md` covers the flags and the key script grammar.

`debug_dumps.md` covers what's in the PNG files.

`controls.md` covers the same debug keys for when you want to drive by hand.
