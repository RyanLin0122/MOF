#pragma once

#include <cstdint>

class GameImage;

// mofclient.c 還原：cltMiniGame_DrawNum — 繪製整數的工具。
// 相較於 Mini_IMG_Number，它支援每次繪製時指定 (x, y, value, alpha)，
// 並帶有對齊模式（a5：0=右對齊，1=左對齊，其他=不移動）。
class cltMiniGame_DrawNum {
public:
    cltMiniGame_DrawNum();
    ~cltMiniGame_DrawNum();

    void InitDrawNum(unsigned int imageType,
                     unsigned int dwResourceID,
                     uint16_t     blockBase,
                     uint8_t      alignMode);
    void SetActive(int active);
    void PrepareDrawing(int x, int y, unsigned int value, int alpha);
    void Draw();

public:
    static constexpr int kMaxDigits = 20;

    int           m_active;                // +0  DWORD[0]

private:
    unsigned int  m_imageType;             // +4  DWORD[1]
    unsigned int  m_dwResourceID;          // +8  DWORD[2]
    uint16_t      m_blockBase;             // +12 WORD[6]
    uint16_t      m_digitCount;            // +14 WORD[7]
    uint8_t       m_alignMode;             // +16 BYTE[16]
    uint8_t       m_pad17[3];
    GameImage*    m_pImages[kMaxDigits];   // +20 ~ +96
};
