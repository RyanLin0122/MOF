#pragma once
#include <cstdint>
#include <cstdio>
#include "global.h"
#include "Info/cltMapInfo.h"

#pragma pack(push, 1)
// 一筆正好 20 bytes；偏移完全對應反編譯的寫入位置
struct strMapAreaKindInfo {
    uint16_t areaKind;  // +0  區域ID（如 B0001） -> 本類 TranslateKindCode
    uint16_t mapKind;   // +2  地圖ID（如 M0027） -> cltMapInfo::TranslateKindCode
    int32_t  x;         // +4  區域左上角 X
    int32_t  y;         // +8  區域左上角 Y
    int32_t  width;     // +12 區域寬
    int32_t  height;    // +16 區域高（原檔欄名 heigth）
};
#pragma pack(pop)

static_assert(sizeof(strMapAreaKindInfo) == 20, "strMapAreaKindInfo must be 20 bytes");

class cltMapAreaKindInfo {
public:
    cltMapAreaKindInfo() : m_items(nullptr), m_count(0) {}
    ~cltMapAreaKindInfo() { Free(); }

    // 反編譯：int __thiscall Initialize(this, char* filename)
    int  Initialize(char* filename);

    // 反編譯：void __thiscall Free(this)
    void Free();

    // 反編譯：str* __thiscall GetMapAreaKindInfo(this, uint16_t code)
    strMapAreaKindInfo* GetMapAreaKindInfo(uint16_t code);

    // 反編譯：static uint16_t __cdecl TranslateKindCode(char* s)
    static uint16_t TranslateKindCode(char* s);

    // 便利
    inline const strMapAreaKindInfo* data() const { return m_items; }
    inline int size() const { return m_count; }

private:
    strMapAreaKindInfo* m_items; // 連續陣列（20 bytes/筆）
    int                 m_count; // 總筆數
};
