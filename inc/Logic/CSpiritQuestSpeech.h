#pragma once

#include <cstdint>

class cltQuestSystem;
class cltBaseInventory;
class cltPetInventorySystem;

class CSpiritQuestSpeech {
public:
    CSpiritQuestSpeech();
    ~CSpiritQuestSpeech();

    void Free();

    int SetSpiritQuestSpeech(cltQuestSystem* questSys,
                             cltBaseInventory* inventory,
                             cltPetInventorySystem* petInventory);

    char* UpdateQuestHunt(std::uint16_t questId);
    char* UpdateQuestCollection(std::uint16_t itemKind);
    char* CheckCompleteQuest();

private:
    static constexpr int kBufferSize = 1024;

    cltQuestSystem*        m_pQuestSystem;
    cltBaseInventory*      m_pInventory;
    cltPetInventorySystem* m_pPetInventory;
    char                   m_szBuffer[kBufferSize];
};
