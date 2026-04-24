#include "Logic/Map.h"
#include "Info/cltMapInfo.h"
#include "Info/cltClimateKindInfo.h"

Map::Map() {}
Map::~Map() {}

void Map::CreateMap(unsigned short /*mapID*/, ClientCharacter* /*pChar*/) {}
void Map::CheckMap() {}
int  Map::InitMap() { return 0; }
void Map::UnInitMap() {}

unsigned short Map::GetMapCaps(unsigned short /*mapID*/) { return 0; }
unsigned short Map::GetTotalMapNum() { return 0; }
stMapInfo*     Map::GetMapInfoByID(unsigned short /*mapID*/) { return nullptr; }
stMapInfo*     Map::GetMapInfoByIndex(int /*index*/) { return nullptr; }
unsigned short Map::GetMapAreaName(unsigned short /*mapID*/) { return 0; }
char*          Map::GetMapID(unsigned short /*mapID*/) { return nullptr; }

void Map::SetViewChar(ClientCharacter* /*pChar*/) {}
void Map::Poll() {}
void Map::PrepareDrawing() {}
void Map::DrawMap() {}
void Map::DrawClip() {}
void Map::PrepareDrawingClimate() {}
void Map::DrawClimate() {}
strClimateInfo* Map::GetClimateKindByMapId(uint16_t /*mapId*/) { return nullptr; }

// mofclient.c: match-lobby map recognition is checked via a map-kind flag
// bit.  Until cltMapInfo is fully mapped, return false (treat every map as
// a normal field map); ClientCharacter::CreateCharacter only uses this to
// decide whether a hp=0 spawn should push a Died order — on a true lobby
// map the server never sends hp=0 in the first place.
bool Map::IsMatchLobbyMap(uint16_t /*mapId*/) { return false; }

int Map::MapXtoScreenX(int x) { return x; }
int Map::MapYtoScreenY(int y) { return y; }

// --- 地圖區域標題資源 (從 mofclient.c 還原) ---

unsigned int Map::GetMapAreaTitleResourceID(unsigned short mapID)
{
    stMapInfo* info = GetMapInfoByID(mapID);
    if (info)
        return info->m_dwResourceID;
    return 0;
}

unsigned short Map::GetMapAreaTitleBlockID(unsigned short mapID)
{
    stMapInfo* info = GetMapInfoByID(mapID);
    if (info)
        return info->m_wBlockID;
    return 0;
}
