#include "Logic/cltNPCManager.h"
#include "Info/cltNPCInfo.h"
#include "Logic/Map.h"
#include "global.h"

void cltNPCManager::AddAllNPC(unsigned int /*mapKind*/) {
    // Stub: real implementation loads and spawns NPC entries for the map.
}

int cltNPCManager::GetPosXByID(std::uint16_t /*npcID*/) {
    // TODO: 遍歷內部 NPC 列表，根據 NPC ID 回傳 X 座標
    // Ground truth: 透過 cltClient_NPC::GetNPCInfo 搜尋，回傳 stNPCInfo offset 20 (DWORD +5)
    return 0;
}

int cltNPCManager::GetPosYByID(std::uint16_t /*npcID*/) {
    // TODO: 遍歷內部 NPC 列表，根據 NPC ID 回傳 Y 座標
    // Ground truth: 透過 cltClient_NPC::GetNPCInfo 搜尋，回傳 stNPCInfo offset 24 (DWORD +6)
    return 0;
}

std::uint16_t cltNPCManager::GetMapName(std::uint16_t npcID) {
    std::uint16_t mapId = g_clNPCInfo.GetMapID(npcID);
    return g_Map.GetMapAreaName(mapId);
}

// GT 0x413DE0 / 0x413FA0 — stubbed until cltClient_NPC is restored. Returning
// 0 / static zero buffer means cltMyCharData::GetMiniGameKind reports 0 (no
// mini-game NPC selected) and AddQuestMark walks zero entries — safe no-op
// behavior that does not crash callers.
std::uint16_t cltNPCManager::GetNPCID(int /*index*/) {
    return 0;
}

std::uint8_t* cltNPCManager::GetNPCType() {
    static std::uint8_t kZeroNPCType[8] = {};
    return kZeroNPCType;
}
