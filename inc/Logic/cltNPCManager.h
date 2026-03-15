#pragma once
#include <cstdint>

class cltNPCManager {
public:
    cltNPCManager() = default;
    ~cltNPCManager() = default;

    // Spawns all NPCs registered for the given map.
    void AddAllNPC(unsigned int mapKind);
};

extern cltNPCManager g_clNPCManager;
