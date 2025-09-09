#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"

// 結構大小恰為 20 bytes；欄位與反編譯寫入偏移一一對應
#pragma pack(push, 1)
struct strCoupleRingKindInfo {
    uint16_t ringKind;          // +0  커플링ID -> TranslateKindCode("C0001"...)
    uint16_t textId;            // +2  텍스트 코드（文字ID）
    int32_t  expRatePercent;    // +4  경험치 상승률(%)（整數百分比）
    int32_t  canSummonSpouse;   // +8  배우자 소환여부（0/1 等）
    uint32_t resourceIdHex;     // +12 커플링리소스 ID（十六進位字串以 %x 解析）
    uint16_t blockId;           // +16 블럭아이디
    uint16_t effectKind;        // +18 이펙트파일명（如 E0732 -> TranslateKindCode）
};
#pragma pack(pop)

static_assert(sizeof(strCoupleRingKindInfo) == 20, "strCoupleRingKindInfo must be 20 bytes");

class cltCoupleRingKindInfo {
public:
    cltCoupleRingKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltCoupleRingKindInfo() { Free(); }

    // 對應反編譯：int __thiscall Initialize(this, char* filename)
    int  Initialize(char* filename);

    // 對應反編譯：void __thiscall Free(this)
    void Free();

    // 對應反編譯：strCoupleRingKindInfo* GetCoupleRingKindInfo(this, uint16_t code)
    strCoupleRingKindInfo* GetCoupleRingKindInfo(uint16_t code);

    // 對應反編譯：static uint16_t __cdecl TranslateKindCode(char* s)
    static uint16_t TranslateKindCode(char* s);

    // 便利
    inline const strCoupleRingKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strCoupleRingKindInfo* m_items; // 連續陣列，每筆 20 bytes
    int                    m_count; // 總筆數
};
