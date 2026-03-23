#pragma once
#include <cstdint>

class cltNPCManager {
public:
    cltNPCManager() = default;
    ~cltNPCManager() = default;

    // Spawns all NPCs registered for the given map.
    void AddAllNPC(unsigned int mapKind);

    // 取得 NPC 的世界座標 (根據 NPC ID)
    int GetPosXByID(std::uint16_t npcID);
    int GetPosYByID(std::uint16_t npcID);

    // Returns the text code for the map area name where this NPC resides.
    std::uint16_t GetMapName(std::uint16_t npcID);
};

extern cltNPCManager g_clNPCManager;
