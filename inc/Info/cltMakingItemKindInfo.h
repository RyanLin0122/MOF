#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"
#include "Info/cltItemKindInfo.h"

#pragma pack(push, 1)
// strMakingItemKindInfo  /  제조 정보 레코드  /  製作配方記錄
//
// 反編譯位置：mofclient.c:309494（Initialize）每筆 record 寫入點固定偏移；
// 步距 60 bytes（v7 += 60，且 operator new(60 * count) 配置）。
// 所有 DWORD 欄位於 32-bit x86 都自然對齊，使用 #pragma pack(1) 不會改變 layout。
//
// txt 欄位（making.txt）：
//   ┌─ index ─┬─ 韓文（標頭）          ─┬─ 中文                  ─┬─ 約束 ─┐
//   │   0     │ 제조 ID                  │ 製造 ID（A****）        │  5碼   │
//   │   1     │ 생산 결과 아이템 ID      │ 產出物品 ID（H****）    │ 非 0   │
//   │   2     │ 분류                     │ 分類                    │ atoi   │
//   │   3     │ 생산 개수                │ 生產數量                │ >0     │
//   │   4     │ 성공확률                 │ 成功機率（萬分比）      │ 1..10000│
//   │   5     │ EXP보너스                │ EXP 加成                │ <100000│
//   │ 6..25   │ 아이템_i / 아이템_i_수량 │ 材料 i / 材料 i 數量    │ 10 組  │
//
struct strMakingItemKindInfo
{
    // +0x00  WORD  제조 ID                /  製造 ID（A**** → 16-bit kindCode）
    uint16_t MakingID;

    // +0x02  WORD  생산 결과 아이템 ID    /  產出物品 ID（H****）
    uint16_t ResultItemID;

    // +0x04  DWORD 분류                   /  分類
    //        若 (>=1000 且 (Category/1000)%2 == 0) 即被收入 index_ 陣列，
    //        以供 GetMakingItemKindInfoByItemID 反查（mofclient.c:309584）。
    int32_t  Category;

    // +0x08  DWORD 생산 개수              /  生產數量（必須 > 0）
    int32_t  ProduceCount;

    // +0x0C  DWORD 성공확률               /  成功機率（萬分比，1..10000）
    int32_t  SuccessPermyriad;

    // +0x10  DWORD EXP보너스              /  EXP 加成（必須 < 100000）
    int32_t  ExpBonus;

    // +0x14 ~ +0x3B：10 組材料，每組 4 bytes（2B Kind + 2B Count）
    //   Ingredient[i].Kind  位於 +0x14 + 4*i  /  材料 i 的物品代碼（G****，可為 0）
    //   Ingredient[i].Count 位於 +0x16 + 4*i  /  材料 i 的數量（可為 0）
    struct {
        uint16_t Kind;   // 아이템_i        /  材料 i 物品代碼
        uint16_t Count;  // 아이템_i_수량   /  材料 i 數量
    } Ingredient[10];

    // sizeof = 0x3C (60) — mofclient.c:309558  operator new(60 * count_)
};
static_assert(sizeof(strMakingItemKindInfo) == 60, "strMakingItemKindInfo must be 60 bytes");
static_assert(offsetof(strMakingItemKindInfo, MakingID)         == 0x00, "");
static_assert(offsetof(strMakingItemKindInfo, ResultItemID)     == 0x02, "");
static_assert(offsetof(strMakingItemKindInfo, Category)         == 0x04, "");
static_assert(offsetof(strMakingItemKindInfo, ProduceCount)     == 0x08, "");
static_assert(offsetof(strMakingItemKindInfo, SuccessPermyriad) == 0x0C, "");
static_assert(offsetof(strMakingItemKindInfo, ExpBonus)         == 0x10, "");
static_assert(offsetof(strMakingItemKindInfo, Ingredient)       == 0x14, "");

#pragma pack(pop)

// cltMakingItemKindInfo  /  제조 정보 관리자  /  配方表管理器
//
// 反編譯位置：mofclient.c:309482（ctor）/ 309494（Initialize）/ 309698（Free）/
//             309718（GetMakingItemKindInfo）/ 309739（GetMakingItemKindInfoByItemID）/
//             309760（TranslateKindCode）。
//
// Class layout（32-bit, 每欄位 DWORD）：
//   +0x00  table_       strMakingItemKindInfo*    配方陣列（總計 60 * count_ bytes）
//   +0x04  count_       int                        配方數
//   +0x08  index_       strMakingItemKindInfo**    分類過濾後的索引陣列
//   +0x0C  indexCount_  int                        index_ 元素數
//
// 注意：mofclient.c constructor 並未 *((DWORD*)this + 2) = 0；i.e. index_ 不被
// 顯式初始化，僅依賴 BSS 零初始化（global instance 才安全）。本還原版於 ctor
// 中明確將 index_ 設為 nullptr，行為對 g_clMakingItemKindInfo（global）等價。
class cltMakingItemKindInfo
{
public:
    // mofclient.c:309482  ctor
    cltMakingItemKindInfo()
        : table_(nullptr)
        , count_(0)
        , index_(nullptr)
        , indexCount_(0)
    {}

    // mofclient.c 沒有顯式 dtor 反編譯出來；但 Free 必為解構實作。
    ~cltMakingItemKindInfo() { Free(); }

    // mofclient.c:309494  Initialize
    //
    // 讀取 making.txt（EUC-KR 文字檔）。回傳 1 = 成功；0 = 解析失敗。
    // 失敗時 GT 不釋放已配置的 table_（為了與反編譯一致，此處也保留同樣行為），
    // 由後續的 Free()/dtor 釋放。
    int Initialize(char* filename);

    // mofclient.c:309698  Free
    void Free();

    // mofclient.c:309718  GetMakingItemKindInfo
    //   依 MakingID（제조 ID）線性掃描 table_。
    strMakingItemKindInfo* GetMakingItemKindInfo(uint16_t makingId);

    // mofclient.c:309739  GetMakingItemKindInfoByItemID
    //   依「Ingredient[0].Kind（首樣材料）」線性掃描 index_（僅含
    //   Category>=1000 且 (Category/1000)%2==0 的配方）。
    strMakingItemKindInfo* GetMakingItemKindInfoByItemID(uint16_t firstIngredientKind);

    // mofclient.c:309760  TranslateKindCode
    //   反編譯與 cltItemKindInfo::TranslateKindCode 邏輯完全相同
    //   （strlen==5 → ((toupper(s[0])+31)<<11) | atoi(s+1)），保留為獨立 static
    //   以對應 g_clItemKindInfo 載入流程外的呼叫者（如 cltItemKindInfo.cpp 解析
    //   m_AddedCraftItemCode）。
    static uint16_t TranslateKindCode(char* s);

    // 除錯／驗證用 accessor — mofclient.c 本身無這些 getter，
    // 但僅供 analysis/making_kindinfo_verify.cpp 在 IMPL 模式下檢查用。
    const strMakingItemKindInfo* data()       const { return table_; }
    int                          count()      const { return count_; }
    const strMakingItemKindInfo* const* indexData()  const { return index_; }
    int                          indexCount() const { return indexCount_; }

private:
    // mofclient.c:342909  IsDigit(char*)（global 函式，這裡內聯為 static 私有）
    //
    // 與標準 isdigit 不同：
    //   - 空字串返回 true（GT 行為）
    //   - 每次迭代允許跳過一個 '+' 或 '-'（非僅起始位置）
    static bool IsDigit(const char* s);

private:
    // +0x00 配方陣列（operator new(60 * count_)）
    strMakingItemKindInfo*  table_;
    // +0x04 配方數
    int                     count_;
    // +0x08 索引陣列（operator new(4 * indexCount_)，存指標）
    strMakingItemKindInfo** index_;
    // +0x0C 索引元素數
    int                     indexCount_;
};
