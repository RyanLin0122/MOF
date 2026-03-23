#pragma once
#include <cstdint>

class CSpiritSpeechMgr {
public:
    char* GetSpiritSpeechScript();
    void UpdateQuestCollection(uint16_t itemKind);
    void UpdateQuestHunt(uint16_t monsterKind);
};
