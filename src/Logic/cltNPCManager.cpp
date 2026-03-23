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
