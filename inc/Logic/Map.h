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
    void DrawClip();
    void PrepareDrawingClimate();
    void DrawClimate();
    struct strClimateInfo* GetClimateKindByMapId(uint16_t mapId);

    // mofclient.c: used by ClientCharacter::CreateCharacter to decide whether
    // a hp=0 spawn should auto-push a Died order (non-lobby only).
    bool IsMatchLobbyMap(uint16_t mapId);

    // Coord conversion
    int MapXtoScreenX(int x);
    int MapYtoScreenY(int y);

    // 螢幕捲動偏移（mofclient.c：m_pMap+19592 / +19596）。
    // 反編譯版本將兩個整數內嵌於 Map 物件中；本還原把它們存於全域
    // dword_A73088 / dword_A7308C，以便 ObjectManager / ScreenChange
    // 等已存在的呼叫面共用同一塊資料。
    int GetScrollX() const;
    int GetScrollY() const;

    // Map title resource (從 stMapInfo 取得地圖區域標題資源)
    unsigned int   GetMapAreaTitleResourceID(unsigned short mapID);
    unsigned short GetMapAreaTitleBlockID(unsigned short mapID);
};
