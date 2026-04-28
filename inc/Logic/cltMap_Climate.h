#pragma once
#include <cstdint>

class GameImage;

// cltClimateUnit — 單顆天氣粒子（雨點、雪花、落雷光點等）。
// mofclient.c 中 56 bytes/unit；cltMap_Climate 內嵌 250 顆。
class cltClimateUnit {
public:
    cltClimateUnit();
    virtual ~cltClimateUnit() = default;

    void Free();
    void Init(int hasDrawBG, unsigned short climateKind, int rangeY);
    void Reset();
    void Poll();
    void PrepareDrawing();
    void Draw();

    // 對齊 mofclient.c：
    //   DWORD+1  m_iX
    //   DWORD+2  m_iY
    //   DWORD+3  m_dwResId
    //   WORD+8   m_wStartBlock
    //   WORD+9   m_wRandRange
    //   WORD+10  m_wMaxBlock = m_wStartBlock + m_wRandRange
    //   DWORD+6  m_dwHasDrawBG（a2）
    //   DWORD+7  m_dwIsSnowing
    //   DWORD+9  m_pImage（PrepareDrawing 暫存）
    //   WORD+20  m_wCurFrame
    //   float+11 m_fFrame（影格累計）
    //   float+12 m_fAnimSpeed
    //   WORD+16  m_wSnowSpeed
    //   DWORD+13 m_iRangeY
    int            m_iX;
    int            m_iY;
    unsigned int   m_dwResId;
    unsigned short m_wStartBlock;
    unsigned short m_wRandRange;
    unsigned short m_wMaxBlock;
    unsigned short m_wSnowSpeed;
    unsigned int   m_dwHasDrawBG;
    unsigned int   m_dwIsSnowing;
    GameImage*     m_pImage;
    unsigned short m_wCurFrame;
    float          m_fFrame;
    float          m_fAnimSpeed;
    int            m_iRangeY;
};

// cltMap_Climate — 地圖天氣（雪／雨／落雷）系統。
// mofclient.c：偏移 +428 內嵌於 Map。
class cltMap_Climate {
public:
    cltMap_Climate();
    ~cltMap_Climate() = default;

    void InitClimate(int hasDrawBG, unsigned short climateKind, int rangeY);
    void Free();
    void Poll();
    void PrepareDrawing();
    void Draw();

    // mofclient.c：Map::Poll 透過此旗標決定要傳給 list 中各 CMap_Item_Climate
    // 的「靜態氣候啟動」狀態。
    int  IsActive() const { return m_iActive; }

private:
    static constexpr int kMaxUnits = 250;

    cltClimateUnit  m_units[kMaxUnits];
    int             m_iActive;       // mofclient.c: DWORD+3501（0=未啟動）
    unsigned short  m_wUnitCount;    // mofclient.c: WORD+7004
    unsigned short  m_wClimateKind;  // mofclient.c: WORD+7005
};
