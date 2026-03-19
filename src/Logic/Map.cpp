#include "Logic/Map.h"

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
