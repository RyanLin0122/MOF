#pragma once

#include <cstdint>

class cltQuestSystem {
public:
    void* GetPlayingQuestInfoByNPCID(std::uint16_t npcKind);
    void* GetPlayingQuestInfoByQuestID(std::uint16_t questKind);
    int CanReward(int questKind);
};

