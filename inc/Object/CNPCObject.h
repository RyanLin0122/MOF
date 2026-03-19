#pragma once

#include <cstdint>

// CNPCObject — NPC 管理器（無狀態 facade，委託 CObjectManager 管理實際物件）
// 全域單例 g_clNPCObject
class CNPCObject {
public:
    CNPCObject();
    virtual ~CNPCObject();

    void AddNPC(std::uint16_t npcID, unsigned int resourceID, std::uint16_t maxFrames,
                float posX, float posY,
                std::uint16_t nameTextCode, std::uint16_t titleTextCode,
                int flipFlag, int toggleFlag);

    void AddQuestMark(std::uint16_t npcID);
    void AddRewardMark(std::uint16_t npcID);
    void DelQuestMark(std::uint16_t npcID);
    void DelRewardMark(std::uint16_t npcID);
};

extern CNPCObject g_clNPCObject;
