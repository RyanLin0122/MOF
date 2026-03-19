#pragma once

#include "Object/CBaseObject.h"
#include <cstdint>

class GameImage;

// cltNPC_Object — NPC 物件，繼承自 CBaseObject
class cltNPC_Object : public CBaseObject {
public:
    cltNPC_Object();
    virtual ~cltNPC_Object();

    virtual void Process() override;
    virtual void Draw() override;

    void Initialize(float posX, float posY,
                    std::uint16_t nameTextCode, std::uint16_t titleTextCode,
                    unsigned int resourceID, std::uint16_t maxFrames,
                    int flipFlag, int toggleFlag,
                    std::uint16_t* chatTextIDs);

private:
    GameImage*      m_pGameImage;           // 目前的遊戲圖像
    std::uint16_t   m_wCurrentFrame;        // 目前動畫格
    float           m_fAniFrame;            // 動畫格累加器
    char            m_szName[32];           // NPC 名稱 (byte 3164)
    char            m_szTitle[32];          // NPC 頭銜 (byte 3196, 含 [] 括弧)
    unsigned int    m_dwResourceID;         // 圖像資源 ID
    std::uint16_t   m_wMaxFrames;           // 最大動畫格數
    int             m_nFlipFlag;            // 方向/翻轉旗標
    int             m_nToggleFlag;          // 動畫方向切換旗標
    std::uint16_t   m_wChatTextIDs[3];      // 聊天文字 ID (最多3筆)
    unsigned int    m_dwLastChatTime;       // 上次聊天時間 (timeGetTime)
    std::uint8_t    m_byChatCount;          // 有效聊天文字數量
    std::uint8_t    m_byChatIndex;          // 目前聊天輪播索引
};
