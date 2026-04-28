#pragma once
#include <cstdint>
#include "Logic/CItemClimateUnit.h"

// cltMap_WeddingFlower — 婚禮地圖花瓣特效系統。
// mofclient.c：偏移 +14444 內嵌於 Map；80 顆 CItemClimateUnit (64 bytes/顆)。
class cltMap_WeddingFlower {
public:
    cltMap_WeddingFlower();
    ~cltMap_WeddingFlower() = default;

    // mofclient.c：a2=每秒粒子密度（mofclient.c 直接傳 10.0），
    //             a3=範圍（傳 256）；目前實作不直接使用 a2/a3。
    void InitWeddingFlower(float density, int range);
    void Free();
    void Poll();
    void PrepareDrawing();
    void Draw();

private:
    static constexpr int kMaxUnits = 80;
    CItemClimateUnit m_units[kMaxUnits];
    unsigned short   m_wCount;   // mofclient.c WORD+2564
};
