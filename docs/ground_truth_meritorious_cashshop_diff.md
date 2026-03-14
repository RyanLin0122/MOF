# Ground Truth 對照報告（mofclient.c）

## 比對目標
- CExpRewardParser
- CMeritoriousGradeParser
- CMeritoriousRewardParser
- CSupplyMeritoriousParser
- cltCashShopItem

## 結論總覽

### 一致
- `CExpRewardParser`：目前版本在初始化流程（含三行表頭跳過、逐行檢查編號、讀取兩個 EXP 欄位、EOF 才回傳成功）與讀值邏輯上，與 `mofclient.c` 一致。
- `CSupplyMeritoriousParser`：目前版本在欄位解析順序、數字驗證、`TranslateKindCode`、`GetSupplyMeritoriousID`（seed + rand + 取 map 第 N 個）皆與 `mofclient.c` 一致。
- `CMeritoriousRewardParser`：初始化、欄位對映與查詢邏輯皆與 `mofclient.c` 一致。
- `CMeritoriousGradeParser`：初始化、欄位對映與升階計算邏輯皆與 `mofclient.c` 一致。

### 不一致（僅 cltCashShopItem）
1. **`ParsingItemType` 字串表不一致（重大）**
   - Ground truth 的 one-time 類型字串為：
     - `ONETIME_FUNCTIONAL`
     - `ONETIME_ONETIME`
     - `ONETIME_MAPUSE`
     - `ONETIME_COSMETIC`
     - `ONETIME_DYE`
   - 目前版本改成：
     - `ONETIME_FUNCTIONAL`
     - `ONETIME_AVATAR`
     - `ONETIME_EMOTICON`
     - `ONETIME_ETC`
     - `ONETIME_DYE`
   - 這會造成類型 21~23 的輸入文字與 ground truth 不相容，可能導致解析失敗或分類錯誤。

2. **`CheckCopyItemExistInNonCopyItem` 掃描範圍差異（中等）**
   - Ground truth 只對「連續記憶體中的前段（以 pointer +72 bytes 前進）」做檢查，遇到找不到即停止。
   - 目前版本改為迭代 `items_` vector 的每個元素（以 sellID 查找），雖邏輯目標相同，但在資料有空洞/排列差異時，停下來的條件與時機可能不同。

3. **`Initialize` 的容器/記憶體實作不同（低，屬結構差異）**
   - Ground truth 使用手動配置連續陣列（每筆 72 bytes）與 vector-like 指標管理。
   - 目前版本使用 `std::vector<stCashShopItemList>` 與 per-item `new`，屬重構差異；功能流程大致一致。

## 逐項說明

### CExpRewardParser
- `Initialize`：皆先跳過三行，再計數資料行、配置儲存體、回到資料起點逐行解析；只有在讀到 EOF 正常結束時回傳成功。`GetCircleQuizRewardExp`/`GetMeritoriousRewardExp` 都做同樣的邊界檢查後取值。

### CSupplyMeritoriousParser
- `Initialize`：欄位順序一致（ID kind code、skip 一欄、item kind code、count、point、libi），數字欄位都先檢查 `IsDigit`。
- `GetSupplyMeritoriousID`：皆是 `srand(timeGetTime())` 後 `rand()%size`，再走訪 map 第 N 個元素。
- `TranslateKindCode`：同一個 5 字元 kind code 轉換公式。

### CMeritoriousRewardParser
- `Initialize`：索引欄需是數字且槽位尚未被佔用；之後讀 kind code、require point、quantity。
- 查詢：以 item kind 線性掃描找資料，取 require point 與 item kind 的方式一致。

### CMeritoriousGradeParser
- `Initialize`：索引欄需是數字且槽位尚未被佔用；之後讀 needPoint、skip 一欄、rank、nextGrade、emblem。
- `CalcMeritoriousGrade`：邏輯皆為「point>0 且 grade!=10 時，找 nextGrade==grade+1 的資料，若 needPoint<=point 則升階，不然維持原階且 emblem=0」。

### cltCashShopItem（差異重點）
- 主要流程（Initialize / CheckPeriodByItemKind / ParsingPrice / TranslateKindCode）基本一致。
- 真正行為差異集中在 `ParsingItemType` 的類型字串映射（21~23）與 `CheckCopyItemExistInNonCopyItem` 的掃描方式。
