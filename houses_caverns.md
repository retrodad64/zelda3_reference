# Houses and caverns

103 of the 133 entrances lead somewhere that is not a dungeon.
That is the test used here: the entrance's palace byte is -1. Dungeon rooms are
in `dungeon_rooms.md`.

An entrance is the door itself. The room is what is behind it, and several
entrances can share a room.

## Summary

| Entrance | Room | Screen | Music | Tileset | Doors | Sprites | Chests | Dark |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `00` | `104` | - | `FF` | `03` | 2 | 1 | 1 | yes |
| `01` | `104` | `2C` | `07` | `03` | 2 | 1 | 1 | yes |
| `06` | `0F0` | `0A` | `12` | `06` | 2 | 10 | - | yes |
| `07` | `0F1` | `03` | `12` | `06` | 2 | 10 | - | yes |
| `0D` | `0F2` | `18` | `F2` | `03` | 3 | - | - | - |
| `0E` | `0F3` | `18` | `F2` | `03` | 3 | 1 | - | - |
| `0F` | `0F4` | `28` | `F2` | `03` | 3 | 1 | - | - |
| `10` | `0F5` | `29` | `F2` | `03` | 3 | 1 | - | - |
| `11` | `0E3` | `22` | `18` | `06` | 3 | 1 | - | - |
| `12` | `0E2` | `02` | `1B` | `06` | 2 | 5 | - | - |
| `13` | `0F8` | `45` | `12` | `06` | 1 | - | 2 | - |
| `14` | `0E8` | `45` | `12` | `06` | 1 | 4 | - | - |
| `16` | `0FB` | `4A` | `12` | `06` | 1 | 3 | - | - |
| `17` | `0EB` | `4A` | `12` | `06` | 1 | 1 | - | - |
| `1A` | `0FD` | `05` | `12` | `06` | 1 | 5 | - | - |
| `1B` | `0ED` | `05` | `12` | `06` | 1 | - | - | - |
| `1C` | `0FE` | `05` | `12` | `06` | 1 | 5 | 1 | - |
| `1D` | `0EE` | `05` | `12` | `06` | 1 | 5 | - | - |
| `1E` | `0FF` | `05` | `12` | `06` | 3 | 1 | 2 | - |
| `1F` | `0EF` | `05` | `12` | `06` | 2 | 4 | 5 | - |
| `20` | `0DF` | `05` | `12` | `06` | 1 | 2 | - | - |
| `21` | `0F9` | `03` | `12` | `06` | 1 | 4 | - | - |
| `22` | `0FA` | `03` | `12` | `06` | 1 | 3 | - | - |
| `23` | `0EA` | `03` | `12` | `06` | 1 | 1 | - | - |
| `2C` | `0E1` | `00` | `18` | `06` | 2 | 2 | - | - |
| `2E` | `0E6` | `0A` | `12` | `06` | 2 | 5 | - | yes |
| `2F` | `0E7` | `03` | `12` | `06` | 2 | 7 | - | yes |
| `30` | `0E4` | `03` | `12` | `14` | 3 | 4 | - | yes |
| `31` | `0E5` | `03` | `12` | `14` | 2 | 6 | - | yes |
| `32` | `055` | `1B` | `03` | `01` | 3 | 3 | 1 | yes |
| `36` | `010` | `5B` | `1C` | `13` | 2 | - | - | - |
| `38` | `008` | `15` | `1B` | `06` | 4 | 1 | - | - |
| `39` | `02F` | `18` | `12` | `06` | 2 | - | 5 | - |
| `3A` | `03C` | `45` | `12` | `06` | 2 | 3 | 4 | - |
| `3B` | `02C` | `45` | `12` | `06` | 5 | 4 | - | - |
| `3C` | `100` | `00` | `0E` | `03` | 2 | 1 | - | - |
| `3D` | `11E` | `74` | `12` | `06` | 3 | 5 | 4 | - |
| `3E` | `101` | `18` | `F2` | `03` | 4 | 1 | - | - |
| `3F` | `101` | `18` | `F2` | `03` | 4 | 1 | - | - |
| `40` | `102` | `18` | `F2` | `03` | 2 | 1 | - | - |
| `41` | `117` | `43` | `12` | `14` | 1 | - | 1 | - |
| `42` | `103` | `18` | `F2` | `03` | 6 | 3 | 1 | - |
| `43` | `103` | `18` | `F2` | `03` | 6 | 3 | 1 | - |
| `44` | `103` | `18` | `F2` | `03` | 6 | 3 | 1 | - |
| `45` | `105` | `1E` | `18` | `0F` | 3 | 1 | 3 | - |
| `46` | `11F` | `18` | `17` | `03` | 4 | 1 | - | - |
| `47` | `106` | `58` | `0E` | `03` | 4 | 1 | 1 | - |
| `48` | `106` | `58` | `F2` | `03` | 4 | 1 | 1 | - |
| `49` | `107` | `29` | `F2` | `03` | 4 | 3 | - | - |
| `4A` | `107` | `18` | `F2` | `03` | 4 | 3 | - | - |
| `4B` | `108` | `18` | `F2` | `03` | 3 | 4 | 1 | - |
| `4C` | `109` | `16` | `F2` | `03` | 2 | 1 | - | yes |
| `4D` | `10A` | `30` `30` | `18` | `06` | 3 | 1 | 1 | yes |
| `4E` | `10B` | `3B` | `18` | `08` | 3 | 3 | 1 | - |
| `4F` | `10C` | `05` | `12` | `06` | 3 | 8 | 1 | - |
| `50` | `10C` | `05` | `12` | `06` | 3 | 8 | 1 | - |
| `51` | `11B` | `32` | `18` | `06` | 3 | 2 | - | - |
| `52` | `11B` | `14` | `18` | `06` | 3 | 2 | - | - |
| `53` | `11C` | `6C` | `F2` | `03` | 4 | 1 | 1 | - |
| `54` | `11C` | `58` | `F2` | `03` | 4 | 1 | 1 | - |
| `55` | `11E` | `2F` | `1B` | `06` | 3 | 5 | 4 | - |
| `56` | `120` | `37` | `1B` | `06` | 5 | 3 | 1 | - |
| `57` | `110` | `5A` | `17` | `03` | 2 | 1 | - | - |
| `58` | `112` | `35` `45` | `12` | `14` | 2 | 2 | - | - |
| `59` | `111` | `69` | `0E` | `11` | 2 | 1 | - | - |
| `5A` | `112` | `53` | `12` | `14` | 2 | 2 | - | - |
| `5B` | `113` | `14` | `18` | `01` | 2 | - | 1 | - |
| `5C` | `114` | `0F` | `18` | `06` | 3 | 2 | - | - |
| `5D` | `115` | `35` | `18` | `06` | 3 | 6 | - | - |
| `5E` | `115` | `70` `2E` `34` `6E` `43` `3A` `77` | `1B` | `06` | 3 | 6 | - | - |
| `5F` | `10D` | `70` | `12` | `08` | 2 | 2 | 2 | - |
| `60` | `10F` | `42` `58` `56` `75` | `17` | `03` | 2 | 1 | - | - |
| `61` | `119` | `18` | `F2` | `0A` | 2 | 1 | - | - |
| `62` | `114` | `70` | `12` | `06` | 3 | 2 | - | - |
| `63` | `116` | `5B` | `18` | `12` | 1 | 1 | - | - |
| `64` | `121` | `22` | `F2` | `11` | 2 | 1 | - | - |
| `65` | `122` | `11` `35` | `17` | `11` | 4 | 2 | - | - |
| `66` | `122` | `51` | `17` | `11` | 4 | 2 | - | - |
| `67` | `118` | `29` | `0E` | `03` | 2 | 1 | - | - |
| `68` | `11A` | `5E` | `18` | `0F` | 3 | 1 | - | - |
| `69` | `10E` | `6F` | `12` | `14` | 2 | 2 | - | - |
| `6A` | `10E` | `77` | `12` | `14` | 2 | 2 | - | - |
| `6B` | `11F` | `02` | `F2` | `03` | 4 | 1 | - | - |
| `6C` | `123` | `35` | `12` | `06` | 2 | 5 | 4 | - |
| `6D` | `124` | `3A` | `12` | `06` | 2 | 1 | 1 | - |
| `6E` | `124` | `13` | `12` | `06` | 2 | 1 | 1 | - |
| `6F` | `125` | `37` | `12` | `06` | 2 | 1 | - | - |
| `70` | `125` | `77` | `12` | `06` | 2 | 1 | - | - |
| `71` | `126` | `2B` `6B` | `1B` | `06` | 2 | 5 | - | - |
| `72` | `126` | `30` | `12` | `06` | 2 | 5 | - | - |
| `73` | `080` | - | `FF` | `01` | - | 3 | 1 | - |
| `74` | `051` | - | `FF` | `04` | 2 | 3 | - | - |
| `75` | `030` | - | `FF` | `02` | 2 | 1 | - | - |
| `7A` | `0E1` | - | `18` | `06` | 2 | 2 | - | - |
| `7B` | `000` | - | `15` | `13` | - | 1 | - | yes |
| `7C` | `018` | - | `1B` | `06` | 2 | - | - | - |
| `7D` | `055` | - | `03` | `01` | 3 | 3 | 1 | yes |
| `7E` | `0E3` | - | `18` | `06` | 3 | 1 | - | - |
| `7F` | `0E2` | - | `1B` | `06` | 2 | 5 | - | - |
| `80` | `02F` | - | `12` | `06` | 2 | - | 5 | - |
| `82` | `003` | - | `12` | `06` | 1 | - | - | - |
| `83` | `127` | `62` | `12` | `06` | 2 | 1 | - | - |
| `84` | `120` | `37` | `1B` | `06` | 5 | 3 | 1 | - |

## Each entrance

### Entrance `00`, room `104`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `FF`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `00`, door setting `0816`.

Link starts at `0978`,`2178`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `0081`, `1281`.

Sprites:

- `73` Uncle And Priest (npc) at `09A0`,`2170`, floor 0

Chests: `12`.

### Entrance `01`, room `104`

Door on overworld screen `2C` at map position `0796`.

Entrance data: music `07`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0816`.

Link starts at `0978`,`21D8`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `0081`, `1281`.

Sprites:

- `73` Uncle And Priest (npc) at `09A0`,`2170`, floor 0

Chests: `12`.

### Entrance `06`, room `0F0`

Door on overworld screen `0A` at map position `0634`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0078`,`1FD8`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `0E61`, `02A3`.

Sprites:

- `6F` Keese (enemy) at `0090`,`1E30`, floor 0
- `6F` Keese (enemy) at `0100`,`1E30`, floor 0
- `6F` Keese (enemy) at `0080`,`1E40`, floor 0
- `6F` Keese (enemy) at `00A0`,`1E40`, floor 0
- `6F` Keese (enemy) at `0090`,`1E70`, floor 0
- `6F` Keese (enemy) at `0030`,`1EA0`, floor 0
- `6F` Keese (enemy) at `0050`,`1EA0`, floor 0
- `6F` Keese (enemy) at `00E0`,`1EC0`, floor 0
- `AD` Old Man (npc) at `01B0`,`1F00`, floor 0
- `6F` Keese (enemy) at `0130`,`1F30`, floor 0

### Entrance `07`, room `0F1`

Door on overworld screen `03` at map position `178E`.

Entrance data: music `12`, tileset `06`, background `01`, quadrants `22`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0378`,`1FC0`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `10B1`, `0242`.

Sprites:

- `6F` Keese (enemy) at `0390`,`1F00`, floor 0
- `6F` Keese (enemy) at `03C0`,`1F00`, floor 0
- `6F` Keese (enemy) at `0380`,`1F10`, floor 0
- `6F` Keese (enemy) at `03D0`,`1F10`, floor 0
- `6F` Keese (enemy) at `0370`,`1F20`, floor 0
- `6F` Keese (enemy) at `03E0`,`1F20`, floor 0
- `6F` Keese (enemy) at `0260`,`1FB0`, floor 0
- `6F` Keese (enemy) at `0290`,`1FB0`, floor 0
- `6F` Keese (enemy) at `0270`,`1FC0`, floor 0
- `6F` Keese (enemy) at `0280`,`1FC0`, floor 0

### Entrance `0D`, room `0F2`

Door on overworld screen `18` at map position `054C`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `05CC`.

Link starts at `0578`,`1FD8`.

Doors: `0083`, `0081`, `1281`.

### Entrance `0E`, room `0F3`

Door on overworld screen `18` at map position `0554`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `05D4`.

Link starts at `0678`,`1FD8`.

Doors: `0022`, `0061`, `1261`.

Sprites:

- `78` Mrs Sahasrahla (npc) at `0660`,`1F40`, floor 0

### Entrance `0F`, room `0F4`

Door on overworld screen `28` at map position `0B36`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0BB6`.

Link starts at `0978`,`1FD8`.

Doors: `2883`, `0081`, `1281`.

Sprites:

- `32` unnamed (npc) at `0970`,`1F40`, floor 0

### Entrance `10`, room `0F5`

Door on overworld screen `29` at map position `0B06`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0B86`.

Link starts at `0A78`,`1FD8`.

Doors: `2822`, `0061`, `1261`.

Sprites:

- `32` unnamed (npc) at `0A80`,`1F40`, floor 0

### Entrance `11`, room `0E3`

Door on overworld screen `22` at map position `06A0`.

Entrance data: music `18`, tileset `06`, background `00`, quadrants `00`/`02`, floor -1, doorway `01`, door setting `0000`.

Link starts at `0678`,`1DD8`.

Doors: `4AB2`, `40B0`, `0E61`.

Sprites:

- `3A` Magic Bat (npc) at `0770`,`1C50`, floor 1

### Entrance `12`, room `0E2`

Door on overworld screen `02` at map position `03A8`.

Entrance data: music `1B`, tileset `06`, background `00`, quadrants `02`/`12`, floor -1, doorway `01`, door setting `0000`.

Link starts at `0578`,`1DD8`.

Doors: `0072`, `0E81`.

Sprites:

- `E3` Fairy (enemy) at `0470`,`1C60`, floor 0
- `E3` Fairy (enemy) at `0480`,`1C60`, floor 0
- `E3` Fairy (enemy) at `0470`,`1C70`, floor 0
- `E3` Fairy (enemy) at `0480`,`1C70`, floor 0
- `EB` unnamed (npc) at `0530`,`1D00`, floor 0

### Entrance `13`, room `0F8`

Door on overworld screen `45` at map position `126E`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `10F8`,`1FD8`.

Doors: `0E71`.

Chests: `28`, `36`.

### Entrance `14`, room `0E8`

Door on overworld screen `45` at map position `07F6`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`12`, floor 1, doorway `01`, door setting `0000`.

Link starts at `1178`,`1DD8`.

Doors: `0E81`.

Sprites:

- `26` Hardhat Beetle (npc) at `1070`,`1C50`, floor 0
- `26` Hardhat Beetle (npc) at `1170`,`1C80`, floor 0
- `26` Hardhat Beetle (npc) at `1070`,`1CC0`, floor 0
- `26` Hardhat Beetle (npc) at `1190`,`1CC0`, floor 0

### Entrance `16`, room `0FB`

Door on overworld screen `4A` at map position `0634`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `16F8`,`1FD8`.

Doors: `0E71`.

Sprites:

- `93` Bumper (enemy) at `1770`,`1ED0`, floor 0
- `26` Hardhat Beetle (npc) at `1790`,`1EA0`, floor 0
- `26` Hardhat Beetle (npc) at `1750`,`1F20`, floor 0

### Entrance `17`, room `0EB`

Door on overworld screen `4A` at map position `0336`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `02`/`12`, floor 1, doorway `01`, door setting `0000`.

Link starts at `1778`,`1DD8`.

Doors: `0E81`.

Sprites:

- `93` Bumper (enemy) at `1770`,`1D40`, floor 0

### Entrance `1A`, room `0FD`

Door on overworld screen `05` at map position `1162`.

Entrance data: music `12`, tileset `06`, background `01`, quadrants `22`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1AF8`,`1FC0`.

Doors: `10A1`.

Sprites:

- `18` Mini Moldorm (enemy) at `1A90`,`1EE0`, floor 0
- `24` unnamed (enemy) at `1A50`,`1E80`, floor 0
- `E3` Fairy (enemy) at `1B60`,`1E80`, floor 0
- `E3` Fairy (enemy) at `1B80`,`1E80`, floor 0
- `24` unnamed (enemy) at `1AF0`,`1F10`, floor 0

### Entrance `1B`, room `0ED`

Door on overworld screen `05` at map position `0E62`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`12`, floor 1, doorway `01`, door setting `0000`.

Link starts at `1B78`,`1DD8`.

Doors: `0E81`.

### Entrance `1C`, room `0FE`

Door on overworld screen `05` at map position `1058`.

Entrance data: music `12`, tileset `06`, background `01`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1D78`,`1FC0`.

Doors: `10B1`.

Sprites:

- `18` Mini Moldorm (enemy) at `1D60`,`1F20`, floor 0
- `18` Mini Moldorm (enemy) at `1D40`,`1F60`, floor 0
- `18` Mini Moldorm (enemy) at `1DA0`,`1F60`, floor 0
- `24` unnamed (enemy) at `1D80`,`1F20`, floor 0
- `24` unnamed (enemy) at `1D80`,`1F80`, floor 0

Chests: `41`.

### Entrance `1D`, room `0EE`

Door on overworld screen `05` at map position `0B56`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`12`, floor 1, doorway `01`, door setting `0000`.

Link starts at `1D78`,`1DD8`.

Doors: `0E81`.

Sprites:

- `18` Mini Moldorm (enemy) at `1D00`,`1C40`, floor 0
- `18` Mini Moldorm (enemy) at `1CB0`,`1CE0`, floor 0
- `18` Mini Moldorm (enemy) at `1C90`,`1DC0`, floor 0
- `24` unnamed (enemy) at `1C30`,`1CB0`, floor 0
- `24` unnamed (enemy) at `1DC0`,`1CC0`, floor 0

### Entrance `1E`, room `0FF`

Door on overworld screen `05` at map position `1274`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `20`/`12`, floor -1, doorway `01`, door setting `0000`.

Link starts at `1F78`,`1FD8`.

Doors: `2E60`, `2E80`, `0E81`.

Sprites:

- `BB` Shopkeeper (npc) at `1E70`,`1E40`, floor 0

Chests: `28`, `44`.

### Entrance `1F`, room `0EF`

Door on overworld screen `05` at map position `1B78`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `20`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1EF8`,`1DD8`.

Doors: `3880`, `0E71`.

Sprites:

- `18` Mini Moldorm (enemy) at `1F70`,`1C90`, floor 0
- `18` Mini Moldorm (enemy) at `1F40`,`1CA0`, floor 0
- `18` Mini Moldorm (enemy) at `1FB0`,`1CA0`, floor 0
- `1E` Crystal Switch (npc) at `1F80`,`1C60`, floor 0

Chests: `36`, `36`, `36`, `36`, `36`.

### Entrance `20`, room `0DF`

Door on overworld screen `05` at map position `07F6`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `20`/`02`, floor 1, doorway `01`, door setting `0000`.

Link starts at `1EF8`,`1BD8`.

Doors: `0E71`.

Sprites:

- `18` Mini Moldorm (enemy) at `1EC0`,`1B50`, floor 1
- `18` Mini Moldorm (enemy) at `1EC0`,`1B60`, floor 1

### Entrance `21`, room `0F9`

Door on overworld screen `03` at map position `1128`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1278`,`1FD8`.

Doors: `0E61`.

Sprites:

- `18` Mini Moldorm (enemy) at `13A0`,`1E50`, floor 0
- `18` Mini Moldorm (enemy) at `1350`,`1EF0`, floor 0
- `18` Mini Moldorm (enemy) at `1310`,`1F30`, floor 0
- `18` Mini Moldorm (enemy) at `12C0`,`1F70`, floor 0

### Entrance `22`, room `0FA`

Door on overworld screen `03` at map position `1238`.

Entrance data: music `12`, tileset `06`, background `11`, quadrants `22`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1478`,`1FC0`.

Doors: `0491`.

Sprites:

- `E3` Fairy (enemy) at `1570`,`1EE0`, floor 0
- `E3` Fairy (enemy) at `1580`,`1F00`, floor 0
- `E3` Fairy (enemy) at `1550`,`1F10`, floor 0

### Entrance `23`, room `0EA`

Door on overworld screen `03` at map position `0CB8`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`02`, floor 1, doorway `01`, door setting `0000`.

Link starts at `1478`,`1DD8`.

Doors: `0E61`.

Sprites:

- `EB` unnamed (npc) at `14B0`,`1CB0`, floor 0

### Entrance `2C`, room `0E1`

Door on overworld screen `00` at map position `12DC`.

Entrance data: music `18`, tileset `06`, background `00`, quadrants `02`/`02`, floor -1, doorway `01`, door setting `0000`.

Link starts at `0278`,`1DD8`.

Doors: `40A2`, `0E61`.

Sprites:

- `EB` unnamed (npc) at `0370`,`1CD0`, floor 0
- `29` unnamed (npc) at `0270`,`1D20`, floor 1

### Entrance `2E`, room `0E6`

Door on overworld screen `0A` at map position `0336`.

Entrance data: music `12`, tileset `06`, background `01`, quadrants `22`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0C78`,`1DC0`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `1091`, `0293`.

Sprites:

- `6F` Keese (enemy) at `0DB0`,`1CB0`, floor 0
- `6F` Keese (enemy) at `0D70`,`1CF0`, floor 0
- `6F` Keese (enemy) at `0D30`,`1D30`, floor 0
- `6F` Keese (enemy) at `0CF0`,`1D70`, floor 0
- `6F` Keese (enemy) at `0CB0`,`1DB0`, floor 0

### Entrance `2F`, room `0E7`

Door on overworld screen `03` at map position `1108`.

Entrance data: music `12`, tileset `06`, background `01`, quadrants `22`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0F78`,`1DC0`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `10B1`, `0232`.

Sprites:

- `6F` Keese (enemy) at `0F00`,`1C40`, floor 0
- `6F` Keese (enemy) at `0F30`,`1C40`, floor 0
- `6F` Keese (enemy) at `0F50`,`1CB0`, floor 0
- `6F` Keese (enemy) at `0EB0`,`1CC0`, floor 0
- `6F` Keese (enemy) at `0EB0`,`1CD0`, floor 0
- `6F` Keese (enemy) at `0F50`,`1CD0`, floor 0
- `6F` Keese (enemy) at `0F50`,`1CF0`, floor 0

### Entrance `30`, room `0E4`

Door on overworld screen `03` at map position `1DA4`.

Entrance data: music `12`, tileset `14`, background `01`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0878`,`1DC0`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `1091`, `0062`, `0073`.

Sprites:

- `6F` Keese (enemy) at `0990`,`1C70`, floor 0
- `6F` Keese (enemy) at `0980`,`1C80`, floor 0
- `6F` Keese (enemy) at `0970`,`1C90`, floor 0
- `AD` Old Man (npc) at `0860`,`1D60`, floor 0

### Entrance `31`, room `0E5`

Door on overworld screen `03` at map position `1450`.

Entrance data: music `12`, tileset `14`, background `00`, quadrants `22`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0AF8`,`1DD8`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `0012`, `0E71`.

Sprites:

- `6F` Keese (enemy) at `0AF0`,`1C90`, floor 0
- `6F` Keese (enemy) at `0B00`,`1C90`, floor 0
- `6F` Keese (enemy) at `0B10`,`1C90`, floor 0
- `6F` Keese (enemy) at `0BB0`,`1CE0`, floor 0
- `6F` Keese (enemy) at `0AF0`,`1D20`, floor 0
- `6F` Keese (enemy) at `0B10`,`1D20`, floor 0

### Entrance `32`, room `055`

Door on overworld screen `1B` at map position `06D8`.

Entrance data: music `03`, tileset `01`, background `00`, quadrants `20`/`02`, floor -1, doorway `01`, door setting `0000`.

Link starts at `0A78`,`0BD8`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `0290`, `0061`, `1261`.

Sprites:

- `73` Uncle And Priest (npc) at `0AE0`,`0A80`, floor 0
- `4B` Green Knife Guard (enemy) at `0B40`,`0B50`, floor 0
- `4B` Green Knife Guard (enemy) at `0AD0`,`0B60`, floor 0

Chests: `12`.

### Entrance `36`, room `010`

Door on overworld screen `5B` at map position `0D9C`.

Entrance data: music `1C`, tileset `13`, background `00`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0078`,`03D8`.

Doors: `0A61`, `0082`.

### Entrance `38`, room `008`

Door on overworld screen `15` at map position `0294`.

Entrance data: music `1B`, tileset `06`, background `00`, quadrants `00`/`12`, floor -1, doorway `01`, door setting `0000`.

Link starts at `1178`,`01D8`.

Doors: `48B2`, `0E81`, `4091`, `1691`.

Sprites:

- `C8` Big Fairy (npc) at `1070`,`0160`, floor 0

### Entrance `39`, room `02F`

Door on overworld screen `18` at map position `0616`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `20`/`12`, floor -1, doorway `01`, door setting `0000`.

Link starts at `1F78`,`05D8`.

Doors: `2E60`, `0E81`.

Chests: `17`, `36`, `36`, `36`, `28`.

### Entrance `3A`, room `03C`

Door on overworld screen `45` at map position `0868`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `22`/`12`, floor -1, doorway `01`, door setting `0000`.

Link starts at `1978`,`07D8`.

Doors: `2E20`, `0E81`.

Sprites:

- `26` Hardhat Beetle (npc) at `1890`,`0680`, floor 0
- `24` unnamed (enemy) at `18A0`,`0740`, floor 0
- `24` unnamed (enemy) at `1920`,`0740`, floor 0

Chests: `41`, `41`, `41`, `41`.

### Entrance `3B`, room `02C`

Door on overworld screen `45` at map position `01D8`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`02`, floor -1, doorway `01`, door setting `0000`.

Link starts at `1878`,`05D8`.

Doors: `2E60`, `2E80`, `2E82`, `2E81`, `0E61`.

Sprites:

- `C8` Big Fairy (npc) at `1970`,`0450`, floor 0
- `E3` Fairy (enemy) at `1890`,`0440`, floor 0
- `E3` Fairy (enemy) at `1860`,`0450`, floor 0
- `E3` Fairy (enemy) at `1880`,`0470`, floor 0

### Entrance `3C`, room `100`

Door on overworld screen `00` at map position `00DE`.

Entrance data: music `0E`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0078`,`21D8`.

Doors: `0061`, `1261`.

Sprites:

- `BB` Shopkeeper (npc) at `00B0`,`21B0`, floor 0

### Entrance `3D`, room `11E`

Door on overworld screen `74` at map position `0330`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1D78`,`23D8`.

Doors: `2E80`, `0E81`, `0E61`.

Sprites:

- `E3` Fairy (enemy) at `1C50`,`2270`, floor 0
- `E3` Fairy (enemy) at `1C60`,`2270`, floor 0
- `E3` Fairy (enemy) at `1C50`,`2280`, floor 0
- `E3` Fairy (enemy) at `1C60`,`2280`, floor 0
- `BB` Shopkeeper (npc) at `1D80`,`2360`, floor 0

Chests: `36`, `36`, `36`, `36`.

### Entrance `3E`, room `101`

Door on overworld screen `18` at map position `0D68`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0DE8`.

Link starts at `0278`,`21D8`.

Doors: `0061`, `1261`, `0081`, `1281`.

Sprites:

- `33` Rupee Pull (npc) at `0280`,`2130`, floor 0

### Entrance `3F`, room `101`

Door on overworld screen `18` at map position `0B18`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0B98`.

Link starts at `0378`,`21D8`.

Doors: `0061`, `1261`, `0081`, `1281`.

Sprites:

- `33` Rupee Pull (npc) at `0280`,`2130`, floor 0

### Entrance `40`, room `102`

Door on overworld screen `18` at map position `144E`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `14CE`.

Link starts at `0478`,`21D8`.

Doors: `0061`, `1261`.

Sprites:

- `1F` Sick Kid (npc) at `0430`,`2180`, floor 0

### Entrance `41`, room `117`

Door on overworld screen `43` at map position `1264`.

Entrance data: music `12`, tileset `14`, background `00`, quadrants `22`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0F78`,`23D8`.

Doors: `0E81`.

Chests: `18`.

### Entrance `42`, room `103`

Door on overworld screen `18` at map position `1BD0`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `1C50`.

Link starts at `0678`,`21D8`.

Doors: `0000`, `1200`, `0061`, `1261`, `0081`, `1281`.

Sprites:

- `BC` Drunkard (npc) at `0660`,`2150`, floor 0
- `29` unnamed (npc) at `06A0`,`21B0`, floor 0
- `35` unnamed (npc) at `0770`,`2170`, floor 0

Chests: `16`.

### Entrance `43`, room `103`

Door on overworld screen `18` at map position `18D0`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `02`/`00`, floor 0, doorway `01`, door setting `FFFF`.

Link starts at `0678`,`2020`.

Doors: `0000`, `1200`, `0061`, `1261`, `0081`, `1281`.

Sprites:

- `BC` Drunkard (npc) at `0660`,`2150`, floor 0
- `29` unnamed (npc) at `06A0`,`21B0`, floor 0
- `35` unnamed (npc) at `0770`,`2170`, floor 0

Chests: `16`.

### Entrance `44`, room `103`

Door on overworld screen `18` at map position `13E6`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `1466`.

Link starts at `0778`,`21D8`.

Doors: `0000`, `1200`, `0061`, `1261`, `0081`, `1281`.

Sprites:

- `BC` Drunkard (npc) at `0660`,`2150`, floor 0
- `29` unnamed (npc) at `06A0`,`21B0`, floor 0
- `35` unnamed (npc) at `0770`,`2170`, floor 0

Chests: `16`.

### Entrance `45`, room `105`

Door on overworld screen `1E` at map position `099E`.

Entrance data: music `18`, tileset `0F`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0A78`,`21D8`.

Doors: `2860`, `0061`, `1261`.

Sprites:

- `16` Elder_bounce (npc) at `0A70`,`2180`, floor 0

Chests: `41`, `28`, `41`.

### Entrance `46`, room `11F`

Door on overworld screen `18` at map position `1A36`.

Entrance data: music `17`, tileset `03`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `1AB6`.

Link starts at `1F78`,`23D8`.

Doors: `0061`, `1261`, `0081`, `1281`.

Sprites:

- `BB` Shopkeeper (npc) at `1F70`,`2360`, floor 0

### Entrance `47`, room `106`

Door on overworld screen `58` at map position `0B18`.

Entrance data: music `0E`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0B98`.

Link starts at `0C78`,`21D8`.

Doors: `2E81`, `1281`, `0061`, `1261`.

Sprites:

- `BB` Shopkeeper (npc) at `0C80`,`21B0`, floor 0

Chests: `2A`.

### Entrance `48`, room `106`

Door on overworld screen `58` at map position `1A36`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `1AB6`.

Link starts at `0D78`,`21D8`.

Doors: `2E81`, `1281`, `0061`, `1261`.

Sprites:

- `BB` Shopkeeper (npc) at `0C80`,`21B0`, floor 0

Chests: `2A`.

### Entrance `49`, room `107`

Door on overworld screen `29` at map position `038E`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `040E`.

Link starts at `0E78`,`21D8`.

Doors: `2E81`, `1281`, `0061`, `1261`.

Sprites:

- `3B` unnamed (npc) at `0E30`,`2150`, floor 0
- `6D` Rat (npc) at `0F70`,`21B0`, floor 0
- `6D` Rat (npc) at `0F80`,`21B0`, floor 0

### Entrance `4A`, room `107`

Door on overworld screen `18` at map position `1B8C`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `9C0C`.

Link starts at `0F78`,`21D8`.

Doors: `2E81`, `1281`, `0061`, `1261`.

Sprites:

- `3B` unnamed (npc) at `0E30`,`2150`, floor 0
- `6D` Rat (npc) at `0F70`,`21B0`, floor 0
- `6D` Rat (npc) at `0F80`,`21B0`, floor 0

### Entrance `4B`, room `108`

Door on overworld screen `18` at map position `14B0`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `1530`.

Link starts at `1078`,`21D8`.

Doors: `2E82`, `0061`, `1261`.

Sprites:

- `0B` Cucco (enemy) at `1090`,`2160`, floor 0
- `0B` Cucco (enemy) at `10C0`,`2160`, floor 0
- `0B` Cucco (enemy) at `1090`,`2190`, floor 0
- `0B` Cucco (enemy) at `1060`,`21A0`, floor 0

Chests: `0C`.

### Entrance `4C`, room `109`

Door on overworld screen `16` at map position `0A18`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0A98`.

Link starts at `1278`,`21D8`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `0061`, `1261`.

Sprites:

- `E9` Potion Shop (npc) at `12A0`,`21B0`, floor 0

### Entrance `4D`, room `10A`

Door on overworld screen `30` at map position `0964`, screen `30` at map position `0964`.

Entrance data: music `18`, tileset `06`, background `01`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1478`,`21C0`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `2E80`, `1091`, `0062`.

Sprites:

- `16` Elder_bounce (npc) at `1590`,`2040`, floor 0

Chests: `17`.

### Entrance `4E`, room `10B`

Door on overworld screen `3B` at map position `072E`.

Entrance data: music `18`, tileset `08`, background `00`, quadrants `20`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `16F8`,`21D8`.

Doors: `0071`, `1271`, `0070`.

Sprites:

- `06` unnamed (enemy) at `16F0`,`2030`, floor 0
- `04` unnamed (enemy) at `1720`,`2030`, floor 0
- `15` Antifairy (enemy) at `16D0`,`2070`, floor 0

Overlords: `1A`, `1A`, `1A`, `1A`.

Chests: `28`.

### Entrance `4F`, room `10C`

Door on overworld screen `05` at map position `0B6E`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1878`,`21D8`.

Doors: `3660`, `0E61`, `0E81`.

Sprites:

- `E3` Fairy (enemy) at `1970`,`2070`, floor 0
- `E3` Fairy (enemy) at `1980`,`2070`, floor 0
- `E3` Fairy (enemy) at `1970`,`2080`, floor 0
- `E3` Fairy (enemy) at `1980`,`2080`, floor 0
- `83` Green Eyegore (enemy) at `1870`,`2140`, floor 0
- `83` Green Eyegore (enemy) at `1880`,`2140`, floor 0
- `83` Green Eyegore (enemy) at `18C0`,`2140`, floor 0
- `83` Green Eyegore (enemy) at `18C0`,`21A0`, floor 0

Chests: `17`.

### Entrance `50`, room `10C`

Door on overworld screen `05` at map position `126E`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1978`,`21D8`.

Doors: `3660`, `0E61`, `0E81`.

Sprites:

- `E3` Fairy (enemy) at `1970`,`2070`, floor 0
- `E3` Fairy (enemy) at `1980`,`2070`, floor 0
- `E3` Fairy (enemy) at `1970`,`2080`, floor 0
- `E3` Fairy (enemy) at `1980`,`2080`, floor 0
- `83` Green Eyegore (enemy) at `1870`,`2140`, floor 0
- `83` Green Eyegore (enemy) at `1880`,`2140`, floor 0
- `83` Green Eyegore (enemy) at `18C0`,`2140`, floor 0
- `83` Green Eyegore (enemy) at `18C0`,`21A0`, floor 0

Chests: `17`.

### Entrance `51`, room `11B`

Door on overworld screen `32` at map position `0906`.

Entrance data: music `18`, tileset `06`, background `11`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1678`,`23C0`.

Doors: `2E80`, `0491`, `0E81`.

Sprites:

- `EB` unnamed (npc) at `1780`,`2290`, floor 0
- `EB` unnamed (npc) at `1650`,`2360`, floor 1

### Entrance `52`, room `11B`

Door on overworld screen `14` at map position `02A2`.

Entrance data: music `18`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1778`,`23D8`.

Doors: `2E80`, `0491`, `0E81`.

Sprites:

- `EB` unnamed (npc) at `1780`,`2290`, floor 0
- `EB` unnamed (npc) at `1650`,`2360`, floor 1

### Entrance `53`, room `11C`

Door on overworld screen `6C` at map position `0796`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0816`.

Link starts at `1878`,`23D8`.

Doors: `0061`, `1261`, `0081`, `1281`.

Sprites:

- `B5` Bomb Shop (npc) at `1890`,`2390`, floor 0

Chests: `46`.

### Entrance `54`, room `11C`

Door on overworld screen `58` at map position `0D68`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0DE8`.

Link starts at `1978`,`23D8`.

Doors: `0061`, `1261`, `0081`, `1281`.

Sprites:

- `B5` Bomb Shop (npc) at `1890`,`2390`, floor 0

Chests: `46`.

### Entrance `55`, room `11E`

Door on overworld screen `2F` at map position `0934`.

Entrance data: music `1B`, tileset `06`, background `00`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1C78`,`23D8`.

Doors: `2E80`, `0E81`, `0E61`.

Sprites:

- `E3` Fairy (enemy) at `1C50`,`2270`, floor 0
- `E3` Fairy (enemy) at `1C60`,`2270`, floor 0
- `E3` Fairy (enemy) at `1C50`,`2280`, floor 0
- `E3` Fairy (enemy) at `1C60`,`2280`, floor 0
- `BB` Shopkeeper (npc) at `1D80`,`2360`, floor 0

Chests: `36`, `36`, `36`, `36`.

### Entrance `56`, room `120`

Door on overworld screen `37` at map position `0212`.

Entrance data: music `1B`, tileset `06`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0178`,`25D8`.

Doors: `2E80`, `0E61`, `0E81`, `0060`, `0062`.

Sprites:

- `B2` Player Bee (npc) at `0170`,`2470`, floor 0
- `E3` Fairy (enemy) at `01B0`,`2480`, floor 0
- `E3` Fairy (enemy) at `01A0`,`2490`, floor 0

Chests: `08`.

### Entrance `57`, room `110`

Door on overworld screen `5A` at map position `0A28`.

Entrance data: music `17`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0AA8`.

Link starts at `0078`,`23D8`.

Doors: `0061`, `1261`.

Sprites:

- `BB` Shopkeeper (npc) at `0070`,`2350`, floor 0

### Entrance `58`, room `112`

Door on overworld screen `35` at map position `01B2`, screen `45` at map position `1274`.

Entrance data: music `12`, tileset `14`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0578`,`23D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `28` Dark World Hint NPC (npc) at `0470`,`22A0`, floor 0
- `BB` Shopkeeper (npc) at `0570`,`2340`, floor 0

### Entrance `59`, room `111`

Door on overworld screen `69` at map position `092C`.

Entrance data: music `0E`, tileset `11`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `09AC`.

Link starts at `0278`,`23D8`.

Doors: `0061`, `1261`.

Sprites:

- `65` Archery Game (enemy) at `02B0`,`23B0`, floor 0

### Entrance `5A`, room `112`

Door on overworld screen `53` at map position `02AA`.

Entrance data: music `12`, tileset `14`, background `00`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0478`,`23D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `28` Dark World Hint NPC (npc) at `0470`,`22A0`, floor 0
- `BB` Shopkeeper (npc) at `0570`,`2340`, floor 0

### Entrance `5B`, room `113`

Door on overworld screen `14` at map position `05B2`.

Entrance data: music `18`, tileset `01`, background `00`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0678`,`23D8`.

Doors: `0061`, `1261`.

Chests: `19`.

### Entrance `5C`, room `114`

Door on overworld screen `0F` at map position `008C`.

Entrance data: music `18`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0878`,`23D8`.

Doors: `2E80`, `0E61`, `0E81`.

Sprites:

- `72` Fairy Pond (npc) at `0870`,`2380`, floor 0
- `28` Dark World Hint NPC (npc) at `0990`,`2340`, floor 0

### Entrance `5D`, room `115`

Door on overworld screen `35` at map position `0CD4`.

Entrance data: music `18`, tileset `06`, background `00`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0A78`,`23D8`.

Doors: `2E62`, `0E81`, `0E61`.

Sprites:

- `C8` Big Fairy (npc) at `0B70`,`2360`, floor 0
- `E3` Fairy (enemy) at `0B70`,`2270`, floor 0
- `E3` Fairy (enemy) at `0B80`,`2270`, floor 0
- `E3` Fairy (enemy) at `0B70`,`2280`, floor 0
- `E3` Fairy (enemy) at `0B80`,`2280`, floor 0
- `72` Fairy Pond (npc) at `0A70`,`2290`, floor 0

### Entrance `5E`, room `115`

Door on overworld screen `70` at map position `0636`, screen `2E` at map position `0224`, screen `34` at map position `0330`, screen `6E` at map position `0224`, screen `43` at map position `178E`, screen `3A` at map position `018C`, screen `77` at map position `0208`.

Entrance data: music `1B`, tileset `06`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0B78`,`23D8`.

Doors: `2E62`, `0E81`, `0E61`.

Sprites:

- `C8` Big Fairy (npc) at `0B70`,`2360`, floor 0
- `E3` Fairy (enemy) at `0B70`,`2270`, floor 0
- `E3` Fairy (enemy) at `0B80`,`2270`, floor 0
- `E3` Fairy (enemy) at `0B70`,`2280`, floor 0
- `E3` Fairy (enemy) at `0B80`,`2280`, floor 0
- `72` Fairy Pond (npc) at `0A70`,`2290`, floor 0

### Entrance `5F`, room `10D`

Door on overworld screen `70` at map position `0612`.

Entrance data: music `12`, tileset `08`, background `00`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1A78`,`21D8`.

Doors: `0061`, `1261`.

Sprites:

- `5B` Spark_Clockwise (enemy) at `1A50`,`2160`, floor 0
- `5C` unnamed (enemy) at `1AA0`,`2160`, floor 0

Chests: `17`, `36`.

### Entrance `60`, room `10F`

Door on overworld screen `42` at map position `06AA`, screen `58` at map position `13E6`, screen `56` at map position `0A9A`, screen `75` at map position `060A`.

Entrance data: music `17`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1E78`,`21D8`.

Doors: `0061`, `1261`.

Sprites:

- `BB` Shopkeeper (npc) at `1E70`,`2150`, floor 0

### Entrance `61`, room `119`

Door on overworld screen `18` at map position `0540`.

Entrance data: music `F2`, tileset `0A`, background `00`, quadrants `20`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `12F8`,`23D8`.

Doors: `0071`, `1271`.

Sprites:

- `29` unnamed (npc) at `12E0`,`2380`, floor 0

### Entrance `62`, room `114`

Door on overworld screen `70` at map position `0964`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0978`,`23D8`.

Doors: `2E80`, `0E61`, `0E81`.

Sprites:

- `72` Fairy Pond (npc) at `0870`,`2380`, floor 0
- `28` Dark World Hint NPC (npc) at `0990`,`2340`, floor 0

### Entrance `63`, room `116`

Door on overworld screen `5B` at map position `0DAE`.

Entrance data: music `18`, tileset `12`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0D78`,`23D8`.

Doors: `0A81`.

Sprites:

- `72` Fairy Pond (npc) at `0D70`,`2380`, floor 0

### Entrance `64`, room `121`

Door on overworld screen `22` at map position `039A`.

Entrance data: music `F2`, tileset `11`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `041A`.

Link starts at `0278`,`25D8`.

Doors: `0061`, `1261`.

Sprites:

- `1A` Smithy (npc) at `0240`,`2570`, floor 0

### Entrance `65`, room `122`

Door on overworld screen `11` at map position `089E`, screen `35` at map position `060A`.

Entrance data: music `17`, tileset `11`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0478`,`25D8`.

Doors: `0061`, `1261`, `0081`, `1281`.

Sprites:

- `31` unnamed (npc) at `0470`,`2580`, floor 0
- `31` unnamed (npc) at `0570`,`2580`, floor 0

### Entrance `66`, room `122`

Door on overworld screen `51` at map position `089E`.

Entrance data: music `17`, tileset `11`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `091E`.

Link starts at `0578`,`25D8`.

Doors: `0061`, `1261`, `0081`, `1281`.

Sprites:

- `31` unnamed (npc) at `0470`,`2580`, floor 0
- `31` unnamed (npc) at `0570`,`2580`, floor 0

### Entrance `67`, room `118`

Door on overworld screen `29` at map position `092C`.

Entrance data: music `0E`, tileset `03`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `09AC`.

Link starts at `1178`,`23D8`.

Doors: `0081`, `1281`.

Sprites:

- `BB` Shopkeeper (npc) at `1190`,`23B0`, floor 0

### Entrance `68`, room `11A`

Door on overworld screen `5E` at map position `0FB2`.

Entrance data: music `18`, tileset `0F`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1578`,`23D8`.

Doors: `2880`, `0081`, `1281`.

Sprites:

- `28` Dark World Hint NPC (npc) at `1580`,`2370`, floor 0

### Entrance `69`, room `10E`

Door on overworld screen `6F` at map position `0934`.

Entrance data: music `12`, tileset `14`, background `01`, quadrants `02`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1C78`,`21C0`.

Doors: `1091`, `10B1`.

Sprites:

- `28` Dark World Hint NPC (npc) at `1C60`,`2060`, floor 0
- `28` Dark World Hint NPC (npc) at `1D80`,`2060`, floor 0

### Entrance `6A`, room `10E`

Door on overworld screen `77` at map position `0212`.

Entrance data: music `12`, tileset `14`, background `01`, quadrants `02`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `1D78`,`21C0`.

Doors: `1091`, `10B1`.

Sprites:

- `28` Dark World Hint NPC (npc) at `1C60`,`2060`, floor 0
- `28` Dark World Hint NPC (npc) at `1D80`,`2060`, floor 0

### Entrance `6B`, room `11F`

Door on overworld screen `02` at map position `072A`.

Entrance data: music `F2`, tileset `03`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `07AA`.

Link starts at `1E78`,`23D8`.

Doors: `0061`, `1261`, `0081`, `1281`.

Sprites:

- `BB` Shopkeeper (npc) at `1F70`,`2360`, floor 0

### Entrance `6C`, room `123`

Door on overworld screen `35` at map position `178C`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0678`,`25D8`.

Doors: `3660`, `0E61`.

Sprites:

- `18` Mini Moldorm (enemy) at `0630`,`2560`, floor 0
- `18` Mini Moldorm (enemy) at `06C0`,`2560`, floor 0
- `18` Mini Moldorm (enemy) at `0680`,`2570`, floor 0
- `18` Mini Moldorm (enemy) at `0630`,`25A0`, floor 0
- `BB` Shopkeeper (npc) at `0680`,`2450`, floor 0

Chests: `28`, `36`, `36`, `44`.

### Entrance `6D`, room `124`

Door on overworld screen `3A` at map position `0A1E`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0878`,`25D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `BB` Shopkeeper (npc) at `0880`,`2560`, floor 0

Chests: `17`.

### Entrance `6E`, room `124`

Door on overworld screen `13` at map position `0506`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0978`,`25D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `BB` Shopkeeper (npc) at `0880`,`2560`, floor 0

Chests: `17`.

### Entrance `6F`, room `125`

Door on overworld screen `37` at map position `040C`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0A78`,`25D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `BB` Shopkeeper (npc) at `0A80`,`2560`, floor 0

### Entrance `70`, room `125`

Door on overworld screen `77` at map position `040C`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0B78`,`25D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `BB` Shopkeeper (npc) at `0A80`,`2560`, floor 0

### Entrance `71`, room `126`

Door on overworld screen `2B` at map position `0330`, screen `6B` at map position `0330`.

Entrance data: music `1B`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0C78`,`25D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `E3` Fairy (enemy) at `0C70`,`2550`, floor 0
- `E3` Fairy (enemy) at `0C80`,`2550`, floor 0
- `E3` Fairy (enemy) at `0C70`,`2560`, floor 0
- `E3` Fairy (enemy) at `0C80`,`2560`, floor 0
- `EB` unnamed (npc) at `0DC0`,`2540`, floor 0

### Entrance `72`, room `126`

Door on overworld screen `30` at map position `0358`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`12`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0D78`,`25D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `E3` Fairy (enemy) at `0C70`,`2550`, floor 0
- `E3` Fairy (enemy) at `0C80`,`2550`, floor 0
- `E3` Fairy (enemy) at `0C70`,`2560`, floor 0
- `E3` Fairy (enemy) at `0C80`,`2560`, floor 0
- `EB` unnamed (npc) at `0DC0`,`2540`, floor 0

### Entrance `73`, room `080`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `FF`, tileset `01`, background `00`, quadrants `20`/`10`, floor -1, doorway `00`, door setting `0000`.

Link starts at `01A8`,`1080`.

No doors in the room's door table.

Sprites:

- `76` Zelda (npc) at `0160`,`1030`, floor 0
- `42` unnamed (enemy) at `0070`,`1090`, floor 0
- `6A` Ball NChain (enemy) at `01A0`,`1090`, floor 0

Chests: `12`.

### Entrance `74`, room `051`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `FF`, tileset `04`, background `11`, quadrants `22`/`02`, floor -1, doorway `00`, door setting `0000`.

Link starts at `02F8`,`0BA8`.

Doors: `0010`, `1410`.

Sprites:

- `EE` Movable Mantle (npc) at `02E0`,`0A20`, floor 0
- `41` Blue Guard (enemy) at `0290`,`0B70`, floor 1
- `41` Blue Guard (enemy) at `0360`,`0B70`, floor 1

### Entrance `75`, room `030`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `FF`, tileset `02`, background `00`, quadrants `00`/`00`, floor -1, doorway `00`, door setting `0000`.

Link starts at `0078`,`0698`.

Doors: `3200`, `3860`.

Sprites:

- `C1` Cutscene Agahnim (enemy) at `0070`,`0650`, floor 0

### Entrance `7A`, room `0E1`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `18`, tileset `06`, background `00`, quadrants `02`/`10`, floor -1, doorway `00`, door setting `0000`.

Link starts at `0370`,`1CA9`.

Doors: `40A2`, `0E61`.

Sprites:

- `EB` unnamed (npc) at `0370`,`1CD0`, floor 0
- `29` unnamed (npc) at `0270`,`1D20`, floor 1

### Entrance `7B`, room `000`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `15`, tileset `13`, background `00`, quadrants `00`/`10`, floor 1, doorway `00`, door setting `0000`.

Link starts at `017F`,`0089`.

Dark. The room header has bit 0 set, so it needs the lantern.

No doors in the room's door table.

Sprites:

- `D6` Ganon (enemy) at `0170`,`0050`, floor 0

### Entrance `7C`, room `018`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `1B`, tileset `06`, background `01`, quadrants `02`/`02`, floor -1, doorway `00`, door setting `0000`.

Link starts at `1073`,`033B`.

Doors: `0230`, `1630`.

### Entrance `7D`, room `055`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `03`, tileset `01`, background `01`, quadrants `20`/`10`, floor -1, doorway `00`, door setting `0000`.

Link starts at `0B9F`,`0A96`.

Dark. The room header has bit 0 set, so it needs the lantern.

Doors: `0290`, `0061`, `1261`.

Sprites:

- `73` Uncle And Priest (npc) at `0AE0`,`0A80`, floor 0
- `4B` Green Knife Guard (enemy) at `0B40`,`0B50`, floor 0
- `4B` Green Knife Guard (enemy) at `0AD0`,`0B60`, floor 0

Chests: `12`.

### Entrance `7E`, room `0E3`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `18`, tileset `06`, background `11`, quadrants `00`/`12`, floor -1, doorway `00`, door setting `0000`.

Link starts at `0778`,`1D7D`.

Doors: `4AB2`, `40B0`, `0E61`.

Sprites:

- `3A` Magic Bat (npc) at `0770`,`1C50`, floor 1

### Entrance `7F`, room `0E2`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `1B`, tileset `06`, background `01`, quadrants `02`/`02`, floor -1, doorway `00`, door setting `0000`.

Link starts at `047F`,`1D89`.

Doors: `0072`, `0E81`.

Sprites:

- `E3` Fairy (enemy) at `0470`,`1C60`, floor 0
- `E3` Fairy (enemy) at `0480`,`1C60`, floor 0
- `E3` Fairy (enemy) at `0470`,`1C70`, floor 0
- `E3` Fairy (enemy) at `0480`,`1C70`, floor 0
- `EB` unnamed (npc) at `0530`,`1D00`, floor 0

### Entrance `80`, room `02F`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `20`/`02`, floor -1, doorway `00`, door setting `0000`.

Link starts at `1E61`,`0589`.

Doors: `2E60`, `0E81`.

Chests: `17`, `36`, `36`, `36`, `28`.

### Entrance `82`, room `003`

No overworld door points at this entrance, so it is reached another way.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`02`, floor -1, doorway `00`, door setting `0000`.

Link starts at `0677`,`0197`.

Doors: `0E61`.

### Entrance `83`, room `127`

Door on overworld screen `62` at map position `0D20`.

Entrance data: music `12`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0E78`,`25D8`.

Doors: `0E61`, `0E81`.

Sprites:

- `EB` unnamed (npc) at `0E70`,`2560`, floor 0

### Entrance `84`, room `120`

Door on overworld screen `37` at map position `0208`.

Entrance data: music `1B`, tileset `06`, background `00`, quadrants `00`/`02`, floor 0, doorway `01`, door setting `0000`.

Link starts at `0078`,`25D8`.

Doors: `2E80`, `0E61`, `0E81`, `0060`, `0062`.

Sprites:

- `B2` Player Bee (npc) at `0170`,`2470`, floor 0
- `E3` Fairy (enemy) at `01B0`,`2480`, floor 0
- `E3` Fairy (enemy) at `01A0`,`2490`, floor 0

Chests: `08`.

## Reading it

Music, tileset, background and quadrant values are ids into the game's own
tables, not names. The tileset is what `kEntranceData_blockset` calls the
blockset, which decides the graphics the room is built from.

Door values are tilemap addresses rather than door numbers. A room's door table
is a list of them ending in `FFFF`, and the room load walks it straight into
`dung_door_tilemap_address`.

Sprite positions are room pixels. The floor column separates the two layers a
room can have.

## What is not here

Which door leads where. The door table says where a door is drawn, not what is
on the other side, and the connection is made from Link's position when he walks
through rather than from a table.

Furniture and scenery. Those are room objects, and reading them needs a decoder
for the object format that does not exist yet.
