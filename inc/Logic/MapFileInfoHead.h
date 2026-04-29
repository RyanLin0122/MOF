#pragma once
//
// MapFileInfoHead — mofclient.c 還原（位址 0x4E1BC0）
//
// .map 檔頭描述子。原 binary 把 Map 物件的前 28 bytes 當作 file header 處理，
// 因此 IDA 額外產出了一個獨立的 ~MapFileInfoHead，用來釋放：
//   - offset +16 (DWORD index 4)：tile-info buffer
//   - offset +24 (DWORD index 6)：portal-info buffer
//
// 在本還原中，Map class 已直接在 ~Map() 內處理 m_pTileInfo / m_pPortalInfo，
// 此 struct 僅保留 binary-equivalent 的型別記號 + 等價 destructor。
//
#include <cstdint>

struct MapFileInfoHead {
    uint16_t m_wMapID;        // +0
    uint16_t m_wMapKind;      // +2
    uint16_t m_wWidth;        // +4
    uint16_t m_wHeight;       // +6
    uint32_t m_dwReserved;    // +8
    uint32_t m_dwReserved2;   // +12
    void*    m_pTileBuffer;   // +16  ← destructor 釋放
    uint32_t m_dwReserved3;   // +20
    void*    m_pPortalBuffer; // +24  ← destructor 釋放

    //----- (004E1BC0) ----------------------------------------------------
    ~MapFileInfoHead()
    {
        if (m_pTileBuffer)   { ::operator delete(m_pTileBuffer);   m_pTileBuffer   = nullptr; }
        if (m_pPortalBuffer) { ::operator delete(m_pPortalBuffer); m_pPortalBuffer = nullptr; }
    }
};
