#pragma once
//
// stQuestMark — mofclient.c 還原（位址 0x494710）
//
// 世界地圖 / DivideMap 上每個任務標記的資料節點。CUIWorldMap / CUIDivideMap
// 用 std::map<uint16_t, stQuestMark*> 持有它們。
//
// 物件 layout（從 HideAllQuestMark 推導）：
//   +0   uint16_t  m_wMapID            // -1 == hidden
//   +2   uint16_t  m_wQuestKind
//   +4   uint16_t  m_wPosX
//   +6   uint16_t  m_wPosY
//   +8   int       m_nIconIdx
//   +12..+19      header 雜項
//   +20  CControlImage  m_clImage      // 嵌入式控制項（destructor 直接呼叫其 dtor）
//
// 注意：destructor 唯一動作是呼叫 m_clImage.~CControlImage()，C++ 會在
// stQuestMark 自動 dtor 中處理，因此這裡不需要手寫。
//
#include <cstdint>
#include "UI/CControlImage.h"

struct stQuestMark {
    uint16_t      m_wMapID;       // +0
    uint16_t      m_wQuestKind;   // +2
    uint16_t      m_wPosX;        // +4
    uint16_t      m_wPosY;        // +6
    int           m_nIconIdx;     // +8
    int           m_nReserved12;  // +12
    int           m_nReserved16;  // +16
    CControlImage m_clImage;      // +20
};
