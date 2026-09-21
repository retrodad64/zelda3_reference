# Dungeon bosses

Twelve boss handlers, in the rooms the sprite tables place them. Everything here is
read out of the game's own data by `tools/dungeon_bosses.py`.

| Boss | Type | Dungeon | Room | Count | HP | Damage | Related sprites |
| --- | --- | --- | --- | --- | --- | --- | --- |
| Giant Moldorm | `09` | Tower of Hera | `007` | 1 | 12 | 8/4/2 | - |
| Giant Moldorm | `09` | Ganon's Tower | `04D` | 1 | 12 | 8/4/2 | - |
| Armos Knight | `53` | Ganon's Tower | `01C` | 6 | 48 | 4/4/4 | `51` Armos Statue (4/4/4) |
| Armos Knight | `53` | Eastern Palace | `0C8` | 6 | 48 | 4/4/4 | `51` Armos Statue (4/4/4) |
| Lanmolas | `54` | Desert Palace | `033` | 3 | 16 | 8/8/8 | - |
| Lanmolas | `54` | Ganon's Tower | `06C` | 3 | 16 | 8/8/8 | - |
| Agahnim | `7A` | Ganon's Tower | `00D` | 1 | 96 | 8/8/8 | `7B` Agahnim Balls (8/8/8), `C1` Cutscene Agahnim (16/8/4) |
| Agahnim | `7A` | Castle Tower | `020` | 1 | 96 | 8/8/8 | `7B` Agahnim Balls (8/8/8), `C1` Cutscene Agahnim (16/8/4) |
| Mothula | `88` | Skull Woods | `029` | 1 | 32 | 16/8/4 | `89` Mothula Beam (16/8/4) |
| Arrghus | `8C` | Swamp Palace | `006` | 1 | 32 | 16/8/4 | - |
| Helmasaur King | `92` | Palace of Darkness | `05A` | 1 | 48 | 16/8/4 | `13` Mini Helmasaur (8/4/2), `70` King Helmasaur Fireball (16/8/4) |
| Kholdstare | `A2` | Ice Palace | `0DE` | 1 | 64 | 32/24/16 | `A3` Kholdstare Shell (16/8/4) |
| Vitreous | `BD` | Misery Mire | `090` | 1 | 128 | 32/24/16 | `BE` Vitreous Eye (32/24/16) |
| Trinexx Rock Head | `CB` | Turtle Rock | `0A4` | 1 | 40 | 32/24/16 | - |
| Blind | `CE` | Thieves Town | `0AC` | 1 | 90 | 16/8/4 | `B7` Blind Maiden (2/1/1) |
| Ganon | `D6` | not on any dungeon map | `000` | 1 | 255 | 64/48/24 | - |

## Reading it

Count is how many of the sprite the room places. Armos Knights is six sprites and
Lanmolas is three, which is why they appear once per room with a count rather than
as separate rows.

Damage is what a touch costs, against green, blue and red mail. The byte in
`kSpriteInit_BumpDamage` is not an amount: its low nibble picks a row of
`kPlayerDamages`, which holds one value per mail. Health is in quarter hearts,
so 8 is a full heart. The figures in brackets after each related sprite are
that sprite's own three.

Related sprites are the ones whose handler name carries the boss's name. Some are
the boss's own projectiles, such as Mothula Beam and King Helmasaur Fireball. Some
are not part of the fight at all: Armos Statue is an ordinary enemy that shares a
name, and Blind Maiden is the follower who turns into Blind. The column is a name
match, not a claim about the fight.

Four bosses appear twice, once in their own dungeon and once in Ganon's Tower.
Ganon himself sits in room `000`, which is not on any dungeon map, because the
Pyramid has no map screen.

HP of 255 means the sprite cannot be damaged by ordinary means and the handler
decides when it may be hurt, which is how Ganon's fight is staged.

`boss_combat.md` goes through each fight in detail.

## What this does not say

How a boss actually attacks. That is behaviour spread through its handler, not a
table, and the spawn calls are written differently enough from one boss to the next
that reading them mechanically gives wrong answers. The Parts column is the honest
version: the sprites named after the boss, which are its projectiles and pieces.
