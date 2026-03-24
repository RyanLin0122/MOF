#include "Logic/CSpiritQuestSpeech.h"

#include <cstdio>
#include <cstring>

#include "global.h"
#include "Info/cltQuestKindInfo.h"
#include "Info/cltNPCInfo.h"
#include "Logic/cltNPCManager.h"
#include "Logic/cltBaseInventory.h"
#include "System/cltQuestSystem.h"
#include "System/cltPetInventorySystem.h"

CSpiritQuestSpeech::CSpiritQuestSpeech()
{
    std::strcpy(m_szBuffer, "");
}

CSpiritQuestSpeech::~CSpiritQuestSpeech()
{
}

void CSpiritQuestSpeech::Free()
{
    m_pQuestSystem = nullptr;
    m_pInventory = nullptr;
    m_pPetInventory = nullptr;
    std::strcpy(m_szBuffer, "");
}

int CSpiritQuestSpeech::SetSpiritQuestSpeech(cltQuestSystem* questSys,
                                              cltBaseInventory* inventory,
                                              cltPetInventorySystem* petInventory)
{
    m_pQuestSystem = questSys;
    m_pPetInventory = petInventory;
    m_pInventory = inventory;
    return 1;
}

char* CSpiritQuestSpeech::UpdateQuestHunt(std::uint16_t questId)
{
    stQuestKindInfo* questInfo = g_clQuestKindInfo.GetQuestKindInfo(questId);
    stPlayingQuestInfo* playingInfo = m_pQuestSystem->GetRunningQuestInfoByQuestID(questId);

    if (playingInfo)
    {
        std::uint16_t targetCount = questInfo->extra.hunt.wCount;
        std::uint16_t currentCount = static_cast<std::uint16_t>(playingInfo->dwValue);
        const char* questName = g_DCTTextManager.GetText(questInfo->wQuestNameCode);

        std::sprintf(m_szBuffer, "%s %d/%d", questName, currentCount, targetCount);
        return m_szBuffer;
    }

    std::strcpy(m_szBuffer, "");
    return m_szBuffer;
}

char* CSpiritQuestSpeech::UpdateQuestCollection(std::uint16_t itemKind)
{
    std::strcpy(m_szBuffer, "");

    std::uint8_t questCount = m_pQuestSystem->GetRunningQuestCount();
    if (!questCount)
        return m_szBuffer;

    for (int i = 0; i < questCount; i++)
    {
        stPlayingQuestInfo* playingInfo = m_pQuestSystem->GetRunningQuestInfoByIndex(static_cast<std::uint8_t>(i));
        stQuestKindInfo* questInfo = g_clQuestKindInfo.GetQuestKindInfo(playingInfo->wQuestID);

        if (questInfo->extra.collection.wQuestItem1 == itemKind ||
            questInfo->extra.collection.wQuestItem2 == itemKind)
        {
            stQuestKindInfo* qi = g_clQuestKindInfo.GetQuestKindInfo(playingInfo->wQuestID);

            std::uint16_t item1 = qi->extra.collection.wQuestItem1;
            std::uint16_t item2 = qi->extra.collection.wQuestItem2;
            std::uint8_t  needCount1 = static_cast<std::uint8_t>(qi->extra.collection.wQuestItemCount1);
            std::uint8_t  needCount2 = static_cast<std::uint8_t>(qi->extra.collection.wQuestItemCount2);

            std::int16_t invQty1 = m_pInventory->GetInventoryItemQuantity(static_cast<int>(item1));
            std::uint16_t petQty1 = m_pPetInventory->GetInventoryItemQuantity(item1);
            std::uint16_t totalQty1 = static_cast<std::uint16_t>(invQty1 + petQty1);

            m_pPetInventory->GetInventoryItemQuantity(item2);
            m_pInventory->GetInventoryItemQuantity(static_cast<int>(item2));

            // Cap at the required amount
            if (totalQty1 > needCount1)
                totalQty1 = needCount1;

            if (needCount2)
            {
                const char* questName = g_DCTTextManager.GetText(qi->wQuestNameCode);
                std::sprintf(m_szBuffer, "%s %d/%d %d/%d", questName, totalQty1, needCount1, needCount2, needCount2);
            }
            else
            {
                const char* questName = g_DCTTextManager.GetText(qi->wQuestNameCode);
                std::sprintf(m_szBuffer, "%s %d/%d", questName, totalQty1, needCount1);
            }
            return m_szBuffer;
        }
    }

    return m_szBuffer;
}

char* CSpiritQuestSpeech::CheckCompleteQuest()
{
    if (!m_pQuestSystem)
        return nullptr;

    std::uint8_t questCount = m_pQuestSystem->GetRunningQuestCount();
    if (!questCount)
        return nullptr;

    for (int i = 0; i < questCount; i++)
    {
        stPlayingQuestInfo* playingInfo = m_pQuestSystem->GetRunningQuestInfoByIndex(static_cast<std::uint8_t>(i));
        std::uint16_t npcId = playingInfo->wNPCID;

        if (m_pQuestSystem->CanReward(static_cast<int>(npcId)))
        {
            stQuestKindInfo* questInfo = g_clQuestKindInfo.GetQuestKindInfo(playingInfo->wQuestID);

            if (questInfo->bPlayType == 1 || questInfo->bPlayType == 2 || questInfo->bPlayType == 3)
            {
                stNPCInfo* npcInfo = g_clNPCInfo.GetNPCInfoByID(npcId);

                // NPC name is at byte offset 4 in stNPCInfo (low 16 bits of _reserved[0])
                std::uint16_t npcNameCode = static_cast<std::uint16_t>(npcInfo->_reserved[0]);
                char* npcName = g_DCTTextManager.GetText(npcNameCode);

                std::uint16_t mapNameCode = g_clNPCManager.GetMapName(npcId);
                char* mapName = g_DCTTextManager.GetText(mapNameCode);

                char* questName = g_DCTTextManager.GetText(questInfo->wQuestNameCode);

                const char* formatText = g_DCTTextManager.GetText(4679);

                std::sprintf(m_szBuffer, formatText, questName, mapName, npcName);
            }

            return m_szBuffer;
        }
    }

    return nullptr;
}
