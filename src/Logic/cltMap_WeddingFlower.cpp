#include "Logic/cltMap_WeddingFlower.h"
#include "Info/cltMapUseItemInfoKindInfo.h"

#include <cstdlib>
#include <cstring>

// mofclient.c 0x4E4B30 — ctor：vector-construct 80 顆 CItemClimateUnit + 計數 0
cltMap_WeddingFlower::cltMap_WeddingFlower() : m_wCount(0) {}

// mofclient.c 0x4E4BB0
void cltMap_WeddingFlower::Free() {
    for (unsigned short i = 0; i < m_wCount; ++i) m_units[i].Free();
    m_wCount = 0;
}

// mofclient.c 0x4E4C00：建立 80 顆花瓣，每顆從 5 種磁性編碼中亂數選一種
//   並組出一個 stack-local 的 strMapUseItemInfoKindInfo 傳給 CItemClimateUnit::Init。
//   原 binary 用 5 個 magic kind code（889192477..481）作為粒子來源 ID；本還原
//   保留同一組 ID。
void cltMap_WeddingFlower::InitWeddingFlower(float /*density*/, int /*range*/) {
    static const unsigned int kFlowerKinds[5] = {
        889192477u, 889192478u, 889192479u, 889192480u, 889192481u
    };
    m_wCount = kMaxUnits;
    for (int i = 0; i < kMaxUnits; ++i) {
        const unsigned int kind = kFlowerKinds[std::rand() % 5];

        // 對應 mofclient.c 在 stack 上構造的 strMapUseItemInfoKindInfo 值：
        strMapUseItemInfoKindInfo fake{};
        fake.ResourceID       = kind;          // byte 20
        fake.StartBlock       = 0u;            // byte 36
        fake.MaxBlocks        = 8u;            // byte 40
        fake.AnimStartBlockID = 5u;            // byte 44
        fake.AnimSpeed        = 40u;           // byte 48
        // 依 kind 決定 AirSpeed (=mofclient.c v5+0)
        switch (kind) {
            case 889192477u:
            case 889192478u:
                fake.AirSpeed = 2u;
                break;
            case 889192479u:
            case 889192480u:
                fake.AirSpeed = 1u;
                break;
            case 889192481u:
                fake.AirSpeed = 3u;
                break;
            default:
                fake.AirSpeed = 0u;
                break;
        }
        fake.Pattern = 1u;                     // byte 64

        m_units[i].Init(&fake);
    }
}

// mofclient.c 0x4E4D00
void cltMap_WeddingFlower::Poll() {
    for (unsigned short i = 0; i < m_wCount; ++i) m_units[i].Poll();
}

// mofclient.c 0x4E4D30
void cltMap_WeddingFlower::PrepareDrawing() {
    for (unsigned short i = 0; i < m_wCount; ++i) m_units[i].PrepareDrawing();
}

// mofclient.c 0x4E4D60
void cltMap_WeddingFlower::Draw() {
    for (unsigned short i = 0; i < m_wCount; ++i) m_units[i].Draw();
}
