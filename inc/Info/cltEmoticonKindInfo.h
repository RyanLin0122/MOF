#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"

// 188 bytes：對應反編譯中單列寫入的固定偏移
#pragma pack(push, 1)
struct stEmoticonWordInfo {
    uint16_t nameId;        // +0  이모티콘 이름（數字）
    uint16_t repItemKind;   // +2  代表道具 Ixxxx（0 表示無）
    uint16_t itemKind;      // +4  道具 Ixxxx（0 表示無）
    uint16_t _padA;         // +6  對齊
    uint32_t kind;          // +8  이모티콘 종류（數字）
    uint32_t resIdHex;      // +12 資源ID（以 %x 讀入）
    uint16_t blockId;       // +16 區塊ID
    uint16_t tooltipId;     // +18 Tooltip文字ID
    uint16_t wordsTextId;   // +20 「表現詞列表」的文字ID
    char     word0[16];     // +22 第一個詞
    char     word1[16];     // +38
    char     word2[16];     // +54
    char     word3[16];     // +70
    char     word4[16];     // +86
    char     word5[16];     // +102
    char     word6[16];     // +118
    char     word7[16];     // +134
    char     word8[16];     // +150
    char     word9[16];     // +166
    uint16_t _padB;         // +182 對齊
    uint32_t wordCount;     // +184 詞數
};
#pragma pack(pop)

// 984 bytes：一個表情項目由 5 筆 stEmoticonWordInfo 打包，含前置表與代表ID
#pragma pack(push, 1)
struct stEmoticonItemInfo {
    uint16_t idList[20];         // +0   20 個「아이템 아이디」(Ixxxx轉碼)
    uint16_t repItemKind;        // +40  代表道具 Ixxxx（群組鍵）
    uint16_t _pad;               // +42  對齊
    stEmoticonWordInfo slot[5];  // +44  五筆 188 bytes（每筆一列）
};
#pragma pack(pop)

static_assert(sizeof(stEmoticonWordInfo) == 188, "stEmoticonWordInfo size must be 188 bytes");
static_assert(sizeof(stEmoticonItemInfo) == 984, "stEmoticonItemInfo size must be 984 bytes");

class cltEmoticonKindInfo {
public:
    cltEmoticonKindInfo();
    virtual ~cltEmoticonKindInfo();

    // 解析 EmoticonList.txt（跳過前三行表頭）
    int  Initialize(char* filename);

    // 釋放
    void Free();

    // 查詢介面
    stEmoticonItemInfo* GetEmoticonItemInfoByIndex(uint8_t idx);
    stEmoticonItemInfo* GetEmoticonItemInfoByKind(int kind);
    stEmoticonItemInfo* GetEmoticonItemInfoByID(uint16_t itemIdCode);

    // ✅ 補上：依 kind 取得「單筆詞資訊」的入口（對應 .cpp 實作）
    stEmoticonWordInfo* GetEmoticonWordInfoByKind(int kind);

    // 組數（反編譯的 *((this)+4)）
    int  GetEmoticonItemCnt();

    // 檢查「아이템 아이디（轉碼後）」是否存在於任一組
    int  IsEmoticonItem(uint16_t itemIdCode);

    // 與其它 *KindInfo 相同規則的轉碼（thiscall 版本）
    uint16_t TranslateKindCode(char* s);

private:
    // 將「188B 列陣列」打包成「984B 組陣列」
    int InitEmoticonItem();

private:
    // ✅ 這兩個成員是 .cpp 會用到的：
    stEmoticonWordInfo* m_rows = nullptr; // 原始列（188B）連續緩衝
    int                 m_rowCnt = 0;       // 原始列數
    stEmoticonItemInfo* m_items = nullptr; // ✅ 組裝後（984B）連續緩衝
    int                 m_itemCnt = 0;       // ✅ 組數
};
