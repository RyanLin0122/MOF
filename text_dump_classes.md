# text_dump 檔案解析類別對照表

整理 `D:\Work\text_dump\` 內 531 個 `.txt` 檔案，依據 `mofclient.c`（反組譯源碼）追蹤其載入點，列出負責解析（呼叫 `Initialize` / `Load*`）的 C++ 類別。

---

## 1. 直接由 `Initialize()` 載入的主資料檔

下列檔案在 `mofclient.c` 內以字串字面值傳入對應類別，多數於遊戲啟動流程（`mofclient.c:209522`–`210187` 區段）依序載入。

| 檔名 | 解析類別 | 反編譯位置 |
|---|---|---|
| `MoFTexts.txt` | `DCTTextManager::Initialize` | mofclient.c:209522 |
| `EXP.txt` | `cltLevelSystem::InitializeStaticVariable` | mofclient.c:209578 |
| `ClassKindInfo.txt` | `cltClassKindInfo::Initialize` | mofclient.c:209619 |
| `CA_Character_Dot.txt` | `CAManager::LoadCADataDot` | mofclient.c:209649 |
| `CA_Character_illustration.txt` | `CAManager::LoadCADataIllust` | mofclient.c:209656 |
| `ObjectList.txt` | `CObjectResourceManager::Initialize` | mofclient.c:209726 |
| `npclist.txt` | `cltNPCInfo::Initialize` | mofclient.c:209739 |
| `shoplist.txt`, `packageshoplist.txt` | `cltShopInfo::Initialize` | mofclient.c:209746 |
| `p_skillinfo.txt` | `cltSkillKindInfo::Initialize`（被動技能） | mofclient.c:209763 |
| `LessonKindInfo.txt` | `cltLessonKindInfo::Initialize` | mofclient.c:209793 |
| `question.txt` | `cltQuestionKindInfo::Initialize` | mofclient.c:209803 |
| `TelDragon.txt` | `cltTeleportDragon::Initialize` | mofclient.c:209811 |
| `CircleQuest.txt` | `CCircleQuestParser::Initialize` | mofclient.c:209818 |
| `CircleQuizInfo.txt` | `CCircleQuizParser::Initialize` | mofclient.c:209825 |
| `CircleQuestCondition.txt` | `CCircleQuestConditionParser::Initialize` | mofclient.c:209832 |
| `specialty.txt` | `cltSpecialtyKindInfo::Initialize` | mofclient.c:209891 |
| `making.txt` | `cltMakingItemKindInfo::Initialize` | mofclient.c:209900 |
| `DCTAbuseWords.txt`, `DCTextTFCAW.txt` | `DCTAbuseWordManager::Initialize` | mofclient.c:209911 |
| `mine.txt`（程式內字面值 `Mine.txt`） | `cltMoF_MineManager::Initialize` | mofclient.c:209941 |
| `transformation.txt` | `cltTransformKindInfo::LoadTransformKindInfo` | mofclient.c:209948 |
| `spirit_script.txt` | `CSpiritSpeechParser::Initialize` | mofclient.c:209980 |
| `spirit_R_script.txt` | `CSpiritGossipParser::Initialize` | mofclient.c:209987 |
| `AnimationObjectList.txt` | `cltAnimationObjectManager::Initialize` | mofclient.c:209994 |
| `CashShopList.txt` | `cltCashShopItem::Initialize` | mofclient.c:210003 |
| `levelreward.txt` | `CExpRewardParser::Initialize` | mofclient.c:210050 |
| `EmblemList.txt`（程式內字面值 `Emblemlist.txt`） | `cltEmblemKindInfo::Initialize` | mofclient.c:210063 |
| `pandoraitem.txt` | `cltPandoraKindInfo::Initialize` | mofclient.c:210077 |
| `EmoticonList.txt` | `cltEmoticonKindInfo::Initialize` | mofclient.c:210094 |
| `titlekindinfo.txt` | `cltTitleKindInfo::Initialize` | mofclient.c:210101 |
| `monstertolerance.txt` | `cltMonsterToleranceKindInfo::Initialize` | mofclient.c:210117 |
| `mapuseiteminfo.txt` | `cltMapUseItemInfoKindInfo::Initialize` | mofclient.c:210135 |
| `petkindinfo.txt` | `cltPetKindInfo::Initialize`（含 `PetDyeKindInfo.txt`） | mofclient.c:210141 |
| `pkrank.txt` | `cltPKRankKindInfo::Initialize` | mofclient.c:210151 |
| `myitemkindinfo.txt` | `cltMyItemKindInfo::Initialize` | mofclient.c:210156 |
| `weddinghallkindinfo.txt` | `cltWeddingHallKindInfo::Initialize` | mofclient.c:210166 |
| `Character_ChatBallon.txt`（程式內字面值 `character_chatballon.txt`） | `cltMoF_ChatBallonInfo::Initialize` | mofclient.c:210181 |
| `transportkindinfo.txt`（程式內字面值 `TransportKindInfo.txt`） | `clTransportKindInfo::Initialize` | mofclient.c:210185 |
| `MapInfo.txt`（程式內字面值 `mapinfo.txt`） | `cltMapInfo::Initialize` | mofclient.c:185435 |
| `ClimateKindInfo.txt` | `cltClimateKindInfo::Initialize` | mofclient.c:185438 |
| `weddinginfo.txt` | `cltMoFC_MarriageScript::Initialize` | mofclient.c:154356 |
| `uihelper.txt` | `CUIHelper::Initialize` | mofclient.c:61531 |
| `Eventpage.txt` | `CUILobbySelChannel::InitNoticeInfo` | mofclient.c:66351 |
| `IntroImage.txt` | 直接於 `GsInitInitialize()` 內以 `cltTextFileManager::fopen` 讀取（無專屬類別） | mofclient.c:369673 |

### 1.1 `cltItemKindInfo::Initialize`（一次接收六個檔名）

`mofclient.c:209635` 一次傳入下列檔名：

| 檔名 | 用途 |
|---|---|
| `itemkindinfo.txt` | 主物品 KindInfo |
| （未在 dump 內）`item_instant.txt` | 即時消耗類 |
| （未在 dump 內）`item_hunt.txt` | 狩獵相關物品 |
| `Old_item_fashion.txt` 對應 `item_fashion.txt` 路徑（程式內字面值 `item_fashion.txt`） | 時裝物品 |
| （未在 dump 內）`koreatext.txt` | 物品韓文文字 |
| `trainningCardInfo.txt`（程式內字面值 `TrainningCardInfo.txt`） | 訓練卡 |

> 註：`Old_itemkindinfo.txt` 與 `Old_item_fashion.txt` 為舊版備份檔，欄位格式與 `itemkindinfo.txt` / `item_fashion.txt` 相同，由同一個 `cltItemKindInfo::Initialize` 解析（dump 中是舊版資料）。

---

## 2. 字符／怪物動畫檔（`cltMonsterAniInfo`）

來源：在 `cltCharKindInfo::InitMonsterAinFrame`（mofclient.c:293288）與 `cltClientCharKindInfo::GetMonsterAniInfo`（mofclient.c:26340）內，
針對每一筆 `charkindinfo.txt` 中登錄的角色，以該行第 9 欄記錄的動畫檔名呼叫
`cltMonsterAniInfo::Initialize(filename)`（mofclient.c:315685）。
玩家角色（`cltCharKindInfo::IsPlayerChar` 為真）共用同一類，只是不在 `InitMonsterAinFrame` 預載而是按需載入。

格式特徵：每行為 `STOP|MOVE|DIE|ATTACK|...|ATTACK_KEY` 動畫表（每個 token 為 `[ResourceID,FrameIdx,X,Y]`）。

下列為 dump 中屬於此類的檔案（共 ~349 個）：

### 2.1 玩家／通用
- `player.txt`
- `player2.txt`

### 2.2 一般怪物／NPC 動畫（依字母排序）
- `Auger.txt`, `BEARWOLF.txt`, `BRO.txt`, `Baitert.txt`, `Bampdecon.txt`, `Big pudding.txt`
- `Crimson.txt`
- `Demoncho.txt`, `Devil sfowl.txt`, `Devlinguner.txt`, `Devlinsowe.txt`, `Devlinwalker.txt`, `Drkimble.txt`
- `FLOWERW.txt`
- `Gooul.txt`, `Greencgo.txt`, `Grimm totem.txt`
- `HONET.txt`, `HONETQ.txt`
- `KnightScrach.txt`
- `LIFEWOOD01.txt`
- `M01.txt`, `M02.txt`
- `Mole.txt`, `Mon-Basillik.txt`, `Mon-Boerloog.txt`, `Mon-Litlebear.txt`, `Mon-Tigerbelos.txt`
- `NAITO.txt`, `Necromancer.txt`, `Nightmare.txt`
- `OU.txt`
- `lightakuring.txt`, `lingus.txt`, `littlegrass.txt`
- `m-trigu.txt`, `maclomonLv1.txt`, `mad.txt`, `maddoll.txt`, `magabite.txt`, `magicer.txt`, `magling.txt`
- `maruta.txt`, `mas.txt`, `material.txt`, `me.txt`, `mentawood.txt`
- `mon-Qgami.txt`, `mon-Sbook.txt`, `mon-clabS.txt`, `mon-fast.txt`, `mon-haroB.txt`, `mon-mira.txt`,
  `mon-mosterseed.txt`, `mon-speter.txt`, `mon-swordS.txt`
- `mon_00.txt`
- `moongchi.txt`, `moongchiBLAKC.txt`, `moosongee.txt`
- `mura.txt`, `mushroom.txt`
- `nail.txt`, `nightwing.txt`
- `ougu.txt`, `ouking.txt`
- `panter.txt`, `pantem_Opisyun.txt`, `poisonflower.txt`, `pore.txt`, `potaball.txt`, `pumkinbi.txt`, `qwiny.txt`
- `raccoon dog.txt`, `red pudding.txt`, `redmong.txt`, `redstar.txt`
- `rijadman.txt`, `robe.txt`, `rorypaiper.txt`
- `saenasia.txt`, `sam.txt`, `sand.txt`, `sateba.txt`, `scalfury.txt`, `scull dragon.txt`, `sentry.txt`
- `sk.txt`, `snake.txt`, `snowgaino.txt`, `sodbaster.txt`, `sode.txt`
- `spider.txt`, `spine tortoise.txt`, `spirit.txt`（粒度為怪物動畫；非 `Spirit.txt`）, `spiter.txt`, `srame-2.txt`
- `ssabba.txt`, `stom2.txt`, `stone Horror.txt`, `stuffed specimen.txt`, `sungnnight.txt`, `swa.txt`, `sword.txt`
- `tokisasoo.txt`, `tortoise.txt`, `totem.txt`
- `treasurebox 01.txt`, `treasurebox 02.txt`, `treasurebox bronze.txt`, `treasurebox gold.txt`, `treasurebox silver.txt`
- `ubisca.txt`
- `valley.txt`, `vamp maid.txt`, `vetuttle.txt`, `vosxjatpdlff.txt`
- `wairy.txt`, `wedding_nor_treasurebox.txt`, `wedding_pre_treasurebox.txt`
- `wid_red.txt`, `wild pig.txt`, `woodworker wolf.txt`, `woolvarian.txt`, `work.txt`
- `yerowmong.txt`, `yugol.txt`

> 註：上述「怪物動畫」這一群檔案，最終全部都是透過 `cltMonsterAniInfo::Initialize` 解析；它們之所以不出現在 `mofclient.c` 字串字面值中，是因為檔名實際儲存在 `charkindinfo.txt` 的欄位裡，由 `cltClientCharKindInfo` 在跑表時動態傳入。

---

## 3. 寵物動畫檔（`cltPetAniInfo`）

來源：`cltClientPetKindInfo::GetPetAniInfo`（mofclient.c:195454）讀取 `petkindinfo.txt` 中該寵物對應的動畫檔欄位後，呼叫 `cltPetAniInfo::Initialize(filename)`（mofclient.c:195247）。

格式同節 2（`STOP|MOVE|DIE|...|ATTACK_KEY`），但首欄資源 ID 多為 `0f000xxx`、`120000xx`、`5a0000xx`、`35000xxx` 等寵物 atlas。

dump 中此類共 181 個，以下列前綴／命名規則歸屬：

| 前綴／群組 | 範例 | 數量 |
|---|---|---|
| `Pet-PenPenLV*` | `Pet-PenPenLV01.txt` … `Pet-PenPenLV05(04).txt` | 20 |
| `Pet-RedDragon*` / `Pet-RedDragonLV*` | `Pet-RedDragon Lv 3.txt`, `Pet-RedDragonLV05.txt` … | 36 |
| `Pet-Teddy-LV*` | `Pet-Teddy-LV01-01.txt` … `Pet-Teddy-LV05-04.txt` | 20 |
| `Pet-TinkerbellLv*` | `Pet-TinkerbellLv01(02).txt` … `Pet-TinkerbellLv8(04).txt` | 18 |
| `Pet-rozen-lv*` | `Pet-rozen-lv01-emerald.txt` … `Pet-rozen-lv02-redpink.txt` | 8 |
| `pet-rozen-lv*`（小寫） | `pet-rozen-lv03-emerald.txt` … `pet-rozen-lv05-redpink.txt` | 12 |
| `pet-deviljean-*` | `pet-deviljean-lv01-emerald.txt` … `pet-deviljean-lv05-redpink.txt` | 20 |
| `pet_Robots_*` | `pet_Robots_lv01_emerald.txt` … `pet_Robots_lv05_redpink.txt` | 20 |
| `tinkerbell LV*`（無 `Pet-` 前綴） | `tinkerbell LV01.txt` … `tinkerbell LV9 (04).txt` | 16 |
| 其他 | `vampirit pet.txt` | 1 |

> 全部 181 檔均透過 `cltPetAniInfo::Initialize` 解析。

---

## 4. 由 `cltMapInfo::Initialize` 間接觸發或關聯的檔案

下列類別的 `Initialize` 在 `mofclient.c` 內以變數（非字面值）呼叫，且為 `cltMapInfo`／地圖資料的延伸：

| 檔名 | 解析類別（推定） | 來源 |
|---|---|---|
| `mapareakindinfo.txt` | `cltMapAreaKindInfo::Initialize` | inc/Info/cltMapAreaKindInfo.h；mofclient.c:310172 |
| `opposition.txt` | `cltExtraRegenMonsterKindInfo::Initialize` | inc/Info/cltExtraRegenMonsterKindInfo.h（欄位完全吻合：ID/Level/MonsterID/X/Y/W/H/RegenType…） |
| `resurrectinfo.txt` | `cltResurrectInfo::Initialize` | mofclient.c:329662；經 `cltMapInfo::Initialize` 內 `cltResurrectInfo::TranslateKindCode` 引用（mofclient.c:311008） |

---

## 5. dump 中無直接 `Initialize` 字面值、但類別已在反編譯確立的檔案

| 檔名 | 解析類別（依 dump 內標頭欄位與類別語意推定） | 備註 |
|---|---|---|
| `CAData.txt` | `CAManager`（item→CA 資源對應表） | 欄位 `ItemID/FileID/FileName/LayerIndex/Kind`，被 `LoadCADataDot`/`LoadCADataIllust` 路徑下的子載入器使用 |
| `Old_itemkindinfo.txt` | `cltItemKindInfo::Initialize`（舊版欄位） | 標頭明示「舊版 아이템 종류 정보」 |
| `Old_item_fashion.txt` | `cltItemKindInfo::Initialize`（舊版時裝欄位） | 標頭明示「舊版 패션 아이템 정보」 |
| `ma_skillkindinfo.txt` | 地圖區域被動／加成類技能 KindInfo（推定 `cltSkillKindInfo` 或同檔家族延伸；標頭：「맵액티브스킬」） | mofclient.c 與 src/inc 均無字面值；屬 Map-Area Active Skill |
| `mp_skillkindinfo.txt` | 地圖區域被動類技能 KindInfo（標頭：「맵패시브스킬」，含使睡眠/變形機率欄位） | 同上：Map-Area Passive Skill |
| `systemcommandinfo.txt` | 聊天系統命令解析器（與 `cltChattingMgr` 相關；dump 標頭：「시스템 커맨드」） | mofclient.c 內無字面值；應為運行期延遲載入 |
| `wovtext.txt` | 推定走 `DCTTextManager` 系列／玩家事件訊息 | 標頭：「미수집인 이벤트 메시지」 |
| `a_skillinfo.txt` | `cltSkillKindInfo::Initialize`（主動技能） | 與 `p_skillinfo.txt` 同呼叫，mofclient.c:209763（`a_skillinfo.txt` 不在 dump，但歸屬同一類） |

---

## 6. 摘要：所有解析類別清單

```
CAManager                     ← CAData / CA_Character_Dot / CA_Character_illustration
CAttendanceBookParser         ← (參考；dump 內無對應檔)
CCircleQuestConditionParser   ← CircleQuestCondition
CCircleQuestParser            ← CircleQuest
CCircleQuizParser             ← CircleQuizInfo
CExpRewardParser              ← levelreward
CMeritoriousGradeParser       ← achievement_castelist (不在 dump)
CMeritoriousRewardParser      ← achievement_compensation (不在 dump)
CObjectManager                ← EffectObjectList (不在 dump)
CObjectResourceManager        ← ObjectList
CQuizEventParser              ← quizinfo (不在 dump)
CSpiritGossipParser           ← spirit_R_script
CSpiritSpeechParser           ← spirit_script
CSpiritSystem                 ← Spirit (不在 dump；spirit.txt 是怪物動畫檔)
CSupplyMeritoriousParser      ← achievement_thing (不在 dump)
CUIHelper                     ← uihelper
CUILobbySelChannel            ← Eventpage
DCTAbuseWordManager           ← DCTAbuseWords / DCTextTFCAW
DCTTextManager                ← MoFTexts
GsInitInitialize (函式)        ← IntroImage
cltAnimationObjectManager     ← AnimationObjectList
cltCashShopItem               ← CashShopList
cltClassKindInfo              ← ClassKindInfo
cltClimateKindInfo            ← ClimateKindInfo
cltCoupleRingKindInfo         ← coupleringkindinfo (不在 dump)
cltDebuffKindInfo             ← debuffinfo (不在 dump)
cltEmblemKindInfo             ← EmblemList
cltEmoticonKindInfo           ← EmoticonList
cltEnchantKindInfo            ← Enchant (不在 dump)
cltExtraRegenMonsterKindInfo  ← opposition
cltInstantDungeonKindInfo     ← indunkindinfo (不在 dump)
cltInstantDungeonMapKindInfo  ← indunmapkindinfo (不在 dump)
cltItemKindInfo               ← itemkindinfo / item_fashion / TrainningCardInfo / Old_itemkindinfo / Old_item_fashion / item_instant / item_hunt / koreatext
cltLessonKindInfo             ← LessonKindInfo
cltLevelSystem                ← EXP
cltMakingItemKindInfo         ← making
cltMapAreaKindInfo            ← mapareakindinfo
cltMapInfo                    ← MapInfo
cltMapUseItemInfoKindInfo     ← mapuseiteminfo
cltMoFC_MarriageScript        ← weddinginfo
cltMoF_ChatBallonInfo         ← Character_ChatBallon
cltMoF_MineManager            ← mine
cltMoF_NameTagInfo            ← character_nametag (不在 dump)
cltMonsterAniInfo             ← player / player2 / 全部怪物動畫檔（節 2）
cltMonsterToleranceKindInfo   ← monstertolerance
cltMyItemKindInfo             ← myitemkindinfo
cltNPCInfo                    ← npclist
cltPKRankKindInfo             ← pkrank
cltPandoraKindInfo            ← pandoraitem
cltPetAniInfo                 ← 全部寵物動畫檔（節 3）
cltPetKindInfo                ← petkindinfo (+ PetDyeKindInfo, 不在 dump)
cltPetSkillKindInfo           ← petskillkindinfo (不在 dump)
cltPortalInfo                 ← PortalList (不在 dump)
cltQuestKindInfo              ← QuestKindInfo + Collection/Delivery/Hunt/OnewayDelivery/IndunInfo (不在 dump)
cltQuestionKindInfo           ← question
cltRegenMonsterKindInfo       ← RegenMonsterInfo (不在 dump)
cltResurrectInfo              ← resurrectinfo
cltShopInfo                   ← shoplist / packageshoplist
cltSkillKindInfo              ← p_skillinfo (+ a_skillinfo, 不在 dump) / ma_skillkindinfo / mp_skillkindinfo
cltSpecialtyKindInfo          ← specialty
cltTeleportDragon             ← TelDragon
cltTitleKindInfo              ← titlekindinfo
cltTransformKindInfo          ← transformation
clTransportKindInfo           ← transportkindinfo
cltWeddingHallKindInfo        ← weddinghallkindinfo
```

---

## 7. 統計

- text_dump 總檔案數：**531**
- `cltMonsterAniInfo` 解析（怪物／角色動畫）：約 **349**
- `cltPetAniInfo` 解析（寵物動畫）：**181**
- 主資料檔（節 1）：約 **44**（其中部分檔名於程式內為其他大小寫拼法，如 `Mine`/`mine`、`EmblemList`/`Emblemlist`、`character_chatballon`/`Character_ChatBallon`）

> 有少量主資料檔在 `mofclient.c` 內被 `Initialize`，但 dump 並未包含：`charkindinfo.txt`、`PortalList.txt`、`SoundListInfo.txt`、`SkillEffect.txt`、`EffectObjectList.txt`、`a_skillinfo.txt`、`Enchant.txt`、`QuestKindInfo.txt`（與五個 Quest 子表）、`RegenMonsterInfo.txt`、`character_nametag.txt`、`PetDyeKindInfo.txt`、`PetSkillKindInfo`、`debuffinfo.txt`、`coupleringkindinfo.txt`、`indun*kindinfo.txt`、`achievement_*.txt`、`quizinfo.txt`。
