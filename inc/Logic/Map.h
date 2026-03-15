#pragma once
#include <cstdint>

class ClientCharacter;
struct stMapInfo;

// Stub for the Map class (g_Map in binary).
// Map is the main gameplay/rendering map object, distinct from cltMapInfo (which is a data-only info class).
class Map {
public:
    Map();
    ~Map();

    // Map lifecycle
    void CreateMap(unsigned short mapID, ClientCharacter* pChar);
    void CheckMap();
    int  InitMap();
    void UnInitMap();

    // Query helpers — stubs delegate to internal cltMapInfo if/when implemented
    unsigned short GetMapCaps(unsigned short mapID);
    unsigned short GetTotalMapNum();
    stMapInfo*     GetMapInfoByID(unsigned short mapID);
    stMapInfo*     GetMapInfoByIndex(int index);
    unsigned short GetMapAreaName(unsigned short mapID);
    char*          GetMapID(unsigned short mapID);

    // Character / viewport
    void SetViewChar(ClientCharacter* pChar);

    // Per-frame
    void Poll();
    void PrepareDrawing();
    void DrawMap();
    void PrepareDrawingClimate();
    void DrawClimate();

    // Coord conversion
    int MapXtoScreenX(int x);
    int MapYtoScreenY(int y);
};
