# Controls and keyboard shortcuts

Every binding on this page comes from `zelda3.ini`, in the `[KeyMap]` and
`[GamepadMap]` sections. Edit that file to change any of them. A command the file
doesn't mention still works, because the game falls back to a compiled in
default, which is why a couple of keys below have no line in the ini.

## Playing

The keyboard maps to an SNES joypad. The order in the ini is fixed, and the
comment above it says which slot is which:

<sub><code>zelda3.ini</code></sub>
```ini
# Order: Up, Down, Left, Right, Select, Start, A, B, X, Y, L, R
Controls = Up, Down, Left, Right, Right Shift, Return, x, z, s, a, c, v
```

Reading those two lines together gives:

| Joypad | Key |
| --- | --- |
| Up, Down, Left, Right | Arrow keys |
| Select | Right Shift |
| Start | Return |
| A | x |
| B | z |
| X | s |
| Y | a |
| L | c |
| R | v |

There are commented out lines just below it for QWERTZ and AZERTY keyboards.
Swap which one is active if your layout needs it.

A gamepad works too, and its default puts the SNES buttons where an Xbox style
pad expects them:

<sub><code>zelda3.ini</code></sub>
```ini
Controls = DpadUp, DpadDown, DpadLeft, DpadRight, Back, Start, B, A, Y, X, Lb, Rb
```

So SNES A is the pad's B, and SNES B is the pad's A. Any command name from
`[KeyMap]` can be used in `[GamepadMap]` as well.

## Savestates

Ten slots, each with three keys.

| Keys | What they do |
| --- | --- |
| F1 to F10 | Load slot 1 to 10 |
| Shift+F1 to Shift+F10 | Save slot 1 to 10 |
| Ctrl+F1 to Ctrl+F10 | Replay slot 1 to 10 |

Replay is not the same as load. A savestate file holds the recorded input log as
well as the machine state, so replaying runs the game forward through those
inputs instead of jumping to the end state. Two keys go with that: `k` clears the
key log and `l` stops a replay in progress.

The 13 reference saves under `saves/ref` have no keys by default. There are
commented out `LoadRef` and `ReplayRef` lines in the ini if you want them.

`--load-save` loads any savestate by path from the command line. See
`command_line.md`.

## Window and display

| Key | What it does |
| --- | --- |
| Alt+Return | Fullscreen on and off |
| Ctrl+Up | Bigger window |
| Ctrl+Down | Smaller window |
| f | Frame rate counter on and off |
| r | Switch renderer |
| Shift+= | Volume up |
| Shift+- | Volume down |
| Shift+m | Music off and on |

`f` and `r` have no line in `zelda3.ini`. They come from the compiled defaults
instead, along with Shift+w in the cheat list below:

<sub><code>src/config.c</code></sub>
```c
  // ClearKeyLog, StopReplay, Fullscreen, Reset, Pause, PauseDimmed, Turbo, ReplayTurbo, WindowBigger, WindowSmaller, DisplayPerf, ToggleRenderer
  _(SDLK_k), _(SDLK_l), A(SDLK_RETURN), C(SDLK_r), S(SDLK_p), _(SDLK_p), _(SDLK_TAB), _(SDLK_t), N, N, _(SDLK_f), _(SDLK_r),
```

Add `DisplayPerf` or `ToggleRenderer` to the ini if you want them somewhere else.

Shift+m sends the game's own fade out command, so the music stops the way it
does when a track ends. Sound effects go through different APU ports and carry
on. Pressing it again restores the track that was playing.

The mouse does two things. Ctrl and the scroll wheel resize the window, the same
as Ctrl+Up and Ctrl+Down. Shift and a double click toggle the window border,
which only works when you aren't fullscreen.

## Speed and pausing

| Key | What it does |
| --- | --- |
| Tab | Turbo, while held |
| t | Fast forward during a replay, toggle |
| p | Pause, screen dimmed |
| Shift+p | Pause, screen as is |
| Ctrl+r | Reset |

Tab is the only one of these you hold. The rest are toggles.

## Cheats

| Key | What it does |
| --- | --- |
| w | Refill health and magic |
| Shift+w | Bombs, arrows and 100 rupees |
| o | Give a key |
| Ctrl+e | Walk through walls |
| Shift+i | Invincible, toggle |

Shift+w is the third command with no line in `zelda3.ini`, so it comes from the
compiled default too.

The first four go through the state recorder, so a replay stays consistent with
them. Shift+i does not, for the same reason the screen jump does not.

Shift+i covers two things. Link takes no damage, and nothing knocks him back, so
he keeps his footing when an enemy walks into him. Enemy contact, bombs and
weapon tinks all stop at the same two guards, and pit falls stop costing health
as well. He still falls into the pit, he just doesn't pay for it.

## Debug dumps

| Key | What it does |
| --- | --- |
| Shift+t | Tile logging on and off, and write a tile PNG |
| Shift+a | Log animation while held, write a sheet on release |
| Shift+g | Prompt for a screen number to jump to |
| Shift+e | List the sprites here, live and from the data |
| Shift+l | Copy the current location to the clipboard |

Shift+T and Shift+A write text to stdout and to `zelda3_debug.log`, and they
write PNG files as well. `debug_dumps.md` explains what's in those images.

Shift+E prints two things to stdout: the sprite slots the game has actually spawned right now,
and every sprite the overworld data places on this screen. The second list is
there whether or not anything has spawned, since sprites only wake up when Link
gets near them.

Shift+L puts where you are on the clipboard and writes the same line to the log,
so a spot you found by playing can be pasted straight into `--goto` or a bug
report:

```
light world, screen 1E, coordinate (3920,1579)
dark world, screen 5B, coordinate (2032,1628)
indoors, room 104, coordinate (2424,8568)
```

Indoors covers houses, caverns and dungeons alike, since they are all rooms. The
coordinates are Link's world position, the same numbers `--goto` takes.

Shift+G opens a small box on screen. Type an overworld screen number in hex, 00
to 7F, where 40 and up is the dark world. Enter jumps, Escape cancels, Backspace
deletes. While the box is open your key presses go to it and not to the game.
`--screen-id` does the same jump from the command line.

The jump writes game variables directly rather than going through the state
recorder, unlike the cheats above. It won't show up in a replay, and using it
during one will throw the recording out of step.

## Changing a binding

Put the command name and the key in `[KeyMap]`. Modifiers are written with a
plus, as in the lines already there:

<sub><code>zelda3.ini</code></sub>
```ini
Pause = Shift+p
Reset = Ctrl+r
Fullscreen = Alt+Return
```

An unrecognised key name prints `Unknown key` on startup and that one binding is
dropped. The rest still load.
