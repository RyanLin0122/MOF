#include "Logic/Map.h"
#include "Info/cltMapInfo.h"

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
