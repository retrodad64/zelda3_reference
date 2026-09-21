# Boss combat

One section per boss, covering what it does, how it moves, what it plays, and
what hurts it. `dungeon_bosses.md` is the summary table; this is the detail
behind it.

Everything here comes from the boss handlers in `src/sprite_main.c` and the
sprite tables in `src/sprite.c`. Where a claim rests on a function name, the name
is quoted, because the names in this reimplementation are descriptive and they
are the best evidence available for behaviour that is otherwise spread across a
few thousand lines of state machine.

## How to read the sound and damage numbers

Sound effects are queued on one of three channels, and the number is an effect
id, not a description. Channel 1 and 2 reach the APU as ports 2 and 3:

<sub><code>src/nmi.c</code></sub>
```c
  zelda_apu_write(APUI02, sound_effect_1);
  zelda_apu_write(APUI03, sound_effect_2);
```

HP is `kSpriteInit_Health`. HP of 255 means ordinary hits do nothing and the
handler decides when the boss can be hurt.

Damage is what one touch costs, given against green, blue and red mail. The byte
in `kSpriteInit_BumpDamage` is not an amount. Its low nibble picks a row of
`kPlayerDamages`, which holds one value per mail:

<sub><code>src/sprite.c</code></sub>
```c
  link_give_damage = kPlayerDamages[3 * (sprite_bump_damage[k] & 0xf) + link_armor];
```

Health is in quarter hearts, so 8 is one full heart and Ganon's 64 is eight.

## How to read the timings

Every duration is a frame count written into one of the sprite timers, and each
timer drops by one per frame:

<sub><code>src/sprite.c</code></sub>
```c
  if (!(submodule_index | flag_unk1)) {
    if (sprite_delay_main[k])
      sprite_delay_main[k]--;
    if (sprite_delay_aux1[k])
      sprite_delay_aux1[k]--;
```

That guard matters. Timers only run while the game is in normal play, so a phase
does not advance during a transition, a menu or a text box.

A frame is 32000 divided by 534, about 59.93 per second, which is what the audio
block size pins it to:

<sub><code>src/main.c</code></sub>
```c
    g_frames_per_block = (534 * have.freq) / 32000;
```

Seconds below are frames over 60, so they run about a tenth of a percent short.
A state with no timer runs until something else ends it, usually a position
check or a health threshold, and those are noted rather than given a duration.

## Giant Moldorm

Type `09`, Tower of Hera room `007` and Ganon's Tower room `04D`. HP 12, touch
19.

The fight is a segmented worm. Drawing is split across
`GiantMoldorm_DrawSegment_AB`, `GiantMoldorm_DrawSegment_C_OrTail` and
`SpriteDraw_Moldorm_Head`, and the body is followed by `Moldorm_HandleTail`.
There is a separate `SpriteDraw_Moldorm_Eyeballs`, which is the part that comes
off as it is hurt.

It has no projectile and no spawn calls. The only attack is contact, which is why
damage 8/4/2 does the work, and the danger is the room's ledges rather than anything
Moldorm fires.

Movement is four velocity writes, so the body drives from a single heading rather
than per-segment steering. Death runs
`GiantMoldorm_IncrementalSegmentExplosion`, one segment at a time.

Timing: three states, `0` straight path, `1` spinning meander and `2` lunge at
player. Only the lunge is timed, at 48 frames or 0.80s, after which it returns to
state 0. The other two end on position, not on a clock. The tail carries its own
48 and 64 frame timers.

Conditions: the two untimed states end on a counter, not a clock. State 0 leaves
when its delay expires and picks the next state from `sprite_G`, which cycles
every third pass:

<sub><code>src/sprite_main.c</code></sub>
```c
      if (++sprite_G[k] == 3)
```

Moldorm also has a speed gate on its own health, and it is the only boss with
one:

```c
  bool low_health = (sprite_health[k] < 3);
  sprite_subtype2[k] += low_health ? 2 : 1;
  if (!(frame_counter & (low_health ? 3 : 7)))
```

Below 3 HP of its starting 12 the animation advances twice as fast and it acts
every 4 frames instead of every 8, so the last quarter of the fight runs at
double rate.

Sounds: `21` on channel 2 and `31` on channel 3.

## Armos Knight

Type `53`, Eastern Palace room `0C8` and Ganon's Tower room `01C`. Six sprites
per room. HP 48 each, damage 4/4/4.

Six knights move as a group, six velocity writes for the set. The only spawn is
`EA`, an unnamed garnish sprite used for the landing effect.

Do not confuse it with `51` Armos Statue, an ordinary enemy at 4/4/4 that
shares the name and has its own `Armos_Draw`.

Timing: no state machine. The knights run off `sprite_delay_main` alone, set to
255 frames or 4.25s, with the animation frame read straight out of the counter:

<sub><code>src/sprite_main.c</code></sub>
```c
      sprite_graphics[k] = kArmosKnight_Gfx1[sprite_delay_main[k] >> 3];
```

Shifting by 3 means one graphic every 8 frames, so a 255 frame cycle steps
through 32 entries.

Sounds: `35` assigned directly, and `16` queued on channel 3, which is the
hop landing.

## Lanmolas

Type `54`, Desert Palace room `033` and Ganon's Tower room `06C`. Three sprites
per room. HP 16 each, damage 8/8/8.

The attack is thrown rock. `Lanmola_SpawnShrapnel` is the routine, and the spawn
list confirms what it throws:

```
spawns: 0x00, 0xC2 Boulder, 0xEA
```

Movement is burrowing, eight velocity writes across three worms, each surfacing
and diving rather than tracking Link continuously.

Timing: a five state loop, `0` to `4` and back to `0`. State 0 waits 127 frames
or 2.12s before starting, state 1 runs an auxiliary 74 frame or 1.23s timer, and
state 3 waits 128 frames or 2.13s. So one full surface and dive cycle is a little
over four seconds of timed waiting plus the untimed travel in states 2 and 4.
State 5 is the death throw, 31 frames or 0.52s.

Conditions: the untimed travel ends on position. A worm has arrived when it is
within 2 pixels of its target in both axes, and the dive completes when its
height goes negative:

```c
      if ((uint16)(x - x2 + 2) < 4 && (uint16)(y - y2 + 2) < 4)
```

```c
      if (sign8(sprite_z[k])) {
```

Sounds: `35` and `0c` on channel 2, the surface and the dive.

## Agahnim

Type `7A`, Castle Tower room `020` and Ganon's Tower room `00D`. HP 96, damage 8/8/8.

The attack is `Agahnim_PerformAttack`, which spawns `7B` Agahnim Balls at 8/8/8.
This is the fight you are meant to win with the sword rather than by hitting the
boss, and the ball sprite carries the damage rather than Agahnim himself.

He also has `SpritePrep_AgahnimsBarrier`, the wall that closes the arena.

Twenty velocity writes, the most of any boss except Ganon and Blind, because he
teleports around the room rather than walking.

Music: `music_control = 0x1d` when the fight starts. He is one of only two bosses
that changes the track.

Timing: eleven states, the second largest machine in the game. The attack cadence
is short, 32 frames or 0.53s in state 1 and state 7 before returning to state 3,
and 39 frames or 0.65s in states 3 and 5. State 6 is the long one at 80 frames or
1.33s. State 2 sets 255 frames, 4.25s, the timer used while he waits rather than
attacks. The shortest value he uses is 15 frames, a quarter of a second.

Conditions: the teleport lands on position rather than on a timer. He has arrived
when he is within 7 pixels of the target column:

```c
      if ((uint16)(sprite_x_lo[k] - sprite_C[k] + 7) < 14 &&
```

An angle band picks the attack direction, taking the branch when the value is at
either end of the circle:

```c
      if (j >= 239 || j < 16) {
```

State 2 also splits on its own timer at the halfway mark, 128 of its 255 frames,
and there is a counter gate at 64 in `sprite_G`.

Sounds: `28` assigned, plus `04` and `05` on channel 2 and `26`, `27`, `29` and
`36` on channel 3, the widest sound set of any boss.

## Mothula

Type `88`, Skull Woods room `029`. HP 32, damage 16/8/4.

Two attacks. `Mothula_SpawnBeams` fires `89` Mothula Beam at 16/8/4, and
`Mothula_HandleSpikes` drives `8A` Spike Block, the moving spikes on the floor.
`Mothula_FlapWings` is the movement, a drift rather than a charge, over six
velocity writes.

Damage is the interesting part. The handler patches the damage table on entry
when the bug fix feature is on:

<sub><code>src/sprite_main.c</code></sub>
```c
  if (enhanced_features0 & kFeatures0_MiscBugFixes) {
    // L4 sword and L3 spin slash can now damage Mothula
    enemy_damage_data[0x884] = 1;
    enemy_damage_data[0x885] = 1;
  }
```

So in the original the best sword does nothing to it, and this build fixes that
behind a flag.

Timing: four named states in order, `0` Delay, `1` Ascend, `2` FlyAbout, `3`
FireBeams. Ascend ends on height rather than time, and sets 64 frames or 1.07s on
the way out. FlyAbout counts down a separate 128 tick counter in `sprite_G`, and
when that expires it sets 63 frames or 1.05s and moves to FireBeams. So beams
come roughly every two and a bit seconds of flying. The 32 frame auxiliary timer
is the invulnerability window after a hit.

Conditions: Ascend ends at a fixed height rather than on its timer:

```c
    if (sprite_z[k] >= 24) {
```

Sounds: `02` and `36` on channel 3.

## Arrghus

Type `8C`, Swamp Palace room `006`. HP 32, damage 16/8/4.

`Arrghus_HandlePuffs` is the whole fight: the boss is shielded by orbiting puffs
that must be pulled off before the centre can be hurt. There are no spawn calls,
so the puffs are handled inside the boss rather than being separate sprites.

Seven velocity writes, for the centre plus the puff ring.

Timing: six states. State 0 approaches for 48 frames or 0.80s, state 1
decelerates for 176 frames or 2.93s
and can branch three ways, and state 2 runs 112 frames or 1.87s before going back
to state 1. The jump sequence is states 3 and 4, 64 then 32 frames, 1.07s then
0.53s, into state 5 where it swims frantically with no timer at all.

Conditions: the jump peaks at height 224, and state 4 ends when the height sign
flips, meaning it has come back down:

```c
    if (sprite_z[k] >= 224) {
```

The puffs orbit on two counters, one advancing every 4 frames and one every 8,
both wrapping at 13:

```c
  if (!(frame_counter & 3) && ++sprite_A[k] == 13)
```

So the inner ring completes a turn in 52 frames or 0.87s and the outer in 104
frames or 1.73s.

State 2 also splits on its own timer, taking one branch below 32 frames
remaining and another below 96.

Sounds: six ids, `20` and `28` on channel 2 and `03`, `06`, `26` and `32` on
channel 3, which fits a fight with many small parts reacting.

## Helmasaur King

Type `92`, Palace of Darkness room `05A`. HP 48, damage 16/8/4.

Two attacks, both named. `HelmasaurKing_SpitFireball` spawns `70` King Helmasaur
Fireball at 16/8/4, with `HelmasaurFireball_TriSplit` and
`HelmasaurFireball_QuadSplit` breaking one fireball into three or four.
`HelmasaurKing_SwingTail` and `KingHelmasaur_OperateTail` are the close range
attack.

Damage runs in two stages. The mask has to come off first, and only two things
take it off:

<sub><code>src/sprite_main.c</code></sub>
```c
void HelmasaurKing_CheckMaskDamageFromHammer(int k) {  // 9e8385
  if (sprite_C[k] >= 3 || !(link_item_in_hand & 10) || (player_oam_y_offset == 0x80))
    return;
```

That `link_item_in_hand & 10` is the hammer test. `KingHelmasaur_CheckBombDamage`
is the other route. `HelmasaurKing_ChipAwayAtMask` tracks the damage,
`HelmasaurKing_ExplodeMask` finishes it and `HelmasaurKing_SpawnMaskDebris`
throws the pieces. Only then does `HelmasaurKing_AttemptDamage` apply.

Movement is `HelmasaurKing_HandleMovement`, sixteen velocity writes.

Timing: two machines. The boss itself is a four state ring, 64, 32, 64 and 64
frames, so 1.07s, 0.53s, 1.07s and 1.07s, returning to state 0. A complete cycle
is about three and three quarter seconds.

The fireball has its own five state machine with the tightest timings in the
fight: state 0 waits 18 frames or 0.30s before migrating down, state 1 runs 31
frames or 0.52s, then states 2 and 3 are the tri split and quad split delays. The
12 frame auxiliary timer, a fifth of a second, is the gap between split
fragments.

Conditions: the mask is a counter, not health. It stops taking damage once
`sprite_C` reaches 3, which is the first test in the hammer check:

```c
  if (sprite_C[k] >= 3 || !(link_item_in_hand & 10) || (player_oam_y_offset == 0x80))
```

State 0 watches the same counter to know the mask is gone, and state 2 gates on
`sprite_E` reaching 3.

Sounds: `21`, `26` and `05` assigned, `1f` and `05` on channel 2, `06` and `36`
on channel 3.

## Kholdstare

Type `A2`, Ice Palace room `0DE`. HP 64, damage 32/24/16.

The shell is a separate sprite, `A3` Kholdstare Shell, with damage 16/8/4, and it has
its own prep in `SpritePrep_KholdstareShell`. The ice has to be melted before the
boss inside is reachable, which is why the shell carries almost as much contact
damage as the boss.

`Kholdstare_SpawnPuffCloudGarnish` is the visual on breaking through. Sixteen
velocity writes, for the boss plus the drifting pieces.

Timing: three states, `0` Accelerate and `1` Decelerate alternating with no
timer, ending on speed rather than on a clock, plus `2` Triplicate at 32 frames
or 0.53s. The long timer is auxiliary, 192 frames or 3.20s, the shell phase
before the boss inside is exposed.

Conditions: both untimed states end on speed. Accelerate runs until the velocity
reaches a target parked in the z fields, and Decelerate runs until both axes
reach zero:

```c
    if (sprite_x_vel[k])
    if (sprite_y_vel[k])
```

So the cycle length depends on how far it has to speed up and slow down, not on a
timer.

Sounds: only one, `02`, assigned directly. The quietest boss in the game.

## Vitreous

Type `BD`, Misery Mire room `090`. HP 128, the highest of any boss that can be
hurt normally, second only to Ganon's 255, damage 32/24/16.

`Vitreous_SpawnSmallerEyes` and `Vitreous_SetMinionsForth` are the fight: the
main body sends the small eyes out, and `BE` Vitreous Eye carries damage 32/24/16, the
same as the parent. There is no projectile beyond the eyes themselves.

Five velocity writes, second only to Moldorm's four, because the body barely
moves and the minions do the travelling.

Timing: three states, `0` dormant, `1` spew lighting and `2` pursue player.
Dormant is where the cadence lives: it sets either 128 frames or 64 frames, 2.13s
or 1.07s, and branches to state 1 or state 2 accordingly, so the gap between
lightning and a charge alternates between roughly one and two seconds. The 16
frame auxiliary timer is the wind up before the lightning lands.

The eyes run their own three state loop, target then pursue then return, with no
timers at all. They are driven entirely by distance.

Conditions: the eyes are pure distance. Pursue ends when the eye is within 4
pixels of its target in both axes, and return ends the same way against its home
position:

```c
      if ((uint8)(sprite_G[k] - sprite_x_lo[k] + 4) < 8 && (uint8)(sprite_anim_clock[k] - sprite_y_lo[
```

That is why they have no timers at all.

Sounds: `35` assigned and `21` on channel 2.

## Trinexx

Type `CB` for the rock head, Turtle Rock room `0A4`. HP 40, damage 32/24/16.

Fourteen functions for one fight. `TrinexxComponents_Initialize` sets up the
separate heads, `Trinexx_CachePosition` and `Trinexx_RestoreXY` keep the body
segments consistent, and `Trinexx_HandleShellCollision` handles the shell.
`Trinexx_WagTail` is the close range attack.

Damage is staged like Helmasaur's but differently:
`Sprite_Trinexx_CheckDamageToFlashingSegment` means only the flashing part of the
body can be hurt at any moment, and `Sprite_Trinexx_FinalPhase` changes the rules
once the heads are gone. `Sprite_TrinexxFire_AddFireGarnish` is the fire breath
effect.

Note the type ids `CC` and `CD` are not the other two heads. Those slots hold
unrelated handlers, so the heads are driven from the rock head's own code.

Timing: a four state ring for the head, 80 frames into states 1 or 2, then 48, 64
and 48 frames back to 0. That is 1.33s, 0.80s, 1.07s and 0.80s. The final phase
is separate, `Sprite_Trinexx_FinalPhase`, which opens with 192 frames or 3.20s
and an 8 frame auxiliary tick, the fastest repeating timer of any boss at 0.13s,
the rate the body segments update once the heads are gone.

Conditions: state 1 ends either on its timer or on Link's position, and there is a
position match against an overlord within 2 pixels:

```c
      if (sprite_subtype[k] == 0xff && (sprite_delay_main[k] == 0 || Sprite_IsBelowLink(k).a == 0)) {
```

Sounds: `22` and `26` assigned, `21`, `2a` and `0c` on channel 2, `31` on
channel 3.

## Blind

Type `CE`, Thieves Town room `0AC`. HP 90, damage 16/8/4.

Three attacks, all named. `Blind_FireballFlurry` is the ranged attack,
`Blind_SpawnLaser` fires the beam with `Sprite_BlindLaser` and
`BlindLaser_SpawnTrailGarnish` drawing its trail, and `Blind_SpawnHead` throws
the heads that keep attacking after they detach, run by `Sprite_Blind_Head`.

Movement has the most detail of any boss, twenty-nine velocity writes, with
`Blind_Decelerate_X` and `Blind_Decelerate_Y` giving it momentum rather than
instant direction changes. `Blind_Animate` and `Blind_AnimateRobes` are separate,
so the robes trail the body.

`Blind_CheckBumpDamage` is its own contact check rather than the shared one.

The related sprite `B7` Blind Maiden, damage class 0, is the follower who becomes the
boss, handled in `SpritePrep_BlindMaiden` and `SpritePrep_Blind_PrepareBattle`.
It is not part of the fight.

Timing: Blind does not use `sprite_ai_state` at all. The machine switches on
`sprite_C`, with eight named states: `0` blinded, `1` retreat to back wall, `2`,
`3` switch walls, `4`, `5` fireball reprisal, `6` behind the curtain and `7`
rerobe.

The long waits are the retreat and the reprisal, 255 and 254 frames, 4.25s and
4.23s. 255 is the ceiling for these timers, and five of the twelve bosses use it.
States 4 and 7 both drop back to state 2, making that the hub. The auxiliary timers carry the rest: 96
frames or 1.60s while blinded, 48 frames or 0.80s in state 2, and 39 frames or
0.65s behind the curtain.

Conditions: Blind is the only boss that steers by lookup table. States 2 and 3
run until the velocity and position match entries in
`kBlind_Oscillate_YVelTarget`, `kBlind_Oscillate_XPosTarget` and
`kBlind_SwitchWall_YPosTarget`:

```c
      if ((sprite_x_lo[k] & ~1) == kBlind_Oscillate_XPosTarget[j])
```

The auxiliary timer also acts as a phase marker rather than just a countdown:
state 1 changes behaviour below 64 remaining and state 6 above 224.

Sounds: `26` assigned, `13` on channel 1 and `22` on channel 3. It is the only
boss that uses channel 1.

## Ganon

Type `D6`, room `000`, which is not on any dungeon map because the Pyramid has no
map screen. HP 255, damage 64/48/24, damage class 9, the hardest hitting contact
in the game. Nothing else reaches that class, and even in red mail it costs three
hearts.

HP 255 means ordinary hits never land. `Ganon_EnableInvincibility` is explicit
about it, and the phases decide when he is open.

Four attacks, all named. `Ganon_SpawnSpiralBat` and `Ganon_HandleFireBatCircle`
are the fire bats, `Ganon_SpawnFallingTilesOverlord` drops the ceiling, and
`Sprite_GanonTrident` with `Ganon_Phase1_AnimateTridentSpin` is the thrown
trident. `Ganon_SelectWarpLocation` is how he moves: he warps rather than walks,
across twenty-five velocity writes.

`Sprite_SpawnPhantomGanon` and `Sprite_PhantomGanon` are the decoys, with
`PhantomGanon_Draw` separate from `Ganon_Draw`.

Music: the only boss with two changes, `music_control = 0x1e` when the fight
begins and `0x1f` at a later phase.

Timing: nineteen cases, by far the largest machine in the game, and the only one
where transitions are gated on health rather than on a timer. Several cases open
with a health test, so the fight advances when you have done enough damage, not
when a clock runs out.

The timed parts are the attack cadence. State 0 opens with 128 frames or 2.13s,
state 1 sets 112 frames or 1.87s, state 3 waits 127 frames or 2.12s before state
6, and state 8 waits 127 frames again. The 255 frame timer, 4.25s, appears in
states 6, 7 and 11, the invulnerable stretches, matching
`Ganon_EnableInvincibility`.

The fast values belong to the bat and tile attacks: 16, 24, 32 and 40 frames,
0.27s to 0.67s. A 224 frame auxiliary timer, 3.73s, runs alongside state 16, the
longest auxiliary of any boss except Blind's 255.

Conditions: this is the fight where the numbers matter most, because the phases
are health gates rather than timers. Ganon starts at 255 and the tests are 209,
161 and 97:

```c
    if (sprite_health[k] < 209)
```

```c
      } else if (sprite_health[k] >= 161) {
      } else if (sprite_health[k] >= 97) {
```

So the fight is four bands: 255 down to 209, 209 to 161, 161 to 97, and 97 down.
The first band is 46 points, the second 48, the third 64.

There is also a reset. State 13 puts health back to 100 before falling through to
state 5:

```c
  case 13:  //
    sprite_health[k] = 100;
    // fall through
```

State 12 has two distance gates, at 96 and 72, which decide how close he has to
be before the next move.

Sounds: `2a` and `0c` on channel 2, `1e` and `28` on channel 3.

## What this does not cover

The sound effect ids are ids. Nothing here says what any of them sound like.

Armos Knight is the one boss with neither a state machine nor a condition gate.
It runs on a single counter and a graphics table, both already in its section.
