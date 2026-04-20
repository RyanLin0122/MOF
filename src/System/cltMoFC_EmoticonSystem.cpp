#include "System/cltMoFC_EmoticonSystem.h"

#include <cstring>

#include "Network/CMoFNetwork.h"
#include "Info/cltEmoticonKindInfo.h"
#include "Logic/cltBaseInventory.h"
#include "Character/ClientCharacterManager.h"
#include "global.h"


cltMoFC_EmoticonSystem::cltMoFC_EmoticonSystem() : cltEmoticonSystem() {
}

cltMoFC_EmoticonSystem::~cltMoFC_EmoticonSystem() {
}

void cltMoFC_EmoticonSystem::FindEmoticonWord(unsigned int accountId, char* chatMsg, std::uint16_t emoticonItemId) {
    ClientCharacter* pChar = g_ClientCharMgr.GetCharByAccount(accountId);
    if (pChar) {
        FindEmoticonWord(pChar->GetName(), chatMsg, emoticonItemId);
    }
}

void cltMoFC_EmoticonSystem::FindEmoticonWord(char* charName, char* chatMsg, std::uint16_t emoticonItemId) {
    ClientCharacter* pChar = g_ClientCharMgr.GetCharByName(charName);
    if (!pChar)
        return;

    stEmoticonItemInfo* info = m_pcltEmoticonKindInfo->GetEmoticonItemInfoByID(emoticonItemId);
    int slotIndex = 0;

    if (emoticonItemId && info) {
        // Search with specific emoticon item
        for (slotIndex = 0; slotIndex < 5; ++slotIndex) {
            if (info->slot[slotIndex].wordCount <= 0)
                continue;

            for (uint32_t w = 0; w < info->slot[slotIndex].wordCount; ++w) {
                const char* word = reinterpret_cast<const char*>(&info->slot[slotIndex]) + 22 + w * 16;
                size_t chatLen = std::strlen(chatMsg);
                size_t wordLen = std::strlen(word);

                if (wordLen == 0) continue;

                const char* p = chatMsg;
                int pos = 0;
                int remaining = static_cast<int>(chatLen);

                while (pos < static_cast<int>(chatLen)) {
                    if (remaining >= static_cast<int>(wordLen) && std::strncmp(p, word, wordLen) == 0) {
                        goto FOUND;
                    }
                    if (IsDBCSLeadByte(*p)) {
                        ++pos;
                        --remaining;
                        ++p;
                    }
                    ++pos;
                    --remaining;
                    ++p;
                }
            }
        }
    }

    if (slotIndex >= 5 || !emoticonItemId || !info) {
        // Fall back to base emoticon (ID=0)
        info = m_pcltEmoticonKindInfo->GetEmoticonItemInfoByID(0);
        for (slotIndex = 0; slotIndex < 5; ++slotIndex) {
            if (info->slot[slotIndex].wordCount <= 0)
                continue;

            for (uint32_t w = 0; w < info->slot[slotIndex].wordCount; ++w) {
                const char* word = reinterpret_cast<const char*>(&info->slot[slotIndex]) + 22 + w * 16;
                size_t chatLen = std::strlen(chatMsg);
                size_t wordLen = std::strlen(word);

                if (wordLen == 0) continue;

                const char* p = chatMsg;
                int pos = 0;
                int remaining = static_cast<int>(chatLen);

                while (pos < static_cast<int>(chatLen)) {
                    if (remaining >= static_cast<int>(wordLen) && std::strncmp(p, word, wordLen) == 0) {
                        goto FOUND;
                    }
                    if (IsDBCSLeadByte(*p)) {
                        ++pos;
                        --remaining;
                        ++p;
                    }
                    ++pos;
                    --remaining;
                    ++p;
                }
            }
        }
        // No match found at all
        g_ClientCharMgr.ReleaseEmoticon(pChar);
        return;
    }

FOUND:
    g_ClientCharMgr.SetEmoticonKind(charName, info->slot[slotIndex].kind);
}

void cltMoFC_EmoticonSystem::SendChattingMsg(char* chatMsg) {
    for (int slot = 168; slot <= 191; ++slot) {
        auto* invItem = m_pcltBaseInventory->GetInventoryItem(static_cast<std::uint16_t>(slot));
        if (!invItem)
            continue;

        stEmoticonItemInfo* info = m_pcltEmoticonKindInfo->GetEmoticonItemInfoByID(invItem->itemKind);
        if (!info)
            continue;

        for (int slotIndex = 0; slotIndex < 5; ++slotIndex) {
            if (info->slot[slotIndex].wordCount <= 0)
                continue;

            for (uint32_t w = 0; w < info->slot[slotIndex].wordCount; ++w) {
                const char* word = reinterpret_cast<const char*>(&info->slot[slotIndex]) + 22 + w * 16;
                size_t chatLen = std::strlen(chatMsg);
                size_t wordLen = std::strlen(word);

                if (wordLen == 0) continue;

                const char* p = chatMsg;
                int pos = 0;
                int remaining = static_cast<int>(chatLen);

                while (pos < static_cast<int>(chatLen)) {
                    if (remaining >= static_cast<int>(wordLen) && std::strncmp(p, word, wordLen) == 0) {
                        g_Network.Chatting(chatMsg, static_cast<std::uint8_t>(slot), invItem->itemKind);
                        return;
                    }
                    if (IsDBCSLeadByte(*p)) {
                        ++pos;
                        --remaining;
                        ++p;
                    }
                    ++pos;
                    --remaining;
                    ++p;
                }
            }
        }
    }

    g_Network.Chatting(chatMsg, 0, 0);
}

int cltMoFC_EmoticonSystem::UseEmotioconItem(std::uint8_t index) {
    return g_Network.UseEmoticonItem(0, m_kindSlots[index]);
}

stEmoticonItemInfo* cltMoFC_EmoticonSystem::GetBaseEmoticonItemInfo() {
    return m_pcltEmoticonKindInfo->GetEmoticonItemInfoByKind(1);
}

stEmoticonItemInfo* cltMoFC_EmoticonSystem::GetEmoticonItemInfoByID(std::uint16_t id) {
    return m_pcltEmoticonKindInfo->GetEmoticonItemInfoByID(id);
}

stEmoticonItemInfo* cltMoFC_EmoticonSystem::GetEmoticonItemInfoByIndex(std::uint8_t index) {
    return m_pcltEmoticonKindInfo->GetEmoticonItemInfoByIndex(index);
}

stEmoticonItemInfo* cltMoFC_EmoticonSystem::GetEmoticonItemInfoByKind(int kind) {
    return m_pcltEmoticonKindInfo->GetEmoticonItemInfoByKind(kind);
}

stEmoticonWordInfo* cltMoFC_EmoticonSystem::GetEmoticonWordInfoByKind(int kind) {
    return m_pcltEmoticonKindInfo->GetEmoticonWordInfoByKind(kind);
}
