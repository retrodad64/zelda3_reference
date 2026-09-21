# NPC reference

55 sprites with a named handler and a bump byte of zero: the people, the
shopkeepers, the followers and the props that share the sprite system with the
enemies. Enemies are in `enemy_combat.md`.

The zero bump test is a proxy rather than a proof. A zero byte is damage class 0,
worth 2/1/1 rather than nothing, and what actually keeps an NPC harmless is that
it never runs the contact damage path.

Dialogue is decoded from the ROM, 397 messages. A message id in the
code is an index into that list. Control codes are folded out for reading, so
`[Waitkey]` and `[Scroll]` become spaces and `[Name]` becomes Link.

## Summary

| Type | Name | Screens | Rooms | Messages | Moves |
| --- | --- | --- | --- | --- | --- |
| `16` | Elder_bounce | - | 2 | - | - |
| `1A` | Smithy | 1 | 1 | 13 | 2 |
| `1C` | Statue | - | 9 | - | 4 |
| `1E` | Crystal Switch | - | 27 | - | - |
| `1F` | Sick Kid | - | 1 | 3 | - |
| `21` | Water Switch | - | 3 | - | - |
| `25` | Talking Tree | 6 | - | - | 1 |
| `26` | Hardhat Beetle | - | 17 | - | - |
| `28` | Dark World Hint NPC | - | 4 | 7 | - |
| `2B` | Hobo | - | - | 1 | - |
| `2D` | Telepathic Tile | - | - | - | - |
| `2E` | Flute Kid | 2 | - | 5 | 1 |
| `33` | Rupee Pull | 8 | 1 | - | 2 |
| `38` | Eye Statue | - | 1 | - | - |
| `39` | Locksmith | 1 | - | 5 | 2 |
| `3A` | Magic Bat | - | 1 | 2 | 4 |
| `59` | Lost Woods Bird | - | - | - | 1 |
| `5A` | Lost Woods Squirrel | - | - | - | 3 |
| `62` | Master Sword | - | - | - | 12 |
| `6C` | Mirror Portal | - | - | - | - |
| `6D` | Rat | - | 8 | - | 2 |
| `72` | Fairy Pond | - | 3 | - | - |
| `73` | Uncle And Priest | - | 3 | - | - |
| `76` | Zelda | - | 2 | 5 | 8 |
| `78` | Mrs Sahasrahla | - | 1 | 4 | - |
| `79` | Bee | 9 | - | 4 | 10 |
| `90` | Wallmaster | - | - | - | - |
| `AC` | Apple | 6 | - | - | 8 |
| `AD` | Old Man | - | 2 | - | 6 |
| `AE` | Pipe_Down | - | 2 | - | - |
| `B2` | Player Bee | - | 1 | 1 | 4 |
| `B3` | Pedestal Plaque | 1 | - | 4 | - |
| `B4` | Purple Chest | 1 | - | - | - |
| `B5` | Bomb Shop | - | 1 | 4 | - |
| `B6` | Kiki | 1 | - | 6 | 14 |
| `B7` | Blind Maiden | - | 1 | - | - |
| `B8` | Dialogue Tester | - | - | - | - |
| `B9` | Bully And Pink Ball | 1 | - | - | - |
| `BA` | Whirlpool | 9 | - | - | - |
| `BB` | Shopkeeper | - | 12 | - | - |
| `BC` | Drunkard | - | 1 | 1 | - |
| `C6` | 4 Way Shooter | - | 12 | - | 2 |
| `C8` | Big Fairy | - | 3 | 1 | - |
| `D2` | Flopping Fish | 1 | - | 1 | 2 |
| `D5` | Dig Game Guy | 1 | - | 6 | 2 |
| `D8` | Heart | 2 | - | 1 | 10 |
| `D9` | Green Rupee | - | - | - | - |
| `E4` | Small Key | - | 1 | - | - |
| `E7` | Mushroom | 1 | - | - | - |
| `E8` | Fake Sword | 1 | - | 1 | - |
| `E9` | Potion Shop | - | 1 | - | - |
| `EC` | Thrown Item | - | - | - | - |
| `ED` | Somaria Platform | - | - | - | 2 |
| `EE` | Movable Mantle | - | 1 | - | 1 |
| `F2` | Medallion Tablet | 2 | - | - | - |

## Each NPC

### Elder_bounce `16`

1 function: `Sprite_16_Elder_bounce`.

dungeon rooms `105` `10A`.

Movement: a state machine on `sprite_subtype2`, 0 velocity writes.

### Smithy `1A`

12 functions: `ReturningSmithy_Draw`, `SmithyFrog_Draw`, `SmithySpark_Draw`, `Smithy_Draw`, `Smithy_Frog`, `Smithy_Homecoming`, `Smithy_Main`, `Smithy_Spark`, `Smithy_SpawnDumbBarrierSprite`, `Smithy_SpawnSpark`, `SpritePrep_Smithy`, `Sprite_1A_Smithy`.

Overworld screens `69`, dungeon rooms `121`.

Tiles: drawn by `ReturningSmithy_Draw`, `SmithyFrog_Draw`, `SmithySpark_Draw`, `Smithy_Draw`, 5 graphics writes.

Movement: a state machine on `sprite_subtype2`, 2 velocity writes.

Dialogue:

- `0D8` Hey you! Welcome! Ask us to do anything! > Temper my sword I just dropped by
- `0D9` I'll give you a big discount! >Sword Tempered... 10 Rupees Wait a minute
- `0DA` Tempered, eh? Are you sure? > Yes I changed my mind
- `0DB` Well, we can't make it any stronger than that... Sorry!
- `0DC` Drop by again anytime you want to. Hi ho! Hi ho! We're off to work!
- `0DD` All right, no problem. We'll have to keep your sword for a while.
- `0DE` Your sword is tempered-up! Now hold it!
- `0DF` If my lost partner returns, we can temper your sword, but now, I can't do anything for you.
- `0E0` Oh! Happy days are here again! You found my partner! ... We are very happy now... Drop by here again! At that time, we will temper your sword perfe...
- `0E1` Ribbit ribbit... Your body did not change! You are not just an ordinary guy, are you? I used to live in Kakariko Town. I wonder what my partner is ...
- `0E2` I'm sorry, we're not done yet. Come back after a while.
- `0E3` Thank you! Thank you!
- `0E4` Hey hey, amateurs shouldn't try to do this. You're just getting in the way!

### Statue `1C`

5 functions: `MovableStatue_Draw`, `SpritePrep_DesertStatue`, `SpritePrep_Statue`, `Sprite_1C_Statue`, `Statue_BlockSprites`.

dungeon rooms `01A` `026` `02B` `040` `04A` `057` `06B` `07B` `0CE`.

Tiles: drawn by `MovableStatue_Draw`.

Movement: 4 velocity writes.

### Crystal Switch `1E`

2 functions: `SpritePrep_CrystalSwitch`, `Sprite_1E_CrystalSwitch`.

dungeon rooms `004` `00B` `013` `01B` `01E` `02A` `02B` `031` `035` `03D` `03E` `05B` and 15 more.

Movement: none, it does not write a velocity.

### Sick Kid `1F`

2 functions: `SpritePrep_SickKid`, `Sprite_1F_SickKid`.

dungeon rooms `102`.

Tiles: 2 graphics writes.

Movement: a state machine on `sprite_ai_state`, 0 velocity writes.

Dialogue:

- `104` Sniffle... Hey brother Link! Do you have a bottle to keep a bug in? ... I see. You don't have one... Cough cough...
- `105` I can't go out 'cause I'm sick... Cough cough... People say I caught this cold from the evil air that is coming down off the mountain... Sniff snif...
- `106` Sniffle... I hope I get well soon... Cough cough...

### Water Switch `21`

1 function: `Sprite_21_WaterSwitch`.

dungeon rooms `035` `037` `076`.

Movement: a state machine on `sprite_ai_state`, 0 velocity writes.

### Talking Tree `25`

7 functions: `SpritePrep_TalkingTree`, `SpritePrep_TalkingTree_SpawnEyeball`, `Sprite_25_TalkingTree`, `TalkingTree_Draw`, `TalkingTree_Eye`, `TalkingTree_Mouth`, `TalkingTree_SpawnBomb`.

Overworld screens `50` `58` `5A` `5D` `6B` `72`.

Tiles: drawn by `TalkingTree_Draw`, 4 graphics writes.

Movement: a state machine on `sprite_subtype2`, 1 velocity write.

### Hardhat Beetle `26`

2 functions: `SpritePrep_HardhatBeetle`, `Sprite_26_HardhatBeetle`.

dungeon rooms `017` `02A` `031` `039` `03C` `056` `058` `067` `07B` `07C` `07D` `09B` and 5 more.

Tiles: 1 graphics write.

Movement: none, it does not write a velocity.

### Dark World Hint NPC `28`

3 functions: `DarkWorldHintNPC_Idle`, `DarkWorldHintNPC_RestoreHealth`, `Sprite_28_DarkWorldHintNPC`.

dungeon rooms `10E` `112` `114` `11A`.

Tiles: 2 graphics writes.

Movement: a state machine on `sprite_subtype2`, 0 velocity writes.

Dialogue:

- `0FE` Hey! I'll tell you a profitable story if you pay me 20 Rupees. How about it? > Pay Rupees Don't want to hear it
- `0FF` Hah! Thank you. They say there is a tiny circle of rocks in the lake at the source of the river. I don't know what will happen, but it might be fun...
- `100` Heh heh. I see. I'm not interested in talking to people who don't have Rupees...
- `101` Heh heh. Thank you. To tell you the truth, I used to be a thief in the Light World... some of my fellow thieves went into hiding because they were ...
- `102` Hah! Thank you. To tell you the truth, I found incredible beauty inside the pyramid, but someone sealed the door. You can't do anything with a stan...
- `103` Heh heh. Thank you. As a matter of fact, monster magic is making it rain in the swamp. If you can move the air with more force than the monsters, t...
- `149` You're new here, aren't you? Did you come here looking for the Power Of Gold? Well, you're too late. It will obey only the first person who touches...

### Hobo `2B`

10 functions: `Hobo_Draw`, `Hobo_SpawnSmoke`, `SpritePrep_Hobo`, `SpritePrep_Hobo_SpawnFire`, `SpritePrep_Hobo_SpawnSmoke`, `Sprite_2B_Hobo`, `Sprite_Hobo_Bubble`, `Sprite_Hobo_Bum`, `Sprite_Hobo_Fire`, `Sprite_Hobo_Smoke`.

Not placed by either table, so something else spawns it.

Tiles: drawn by `Hobo_Draw`, 7 graphics writes.

Movement: a state machine on `sprite_subtype2`, 0 velocity writes.

Dialogue:

- `0D7` Yo! Link! You seem to be in a heap of trouble, but this is all I can give you.

### Telepathic Tile `2D`

1 function: `Sprite_2D_TelepathicTile`.

Not placed by either table, so something else spawns it.

Movement: none, it does not write a velocity.

### Flute Kid `2E`

6 functions: `FluteKid_Human`, `FluteKid_SpawnQuaver`, `SpritePrep_FluteKid`, `Sprite_2E_FluteKid`, `Sprite_FluteKid_Quaver`, `Sprite_FluteKid_Stumpy`.

Overworld screens `2A` `6A`.

Tiles: 7 graphics writes.

Movement: a state machine on `sprite_subtype2`, 1 velocity write.

Dialogue:

- `0E5` After wandering into this world I turned into this shape. ... ... ... I enjoyed playing the flute in the original world... ... ... ... There was a ...
- `0E6` Then I will lend you my shovel. Good luck!
- `0E7` ... ... ... I see. I won't ask you again... Good bye.
- `0E8` Did you find my flute? ... ... ... Please keep looking for it...
- `0E9` Thank you, Link. But it looks like I can't play my flute any more. Please take it. If by chance you go to the village I lived in, please give it to...

### Rupee Pull `33`

3 functions: `RupeePull_SpawnPrize`, `SpritePrep_RupeePull`, `Sprite_33_RupeePull`.

Overworld screens `00` `1E` `34` `47` `4A` `5B` `74` `7C`, dungeon rooms `101`.

Movement: 2 velocity writes.

### Eye Statue `38`

1 function: `Sprite_38_EyeStatue`.

dungeon rooms `01B`.

Movement: none, it does not write a velocity.

### Locksmith `39`

2 functions: `SpritePrep_Locksmith`, `Sprite_39_Locksmith`.

Overworld screens `3A`.

Movement: a state machine on `sprite_ai_state`, 2 velocity writes.

Dialogue:

- `107` ... ... ... ... ... ... ... ... ...
- `109` I heard that you know I used to be a thief, right? Well, I'll open a chest for you. Will you keep it secret from everyone else? Would you please pr...
- `10A` OK, if that's the way you want it, I hope you drag that chest around forever!
- `10B` Remember, you promised... Don't tell anyone.
- `10C` All right, bring that chest over here... Seriously, keep this a secret from everyone.

### Magic Bat `3A`

3 functions: `SpritePrep_MagicBat`, `Sprite_3A_MagicBat`, `Sprite_MagicBat_SpawnLightning`.

dungeon rooms `0E3`.

Tiles: 2 graphics writes.

Movement: a state machine on `sprite_ai_state`, 4 velocity writes.

Dialogue:

- `110` Hey! Blast you for waking me from my deep, dark sleep! ...I mean, thanks a lot, sir! But now I will get my revenge on you. Get ready for it! ...Err...
- `111` Heh heh heh! I laugh at your misfortune! Now your magic power will drop by one half! Congratulations! Now, do your best, even though I'm sure it wo...

### Lost Woods Bird `59`

2 functions: `SpritePrep_LostWoodsBird`, `Sprite_59_LostWoodsBird`.

Not placed by either table, so something else spawns it.

Tiles: 2 graphics writes.

Movement: a state machine on `sprite_ai_state`, 1 velocity write.

### Lost Woods Squirrel `5A`

2 functions: `SpritePrep_LostWoodsSquirrel`, `Sprite_5A_LostWoodsSquirrel`.

Not placed by either table, so something else spawns it.

Tiles: 1 graphics write.

Movement: 3 velocity writes.

### Master Sword `62`

13 functions: `MasterSword_Draw`, `MasterSword_Main`, `MasterSword_SpawnLightBeam`, `MasterSword_SpawnLightFountain`, `MasterSword_SpawnLightWell`, `MasterSword_SpawnPendantProp`, `MasterSword_SpawnReplacementLightBeam`, `SpritePrep_MasterSword`, `Sprite_62_MasterSword`, `Sprite_MasterSword_LightBeam`, `Sprite_MasterSword_LightFountain`, `Sprite_MasterSword_LightWell`, `Sprite_MasterSword_Prop`.

Not placed by either table, so something else spawns it.

Tiles: drawn by `MasterSword_Draw`, 8 graphics writes.

Movement: a state machine on `sprite_subtype2`, 12 velocity writes.

### Mirror Portal `6C`

1 function: `Sprite_6C_MirrorPortal`.

Not placed by either table, so something else spawns it.

Movement: none, it does not write a velocity.

### Rat `6D`

2 functions: `SpritePrep_Rat`, `Sprite_6D_Rat`.

dungeon rooms `002` `011` `021` `022` `041` `064` `065` `107`.

Tiles: 1 graphics write.

Movement: 2 velocity writes.

### Fairy Pond `72`

2 functions: `SpritePrep_FairyPond`, `Sprite_72_FairyPond`.

dungeon rooms `114` `115` `116`.

Tiles: 2 graphics writes.

Movement: none, it does not write a velocity.

### Uncle And Priest `73`

2 functions: `SpritePrep_UncleAndPriest_bounce`, `Sprite_73_UncleAndPriest`.

dungeon rooms `012` `055` `104`.

Tiles: 1 graphics write.

Movement: a state machine on `sprite_E`, 0 velocity writes.

### Zelda `76`

9 functions: `AltarZelda_DrawBody`, `CutsceneAgahnim_SpawnZeldaOnAltar`, `SpriteDraw_AltarZeldaWarp`, `SpritePrep_Zelda_bounce`, `Sprite_76_Zelda`, `Sprite_CutsceneAgahnim_Zelda`, `Zelda_AtSanctuary`, `Zelda_EnteringSanctuary`, `Zelda_InCell`.

dungeon rooms `012` `080`.

Tiles: drawn by `AltarZelda_DrawBody`, `SpriteDraw_AltarZeldaWarp`, 2 graphics writes.

Movement: a state machine on `sprite_subtype2`, 8 velocity writes.

Dialogue:

- `01C` Thank you, Link. I had a feeling you were getting close.
- `01D` Yes, it was Link who helped me escape from the dungeon! When I was captive the wizard said, "Once I have finished with you, the final one, the seal...
- `01E` Link, be careful out there! I know you can save Hyrule!
- `024` All right, let's get out of here before the wizard notices. I know a secret path, but first we have to go to the first floor. Let's go!
- `025` Link, listen carefully. The wizard is magically controlling all the soldiers in the castle. I fear the worst for my father... The wizard is an inhu...

### Mrs Sahasrahla `78`

2 functions: `SpritePrep_MrsSahasrahla`, `Sprite_78_MrsSahasrahla`.

dungeon rooms `0F3`.

Tiles: 2 graphics writes.

Movement: a state machine on `sprite_ai_state`, 0 velocity writes.

Dialogue:

- `02B` Who? Oh, it's you, Link! What can I do for you, young man? The elder? Oh, no one has seen him since the wizard began collecting victims... ... ... ...
- `02C` Long ago, a prosperous people known as the Hylia inhabited this land... Legends tell of many treasures that the Hylia hid throughout the land... Th...
- `02D` Anyway, look for the elder. There must be someone in the village who knows where he is. You take care now, Link...
- `02E` Ohhh,Link. You've changed! You look marvelous... Please save us from Agahnim the wizard!

### Bee `79`

14 functions: `Bee_Bzzt`, `Bee_DormantHive`, `Bee_HandleInteractions`, `Bee_HandleZ`, `Bee_Main`, `Bee_PutInBottle`, `BottleMerchant_BuyBee`, `GoldBee_SpawnSelf`, `InitializeSpawnedBee`, `PlayerBee_HoneInOnTarget`, `ShopItem_Bee`, `SpawnBeeFromHive`, `SpritePrep_NiceBee`, `Sprite_79_Bee`.

Overworld screens `00` `0A` `13` `18` `1D` `2E` `55` `56` `6E`.

Tiles: 1 graphics write.

Movement: a state machine on `sprite_ai_state`, 10 velocity writes.

Dialogue:

- `0C8` You caught a bee! What will you do? > Keep it in a bottle Set it free
- `0CA` You don't have any empty bottles. You have no choice... Just set it free.
- `16D` No no no... I can't sell the merchandise because you don't have an empty bottle.
- `17C` I'm sorry, but you don't seem to have enough Rupees...

### Wallmaster `90`

1 function: `Sprite_90_Wallmaster`.

Not placed by either table, so something else spawns it.

Tiles: 1 graphics write.

Movement: a state machine on `sprite_ai_state`, 0 velocity writes.

### Apple `AC`

3 functions: `SpawnApple`, `Sprite_AC_Apple`, `Sprite_Apple`.

Overworld screens `0A` `10` `15` `1B` `6E` `74`.

Movement: 8 velocity writes.

### Old Man `AD`

4 functions: `OldMan_EnableCutscene`, `OldMan_RevertToSprite`, `SpritePrep_OldMan_bounce`, `Sprite_AD_OldMan`.

dungeon rooms `0E4` `0F0`.

Tiles: 2 graphics writes.

Movement: a state machine on `sprite_subtype2`, 6 velocity writes.

### Pipe_Down `AE`

1 function: `Sprite_AE_Pipe_Down`.

dungeon rooms `014` `015`.

Tiles: 1 graphics write.

Movement: none, it does not write a velocity.

### Player Bee `B2`

2 functions: `PlayerBee_HoneInOnTarget`, `Sprite_B2_PlayerBee`.

dungeon rooms `120`.

Tiles: 1 graphics write.

Movement: a state machine on `sprite_ai_state`, 4 velocity writes.

Dialogue:

- `0C8` You caught a bee! What will you do? > Keep it in a bottle Set it free

### Pedestal Plaque `B3`

2 functions: `SpritePrep_PedestalPlaque`, `Sprite_B3_PedestalPlaque`.

Overworld screens `30`.

Movement: none, it does not write a velocity.

Dialogue:

- `0B6` <Ankh><Waves><Ankh><Snake><Ankh><Waves><Ankh><Snake><Ankh><Waves><Waves><Waves><Waves><Ankh><Snake><Ankh><Waves><Ankh><Snake> <Ankh><Snake><Ankh><A...
- `0B7` The Hero's triumph on Cataclysm's Eve Wins three symbols of virtue. The Master Sword he will then retrieve, Keeping the Knight's line true.
- `0BC` <Ankh><Snake><Ankh><Waves><Ankh><Snake><Ankh><Waves><Ankh><Waves><Ankh> <Ankh><Waves><Ankh><Snake><Ankh><Snake><Ankh><Waves><Ankh><Snake> <Ankh><Wa...
- `0BD` To open the way to go forward, Make your wish here And it will be granted.

### Purple Chest `B4`

2 functions: `SpritePrep_PurpleChest`, `Sprite_B4_PurpleChest`.

Overworld screens `62`.

Movement: none, it does not write a velocity.

### Bomb Shop `B5`

7 functions: `BombShopEntity_Draw`, `BombShop_ClerkExhalation`, `Sprite_B5_BombShop`, `Sprite_BombShop_Bomb`, `Sprite_BombShop_Clerk`, `Sprite_BombShop_Huff`, `Sprite_BombShop_SuperBomb`.

dungeon rooms `11C`.

Tiles: drawn by `BombShopEntity_Draw`, 3 graphics writes.

Movement: a state machine on `sprite_subtype2`, 0 velocity writes.

Dialogue:

- `119` Thank you very much. Thank you very much.
- `11A` Thank you very much. You can drop this Bomb off anywhere. (Press the <A> Button.) Please don't forget it.
- `16E` You can't carry any more now, but you may need some later!
- `17C` I'm sorry, but you don't seem to have enough Rupees...

### Kiki `B6`

6 functions: `Kiki_Flee`, `Kiki_LyingInwait`, `Kiki_OfferEntranceService`, `Kiki_OfferInitialService`, `SpritePrep_Kiki`, `Sprite_B6_Kiki`.

Overworld screens `5E`.

Tiles: 6 graphics writes.

Movement: a state machine on `sprite_subtype2`, 14 velocity writes.

Dialogue:

- `11B` Ki ki ki! If you give me 100 Rupees, I will open the entrance for you. Ki ki ki! What will you do? > Ask him to open it Try to open it yourself
- `11C` Ki ki ki! Hmph! Do it yourself, then! Kik ki ki!
- `11D` Ki ki! Good choice! Then I get 100 of your Rupees. Kik ki ki!
- `11E` I'm Kiki the monkey ki ki! I love Rupees more than anything. Can you spare me 10 Rupees? What will you do? > Give him 10 Rupees Never give him anyt...
- `11F` Ki ki ki ki! Good choice! I will accompany you for a while. Kik kiki!
- `120` Ki ki! Harumph! I have no reason to talk to you, then. Bye bye! Kik ki ki!

### Blind Maiden `B7`

2 functions: `SpritePrep_BlindMaiden`, `Sprite_B7_BlindMaiden`.

dungeon rooms `045`.

Movement: none, it does not write a velocity.

### Dialogue Tester `B8`

1 function: `Sprite_B8_DialogueTester`.

Not placed by either table, so something else spawns it.

Movement: none, it does not write a velocity.

### Bully And Pink Ball `B9`

1 function: `Sprite_B9_BullyAndPinkBall`.

Overworld screens `43`.

Movement: a state machine on `sprite_subtype2`, 0 velocity writes.

### Whirlpool `BA`

2 functions: `SpritePrep_Whirlpool`, `Sprite_BA_Whirlpool`.

Overworld screens `0F` `12` `15` `1B` `33` `35` `3F` `55` `7F`.

Movement: none, it does not write a velocity.

### Shopkeeper `BB`

4 functions: `Shopkeeper_Draw`, `Shopkeeper_StandardClerk`, `SpritePrep_Shopkeeper`, `Sprite_BB_Shopkeeper`.

dungeon rooms `0FF` `100` `106` `10F` `110` `112` `118` `11E` `11F` `123` `124` `125`.

Tiles: drawn by `Shopkeeper_Draw`, 2 graphics writes.

Movement: a state machine on `sprite_subtype2`, 0 velocity writes.

### Drunkard `BC`

1 function: `Sprite_BC_Drunkard`.

dungeon rooms `103`.

Tiles: 2 graphics writes.

Movement: none, it does not write a velocity.

Dialogue:

- `175` Whoa... I saw her. A very nice young lady at the Waterfall Of Wishing in the hills where the river begins... Link, you should meet her at least onc...

### 4 Way Shooter `C6`

1 function: `Sprite_C6_4WayShooter`.

dungeon rooms `026` `035` `037` `07B` `07D` `08D` `092` `09B` `0B1` `0B3` `0C1` `0D1`.

Movement: 2 velocity writes.

### Big Fairy `C8`

3 functions: `SpritePrep_BigFairy`, `Sprite_BigFairy`, `Sprite_C8_BigFairy`.

dungeon rooms `008` `02C` `115`.

Tiles: 1 graphics write.

Movement: a state machine on `sprite_ai_state`, 0 velocity writes.

Dialogue:

- `15A` I will sooth your wounds and comfort your weariness... Close your eyes and relax...

### Flopping Fish `D2`

1 function: `Sprite_D2_FloppingFish`.

Overworld screens `3B`.

Tiles: 3 graphics writes.

Movement: a state machine on `sprite_ai_state`, 2 velocity writes.

Dialogue:

- `176` Take some Rupees, but don't tell anyone I gave them to you. Keep it between us, OK?

### Dig Game Guy `D5`

1 function: `Sprite_D5_DigGameGuy`.

Overworld screens `68`.

Tiles: 2 graphics writes.

Movement: a state machine on `sprite_ai_state`, 2 velocity writes.

Dialogue:

- `187` Welcome to the treasure field. The object is to dig as many holes as you can in 30 seconds. Any treasures you dig up will be yours to keep. It's on...
- `188` Then I will lend you a shovel. When you have it in your hand, start digging! (Press the <Y> Button to dig.)
- `189` I see. Then I give up. Save some Rupees and come back.
- `18A` OK! Time's up, game over. Come back again. Good bye...
- `18B` Come back again! I will be waiting for you.
- `18C` I can't tell you details, but it's not a convenient time for me now. Come back here again. Sorry.

### Heart `D8`

8 functions: `HeartUpgrade_CheckIfAlreadyObtained`, `HeartUpgrade_SetObtainedFlag`, `ShopItem_Heart`, `SpritePrep_HeartContainer`, `SpritePrep_HeartPiece`, `Sprite_D8_Heart`, `Sprite_HeartContainer`, `Sprite_HeartPiece`.

Overworld screens `1A` `32`.

Tiles: 2 graphics writes.

Movement: 10 velocity writes.

Dialogue:

- `17C` I'm sorry, but you don't seem to have enough Rupees...

### Green Rupee `D9`

1 function: `Sprite_D9_GreenRupee`.

Not placed by either table, so something else spawns it.

Movement: none, it does not write a velocity.

### Small Key `E4`

2 functions: `SpritePrep_SmallKey`, `Sprite_E4_SmallKey`.

dungeon rooms `087`.

Movement: none, it does not write a velocity.

### Mushroom `E7`

2 functions: `SpritePrep_Mushroom`, `Sprite_E7_Mushroom`.

Overworld screens `00`.

Tiles: 1 graphics write.

Movement: none, it does not write a velocity.

### Fake Sword `E8`

3 functions: `FakeSword_Draw`, `SpritePrep_FakeSword`, `Sprite_E8_FakeSword`.

Overworld screens `00`.

Tiles: drawn by `FakeSword_Draw`.

Movement: none, it does not write a velocity.

Dialogue:

- `06F` This is it! The Master Sword! ... ... ... No, this can't be it... Too bad.

### Potion Shop `E9`

2 functions: `SpritePrep_PotionShop`, `Sprite_E9_PotionShop`.

dungeon rooms `109`.

Movement: a state machine on `sprite_subtype2`, 0 velocity writes.

### Thrown Item `EC`

2 functions: `SpriteDraw_ThrownItem_Gigantic`, `Sprite_EC_ThrownItem`.

Not placed by either table, so something else spawns it.

Tiles: drawn by `SpriteDraw_ThrownItem_Gigantic`.

Movement: none, it does not write a velocity.

### Somaria Platform `ED`

9 functions: `SomariaPlatformAndPipe_HandleMovement`, `SomariaPlatform_DragLink`, `SomariaPlatform_Draw`, `SomariaPlatform_HandleDrag`, `SomariaPlatform_HandleDragX`, `SomariaPlatform_HandleDragY`, `SomariaPlatform_HandleJunctions`, `SomariaPlatform_LocatePath`, `Sprite_ED_SomariaPlatform`.

Not placed by either table, so something else spawns it.

Tiles: drawn by `SomariaPlatform_Draw`.

Movement: a state machine on `sprite_E`, 2 velocity writes.

### Movable Mantle `EE`

2 functions: `MovableMantle_Draw`, `Sprite_EE_MovableMantle`.

dungeon rooms `051`.

Tiles: drawn by `MovableMantle_Draw`.

Movement: 1 velocity write.

### Medallion Tablet `F2`

3 functions: `MedallionTablet_Draw`, `MedallionTablet_Main`, `Sprite_F2_MedallionTablet`.

Overworld screens `03` `30`.

Tiles: drawn by `MedallionTablet_Draw`, 1 graphics write.

Movement: a state machine on `sprite_ai_state`, 0 velocity writes.

## What is not here

Which message a branch picks at run time. Many NPCs choose between several
messages on a flag, an item or a previous answer, and the list above is every
message the handler can reach, not the order it reaches them in.

The tile numbers themselves. A draw function builds its OAM from a table keyed
on the frame, so the graphics a sprite uses are a function of its state rather
than a fixed list.
