# cltHelpSystem / cltSkillSystem / cltPlayerAbility 與 `mofclient.c` 等價性差異分析

> 基準：`mofclient.c`（ground truth）
> 
> 本文件只列出「行為不一致」與「高風險不等價」項目；不在此階段修改程式。

## 1) cltHelpSystem

### A. 事件流程差異
- `OnReceiptedQuest`：現行實作只呼叫 `Check_SetSchedule` 與 `Check_DoLesson`，**缺少** ground truth 的 `Check_GoTo_Curuno`。  
  - ground truth 會在接任務時觸發此提示路徑。  
- `OnMeetNPC`：現行為空函式；ground truth 會呼叫 `Check_GoTo_Rora_For_Reward`。
- `OnLevelUp`：現行多呼叫了 `Check_EquipItem`；ground truth 的 `OnLevelUp` 不含這一步。

### B. 條件判斷差異
- `Check_ReceiptQuest`：現行僅看 `m_needReceiptQuestCheck`；ground truth 還會檢查 `N0015` NPC 相關進行中任務不存在時才彈窗（`GetPlayingQuestInfoByNPCID` 條件）。
- `Check_GoTo_Rora_For_Reward`：現行只要 `Q1501` 就彈；ground truth 還要求該任務存在且完成旗標未完成（`!*((_BYTE*)v3+4)`）。
- `Check_AttackMonster`：現行用 `mapKind == M0004`；ground truth 用 `Map::GetMapCaps(...) == 1`，判定範圍不同。
- `Check_SetScheduleQuestComplete`：現行無條件彈 17；ground truth 會檢查 `Q0001` 是否可領獎勵（`CanReward`）才彈。
- `Check_DoLessonEachSubject`：現行只看地圖就直接彈 18；ground truth 會檢查 `Q0002` 進行狀態與各課程可訓練條件，並依進度彈 18/19/20/21。
- `Check_GoToRoraForRewar`：現行無條件彈 22；ground truth 需要 `Q0002` 條件與四系 lesson total point 都 > 0 才彈。

### C. 未還原（目前為 stub / no-op）
- `Check_Party_Caution1`：現行空實作；ground truth 依隊伍狀態、怪物資料與隊員數決定是否彈 10。  
- `Check_SkillHelper`：現行空實作；ground truth 在特定地圖/等級/技能數時開啟 UI Help 頁面。  
- `Check_MajorHelper`：現行空實作；ground truth 在特定地圖/年級/專長狀態時開啟 UI Help 頁面。

---

## 2) cltSkillSystem

### A. 物件模型與 API 介面不一致
- `Initialize`：ground truth 版本含 `cltTitleSystem*` 參數並直接接收技能陣列；現行介面拆成兩個 overload，且沒有 title system 依賴。
- `AddSkill`：ground truth 簽名為 `AddSkill(skill, int* replaced, uint16_t* replacedKind)`，可覆蓋前置技能（升階替換）；現行只有 `AddSkill(skill)`，**不回傳替換資訊**，且不做「前置技能替換」。

### B. 核心行為差異
- `AddSkill`：ground truth 若新技能有 `prev`，會在已持有清單中替換舊技能；現行一律 `push_back`（只檢查重複/上限），行為不等價。  
- `AddSkill` 觸發事件：ground truth 會呼叫 `cltTitleSystem::OnEvent_acquireskill`；現行缺少。
- `AddSkill` 圓陣回呼：ground truth 回呼傳入 `this`（skill system instance）；現行傳入 circle kind，callback 語義不同。
- `BuySkill`：ground truth 會先扣 lesson 點數（四系），再加技能；現行只 `AddSkill`，**不扣點**。
- `IsAcquireSkill_Run`：ground truth 走 `IsAcquiredSkill(P00001)`（含升階鏈）；現行用 `IsExistSkill(P00001)`（只看直接持有），可能漏判。
- `GetGeneralPartyAdvantageSkillKind`：ground truth 會找出符合條件的被動技能 kind；現行固定回傳 0。

### C. 有條件偏差 / 邊界邏輯差異
- `GetPassiveSkill`：ground truth `workingOnly` 參數會過濾 `!skillInfo[49]`；現行忽略第三參數，僅做 active/passive + validity。  
- `UpdateValidity`：ground truth 檢查特定武器型別需求（對照 `skillInfo +72..` 與裝備 item type）；現行僅檢查「有裝任一有效武器」即可，條件更寬鬆。  
- `CanAcquireSkill`：ground truth 透過 `IsAcquiredSkill` 判重（含升階鏈）而非 `IsExistSkill`，現行可能允許不該可學的同鏈技能。  
- `Initialize` 未見 ground truth 類似 `Free/clear` 的流程語義，現行 vector 初始化流程與固定陣列模型仍有語義差距（特別是保留/覆蓋時序）。

---

## 3) cltPlayerAbility（你訊息中的 `clyPlayerAbility` 應為此類別）

### A. 依賴系統缺失導致大量行為不可等價
- ground truth `Initialize` 依賴多個系統：`CPlayerSpirit`、`cltEmblemSystem`、`cltMonsterToleranceSystem`、`cltRestBonusSystem`、`cltPetSystem`、`cltMyItemSystem` 等；現行版本只保留少數指標，缺少多數來源，導致後續計算行為顯著簡化。

### B. 屬性/能力計算明顯簡化
- `GetStr/GetDex/GetInt/GetVit`：ground truth = base + 裝備 + 使用物品 + 使用技能 + 寵物 + 派對；現行直接回傳 base 值。  
- `GetAPower/GetDPower`：ground truth 為大型綜合公式（包含 class/monster tolerance/equipment/skill/item/emblem/pet/party 等）；現行僅極簡近似（`GetAPower` 幾乎等於 base str，`GetDPower` 只做 `baseVit + a4`）。
- `GetNeedManaForUsingSkill`：ground truth 使用 `SkillManaAdvantage` 計算折減且下限 1；現行幾乎不折減（只做 `max(1, baseSkillMana)`）。

### C. 多個方法為固定值 / 未還原
- 現行固定回傳 0（或 false）的方法，在 ground truth 有完整邏輯：
  - `CanMultiAttack`, `GetAttackSpeedAdvantage`, `GetDamageHP2ManaRate`, `GetDropRateAdvantage`, `GetDeadPenaltyExpAdvantage`, `GetExpAdvantage`, `IsActiveFastRun`, `IsActiveNonDelayAttack`, `GetAttackAtb`
  - `GetItemRecoverHPAdvantage`, `GetItemRecoverManaAdvantage`, `GetMagicResistRateAdvantage`, `GetMagicResistRate`
  - `GetPartyAPowerAdvantage`, `GetPartyDPowerAdvantage`, `GetPartyHitRateAdvantage`
  - `GetCriticalHitRateAdvantage`, `GetMissRateAdvantage`, `GetSkillAPowerAdvantage`
  - `GetShopItemPriceAdvangtage`, `GetSellingAgencyTaxAdvantage`
  - `GetMaxFaintingInfo`, `GetMaxConfusionInfo`, `GetMaxFreezingInfo`（ground truth 會整合 skill/using-skill/emblem 三來源，取最大）
- `GetBuffNum/GetMaxBuffNum/CanAddBuff`：ground truth 動態計算（含 class default + skill add，且上限 10）；現行只看兩個成員變數，未接軌真實狀態。

### D. 最大 HP/MP 公式仍需再確認
- 現行 `GetMaxHP/GetMaxMP` 使用簡化線性公式；ground truth 實際公式在 `cltPlayerAbility` 內含更多上下文（class / 各系統加成），目前不可視為等價。

---

## 4) 結論（本階段）

- `cltHelpSystem`：有多處「條件缺失 / 過度簡化 / 空實作」，與 ground truth **不等價**。  
- `cltSkillSystem`：關鍵流程（加技能替換、買技能扣點、validity 條件、一般隊伍加成技能判定）存在差異，與 ground truth **不等價**。  
- `cltPlayerAbility`：目前屬「大量未還原 + 返回常數」狀態，與 ground truth **不等價**，而且差異範圍最大。

