#pragma once

struct stObjectInfo {
    unsigned short m_wObjectID;     // offset 0
    unsigned short _pad1;           // offset 2
    unsigned int   m_dwResourceID;  // offset 4 (hex resource ID)
    unsigned short m_wBlockID;      // offset 8 (animation block)
    unsigned short m_wKind;         // offset 10 (0=static, 1=?, 2=?)
    int            m_bFlipX;        // offset 12 (Y/N -> 1/0)
    int            m_bFlipY;        // offset 16 (Y/N -> 1/0)
    unsigned short m_wScale;        // offset 20 (e.g. 100)
    unsigned short _pad2;           // offset 22
    int            m_bAlpha;        // offset 24 (Y/N -> 1/0)
}; // total = 28 bytes (0x1C)
