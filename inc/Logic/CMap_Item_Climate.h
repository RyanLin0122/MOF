#pragma once
#include <cstdint>
#include "Logic/CItemClimateUnit.h"

// CMap_Item_Climate — 道具觸發的地圖天氣（玩家施放某些消耗品時生成的特效）。
// mofclient.c：堆配置 0x3FAC bytes（~16300）；包含 250 顆 CItemClimateUnit。
// Map 以 std::list<CMap_Item_Climate*> 管理多筆同時生效的氣候。
class CMap_Item_Climate {
public:
    CMap_Item_Climate();
    ~CMap_Item_Climate() = default;

    // mofclient.c 0x4E4830
    void Free();
    // mofclient.c 0x4E4890：成功回 1，失敗回 0。
    int  InitMapItemClimate(unsigned short itemID);
    // mofclient.c 0x4E4920 / 4E4960 / 4E49A0
    void Poll();
    void PrepareDrawing();
    void Draw();

    // 屬性 setters / getters（mofclient.c 0x4E49E0..0x4E4B30）
    void SetActiveStaticMapClimate(int v);
    void SetActiveMapItemClimate(int v);
    void SetSubMapItemClimate(int v);
    void SetMapItemClimateRemainTime(unsigned int t);
    void SetMapItemClimateUserName(char* name);
    int  IsActiveMapItemClimate() const;
    int  IsSubMapItemClimate() const;
    int  IsActiveStaticMapClimate() const;
    unsigned short GetUseMapItemClimateKind() const;
    char* GetMapItemClimateUserName();
    unsigned int GetMapItemClimateRemainTime() const;
    unsigned int GetMapItemClimateIconResourceID() const;
    unsigned int GetMapItemClimateIconResourceBlockID() const;
    void SetMapItemClimateKind(unsigned short kind);
    unsigned short GetMapItemClimateKind() const;
    void SetTimerID();
    unsigned int GetTimerID() const;

private:
    static constexpr int kMaxUnits = 250;
    static constexpr int kUserNameMaxLen = 256;

    CItemClimateUnit m_units[kMaxUnits];

    int            m_iActive;             // mofclient.c DWORD+4001
    int            m_iActiveStatic;       // DWORD+4002
    int            m_iSubFlag;            // DWORD+4003
    unsigned short m_wUnitCount;          // WORD+8008
    unsigned short m_wKind;               // WORD+8009
    unsigned short m_wUseItemKind;        // WORD+8010
    unsigned int   m_dwIconResId;         // DWORD+4006
    unsigned int   m_dwRemainTime;        // DWORD+4007
    unsigned int   m_dwIconBlockId;       // DWORD+4008
    unsigned int   m_dwField4009;         // DWORD+4009
    unsigned int   m_dwTimerID;           // DWORD+4074
    char           m_szUserName[kUserNameMaxLen];  // byte+16040
};
