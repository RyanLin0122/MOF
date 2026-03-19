#pragma once

#include "Object/CBaseObject.h"
#include <cstdint>

class GameImage;

// cltNPC_Taxi — 計程車 NPC 物件，繼承自 CBaseObject
// 與一般 cltNPC_Object 不同，使用 4 組資源 ID 進行多狀態動畫
// Ground truth: 大小 0xCC8 = 3272 bytes
class cltNPC_Taxi : public CBaseObject {
public:
    cltNPC_Taxi();
    virtual ~cltNPC_Taxi();

    virtual void Process() override;
    virtual void Draw() override;

    void Initialize(float posX, float posY,
                    std::uint16_t nameTextCode, std::uint16_t titleTextCode,
                    unsigned int resourceID, int flipFlag,
                    std::uint16_t* chatTextIDs);

private:
    GameImage*      m_pGameImage;           // DWORD offset 788
    std::uint16_t   m_wCurrentFrame;        // WORD offset 1578
    float           m_fAniFrame;            // float offset 790
    char            m_szName[32];           // offset 3164
    char            m_szTitle[48];          // offset 3196
    unsigned int    m_dwResourceIDs[4];     // DWORD offset 810-813 (4 組資源 ID)
    int             m_nFlipFlag;            // DWORD offset 808
    std::uint8_t    m_byState;              // BYTE offset 3256 (目前狀態 0-3)
    std::uint8_t    m_byMaxFrame;           // BYTE offset 3257 (每狀態最大幀數)
    std::uint16_t   m_wChatTextIDs[3];      // 聊天文字 ID
    unsigned int    m_dwLastChatTime;       // DWORD offset 816
    std::uint8_t    m_byChatCount;          // BYTE offset 3268
    std::uint8_t    m_byChatIndex;          // BYTE offset 3269
};
