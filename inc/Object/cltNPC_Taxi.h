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
    GameImage*      m_pGameImage;           // DWORD 788 (byte 3152)
    std::uint16_t   m_wCurrentFrame;        // WORD 1578 (byte 3156)
    // 2 bytes auto padding for float alignment
    float           m_fAniFrame;            // float/DWORD 790 (byte 3160)
    char            m_szName[32];           // byte 3164
    char            m_szTitle[32];          // byte 3196 (ground truth: 32 bytes, not 48)
    int             _reserved1;             // byte 3228 (padding, NPC_Object has resourceID here)
    int             m_nFlipFlag;            // DWORD 808 (byte 3232)
    int             _reserved2;             // byte 3236 (padding)
    unsigned int    m_dwResourceIDs[4];     // DWORD 810-813 (byte 3240-3255)
    std::uint8_t    m_byState;              // BYTE 3256 (目前狀態 0-3)
    std::uint8_t    m_byMaxFrame;           // BYTE 3257 (每狀態最大幀數)
    std::uint16_t   m_wChatTextIDs[3];      // byte 3258 聊天文字 ID
    unsigned int    m_dwLastChatTime;       // DWORD 816 (byte 3264)
    std::uint8_t    m_byChatCount;          // BYTE 3268
    std::uint8_t    m_byChatIndex;          // BYTE 3269
};
