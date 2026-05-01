#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

class cltItemKindInfo;

#pragma pack(push, 1)

// strPandoraKindInfo  /  판도라 정보 레코드  /  潘朵拉抽獎組記錄
//
// 反編譯位置：mofclient.c:318269（Initialize）每筆 record 寫入點固定偏移；
// 步距 124 bytes（v21 += 62 _WORD = 124 bytes，且 operator new(124 * count)）。
// 每組（GroupID）含 10 個獎勵欄位（slot），每個 slot 12 bytes。
// 32-bit x86 自然對齊；使用 #pragma pack(1) 可保證 layout 與反編譯記憶體存取
// 一致（例如 *((WORD*)v5 + 6 * v6 + 4) 對應 slot 內 ItemID 偏移）。
//
// pandoraitem.txt 欄位（每行 = 1 + 10 * 5 = 51 columns，TAB 分隔）：
//   col  0     :  그룹 ID                /  群組 ID（A**** 5碼 → 16-bit）
//   col 1+5*i :  확률(슬롯i+1)           /  抽中此 slot 的機率（萬分比）
//   col 2+5*i :  아이템 ID               /  獎勵物品 ID（5碼 → 16-bit）
//   col 3+5*i :  아이템 명               /  獎勵物品名稱（解析時跳過）
//   col 4+5*i :  최소수량                /  最小數量（>0；若機率非 0）
//   col 5+5*i :  최대수량                /  最大數量（>=最小；若機率非 0）
//
// 機率欄為 0 時，該 slot 視為「空」，後續四欄忽略不檢查（但 strtok 仍消費 token）。
struct strPandoraSlot
{
    // +0x00  DWORD  확률                  /  機率（萬分比；GenerateItem 用 rand()%10000 判定）
    int32_t  Probability;

    // +0x04  WORD   아이템 ID             /  獎勵物品代碼（cltItemKindInfo::TranslateKindCode）
    uint16_t ItemID;

    // +0x06  WORD   최소수량              /  最小數量
    uint16_t MinCount;

    // +0x08  WORD   최대수량              /  最大數量（>= MinCount）
    uint16_t MaxCount;

    // +0x0A  WORD   padding（對齊到 12 bytes / 下一 DWORD 4-byte 邊界）
    uint16_t _pad;
};
static_assert(sizeof(strPandoraSlot) == 12,                         "strPandoraSlot must be 12 bytes");
static_assert(offsetof(strPandoraSlot, Probability) == 0x00,        "");
static_assert(offsetof(strPandoraSlot, ItemID)      == 0x04,        "");
static_assert(offsetof(strPandoraSlot, MinCount)    == 0x06,        "");
static_assert(offsetof(strPandoraSlot, MaxCount)    == 0x08,        "");

struct strPandoraKindInfo
{
    // +0x00  WORD  그룹 ID                /  群組 ID（A**** → 16-bit kindCode；GetPandoraKindInfo 鍵）
    uint16_t GroupID;

    // +0x02  WORD  padding（GT 將 entry 視為 _WORD[62]；首兩個 WORD 中只用第 0 個，
    //                       第 1 個是對齊空缺，因為 slot 0 的 Probability(DWORD) 從 +0x04 開始）
    uint16_t _pad;

    // +0x04 ~ +0x7B：10 個 slot（每個 12 bytes，由 (DWORD prob + 3*WORD + WORD pad) 組成）
    //   Slots[i].Probability 位於 +0x04 + 12*i
    //   Slots[i].ItemID      位於 +0x08 + 12*i
    //   Slots[i].MinCount    位於 +0x0A + 12*i
    //   Slots[i].MaxCount    位於 +0x0C + 12*i
    strPandoraSlot Slots[10];

    // sizeof = 0x7C (124) — mofclient.c:318314  operator new(124 * count_)
};
static_assert(sizeof(strPandoraKindInfo) == 124,                    "strPandoraKindInfo must be 124 bytes");
static_assert(offsetof(strPandoraKindInfo, GroupID) == 0x00,        "");
static_assert(offsetof(strPandoraKindInfo, Slots)   == 0x04,        "");

#pragma pack(pop)

// cltPandoraKindInfo  /  판도라 정보 관리자  /  潘朵拉抽獎表管理器
//
// 反編譯位置：
//   mofclient.c:207151  ctor (_E408)
//   mofclient.c:207163  dtor (_E409)
//   mofclient.c:318263  InitializeStaticVariable
//   mofclient.c:318269  Initialize（讀 pandoraitem.txt）
//   mofclient.c:318398  Free
//   mofclient.c:318409  GetPandoraKindInfo（依 GroupID 線性掃描）
//   mofclient.c:318430  GenerateItem（依機率隨機產出物品/數量）
//   mofclient.c:318457  TranslateKindCode（5 碼 → 16-bit）
//
// Class layout（32-bit, 每欄位 DWORD）：
//   +0x00  table_   strPandoraKindInfo*    抽獎組陣列（總計 124 * count_ bytes）
//   +0x04  count_   int                    抽獎組數
//
// 解析時為了一致性，static m_pclItemKindInfo 用以呼叫 IsValidItem（slot 物品有效性）。
class cltPandoraKindInfo
{
public:
    // mofclient.c:207151  ctor — 僅 zero-init（SDK 反編譯未顯式列出，依 BSS/作用一致重建）
    cltPandoraKindInfo()
        : table_(nullptr)
        , count_(0)
    {}

    // mofclient.c:207163  dtor — 透過 _E409 → ~cltPandoraKindInfo，呼叫 Free
    ~cltPandoraKindInfo() { Free(); }

    // mofclient.c:318263  InitializeStaticVariable
    //   保存 cltItemKindInfo 指標供 Initialize 中 IsValidItem 使用
    static void InitializeStaticVariable(cltItemKindInfo* a1);

    // mofclient.c:318269  Initialize — 讀取 pandoraitem.txt
    //
    // 流程（與 GT 完全一致）：
    //   1. 開檔；若失敗回傳 0
    //   2. 連續呼叫 fgets 三次，跳過 3 行 header（comment、index 行、欄位中文）
    //   3. fgetpos 紀錄位置；while(fgets) ++count 計算行數
    //   4. operator new(124 * count) 並 memset 0
    //   5. fsetpos 回到 header 後；逐行解析。每行解析失敗（不含 EOF）→ 回 0
    //   6. 全部行解析完且 fgets 回 NULL → 回 1
    //
    // 每行解析（最多 10 個 slot）：
    //   - 首 token：GroupID = TranslateKindCode（不檢查 IsDigit）
    //   - 每 slot 5 token：
    //       prob   : IsDigit + atoi
    //       item   : cltItemKindInfo::TranslateKindCode（不檢查 IsDigit）
    //       name   : 略過（消費 1 token）
    //       min    : IsDigit + atoi
    //       max    : IsDigit + atoi
    //   - 若 prob != 0：item 必有效、min 必非 0、max 非 0 且 max >= min
    int Initialize(char* String2);

    // mofclient.c:318398  Free
    void Free();

    // mofclient.c:318409  GetPandoraKindInfo — 依 GroupID 線性搜尋
    strPandoraKindInfo* GetPandoraKindInfo(uint16_t groupId);

    // mofclient.c:318430  GenerateItem — 對指定 GroupID 抽獎一次
    //
    //   *outItem  = slot.ItemID
    //   *outCount = MinCount + rand() % (MaxCount - MinCount + 1)
    //
    //   依序檢查 10 個 slot；prob == 0 視為空 slot；若連續 10 個皆 (rand%10000 >= prob)
    //   則視為失敗回 0。命中第 v6 個 slot 後立即回 1。
    int GenerateItem(uint16_t groupId, uint16_t* outItem, uint16_t* outCount);

    // mofclient.c:318457  TranslateKindCode
    //   strlen 必須 == 5；hi = (toupper(s[0]) + 31) << 11；lo = atoi(s+1)。
    //   lo 必 < 0x800，否則回 0。回傳 (hi | lo)。
    //   注意：簽章為 char*，不接受 const char*（與反編譯 prototype 完全一致）。
    static uint16_t TranslateKindCode(char* a1);

    // 除錯／驗證用 accessor — mofclient.c 本身無這些 getter
    const strPandoraKindInfo* data()  const { return table_; }
    int                       count() const { return count_; }

private:
    // mofclient.c:342909  IsDigit(char*)
    //   - 空字串 → true
    //   - 每次迭代允許跳過一個 '+' 或 '-'（非僅起始位置）
    //   - 否則檢查 isdigit；通過則前進
    static bool IsDigit(const char* s);

private:
    // +0x00  抽獎組陣列（operator new(124 * count_)；Free 中 operator delete）
    strPandoraKindInfo*  table_;

    // +0x04  抽獎組數
    int                  count_;

    // 反編譯位置：mofclient.c:25386
    static cltItemKindInfo* m_pclItemKindInfo;
};
