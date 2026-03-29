# MOF 程式碼導覽

這份 README 聚焦在專案的核心模組、資料流與目錄對應關係；根目錄的工具、額外文件與測試程式不在本文範圍內。

## 專案定位

MOF 是一個以 Windows 平台為目標的 MMORPG 用戶端程式碼庫。程式進入點為`main.c`。`global.h` / `global.cpp` 為全域單例與管理器核心，串接角色、地圖、網路、UI、特效、音效、字型與資源管理等子系統。資料夾`inc/` 放置對外介面與型別宣告，`src/` 則提供對應實作。這種一對一的資料夾布局，讓每個模組都能用相同命名快速對照標頭與實作檔。

## 目錄原則

- `inc/`：標頭檔、資料結構、類別宣告、跨模組共用介面。
- `src/`：各模組的 `.cpp` 實作。
- 多數子目錄在 `inc/` 與 `src/` 之間是成對存在，例如 `Character`、`System`、`UI`、`Effect`、`Image` 等。
- 執行期會由遊戲主循環調用所有子系統生命週期與互動。
- 目前開發總進度為 38%，
- 剩餘部分為:
  - 34% UI
  - 8% 小遊戲
  - 20% 主遊戲邏輯 (Character、Monster、NPC、CharacterAnimation、Map、Skill)

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
以下為目前版本的實作。
未來會補齊 `main.c` 的遊戲主循環。

`inc/global.h` 與 `src/global.cpp` 宣告了整個專案的全域變數：

- 定義 Direct3D 裝置、視窗控制、畫面尺寸、輸入旗標與渲染狀態。
- 集中宣告並實體化主要全域物件，例如：
  - `g_Network`：網路通訊。
  - `g_ClientCharMgr`：角色實例管理。
  - `g_ObjectManager` / `g_ObjectResourceManager`：場景物件與其資源。
  - `g_EffectManager`：動態特效生命週期。
  - `g_UIMgr` / `g_pUITutorial`：UI 管理。
  - `g_clQuestSystem`、`g_clSpiritSystem`、`g_clClassSystem` 等規則子系統。
  - `g_clTextFileManager`、`g_DCTTextManager`、`g_MoFFont`：文字與字型基礎設施。
- 也提供角色選單、教學流程、鏡頭偏移與渲染用共享資料。

如果要理解系統之間如何接線，建議先從這兩個檔案閱讀。

### 2. 典型資料流

執行時的主要資料流大致如下：

1. **Network** 收到封包並解譯遊戲事件。
2. **System / Info / Logic** 根據封包與設定資料，更新角色、任務、物品、地圖與各種規則狀態。
3. **Character / Object / Effect** 依據最新狀態建構場景上的角色、物件與特效。
4. **Image / Font / Text / Sound** 提供畫面、字型與音效所需的底層資源。
5. **UI** 將角色資訊、背包、技能、任務、商店等內容轉成互動式介面。

## `inc/` 與 `src/` 模組導覽

以下內容依照主要子目錄說明，方便快速理解責任邊界。

### `Character/`

角色模組是遊戲世界中的動態主體核心。

- `ClientCharacter` 負責角色本體：位置、血量、狀態、動畫、攻擊與受擊流程、名稱與外觀資料、命令佇列，以及與特效 / UI 的接點。
- `ClientCharacterManager` 管理所有角色實例，包含以帳號或名稱查找角色、設定主角、建立角色、更新裝備與重置移動目標等。

如果要追查角色移動、戰鬥表現或玩家/NPC/怪物的共同繪製流程，這裡是第一站。

### `Network/`

網路模組包裝了 MMORPG 客戶端需要的大量封包送收行為。

- `DCTNetwork` 是較底層的通訊基底。
- `CMofMsg` 處理訊息格式與遊戲協定相關資料。
- `CMoFNetwork` 則提供高階遊戲操作 API，例如登入、移動、聊天、攻擊、拾取物品、裝備切換、技能使用、任務互動、交易、組隊與角色建立/刪除。

這個模組是「伺服器事件 → 客戶端狀態更新」的入口，也是「本地操作 → 封包送出」的出口。

### `Info/`

`Info` 主要保存「靜態資料表」或「遊戲資料庫」的讀取結果，是許多系統做規則判斷的基礎。

常見資料類型包括：

- 職業、角色、技能、物品、NPC、地圖、傳送門。
- 任務、課程、寵物、稱號、徽章、附魔、礦點、氣候。
- 世界地圖顏色、外觀、特效種類、婚禮場景、特殊副本等。

這些類別通常對應「某種 KindInfo / Info 檔案」，由其他系統查表使用，而不是直接承擔畫面或流程控制。

### `System/`

`System` 是最龐大的目錄，承接大部分遊戲規則與玩法流程。

代表性子系統包括：

- 角色養成：`cltLevelSystem`、`cltClassSystem`、`cltPlayerAbility`。
- 物品與裝備：`cltEquipmentSystem`、`cltUsingItemSystem`、`cltMakingItemSystem`、`cltStorageSystem`。
- 社交互動：`cltPartySystem`、`cltClientPartySystem`、`cltMarriageSystem`、`cltTradeSystem`。
- 寵物與召喚：`cltPetSystem`、`cltPetSkillSystem`、`cltPetInventorySystem`、`cltPetKeepingSystem`。
- 任務與引導：`cltQuestSystem`、`cltTutorialSystem`、`cltHelpSystem`、`CQuizEventSystem`。
- 戰鬥延伸狀態：`cltDebuffSystem`、`cltUsingSkillSystem`、`cltMonsterToleranceSystem`、`cltTransformSystem`。
- 經濟與特殊功能：`cltCashshopSystem`、`CPrivateMarketSystem`、`CMeritoriousSystem`、`cltRestBonusSystem`。

可把 `System` 理解成「將資料表與角色狀態轉為可遊玩規則」的主戰場。

### `Logic/`

`Logic` 負責比 `System` 更偏向共用邏輯、世界狀態與輔助資料結構。

包含：

- `Map`：地圖資料與場景層級邏輯。
- `cltBaseInventory`、`cltItemList`：背包與物品集合操作。
- `cltNPCManager`、`cltFieldItemManager`：NPC 與地面物件管理。
- `CMessageBoxManager`、`CShortKey`：互動輔助與快捷鍵。
- `clTransportKindInfo`、`clClientTransportKindInfo`、`clTransportAniInfo`：移動/載具類邏輯。
- `CSpiritSpeechMgr`、`CMeritoriousParsers`、`CQuizEventParser`：特定內容解析或事件支援。

若 `System` 是玩法規則，`Logic` 就更像支撐玩法運作的基礎行為層。

### `Object/`

`Object` 模組管理場景中的非角色實體與其繪製順序。

- `CBaseObject` 提供場景物件基礎能力。
- `CStaticObject`、`CPortalObject`、`CNPCObject`、`CEffectObject`、`clTransportObject` 等負責不同型別的地圖物件。
- `CObjectManager` 使用鏈結結構維護場景物件集合，支援新增、刪除、碰撞、逐幀更新與繪製。
- `CObjectResourceManager` 負責物件資源關聯。
- `DrawObject_All` 與 `cltAnimationObjectManager` 則處理更高層的整體繪製與動畫物件管理。

這一層讓角色以外的世界元件能穩定加入場景管線。

### `Effect/`

`Effect` 目錄收納大量戰鬥、技能、介面與場景特效實作，是視覺表現的重要部分。

- `CEffectBase` 是特效基底類別。
- `CEffectManager` 維護特效雙向鏈結串列，負責建立、更新、繪製與刪除特效。
- `CEAManager`、`CCAEffect`、`CBulletBase` / `CBulletArrow` 負責不同型態特效資產與投射物。
- 其餘如 `CEffect_Battle_*`、`CEffect_Skill_*`、`CEffect_Player_*`、`CEffect_Field_*`、`CEffect_UI_*` 等檔案，對應具體效果實作。

檔案數量很多，但命名規則清楚：從前綴大多可直接辨識用途與觸發場景。

### `Image/`

`Image` 提供 2D/貼圖/裝置層的圖像資源管理。

- `GameImage` 是主要的影像/精靈操作單位。
- `cltImageManager` 維護固定大小的 `GameImage` 物件池，避免重複配置。
- `ResourceMgr`、`ImageResource`、`ImageResourceListDataMgr`、`TextureListDataMgr` 處理圖像資源與清單。
- `CDeviceManager`、`CDeviceResetManager` 負責與 Direct3D 裝置生命週期相關的工作。
- `VertexBufferDataMgr`、`GIVertex` 支援頂點資料與繪圖幾何。

可以把這層看成 UI、角色與特效共享的圖像底座。

### `UI/`

UI 是另一個大型模組，涵蓋畫面互動與介面元件組裝。

主要結構可分為：

- **基底控制項**：`CControlBase`、`CControlButtonBase`、`CControlImage`、`CControlText`、`CControlEditBox`。
- **容器與頁籤**：`CControlBoxBase`、`CControlTab`、`CControlTabMgr`、`CControlPageScroll`。
- **業務型介面元件**：背包、技能、任務、商店、製作、課程、能力、裝備等各式 `CControlBox*` 類別。
- **整體管理**：`CUIManager` 彙整各 UI 子元件，`CUITutorial` 專注於教學引導畫面。

從檔名就能看出此模組採「基礎控制項 + 多種業務控制盒」的典型 MMORPG UI 設計。

### `Sound/`

音效模組封裝 Windows / DirectSound 音效播放流程。

- `CWaveFile` 處理波形資料讀取。
- `CSound`、`CStreamingSound`、`CSoundManager` 負責一般音效與串流音效生命週期。
- `COgg` 補足 OGG 格式支援。
- `GameSound` 則較偏向遊戲層的音效使用入口。

### `Text/` 與 `Font/`

這兩個模組共同負責文字顯示能力。

- `Text` 目錄包含 `DCTTextManager`、`cltTextFileManager`、`DCTIMM` / `DCTIMMList` 與 `Huffman`，涵蓋文字資源、輸入法與編碼相關工作。
- `Font` 目錄包含 `MoFFont`、`CMoFFontTextureManager`、`TextInfoListMgr`、`TextLineInfoListMgr`、`FileCrypt` 等，用於字型貼圖、字型資訊與文字排版資源管理。

只要是畫面上要顯示名字、聊天、提示、介面文案，通常都會走到這兩層。

### `FileSystem/`

這裡聚焦在封裝專案自己的封包/虛擬檔案系統概念。

- `CMOFPacking` 對應資源封裝邏輯。
- `nfs` 則提供檔案系統層的存取支援。

雖然本文不展開根目錄工具，但就一般程式碼而言，這一層是資源讀取的重要支點。

### `Util/`

輕量共用工具集中在這裡：

- `cltTimer`：時間與節奏控制。
- `StringStream`：字串流工具。
- `ScriptParameter`：腳本參數解析輔助。

### `Other/`

放置無法明確歸入大型模組，但仍與遊戲規則相關的元件，例如：

- `CAttendanceBookParser`
- `cltAttackAtb`
- `cltBaseAbility`

這些檔案多半扮演特定功能支援或資料解析角色。

## 模組之間如何互相配合

用開發者角度來看，可以用下面方式理解這套程式碼：

- **`Info` 提供靜態資料**：各種 KindInfo 與表格查詢來源。
- **`System` 套用遊戲規則**：等級、職業、任務、交易、寵物、技能等玩法判斷。
- **`Logic` 保存世界邏輯與輔助狀態**：地圖、背包、NPC、訊息與快捷鍵。
- **`Character` / `Object` 形成場景內容**：角色與非角色物件進入世界。
- **`Effect` / `Image` / `Font` / `Text` / `Sound` 提供表現層**：畫面、文字與音效輸出。
- **`UI` 對玩家暴露操作界面**。
- **`Network` 串接伺服器事件**，將遠端狀態同步進本地各模組。

## 建議閱讀順序

如果是第一次接觸這個專案，建議按以下順序閱讀：

1. `inc/global.h` / `src/global.cpp`：掌握全域管理器與主要依賴。
2. `Character/`、`Network/`：了解角色模型與伺服器互動入口。
3. `System/`、`Logic/`、`Info/`：理解遊戲規則如何落地。
4. `Object/`、`Effect/`、`Image/`：掌握場景與視覺表現。
5. `UI/`、`Text/`、`Font/`、`Sound/`：補齊互動與呈現層。

## README 範圍聲明

- 本文件**只說明 `inc/` 與 `src/` 的一般程式碼**。
- **不涵蓋測試內容**，也不整理 `src/Test` / `inc/Test`。
- **不介紹根目錄工具鏈、外部文件或其他附帶檔案**。
