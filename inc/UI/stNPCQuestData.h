#pragma once
//
// stNPCQuestData — mofclient.c 還原（位址 0x4D8720 / 0x4D8730）
//
// CInterfaceData 內嵌欄位（offset +360）；用來記錄目前 NPC 對話中：
//   - 任務 ID (m_wQuestID)
//   - 任務狀態 / 步驟
//   - flag bits
//
// 物件 layout（對齊 mofclient.c 寫法）：
//   +0   uint16_t  m_wQuestID
//   +2   uint16_t  m_wStatus
//   +4   uint8_t   m_byFlag
//   +5   pad
//   +6   uint16_t  m_wStep
//
#include <cstdint>

struct stNPCQuestData {
    uint16_t m_wQuestID;   // +0
    uint16_t m_wStatus;    // +2
    uint8_t  m_byFlag;     // +4
    uint8_t  m_pad5;
    uint16_t m_wStep;      // +6

    stNPCQuestData() { Init(); }

    //----- (004D8730) ----------------------------------------------------
    void Init()
    {
        m_wQuestID = 0;
        m_wStatus  = 0;
        m_byFlag   = 0;
        m_pad5     = 0;
        m_wStep    = 0;
    }
};
