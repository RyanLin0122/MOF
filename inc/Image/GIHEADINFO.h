#pragma once
//
// GIHEADINFO — mofclient.c 還原（位址 0x54AD60）
//
// .gi 檔頭格式描述子。與 ImageResource 共用前段欄位（version / size / format），
// 但在 mofclient.c 的 IDA dump 中只給了 destructor 一份單獨實體，函式在
// 釋放兩個動態配置的 buffer：影格資訊陣列（offset +20）與像素資料（offset +32）。
//
// 由於 ImageResource 在本還原已完成獨立的 ~ImageResource，這個結構僅作為
// 對齊原 binary 二進位 size 的「檔頭視圖」存在；實際遊戲執行時不會被單獨
// 配置，destructor 在這裡為空即可。
//
#include <cstdint>

struct GIHEADINFO {
    uint32_t m_dwUnknown0;   // +0
    int32_t  m_nVersion;     // +4
    uint16_t m_wWidth;       // +8
    uint16_t m_wHeight;      // +10
    uint32_t m_dwImageSize;  // +12
    uint32_t m_dwDecompSize; // +16
    void*    m_pFrames;      // +20  ← destructor 釋放
    uint32_t m_dwFormat;     // +24
    uint32_t m_dwReserved;   // +28
    void*    m_pPixels;      // +32  ← destructor 釋放
    uint32_t m_dwTexture;    // +36

    //----- (0054AD60) ----------------------------------------------------
    ~GIHEADINFO()
    {
        if (m_pFrames) { ::operator delete(m_pFrames); m_pFrames = nullptr; }
        if (m_pPixels) { ::operator delete(m_pPixels); m_pPixels = nullptr; }
    }
};
