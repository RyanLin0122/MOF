#pragma once
#include <cstdint>

class GameImage;
struct strMapUseItemInfoKindInfo;

// CItemClimateUnit — 道具 / 婚禮花瓣使用的天氣粒子。
// mofclient.c：64 bytes/unit；cltMap_WeddingFlower 內嵌 80 顆，
// CMap_Item_Climate 內嵌 250 顆。
class CItemClimateUnit {
public:
    CItemClimateUnit();
    virtual ~CItemClimateUnit() = default;

    void Free();
    void Init(unsigned short itemID);                            // mofclient.c 0x4E4440
    void Init(const strMapUseItemInfoKindInfo* info);            // mofclient.c 0x4E44B0
    void Reset();                                                 // mofclient.c 0x4E4510
    void Poll();                                                  // mofclient.c 0x4E45C0
    void PrepareDrawing();                                        // mofclient.c 0x4E46D0
    void Draw();                                                  // mofclient.c 0x4E4750

    // 對齊 mofclient.c 64-byte 佈局：
    int            m_iX;             // DWORD+1
    int            m_iY;             // DWORD+2
    int            m_iX2;            // DWORD+3
    int            m_iLandY;         // DWORD+4：落地 Y
    int            m_iAirSpeed;      // DWORD+5：空中速度（1/2/3 = 落地/吹散/上飄）
    int            m_iPattern;       // DWORD+6：模式
    int            m_iAnimSpeed;     // DWORD+7
    unsigned short m_wField16;       // WORD+16
    unsigned short m_wField17;       // WORD+17
    unsigned short m_wField18;       // WORD+18
    unsigned short m_wCurFrame;      // WORD+19
    unsigned short m_wMaxFrame;      // WORD+20
    unsigned int   m_dwResourceID;   // DWORD+11
    int            m_iStateFlag;     // DWORD+12（0 = 空中，1 = 已落地）
    GameImage*     m_pImage;         // DWORD+13
    float          m_fAnimSpeedF;    // float+14（mofclient.c：AnimSpeed * 0.01）
    float          m_fFrame;         // float+15
};
