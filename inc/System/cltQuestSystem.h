#pragma once

#include <cstdint>

class cltQuestSystem {
public:
    void* GetPlayingQuestInfoByNPCID(std::uint16_t npcKind);
    void* GetPlayingQuestInfoByQuestID(std::uint16_t questKind);
    int CanReward(int questKind);
    int IsCompleteQuest(std::uint16_t questKind); // TODO
    int IsGiveupQuestPermanently(); // TODO: real quest state logic
    void CompleteFunctionQuest(int functionType, std::uint16_t* questKinds, unsigned int* questValues);
};

