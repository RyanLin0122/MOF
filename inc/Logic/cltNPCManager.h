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

    // GT 0x413DE0 / 0x413FA0 — these access cltClient_NPC instances inside the
    // manager. The cltClient_NPC class hasn't been ported yet; these helpers
    // provide safe defaults (ID=0 / type=all-zero) until the underlying
    // class is restored. They are required by:
    //   - cltMyCharData::GetMiniGameKind  → reads NPC type bytes [3..6]
    //   - cltMyCharData::AddQuestMark     → iterates map's NPC IDs
    std::uint16_t GetNPCID(int index);
    std::uint8_t* GetNPCType();
};

extern cltNPCManager g_clNPCManager;
