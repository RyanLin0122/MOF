#pragma once

#include "Object/CBaseObject.h"
#include <cstdint>

// cltQuestMark — 任務標記物件，繼承自 CBaseObject
// Ground truth: 大小 0xC60 = 3168 bytes
// 在 NPC 頭上顯示任務標記圖示
class cltQuestMark : public CBaseObject {
public:
    cltQuestMark();
    virtual ~cltQuestMark();

    void Initialize(std::uint16_t npcID, float posX, float posY, unsigned int type);
};

// cltRewardMark — 獎勵標記物件，繼承自 CBaseObject
// Ground truth: 大小 0xC60 = 3168 bytes
// 在 NPC 頭上顯示獎勵標記圖示
class cltRewardMark : public CBaseObject {
public:
    cltRewardMark();
    virtual ~cltRewardMark();

    void Initialize(std::uint16_t npcID, float posX, float posY, unsigned int type);
};
