#pragma once

#include <cstdint>
#include "UI/CControlChatBallon.h"

class GameImage;
class ClientCharacter;
class ClientCharacterManager;

// cltMoF_SpiritObject — 精靈物件
// 跟隨玩家角色的守護精靈，有巡邏/追蹤行為和聊天氣泡
class cltMoF_SpiritObject {
public:
    cltMoF_SpiritObject();
    virtual ~cltMoF_SpiritObject();

    void SetActive(ClientCharacter* pChar, int active);
    int  GetActive();
    void Initialize(ClientCharacterManager* pMgr);
    int  GetFront();
    void SetChar(ClientCharacter* pChar, std::uint16_t param);
    void UpdateSpirit(std::uint8_t level);
    void Poll();
    void PrepareDrawing(int showSpeech);
    void Draw();
    void MoveTrace();
    void MovePatrol();

private:
    int                     m_nActive;          // DWORD 1 (byte 4): 是否啟動
    std::uint8_t            m_byLevel;          // BYTE 8: 等級
    std::uint8_t            _padByte9;          // BYTE 9: padding
    std::uint16_t           m_wClassCode;       // WORD 5 (byte 10): 職業碼
    std::uint16_t           m_wCharKind;        // WORD 6 (byte 12): 角色種類
    std::uint16_t           _padWord7;          // byte 14-15: padding for float alignment
    float                   m_fPosX;            // float 4 (byte 16): 世界座標 X
    float                   m_fPosY;            // 世界座標 Y
    unsigned int            m_dwResourceID;     // 精靈圖像資源 ID
    unsigned int            m_dwStartBlockID;   // 起始 block ID
    unsigned int            m_dwCurrentBlock;   // 目前 block
    std::uint16_t           m_wBlockCount;      // block 數量
    float                   m_fFrameAccum;      // 動畫累加器
    int                     m_nDirection;        // 面向 (0=左, 1=右)
    int                     m_nShowSpeech;       // 是否顯示對話
    GameImage*              m_pGameImage;        // 目前圖像
    ClientCharacter*        m_pOwnerChar;        // 主人角色
    ClientCharacterManager* m_pCharMgr;          // 角色管理器

    CControlChatBallon      m_ChatBallon;        // 聊天氣泡

    int   m_nPatrolDir;      // 巡邏方向 (1 或 -1)
    int   m_nOffsetX;        // X 偏移
    int   m_nCharHeight;     // 角色高度偏移
    int   m_nAcceleration;   // 加速度
    float m_fTraceScale;     // 追蹤縮放因子
    int   m_nTraceFlag;      // 追蹤旗標
    float m_fPatrolScale;    // 巡邏縮放因子
    int   m_nPatrolReturn;   // 巡邏返回旗標
};
