#pragma once
//
// CA_CACHE_DATA — mofclient.c 還原（位址 0x527050）
//
// CA_GI_Cache 的元素：每筆是一個「已分配 vertex buffer 的 GameImage + 額外
// header bytes」，固定 stride = 0x204 (= 516 bytes)。本構造繼承 GameImage 並
// 在尾端加入 ~40 bytes 自有資料，供 CA / CCA 模組做 GI 結果快取。
//
// 對齊原 binary 寫入：
//   - GameImage::GameImage(this)
//   - memset(this+478, 0, 0x20u)
//   - *((WORD*)this+255) = 0   // offset 510
//   - *((WORD*)this+238) = 0   // offset 476
//   - *((WORD*)this+256) = 0   // offset 512
//
#include <cstdint>
#include "Image/GameImage.h"

class CA_CACHE_DATA : public GameImage {
public:
    CA_CACHE_DATA();
    ~CA_CACHE_DATA() override = default;

private:
    // 從 GameImage 物件結尾開始的擴充欄位，總大小撐到 516 bytes (0x204)。
    // 真實的 GameImage 大小在本還原可能與原 binary 不同；以下用 sentinel
    // 欄位嚴格對齊 mofclient.c 的偏移寫入語意，但不假設絕對 byte offset。
    uint16_t m_wTagAt476;       // 對應 *((WORD*)this+238) = 0
    uint8_t  m_aZero478[32];    // 對應 memset(this+478, 0, 0x20u)
    uint16_t m_wAt510;          // 對應 *((WORD*)this+255) = 0
    uint16_t m_wAt512;          // 對應 *((WORD*)this+256) = 0
};
