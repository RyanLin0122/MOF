#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"

// 由反編譯還原的資料結構（總計 28 bytes）
#pragma pack(push, 1)
struct strPKRankKindInfo {
    uint16_t kind;        // 由 "R0001" 之類字串透過 TranslateKindCode 轉為 16-bit
    uint16_t textId;      // 第3欄：名稱文字ID（反編譯以 WORD 儲存）
    int32_t  needPoint;   // 第4欄：所需點數（偏移 +4）
    uint32_t resIdHex;    // 第5欄：資源ID，使用 "%x" 以16進位解析（依反編譯）
    uint16_t blockId;     // 第6欄：Block ID（WORD）
    uint16_t _pad;        // 對齊填補，保持結構 28 bytes
    int32_t  losePenalty; // 第7欄：敗北扣分（可為負）
    int32_t  meritPoint;  // 第8欄：戰功給予
    int32_t  publicPoint; // 第9欄：公績給予(X)
};
#pragma pack(pop)

class cltPKRankKindInfo {
public:
    cltPKRankKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltPKRankKindInfo() { Free(); }

    // 對應反編譯：int __thiscall Initialize(char* filename)
    int Initialize(char* filename);

    // 對應反編譯：void __thiscall Free()
    void Free();

    // 對應反編譯：strPKRankKindInfo* GetPKRankKindInfo(uint16_t code)
    strPKRankKindInfo* GetPKRankKindInfo(uint16_t code);

    // 對應反編譯：uint16_t GetPKRankKindByPoint(int point)
    uint16_t GetPKRankKindByPoint(int point);

    // 對應反編譯：static uint16_t TranslateKindCode(char* s)
    static uint16_t TranslateKindCode(char* s);

    // 便利存取
    inline const strPKRankKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strPKRankKindInfo* m_items; // 指向連續陣列，每筆 28 bytes
    int                m_count; // 總筆數
};
