# Ground Truth 對照分析（mofclient.c）

## 範圍
- `CMeritoriousSystem`
- `cltTutorialSystem`
- `cltTASSystem`
- `cltCashshopSystem`
- `cltPetKeepingSystem`

Ground truth 來源為 `mofclient.c` 對應函式實作；比對對象為目前 `src/System/*.cpp` 版本。

---

## 1) CMeritoriousSystem

### 結論
**不一致（多處核心邏輯差異）**。

### 主要差異
1. **戰功任務啟動邏輯不同**
   - Ground truth `StartWarMeritoriousQuest` 是接收外部給定的怪物 kind 陣列，逐一寫入內部 map，並將 `warQuestPlaying` 開啟、記錄難度/等級欄位。沒有「依 mapKind+difficulty 自動生成怪物清單」的邏輯。
   - 目前版本若 `warQuestMonCount_ == 0`，會依 `mapKind + i + 1`、`3 + d + i` 自動產生怪物與目標數。

2. **戰功任務完成條件計算方式不同**
   - Ground truth `CanCompleteWarMeritoriousQuest` 會用 `CMonsterGroupPerLevel::GetMonsterKillCount(...)` 減去 emblem 加成修正後再比較需求值。
   - 目前版本是比對本地 `warQuestMonKills_` 與 `warQuestMonGoals_` 陣列，未使用 ground truth 的全域怪物擊殺統計路徑。

3. **戰功獎勵公式不同**
   - Ground truth `GetRewardWarMeritoriousExp` 直接取 `CExpRewardParser::GetMeritoriousRewardExp(目前 war 難度/欄位)`；`CompleteWarMeritoriousQuest` 增加戰功點數為 `emblem優勢 + 10`。
   - 目前版本採「基礎值 + 難度加權」的自訂公式（如 `kWarBaseExp`、`kWarDifficultyExpBonus`、`kWarDifficultyPointBonus`），與 ground truth 常數/來源不同。

4. **補給任務開始與狀態判斷語意不同**
   - Ground truth `StartSupplyMeritoriousQuest` 只設定 `supplyQuestKind`，而 `CanStartSupplyMeritoriousQuest` 在反組譯結果中是以 `word18 != 0` 回傳（語意明顯與命名衝突，但這是 ground truth 目前可見行為）。
   - 目前版本採「戰功任務/補給任務都未進行才可開始」的語意化判斷。

5. **補給任務資料來源與獎勵結算流程不同**
   - Ground truth `CompleteSupplyMeritoriousQuest/CanCompleteSupplyMeritoriousQuest` 直接查 `CSupplyMeritoriousParser` map 的 itemKind / needCount / point / libi。
   - 目前版本混用本地常數表（如 `kSupplyItemNeedByKind`）與較簡化流程，非完全依 parser 資料驅動。

---

## 2) cltTutorialSystem

### 結論
**不一致（目前版本屬高階重構/模擬版，非等價移植）**。

### 主要差異
1. **狀態來源不同**
   - Ground truth `Poll` 用全域 `g_nTutorialState` 驅動。
   - 目前版本使用物件內 `tutorialState_` 與 `tutorialActive_`。

2. **初始化流程不同**
   - Ground truth `InitalizeTutorialSystem` 會大量操作全域系統：快捷鍵、訊息框、`g_clMyCharData` 初始化、`ClientCharacterManager` 實體建立、quest/meritorious reset、UI/help 狀態整合等。
   - 目前版本改為本地化 helper 流程（`BuildProfileFromType`、`BuildInitialWorld` 等），大量全域 side effect 被抽象化/模擬。

3. **戰鬥/撿物/用藥流程細節不同**
   - `AttackMonster`：Ground truth 依真實輸入鍵狀態與角色命令系統 (`SetOrderAttack/PushOrder`)；目前版本是模擬 tick 與本地資料結構扣血。
   - `PickUpItem`：Ground truth 使用場上掉落管理、訊息字串與系統訊息流程、再發獎勵道具；目前版本使用簡化 `drops_` 和距離推進。
   - `UseItem`：兩者都在 HP 門檻後開 timer，但目前版本仍是本地 timer/狀態。

4. **UI 教學步驟推進方式不同**
   - Ground truth 直接呼叫 `CUITutorial::AddTutorial(...)` 並在特定時機遞增全域狀態。
   - 目前版本透過 `AddTutorialStep(...)` 與本地訊息佇列抽象處理。

---

## 3) cltTASSystem

### 結論
**部分一致、但多處公式與資料欄位語意不一致**。

### 主要差異
1. **刪學生費用公式不同（重大）**
   - Ground truth `GetDelStudentCost(level)` 回傳 `level * 100`。
   - 目前版本回傳 `1000 * level`（10 倍）。

2. **可累積師徒經驗百分比公式不同（重大）**
   - Ground truth `GetMaxGatherExpPercent(teacherLv, myLv)`：`max((teacherLv - myLv) * 5, 0)`，上限 100。
   - 目前版本採分段常數（1~5），且比較方向與語意不同。

3. **升級時累積師徒經驗計算不同（重大）**
   - Ground truth `OnLevelUp` 使用 `GetNeedExp(level)` 與百分比乘算後累加，並觸發 20/30 級獎勵金額。
   - 目前版本以每級固定 `p * 100` 累加，未使用 `GetNeedExp`。

4. **Refresh(type=1) 的第一筆資料處理語意不同**
   - Ground truth 第一筆是 teacher 槽：有名則更新 `myTeacherLevel`，無名且既有老師則刪除老師。
   - 目前版本只更新 `teacher_.level`，未完整覆蓋 teacher 名稱/結構（實際語意偏差）。

5. **其餘流程多為結構性重寫**
   - 如 `Initialize`、`SetTeacher`、`AddStudent`、`CanSendPostit` 的資料搬移與容器實作，整體功能輪廓相近，但非逐欄位等價。

---

## 4) cltCashshopSystem

### 結論
**不一致（含數個明顯功能降級）**。

### 主要差異
1. **價格計算來源被替換（重大）**
   - Ground truth `GetBuyingItemAllPrices` 呼叫 `cltCashShopItem::GetTotalPrice(...)`。
   - 目前版本使用 `id % 10000` 的 fallback 規則，非真實商品定價。

2. **Web 組字串來源被替換（重大）**
   - Ground truth `GetBuyingItemStringForWeb` 呼叫 `cltCashShopItem::GetCompositionItemData(...)`。
   - 目前版本僅把 item id 用逗號串接。

3. **搬移已購現金商品到背包的額外值判定不同（中～重大）**
   - Ground truth 依 `cltItemKindInfo::GetItemKindInfo(itemKind)` 的多個屬性位判斷，必要時才帶入 `extraArg`。
   - 目前版本改為 `(itemKind % 7 == 0 || itemKind % 11 == 0)` 的人工規則。

4. **多筆搬移可行性檢查流程不同（中等）**
   - Ground truth 建立暫存 `cltItemList`，逐筆加入後再做 `CanAddInventoryItem` 驗證，處理細節與容器邊界較完整。
   - 目前版本以固定陣列暫存後逐項檢查，流程較簡化。

---

## 5) cltPetKeepingSystem

### 結論
**不一致（封包格式與資料搬移細節差異大）**。

### 主要差異
1. **初始化/序列化格式不同（重大）**
   - Ground truth `Initialize` 與 `FillOutPetKeepingInfo` 只傳「非空 inventory slot」與「實際技能數量」，先寫 count 再逐筆 (slotIndex, kind, qty)。
   - 目前版本固定把 255 格 item 與 100 格技能全量讀寫，格式與 ground truth 不同。

2. **KeepingPet 內容來源不完整（重大）**
   - Ground truth 會把 pet inventory 的 slot index/kind/qty 與技能內容完整複製到 keeping 資料。
   - 目前版本 skills 區塊因缺 accessor 直接填 0（程式內已註記 `// no accessor in current codebase`），導致放養後技能資訊遺失。

3. **CanReleaseKeepingPet 的回傳碼/判斷順序需完全貼 ground truth（中等）**
   - Ground truth：lock 或找不到寵物回傳 1；獎勵金額無法增加金錢回傳 106；包包非空回傳 1900。
   - 目前版本大致接近，但 `moneySystem_` 空指標時也走 106（ground truth 直接假設 money system 存在）。

4. **TakeKeepingPet 的語意仍可疑（結構差異）**
   - Ground truth 是清掉目標 keeping slot 後呼叫 `KeepingPet(nowTime)`；目前版本沿用同樣結構，但因 `KeepingPet` 本身資料來源簡化，還原品質與 ground truth 有落差。

---

## 總結
- 五個系統中，**沒有任何一個可視為完整與 ground truth 一致**。
- 差異最大者：`cltTutorialSystem`（整體架構重寫）、`CMeritoriousSystem`（任務/獎勵核心公式不同）、`cltCashshopSystem`（價格與 web 資料來源被替代）、`cltPetKeepingSystem`（封包格式與技能/道具序列化不同）。
- `cltTASSystem` 雖保留大部分 API 形狀，但關鍵數值公式（刪學生費用、師徒經驗百分比、升級累積）與 ground truth 不同，會直接影響遊戲數值。
