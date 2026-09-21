# Scenes from the pug sandbox

The pug hero demo has an entity sandbox: Link in a place of your own, with any creature,
person or object the demo knows put down beside him. When it saves a place it writes a
`.scene` file beside the map, which says the same place in the cartridge's own numbers.
`--scene` reads one and builds that place in the real game, so a thing can be compared
between the two.

```
./zelda3 --load-save "saves/ref/Chapter 2 - After Eastern Palace.sav" --scene my_place.scene
```

## What gets built

The scene goes on a big overworld area, the one kind a sandbox place fits in: Kakariko (screen
18) for a light world scene and the village of outcasts (screen 58) for a dark one. The area is
loaded the way `--goto` loads one, for its graphics, its palettes and its music, and then its
map16 grid is written over:

1. Every cell becomes wall, the most common all wall cell the area has.
2. The scene's cells go in one cell in from the area's top left, so there is a ring of wall
   round it. Plain ground is the most common all open cell the area has, which is grass in
   Kakariko. A cell the scene gives as a map16 is that cell.
3. A `wall` cell is walled, but only where it is open ground, so a cliff or a tree the scene's
   own ground already has keeps its look.
4. A `tile` puts a map16 on a cell. That is how bushes, stones, signs and the huge rock arrive,
   so they are the game's own liftable cells and behave like them.

The screen is drawn again from the grid, the area's own sprites are cleared along with its
list, so none of them walk in, and every `sprite` is put down the way the game puts down one
from an area's list. It gets its own setup on the next frame.

Link lands where `link` says, facing the way `face` says, and his kit becomes the scene's: the
items, the bottles and what is in them, the pendants and crystals, his hearts, health, magic,
rupees, bombs, arrows and keys. The sword, shield and mail are drawn again a few frames later,
once the screen's own graphics are up.

## The format

Plain text, one thing a line, a word and then its numbers. Positions are pixels from the
scene's top left corner, and hexadecimal numbers are the cartridge's own.

| Line | Says |
| --- | --- |
| `scene 1` | The format. Anything else is refused. |
| `realm light` | `light` or `dark`. |
| `size 48 40` | Cells across and down. |
| `link 376 297` | The top left of Link's sprite. |
| `face down` | Which way he looks. |
| `hearts 3`, `health 24`, `magic 128` | Containers, health in eighths, the meter. |
| `rupees`, `bombs`, `arrows`, `keys` | What he carries. |
| `kills 1`, `hits 0` | Creatures put down and blows taken since the tongue in the wall last paid, which is what it pays on. |
| `item bow 2` | One line a thing in his kit, by the name the sandbox gives it. |
| `ground 0 - - 034 ...` | A row of cells: `-` for plain ground, else a map16. |
| `wall 3 7` | A cell to wall off where it is open. |
| `tile 12 9 036` | A map16 on a cell. |
| `sprite 08 128 96` | A sprite, where the game would place it. |
| `skip vase 64 80` | Something that cannot be put here, counted and left out. |

## What is not the same

- The game holds sixteen sprites at once. A scene with more puts down the first sixteen and
  says how many it left out.
- A person is set up by the game's own code for wherever he is, and some of them are only set
  up for their own house or room. Put down on the overworld, one may stand somewhere else or
  not appear at all. That is the original doing what it does, which is what the comparison
  is for.
- Vases are room objects, and the overworld has none, so a vase is skipped.
- The sandbox's own dark grey floor is not a cell the cartridge has, so it comes out as the
  area's grass. A map painted from the overworld's cells comes out as those cells.
- The camera stays inside the area, so it frames the scene differently from the sandbox's,
  which stays inside the scene.
