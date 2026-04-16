# MOF 程式碼導覽

這份 README 聚焦在專案的核心模組、資料流與目錄對應關係；根目錄的工具、額外文件與測試程式不在本文範圍內。

## 專案定位

MOF 是一個以 Windows 平台為目標的 MMORPG 用戶端程式碼庫，所有 C++ 程式碼皆由反編譯檔 `mofclient.c` 還原而來。程式進入點為 `main.cpp`（`wWinMain`）。`global.h` / `global.cpp` 為全域單例與管理器核心，串接角色、地圖、網路、UI、特效、音效、字型、小遊戲與資源管理等子系統。資料夾 `inc/` 放置對外介面與型別宣告，`src/` 則提供對應實作。這種一對一的資料夾布局，讓每個模組都能用相同命名快速對照標頭與實作檔。

## 目錄原則

- `inc/`：標頭檔、資料結構、類別宣告、跨模組共用介面。
- `src/`：各模組的 `.cpp` 實作。
- 多數子目錄在 `inc/` 與 `src/` 之間是成對存在，例如 `Character`、`System`、`UI`、`Effect`、`Image`、`MiniGame` 等。
- 執行期會由遊戲主循環調用所有子系統生命週期與互動。
- 目前已還原的程式碼共 749 個標頭/原始檔（不含 Test 與 fmod），約 114,000 行，其中 `src/` 下有 372 個 `.cpp` 實作檔。
- 主要未還原的部分集中在主遊戲邏輯（Monster、NPC、CharacterAnimation、Skill 等戰鬥循環）。

## Quick Start

### Dependency
1. fmod: 負責音效播放，library已經附在程式碼裡。
2. d3dx9: 需要安裝舊版DirectX SDK，安裝方法參考:
	- 下載網址: https://www.microsoft.com/en-us/download/details.aspx?id=6812
	- 安裝前要先解除安裝: Microsoft Visual C++ 2010 x64 Redistribute

### 遊戲資源下載
1. 第一次執行要先把遊戲資源打包成mof.pak和mof.paki
	- 先把mofdata.zip解壓縮，放入專案根目錄。
	- 下載網址:https://drive.google.com/file/d/1ju1TbYRCycJYOiEkXaXSy9MshcT1ANUA/view?usp=sharing
	- 把 create_vfs_archive(); 解除註解。
	- 執行一次打包流程，建立.pak資源檔。


## 核心架構概觀

### 1. 全域狀態與主幹管理器

`inc/global.h` 與 `src/global.cpp` 宣告了整個專案的全域變數：

- 定義 Direct3D 裝置、視窗控制、畫面尺寸、輸入旗標與渲染狀態。
- 集中宣告並實體化主要全域物件，例如：
  - `g_Network`：網路通訊。
  - `g_ClientCharMgr`：角色實例管理。
  - `g_CAManager` / `g_clCAClone`：角色動畫資產管理與裝備外觀 fallback。
  - `g_ObjectManager` / `g_ObjectResourceManager` / `g_DrawObject_ALL`：場景物件與其資源。
  - `g_EffectManager` / `g_pEffectManager_Before_Chr` / `g_pEffectManager_After_Chr`：動態特效生命週期（分角色前後兩層）。
  - `g_EffectManager_MiniGame`：小遊戲專用特效管理器。
  - `g_UIMgr` / `g_pUITutorial` / `g_pMsgBoxMgr`：UI 管理與訊息框。
  - `g_clQuestSystem`、`g_clSpiritSystem`、`g_clClassSystem`、`g_clSkillSystem`、`g_clEquipmentSystem`、`g_clUsingSkillSystem` 等規則子系統。
  - `g_clTextFileManager`、`g_DCTTextManager`、`g_MoFFont`、`g_IMMList`：文字、字型與輸入法基礎設施。
  - `g_clDeviceManager`：Direct3D 裝置管理。
  - `g_GameSoundManager`：音效管理。
  - `g_clMiniGame`：小遊戲基底實例。
  - `g_clPetSystem`、`g_clMoneySystem`、`g_clPetKindInfoBase`：寵物與經濟系統。
  - `g_clMyCharData`：當前玩家角色資料。
- 也提供角色選擇欄位（3 格 × 96 bytes）、小遊戲全域狀態機、鏡頭偏移與渲染用共享資料。

如果要理解系統之間如何接線，建議先從這兩個檔案閱讀。

### 2. 典型資料流

執行時的主要資料流大致如下：

1. **Network** 收到封包並解譯遊戲事件。
2. **System / Info / Logic** 根據封包與設定資料，更新角色、任務、物品、地圖與各種規則狀態。
3. **Character / Object / Effect** 依據最新狀態建構場景上的角色、物件與特效。
4. **Image / Font / Text / Sound** 提供畫面、字型與音效所需的底層資源。
5. **UI** 將角色資訊、背包、技能、任務、商店等內容轉成互動式介面。
6. **MiniGame** 獨立於主循環，管理各職業課程小遊戲的流程與繪製。

## `inc/` 與 `src/` 模組導覽

以下內容依照主要子目錄說明，方便快速理解責任邊界。括號內標示 `src/` 下的 `.cpp` 檔案數。

### `Character/`（7 個實作檔）

角色模組是遊戲世界中的動態主體核心。

- `ClientCharacter` 負責角色本體：位置、血量、狀態、動畫、攻擊與受擊流程、名稱與外觀資料、命令佇列，以及與特效 / UI 的接點。
- `ClientCharacterManager` 管理所有角色實例，包含以帳號或名稱查找角色、設定主角、建立角色、更新裝備與重置移動目標等。
- `CAManager` 為角色動畫資產管理器（應用層單例 `g_CAManager`），儲存從 `.ca` 二進位動畫檔解析出的 dot（世界精靈）與 illust（UI 肖像）動畫資料，並提供 item-ID → 圖層對照表。
- `CCA` 是 dot 層動畫的核心類別（292 bytes），負責逐幀 Process / Draw、裝備圖層切換（`SetItemID`/`LayerPutOn`/`LayerPutOff`）與 fallback 策略（缺件時自動降級至髮型/鞋子等圖層）。
- `CCAClone` 為 CCA 的克隆變體，用於角色預覽或多視角場景。
- `CCAillust` 為 CCA 的 UI 肖像版本（112 bytes），持有 16 個圖層槽，搭配 `ID3DXSprite` 批次繪製。
- `CCANormal` 處理 NPC/怪物等無裝備換裝需求的 CA 動畫。
- `CATypes.h` 定義 `CA_DRAWENTRY`、`FRAMEINFO`、`LAYERINFO`、`TIMELINEINFO`、`ITEMCAINFO_DOT`/`ITEMCAINFO_ILLUST` 等核心資料結構。

如果要追查角色移動、戰鬥表現或玩家/NPC/怪物的共同繪製流程，這裡是第一站。

### `Network/`（3 個實作檔）

網路模組包裝了 MMORPG 客戶端需要的大量封包送收行為。

- `DCTNetwork` 是較底層的通訊基底。
- `CMofMsg` 處理訊息格式與遊戲協定相關資料。
- `CMoFNetwork` 則提供高階遊戲操作 API，例如登入、移動、聊天、攻擊、拾取物品、裝備切換、技能使用、任務互動、交易、組隊與角色建立/刪除。

這個模組是「伺服器事件 → 客戶端狀態更新」的入口，也是「本地操作 → 封包送出」的出口。

### `Info/`（47 個實作檔）

`Info` 主要保存「靜態資料表」或「遊戲資料庫」的讀取結果，是許多系統做規則判斷的基礎。

常見資料類型包括：

- 職業、角色、技能、物品、NPC、地圖、傳送門。
- 任務、課程、寵物、稱號、徽章、附魔、礦點、氣候。
- 世界地圖顏色、外觀、特效種類、婚禮場景、特殊副本。
- 掉落物品、再生怪物、潘朵拉寶箱、問答題庫、復活點等。

這些類別通常對應「某種 KindInfo / Info 檔案」，由其他系統查表使用，而不是直接承擔畫面或流程控制。

### `System/`（53 個實作檔）

`System` 是最龐大的目錄，承接大部分遊戲規則與玩法流程。

代表性子系統包括：

- 角色養成：`cltLevelSystem`、`cltClassSystem`、`cltPlayerAbility`、`cltGradeSystem`。
- 物品與裝備：`cltEquipmentSystem`、`cltUsingItemSystem`、`cltMakingItemSystem`、`cltStorageSystem`、`cltExStorageSystem`、`cltMyItemSystem`。
- 社交互動：`cltPartySystem`、`cltClientPartySystem`、`cltMarriageSystem`、`cltTradeSystem`。
- 寵物與召喚：`cltPetSystem`、`cltPetSkillSystem`、`cltPetInventorySystem`、`cltPetKeepingSystem`、`cltPetMarketMySalePetSystem`。
- 任務與引導：`cltQuestSystem`、`cltTutorialSystem`、`cltHelpSystem`、`CQuizEventSystem`、`cltLessonSystem`。
- 戰鬥延伸狀態：`cltDebuffSystem`、`cltUsingSkillSystem`、`cltSkillSystem`、`cltMonsterToleranceSystem`、`cltTransformSystem`、`cltOverMindSystem`、`cltWorkingPassiveSkillSystem`。
- 經濟與特殊功能：`cltCashshopSystem`、`CPrivateMarketSystem`、`CMeritoriousSystem`、`cltRestBonusSystem`、`cltMoneySystem`。
- 外觀與表情：`cltBasicAppearSystem`、`cltEmoticonSystem`、`cltMoFC_EmoticonSystem`、`cltSexSystem`。
- 其他：`cltEmblemSystem`、`cltSpecialtySystem`、`cltTitleSystem`、`cltPKRankSystem`、`cltCountrySystem`、`cltEnchantSystem`、`cltQuickSlotSystem`、`cltTASSystem`、`cltNPCRecallSystem`、`cltTestingSystem`、`cltMyMsgSystem`、`CSpiritSystem`、`cltMoF_CharacterActionStateSystem`。

可把 `System` 理解成「將資料表與角色狀態轉為可遊玩規則」的類別。

### `Logic/`（36 個實作檔）

`Logic` 負責比 `System` 更偏向共用邏輯、世界狀態與輔助資料結構。

包含：

- `Map`：地圖資料與場景層級邏輯。
- `cltBaseInventory`、`cltItemList`：背包與物品集合操作。
- `cltNPCManager`、`cltFieldItemManager`、`cltFieldItem`：NPC 與地面物件管理。
- `CShortKey`：快捷鍵綁定。
- `DirectInputManager`、`XJoyStick`：DirectInput 鍵盤/搖桿輸入管理。
- `clTransportKindInfo`、`clClientTransportKindInfo`、`clTransportAniInfo`：移動/載具類邏輯。
- `CSpiritSpeechMgr`、`CSpiritSpeech`、`CSpiritGossip`、`CSpiritQuestSpeech`、`SpiritSpeechParser`、`SpiritGossipParser`：精靈語音與閒聊系統。
- `CPlayerSpirit`：玩家精靈邏輯。
- `CMeritoriousParsers`、`CQuizEventParser`：特定內容解析或事件支援。
- `CMonsterGroupPerLevel`：每等級怪物群組資料。
- `cltMyCharData`：當前玩家角色的完整初始化資料。
- `cltChattingMgr`：聊天管理。
- `cltConfiguration`：客戶端設定。
- `cltMapTitle`：地圖標題顯示。
- `cltMoF_ClientMine`、`cltMoF_MineManager`：礦點與採礦管理。
- `cltMatchManager`、`cltPKManager`、`cltPKFlagManager`：對戰與 PK 管理。
- `cltCashShopItem`：商城物品資料。
- `cltMiniGame_Button`：小遊戲按鈕元件。
- `cltSystemMessage`、`cltHelpMessage`：系統與輔助訊息。
- `cltShopInfo`：商店資訊。

`Logic` 是支撐玩法運作的基礎行為層。

### `MiniGame/`（22 個實作檔）

對應遊戲中的「職業課程小遊戲」。

- `cltMoF_BaseMiniGame` 是所有小遊戲的共通基底，提供排名資料、讀秒計時、按鈕焦點管理、送分與請求排名等邏輯。
- `cltMini_Sword` / `cltMini_Sword_2`：劍士小遊戲（兩個版本），狀態機由全域 `g_cGameSwordState` / `g_cGameSword_2State` 管理。
- `cltMini_Exorcist` / `cltMini_Exorcist_2`：驅魔小遊戲（卡片 + 方向鍵應對），狀態機由全域 `g_cGameExorcistState` / `g_cGameExorcist_2State` 管理。
- `CHospital`：醫院小遊戲場景控制器，管理 3x3 病床網格與治療流程。
- `CMedical`：治療包選單與醫療操作。
- `BoardMgr` / `Ani_Board`：棋盤與動畫面板管理。
- `BackGroundMgr`：小遊戲背景管理。
- `InfoMgr`：小遊戲資訊面板。
- `AccuracyBar` / `PowerBar`：精確度與力量條 UI 元件。
- `Mini_AniCtrl`、`Mini_IMG_Number`、`Mini_Speek`、`Mini_Timer`：小遊戲內部的動畫控制、數字顯示、對話與計時器。
- `MedicalKit_Button`、`PatientRecallMgr`、`Sword2`：各遊戲的專用元件。
- `cltMiniGame_DrawNum`：小遊戲分數數字繪製。
- `cltValidMiniGameScore`：分數驗證。

每個小遊戲都繼承 `cltMoF_BaseMiniGame`，以 `Poll()` 虛擬函式驅動每幀更新。

### `Object/`（16 個實作檔）

`Object` 模組管理場景中的非角色實體與其繪製順序。

- `CBaseObject` 提供場景物件基礎能力。
- `CStaticObject`、`CPortalObject`、`CNPCObject`、`CEffectObject`、`clTransportObject` 等負責不同型別的地圖物件。
- `cltNPC_Object`、`cltNPC_Taxi`：NPC 物件與計程車 NPC（多狀態動畫）。
- `cltPetObject`：寵物物件，處理寵物跟隨、升級、染色與特效。
- `cltMoF_SpiritObject`：守護精靈物件，具巡邏/追蹤行為與聊天氣泡。
- `cltQuestMark`：任務標記與獎勵標記物件。
- `cltAniObject`：動畫物件基底。
- `CObjectManager` 使用鏈結結構維護場景物件集合，支援新增、刪除、碰撞、逐幀更新與繪製。
- `CObjectResourceManager` 負責物件資源關聯。
- `DrawObject_All` 與 `cltAnimationObjectManager` 則處理更高層的整體繪製與動畫物件管理。
- `stObjectInfo` 定義場景物件的基礎資料結構（28 bytes）。

這一層讓角色以外的世界元件能穩定加入場景管線。

### `Effect/`（84 個實作檔）

`Effect` 目錄收納大量戰鬥、技能、介面與場景特效實作，是視覺表現的重要部分。

- `CEffectBase` 是特效基底類別。
- `CEffectManager` 維護特效雙向鏈結串列，負責建立、更新、繪製與刪除特效。
- `CEAManager`、`CCAEffect`、`CBulletBase` / `CBulletArrow` 負責不同型態特效資產與投射物。
- `CEffectUseHitMulti`：多段命中特效。
- 戰鬥特效：`CEffect_Battle_BowShoot`、`CEffect_Battle_DownCut`、`CEffect_Battle_GunShoot`、`CEffect_Battle_HorizonCut`、`CEffect_Battle_Pierce`、`CEffect_Battle_StaffShoot`、`CEffect_Battle_TwoHandCut`、`CEffect_Battle_UpperCut`、`CEffect_Battle_MagicBook` 等。
- 技能特效：`CEffect_Skill_Heal`、`CEffect_Skill_Freezing`、`CEffect_Skill_Blocking`、`CEffect_Skill_SpeedUP`、`CEffect_Skill_Resurrection`、`CEffect_Skill_Type_Directed`、`CEffect_Skill_Type_Once`、`CEffect_Skill_Type_Sustain`、`CEffect_Skill_Type_ShootUnit` 等。
- 場景特效：`CEffect_Field_DamageNumber`、`CEffect_Field_ExpNumber`、`CEffect_Field_ItemPick`、`CEffect_Field_Walkdust`、`CEffect_Field_Warp`、`CEffect_Portal`、`CEffect_MapEffect` 等。
- 玩家特效：`CEffect_Player_Dead`、`CEffect_Player_EnchantLevel`、`CEffect_Player_Goggles`、`CEffect_Player_MapConqueror` 等。
- 怪物特效：`CEffect_Mon_DarkBall`、`CEffect_Mon_FireSpark`、`CEffect_Mon_ElecSpark`、`CEffect_Mon_Die_Ghost` 等。
- 小遊戲特效：`CEffect_MiniGame_Class_Sword`、`CEffect_MiniGame_Archer_String`、`CEffect_MiniGame_Priest_Heal`、`CEffect_MiniGame_Wizard_String` 等。
- UI 特效：`CEffect_UI_ButtonSide`、`CEffect_UI_HP_MP_Number`、`CEffect_UI_SkillSet`、`CEffect_UI_WorldMap_Light`。

檔案數量很多，但命名規則清楚：從前綴大多可直接辨識用途與觸發場景。

### `Image/`（16 個實作檔）

`Image` 提供 2D/貼圖/裝置層的圖像資源管理。

- `GameImage` 是主要的影像/精靈操作單位。
- `cltImageManager` 維護固定大小的 `GameImage` 物件池，避免重複配置。
- `ResourceMgr`、`ImageResource`、`ImageResourceListDataMgr`、`TextureListDataMgr` 處理圖像資源與清單。
- `CDeviceManager`、`CDeviceResetManager` 負責與 Direct3D 裝置生命週期相關的工作。
- `VertexBufferDataMgr`、`GIVertex` 支援頂點資料與繪圖幾何。
- `BackGroundImage`：背景影像管理，支援滾動圖片與純黑矩形背景。
- `LoadingThread`：背景載入執行緒。
- `Image.h`：影像頂點結構定義。
- `cltBaseResource`、`cltGIResource`：基底資源與 GameImage 資源封裝。
- `comp.h`：壓縮/解壓縮工具。

可以把這層看成 UI、角色與特效共享的圖像底座。

### `UI/`（62 個實作檔）

UI 是另一個大型模組，涵蓋畫面互動與介面元件組裝。

主要結構可分為：

- **基底控制項**：`CControlBase`、`CControlButtonBase`、`CControlButton`、`CControlImage`、`CControlImageTriple`、`CControlText`、`CControlEditBox`、`CControlCheckButton`。
- **容器與頁籤**：`CControlBoxBase`、`CControlTab`、`CControlTabMgr`、`CControlPageScroll`、`CControlAlphaBox`。
- **捲動與列表**：`CControlScrollBar`、`CControlScrollText`、`CControlCount`、`CControlCountBar`、`CControlCountScrollButton`、`CTextBoxScroll`、`CListMgr`。
- **數值與顯示**：`CControlNumberBox`、`CControlNumberImage`、`CControlMoveText`、`CControlPosButton`。
- **業務型介面元件**：
  - 背包：`CControlBoxInvenItem`、`CControlMyItem`。
  - 技能：`CControlBoxSkill`、`CControlBoxBuySkill`。
  - 任務：`CControlBoxQuest`、`CControlBoxQuestAlarm`。
  - 商店：`CControlBoxShopItem`、`CControlBoxCashShop`、`CControlBoxCashShopStrgItem`。
  - 製作：`CControlBoxMaking`。
  - 課程：`CControlBoxLessonList`、`CControlBoxLessonSchedule`。
  - 能力：`CControlBoxAbility`、`CControlBoxSpecialty`。
  - 裝備：`CControlBoxEquip`。
  - 寵物：`CControlBoxPetInfo`、`CControlBoxBuyPetFeed`。
  - 角色：`CControlBoxCreateChar`、`CControlCharBox`、`CControlCA`。
  - 個人商店：`CControlBoxPrivateShop`。
  - 快速欄：`CControlBoxQSL`。
  - 彈出選單：`CControlBoxPopMenu`。
  - 拍賣相關：`CControlBoxMySAItem`、`CControlBoxSAReceiveItem`。
  - 聊天：`CControlChatPrint`、`CControlChatBallon`。
- **訊息框與提示**：`CMessageBoxManager`、`CUIMessageBoxTypes`、`CToolTip`、`CToolTipManager`、`stToolTipData`。
- **拖曳**：`CMoveIcon`。
- **編輯框背景**：`CEditBoxBackgroundImage`。
- **世界地圖**：`CUIWorldMap`。
- **整體管理**：`CUIManager` 彙整各 UI 子元件，`CUITutorial` 專注於教學引導畫面，`CUIBase` 提供 UI 基底。

從檔名就能看出此模組採「基礎控制項 + 多種業務控制盒」的典型 MMORPG UI 設計。

### `Sound/`（4 個實作檔）

音效模組封裝 Windows / DirectSound 音效播放流程。

- `CWaveFile` 處理波形資料讀取。
- `CSound`、`CStreamingSound`（定義於 `CSoundSystem.h`）負責一般音效與串流音效生命週期。
- `COgg` 補足 OGG 格式支援。
- `GameSound` 則較偏向遊戲層的音效使用入口。

### `Text/`（5 個實作檔）與 `Font/`（7 個實作檔）

這兩個模組共同負責文字顯示能力。

- `Text` 目錄包含 `DCTTextManager`、`cltTextFileManager`、`DCTIMM` / `DCTIMMList` 與 `Huffman`，涵蓋文字資源、輸入法與編碼相關工作。
- `Font` 目錄包含 `MoFFont`、`CMoFFontTextureManager`、`TextInfoListMgr`、`TextLineInfoListMgr`、`FileCrypt`、`FTInfo`、`TILInfo`、`TLILInfo`、`stFontInfo` 等，用於字型貼圖、字型資訊與文字排版資源管理。

只要是畫面上要顯示名字、聊天、提示、介面文案，通常都會走到這兩層。

### `FileSystem/`（2 個實作檔）

這裡聚焦在封裝專案自己的封包/虛擬檔案系統概念。

- `CMOFPacking` 對應資源封裝邏輯（單例模式，提供 `PackFileOpen`、`DataPacking`、`FileReadBackGroundLoading` 等方法）。
- `nfs` 則提供檔案系統層的存取支援。

雖然本文不展開根目錄工具，但就一般程式碼而言，這一層是資源讀取的重要支點。

### `Util/`（3 個實作檔）

輕量共用工具集中在這裡：

- `cltTimer`：時間與節奏控制。
- `StringStream`：字串流工具。
- `ScriptParameter`：腳本參數解析輔助。

### `Other/`（3 個實作檔）

放置無法明確歸入大型模組，但仍與遊戲規則相關的元件，例如：

- `CAttendanceBookParser`：簽到簿解析。
- `cltAttackAtb`：攻擊屬性。
- `cltBaseAbility`：基礎能力值。

### `Helpers/`（1 個實作檔）

- `ExTextCodeHelpers.cpp`：文字代碼輔助函式（`ExGetTextCodeHuntItemType`、`ExGetTextCodeFashionItemType`、`ExGetTextCodeInstantItemType` 等）。

## 模組之間如何互相配合

用開發者角度來看，可以用下面方式理解這套程式碼：

- **`Info` 提供靜態資料**：各種 KindInfo 與表格查詢來源。
- **`System` 套用遊戲規則**：等級、職業、任務、交易、寵物、技能等玩法判斷。
- **`Logic` 保存世界邏輯與輔助狀態**：地圖、背包、NPC、輸入、訊息與快捷鍵。
- **`Character` / `Object` 形成場景內容**：角色（含 CA 動畫系統）與非角色物件進入世界。
- **`MiniGame` 驅動課程小遊戲**：獨立的狀態機與繪製管線。
- **`Effect` / `Image` / `Font` / `Text` / `Sound` 提供表現層**：畫面、文字與音效輸出。
- **`UI` 對玩家暴露操作界面**。
- **`Network` 串接伺服器事件**，將遠端狀態同步進本地各模組。

## 建議閱讀順序

如果是第一次接觸這個專案，建議按以下順序閱讀：

1. `inc/global.h` / `src/global.cpp`：掌握全域管理器與主要依賴。
2. `main.cpp`：了解程式進入點、D3D 初始化與主迴圈架構。
3. `Character/`、`Network/`：了解角色模型（含 CA 動畫系統）與伺服器互動入口。
4. `System/`、`Logic/`、`Info/`：理解遊戲規則如何落地。
5. `Object/`、`Effect/`、`Image/`：掌握場景與視覺表現。
6. `UI/`、`Text/`、`Font/`、`Sound/`：補齊互動與呈現層。
7. `MiniGame/`：了解小遊戲子系統。

## README 範圍聲明

- 本文件**只說明 `inc/` 與 `src/` 的一般程式碼**。
- **不涵蓋測試內容**，也不整理 `src/Test` / `inc/Test`。
- **不介紹根目錄工具鏈、外部文件或其他附帶檔案**。
