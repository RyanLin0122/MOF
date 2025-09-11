#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "global.h"
#include "Info/cltItemKindInfo.h"

#pragma pack(push, 1)
// 每筆製作配方（精準對齊反編譯：大小 = 60 bytes，步距 60）
struct strMakingItemKindInfo
{
    // 0x00
    uint16_t MakingID;        // 제조 ID         // 製造ID（A**** → 16位代碼）
    // 0x02
    uint16_t ResultItemID;    // 생산 결과 아이템 ID // 產出物品ID（H****）
    // 0x04
    int32_t  Category;        // 분류             // 分類（>=1000 且 千位為偶數者會建立索引）
    // 0x08
    int32_t  ProduceCount;    // 생산 개수        // 生產數量 (>0)
    // 0x0C
    int32_t  SuccessPermyriad;// 성공확률         // 成功機率（萬分比，<=10000）
    // 0x10
    int32_t  ExpBonus;        // EXP보너스        // 經驗加成（<100000）

    // 0x14 ~ 0x3B：10 組材料（每組 4 bytes：2B 物品代碼 + 2B 數量）
    struct {                   // 아이템_i / 아이템_i_수량 // 材料i / 數量
        uint16_t Kind;         // 物品代碼（G****）
        uint16_t Count;        // 數量（可為 0）
    } Ingredient[10];
};
static_assert(sizeof(strMakingItemKindInfo) == 60, "strMakingItemKindInfo must be 60 bytes");

#pragma pack(pop)

class cltMakingItemKindInfo
{
public:
    cltMakingItemKindInfo() : table_(nullptr), count_(0), index_(nullptr), indexCount_(0) {}
    ~cltMakingItemKindInfo() { Free(); }

    // 初始化：讀取 making.txt，成功回傳 1；任何解析失敗回傳 0
    int Initialize(char* filename);

    // 釋放所有配置
    void Free();

    // 依 제조ID（MakingID）取得配方
    strMakingItemKindInfo* GetMakingItemKindInfo(uint16_t makingId);

    // 依「材料#1 的物品代碼」取得配方（僅在分類千位為偶數且 >=1000 的配方中搜尋）
    strMakingItemKindInfo* GetMakingItemKindInfoByItemID(uint16_t firstIngredientKind);

    // 與反編譯一致的 5 碼 KindCode 轉換（A/N/G/H + 4位數，組成 16位代碼）
    static uint16_t TranslateKindCode(char* s);

    //（除錯用）取得資料與數量
    const strMakingItemKindInfo* data() const { return table_; }
    int count() const { return count_; }

private:
    // 反編譯中用到 cltItemKindInfo::TranslateKindCode，內部以等價實作替代
    static uint16_t TranslateItemKindCode(const char* s);
    static bool     IsDigitString(const char* s);

private:
    strMakingItemKindInfo* table_;      // 配方表
    int                     count_;      // 配方數
    strMakingItemKindInfo** index_;      // 索引：僅收錄 Category>=1000 且 (Category/1000)%2==0 的配方
    int                     indexCount_; // 索引數
};
