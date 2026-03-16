#pragma once

#include "System/cltEmoticonSystem.h"

struct stEmoticonItemInfo;
struct stEmoticonWordInfo;

class cltMoFC_EmoticonSystem : public cltEmoticonSystem {
public:
    cltMoFC_EmoticonSystem();
    ~cltMoFC_EmoticonSystem() override;

    void FindEmoticonWord(unsigned int accountId, char* chatMsg, std::uint16_t emoticonItemId);
    void FindEmoticonWord(char* charName, char* chatMsg, std::uint16_t emoticonItemId);
    void SendChattingMsg(char* chatMsg);
    int UseEmotioconItem(std::uint8_t index);
    stEmoticonItemInfo* GetBaseEmoticonItemInfo();
    stEmoticonItemInfo* GetEmoticonItemInfoByID(std::uint16_t id);
    stEmoticonItemInfo* GetEmoticonItemInfoByIndex(std::uint8_t index);
    stEmoticonItemInfo* GetEmoticonItemInfoByKind(int kind);
    stEmoticonWordInfo* GetEmoticonWordInfoByKind(int kind);
};
