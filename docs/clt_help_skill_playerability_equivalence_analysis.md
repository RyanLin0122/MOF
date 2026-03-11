# cltHelpSystem / cltSkillSystem / cltPlayerAbility 與 `mofclient.c` 等價性差異分析（現況）

> 基準：`mofclient.c`（ground truth）
>
> 範圍：僅分析差異，**這一版不改程式邏輯**。

## 1) `cltHelpSystem`

### 已對齊（主流程）
以下核心事件流目前與 `mofclient.c` 一致：
- `OnReceiptedQuest` 依序檢查 `SetSchedule` / `DoLesson` / `GoTo_Curuno`。
- `OnMeetNPC` 會檢查 `GoTo_Rora_For_Reward`。
- `OnMapEntered` 會做 `ReceiptQuest/AttackMonster/ViewWorldMap/SeeDiary/DoLessonEachSubject`。
- `OnLevelUp` 會檢查 bonus point / 可學主動技 / 升武器 / 升職。
- `Check_ReceiptQuest`、`Check_GoTo_Rora_For_Reward`、`Check_AttackMonster`、`Check_SetScheduleQuestComplete`、`Check_DoLessonEachSubject`、`Check_GoToRoraForRewar` 的判斷條件目前都已跟上 ground truth。

### 仍不等價差異
1. `Check_Party_Caution1` 仍是空實作。  
   - Ground truth 會在「已組隊」時讀怪物資料，依 `monsterExp / partyMemberNum == 0` 觸發 help 10。
2. `Check_SkillHelper` 仍是空實作。  
   - Ground truth 在 `T0004`、等級=3、技能數<=1 時會打開 UI Help 第 3 頁。
3. `Check_MajorHelper` 仍是空實作。  
   - Ground truth 在 `T0004`、年級=2、且三類專長皆未取得時會打開 UI Help 第 12 頁。

---

## 2) `cltSkillSystem`

### 仍不等價差異
1. `GetPassiveSkill(out, onlyValid, workingOnly)` 的 `workingOnly` 邏輯與 ground truth 相反。  
   - Ground truth：`workingOnly==1` 時應該只回傳 `workingType!=0` 的被動技。  
   - 現況：`workingOnly==0` 時反而排除了 `workingType!=0`，導致預設路徑只拿到「非 working」被動技。
2. `AddSkill` 的「普通新增」分支有額外保護行為。  
   - Ground truth 在 `prev==0` 時直接寫入陣列（不做重複與上限防護）。  
   - 現況有 `!IsExistSkill` 與 `<100` 才 push 的限制。若外部誤調用，行為會不同。

### 其餘重點（目前看起來已對齊）
- `AddSkill` 的前置技能替換、quickslot 通知、circle callback（傳 `this`）、title event、`UpdateValidity` 觸發。
- `BuySkill` 的四系 lesson 點扣除順序。
- `IsAcquireSkill_Run` 使用 `IsAcquiredSkill(P00001)`。
- `UpdateValidity` 的 class mask + 16 種武器需求檢查模型。
- `GetGeneralPartyAdvantageSkillKind` 已實作掃描判定。

---

## 3) `cltPlayerAbility`

### 仍不等價差異（重要）
1. `ResetAbility` 與 `CanResetAbility` 規則不同。  
   - Ground truth：重置後 `STR/DEX/INT/VIT` 都設為 4，`bonusPoint = 5 * (level+1)`，並 `UpdateValidity()`。  
   - 現況：重置為 `initial*`，不會回到固定 4，也不會重算 bonus point。
2. `GetMaxHP` / `GetMaxMP` 少算 emblem 加成。  
   - Ground truth 把 `cltEmblemSystem` 的 MaxHP/MaxMP advantage 納入百分比加成。  
   - 現況只算 skill + equipment + using-item。
3. `GetHPRate` 取樣基準不同。  
   - Ground truth 透過虛擬路徑取「含上下文的最大 HP」（可帶 party 脈絡）。  
   - 現況直接用 `GetMaxHP(m_baseVit)`，未帶 party/最終 vit 計算路徑。
4. `GetAPower` / `GetDPower` 忽略目標 `charKind` 屬性。  
   - Ground truth 會從 `cltCharKindInfo` 取目標 atb（例如 demon 等），並套用對應 AP/DP 與特例加成。  
   - 現況固定 `atb = -1`，`charKind` 幾乎被忽略，導致 atb 特化加成失效（包含 demon 分支）。
5. `GetAttackAtb(int)` 缺少「外部覆寫 attack attribute」分支。  
   - Ground truth 先嘗試由傳入參數（虛擬呼叫）取得 attack atb，失敗才退回武器 atb。  
   - 現況永遠回傳武器 atb。

### 其餘已接近 ground truth 的項目
- `GetStr/GetDex/GetInt/GetVit`（裝備/物品/技能/寵物/隊伍加成匯總）
- `CanMultiAttack`、`GetAttackSpeedAdvantage`、`GetDamageHP2ManaRate`、`GetExpAdvantage`
- `GetBuffNum/GetMaxBuffNum/CanAddBuff`
- `GetNeedManaForUsingSkill`、`GetMaxFainting/Confusion/FreezingInfo`

---

## 結論

- `cltHelpSystem`：主幹流程大致對齊，剩 3 個 helper 仍為 stub。  
- `cltSkillSystem`：主要差異集中在 `GetPassiveSkill` 的 `workingOnly` 判斷（功能性差異）與 `AddSkill` 防護行為。  
- `cltPlayerAbility`：仍有數個核心公式/流程不等價（重置規則、HP/MP 加成來源、AP/DP atb 路徑、AttackAtb 覆寫邏輯）。
