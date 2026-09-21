# Ground types

Every tile carries a one byte attribute saying what happens when Link stands on
it. All 256 values are accounted for, spread across 54 behaviours. The table
below is generated from the switch in `src/tile_detect.c`, so it is the list the
game actually uses.

The last column is what the warp placement in `src/debug_locate.c` calls safe: a
tile Link can stand on, move off, and take no damage from. `outdoors` means the
behaviour splits, being ordinary ground on the overworld and a wall inside.

## Reading an attribute

The attribute for a point comes out of the map16 buffer. Outdoors it is a
lookup, and the x is in eight pixel units while the y is not:

<sub><code>src/sprite.c</code></sub>
```c
    tiletype = Overworld_GetTileAttributeAtLocation(*x >>= 3, y);
```

Indoors the same byte comes straight from the room's attribute table:

```c
    int t = (floor >= 1) ? 0x1000 : 0;
    t += (*x & 0x1f8) >> 3;
    t += (y & 0x1f8) << 3;
    tiletype = dung_bg2_attr_table[t];
```

Shift+T writes a three by three grid of these around Link's feet into
`zelda3_debug.log`, which is the quickest way to see what he is standing on.

## The table

| Value | Behaviour | What it is | Link can stand |
| --- | --- | --- | --- |
| `00`, `05` to `07`, `14` to `17`, `21`, `23` to `25`, `38` to `3C`, `41`, `45`, `47`, `49`, `5E` to `5F`, `61` to `62`, `64` to `66`, `A6` to `A7`, `BE` to `BF`, `D0` to `EF` | `TileBehavior_NothingOW` | Plain ground. Nothing happens. | yes |
| `01` to `03`, `26`, `43` | `TileBehavior_StandardCollision` | Wall. Link is stopped. | no |
| `6C` to `6F` | `TileBehavior_NormalOrWall` | Plain ground outdoors, wall indoors. | outdoors |
| `04` | `TileBehavior_ThickGrassOrWall` | Thick grass outdoors, wall indoors. | outdoors |
| `0B` | `TileBehavior_DeepWaterOrWall` | Deep water outdoors, wall indoors. | no |
| `08` | `TileBehavior_DeepWater` | Deep water. Link swims or drowns. | no |
| `09` | `TileBehavior_ShallowWater` | Shallow water. Link wades. | no |
| `0A` | `TileBehavior_ShortWaterLadder` | Ledge into water. | no |
| `0C` | `TileBehavior_OverlayMask_0C` | Overlay mask. | no |
| `0D` | `TileBehavior_SpikeFloor` | Spike floor. Damages Link. | no |
| `0E` | `TileBehavior_GanonIce` | Ice. Link slides. | no |
| `0F` | `TileBehavior_PalaceIce` | Ice. Link slides. | no |
| `10` to `13` | `TileBehavior_Slope` | Slope. Link is pushed along it. | no |
| `18` to `1B` | `TileBehavior_SlopeOuter` | Outer corner of a slope. | no |
| `1C` | `TileBehavior_OverlayMask_1C` | Overlay mask. | no |
| `1D` | `TileBehavior_NorthSingleLayerStairs` | Stairs, single layer. | no |
| `1E` to `1F` | `TileBehavior_NorthSwapLayerStairs` | Stairs that swap layer. | no |
| `20`, `B0` to `BD` | `TileBehavior_Pit` | Pit. Link falls and loses health. | no |
| `22`, `30` to `37` | `TileHandlerIndoor_22` | Indoor stairs and floor changes. | no |
| `27` | `TileBehavior_Hookshottables` | Hookshot can grab it. | no |
| `28` | `TileBehavior_Ledge_North` | Ledge, jump down north. | no |
| `29` | `TileBehavior_Ledge_South` | Ledge, jump down south. | no |
| `2A` to `2B` | `TileBehavior_Ledge_EastWest` | Ledge, jump down east or west. | no |
| `2C`, `2E` | `TileBehavior_Ledge_NorthDiagonal` | Diagonal ledge, north. | no |
| `2D`, `2F` | `TileBehavior_Ledge_SouthDiagonal` | Diagonal ledge, south. | no |
| `3D` to `3F` | `TileHandlerIndoor_3E` | Indoor handler. | no |
| `40` | `TileBehavior_ThickGrass` | Thick grass. Link walks through it. | yes |
| `44` | `TileBehavior_Spike` | Spike. Damages Link. | no |
| `46` | `TileBehavior_HylianPlaque` | Readable plaque. | no |
| `48`, `4A` | `TileBehavior_DiggableGround` | Ground the shovel works on. Plain underfoot. | yes |
| `4B` | `TileBehavior_Warp` | Warp tile. | no |
| `50` to `56` | `TileBehavior_Liftable` | Rock or pot Link can lift. | no |
| `57` | `TileBehavior_BonkRocks` | Rocks broken by dashing. | no |
| `58` to `5D` | `TileBehavior_Chest` | Chest. | no |
| `60` | `TileBehavior_RupeeTile` | Rupee under the ground. | no |
| `63` | `TileBehavior_MinigameChest` | Minigame chest. | no |
| `67` | `TileBehavior_CrystalPeg_Up` | Crystal peg, raised. | no |
| `68` | `TileBehavior_Conveyor_Upwards` | Conveyor, north. | no |
| `69` | `TileBehavior_Conveyor_Downwards` | Conveyor, south. | no |
| `6A` | `TileBehavior_Conveyor_Leftwards` | Conveyor, west. | no |
| `6B` | `TileBehavior_Conveyor_Rightwards` | Conveyor, east. | no |
| `70` to `7F` | `TileBehavior_ManipulablyReplaced` | Tile replaced by an event. | no |
| `80` to `81`, `84` to `8D` | `TileHandlerIndoor_80` | Indoor handler. | no |
| `82` to `83` | `TileHandlerIndoor_82` | Indoor handler. | no |
| `8E` to `8F` | `TileBehavior_Entrance` | Entrance. | no |
| `90` to `97` | `TileBehavior_LayerToggleShutterDoor` | Shutter door, layer toggle. | no |
| `98` to `9F`, `A8` to `AF` | `TileBehavior_LayerAndDungeonToggleShutterDoor` | Shutter door, layer and dungeon toggle. | no |
| `A0` to `A1`, `A4` to `A5` | `TileBehavior_DungeonToggleManualDoor` | Manual door. | no |
| `A2` to `A3` | `TileBehavior_DungeonToggleShutterDoor` | Shutter door. | no |
| `C0` to `CF` | `TileBehavior_LightableTorch` | Torch that can be lit. | no |
| `F0` to `FF` | `TileBehavior_FlaggableDoor` | Door with a flag. | no |
| `42` | `TileBehavior_GraveStone` | Gravestone. Can be pulled. | no |
| `4C` to `4D` | `TileBehavior_UnusedCornerType` | Unused corner. | no |
| `4E` to `4F` | `TileBehavior_EasternRuinsCorner` | Eastern ruins corner. | no |

## Notes

Three behaviours mean different things indoors and outdoors, and the names above
are mine, since the source has no comment on those cases. They are the blocks at
`0x04`, `0x0b` and `0x6c` to `0x6f`, each of which tests `is_indoors` and treats
the tile as a wall when it is set.

Two behaviours damage Link directly: `TileBehavior_SpikeFloor` at `0x0D` and
`TileBehavior_Spike` at `0x44`. `TileBehavior_Pit` costs health through the fall
rather than the tile itself, which is why Shift+I guards the pit damage
separately from the enemy damage.

`TileBehavior_DiggableGround` at `0x48` and `0x4A` is ordinary ground. It is easy
to miss because the name suggests something special, and leaving it out of the
safe set was what made the first version of the sprite warp fail to place Link
anywhere.
