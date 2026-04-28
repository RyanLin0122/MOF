#pragma once
#include <cstdint>
#include <cstdio>
#include <list>
#include "Image/Image.h"
#include "Info/cltMapInfo.h"
#include "Info/cltMapCollisonInfo.h"
#include "Logic/cltMap_Climate.h"
#include "Logic/cltMap_WeddingFlower.h"
#include "Logic/CMap_Item_Climate.h"

class ClientCharacter;
class GameImage;

// Map — 主要遊戲地圖物件（mofclient.c：g_Map 全域實體，0x4E1A60 起）。
//
// 與 cltMapInfo（純資料表）不同，Map 同時負責：
//   1) 載入單張地圖檔（.map 檔頭、tile / portal 陣列、碰撞點陣）
//   2) 依視角角色推進相機捲動（m_iScrollX / m_iScrollY）
//   3) 統合天氣（cltMap_Climate）、婚禮花瓣（cltMap_WeddingFlower）、
//      MapItemClimate list 等子系統的 Poll / Draw 流程
//   4) 產生 portal / NPC / 動畫物件（透過 g_ObjectManager 等）
//
// 32-bit decomp 中 Map 物件約 20240 bytes，含內嵌 Image×2、cltMap_Climate、
// cltMap_WeddingFlower、cltMapInfo、cltMapCollisonInfo。本還原以命名欄位
// 取代偏移；x64 下 size 不對齊原 binary 但行為等價。
class Map {
public:
    Map();
    ~Map();

    // ---- Lifecycle ------------------------------------------------------
    int  InitMap();        // mofclient.c 0x4E3150：載入 mapinfo / climate
    void UnInitMap();      // mofclient.c 0x4E3190：釋放 tile / portal 陣列等
    void CheckMap();       // mofclient.c 0x4E1D00：debug — 輸出 MapFileInfo.txt
    void CreateMap(unsigned short mapID, ClientCharacter* pChar);  // 0x4E25C0

    // ---- Per-frame -------------------------------------------------------
    void Poll();                                         // 0x4E2B90
    void Poll_VibrationMode(int mode, float arg, int);   // 0x4E37A0
    void PrepareDrawing();                               // 0x4E2CC0
    void DrawMap();                                      // 0x4E3060
    void DrawClip();                                     // 0x4E3D00
    void PrepareDrawingClimate();                        // 0x4E3300
    void DrawClimate();                                  // 0x4E3340
    void GetClimateIconResouce(unsigned short mapID,
                                unsigned int* outResId,
                                unsigned short* outBlockId);  // 0x4E3380

    // ---- 視角 ------------------------------------------------------------
    void  SetViewChar(ClientCharacter* pChar);          // 0x4E29A0
    int   MapXtoScreenX(int x);                         // 0x408430
    int   MapYtoScreenY(int y);                         // 0x408440
    int   GetScrollX() const;
    int   GetScrollY() const;

    // ---- cltMapInfo / cltClimateKindInfo 委派 ----------------------------
    stMapInfo*     GetMapInfoByID(unsigned short mapID);     // 0x4E32B0
    stMapInfo*     GetMapInfoByIndex(int index);             // 0x4E32D0
    unsigned short GetTotalMapNum();                         // 0x4E32F0
    char*          GetMapID(unsigned short mapID);           // 0x4E33B0
    unsigned short GetMapAreaName(unsigned short mapID);          // 0x4E33C0
    unsigned short GetExtensionMapAreaName(unsigned short mapID); // 0x4E33E0
    char*          GetMapFileName(unsigned short mapID);          // 0x4E3400
    unsigned int   GetMapBgResourceID(unsigned short mapID);      // 0x4E3480
    unsigned int   GetMapNearResourceID(unsigned short mapID);    // 0x4E34A0
    unsigned int   GetMapMiddleBgResourceID(unsigned short mapID);// 0x4E34C0
    unsigned int   GetMapAreaTitleResourceID(unsigned short mapID);// 0x4E34E0
    unsigned short GetMapAreaTitleBlockID(unsigned short mapID);  // 0x4E3500
    unsigned short GetMapCaps(unsigned short mapID);              // 0x4E3520
    unsigned short GetMapVorF(unsigned short mapID);              // 0x4E3540
    unsigned short GetRestrictionLevel(unsigned short mapID);     // 0x4E3560
    unsigned int   GetDrawBG(unsigned short mapID);               // 0x4E3580
    char*          GetBGMFileName(unsigned short mapID);          // 0x4E35A0
    char*          GetAmbientFileName(unsigned short mapID);      // 0x4E3650
    unsigned short GetClimateKind(unsigned short mapID);          // 0x4E36C0
    bool           IsMatchLobbyMap(unsigned short mapID);         // 0x4E36E0
    bool           IsPvPMap(unsigned short mapID);                // 0x4E3700
    bool           IsPvPLobbyMap(unsigned short mapID);           // 0x4E3720
    bool           IsContinent(unsigned short mapID);             // 0x4E3740
    strClimateInfo* GetClimateKindByMapId(unsigned short mapID);  // 0x4E3760
    strClimateInfo* GetClimateKindByClimateKind(unsigned short kind); // 0x4E3780

    // ---- Map item climate（道具觸發的天氣效果）-------------------------
    void StartMapItemClimate(unsigned short itemID,
                              unsigned int duration, char* userName);  // 0x4E3930
    void EndMapItemClimate(unsigned short itemID);                     // 0x4E3BB0
    void DeleteAllMapItemClimate();                                    // 0x4E3C50
    CMap_Item_Climate* GetMapItemClimate(int index);                   // 0x4E3CA0
    int                GetMapItemClimateNumber();                      // 0x4E3CF0

    // ---- 內嵌子系統（mofclient.c 偏移 +428 / +14444 / +19612 / +19776）
    //   本還原採用命名成員；offset 不對齊原 binary 但行為等價。
    cltMap_Climate       m_clMap_Climate;
    cltMap_WeddingFlower m_clMap_WeddingFlower;
    cltMapInfo           m_clMapInfo;
    cltMapCollisonInfo   m_clMapCollisonInfo;

    cltMapCollisonInfo* GetMapCollisonInfo() { return &m_clMapCollisonInfo; }

public:
    // ---- 檔頭 / tile array / portal array（mofclient.c 偏移 0..27）-------
    // mofclient.c .map 檔頭結構（byte offset 0..27）：本還原以命名欄位重現
    // 語意；x64 下 m_pTileInfo / m_pPortalInfo 為 8-byte 指標，但檔讀寫只
    // 操作 fread/fwrite 的固定大小，故與 binary 行為等價。
    unsigned int     m_dwType;            // 檔頭 +0..3：版本（11 = 含碰撞點陣）
    unsigned int     m_dwHeader_4;        // 檔頭 +4..7：保留欄位
    unsigned short   m_wHeader_8;         // 檔頭 +8..9
    unsigned short   m_wHeader_10;        // 檔頭 +10..11
    unsigned short   m_wTileWidth;        // 檔頭 +12..13：tile-X 數
    unsigned short   m_wTileHeight;       // 檔頭 +14..15：tile-Y 數
    void*            m_pTileInfo;         // dtor 釋放：每 tile 24 bytes 的陣列
    unsigned short   m_wPortalCount;      // 檔頭續：+20..21，portal 數
    void*            m_pPortalInfo;       // dtor 釋放：每 portal 12 bytes 的陣列

    // ---- 背景 / tile image 指標（DWORD+100..106） ------------------------
    GameImage**      m_ppTileImageArray;  // DWORD+101：tile 影像指標陣列
    GameImage*       m_pBgImage1;         // DWORD+102
    GameImage*       m_pBgImage2;         // DWORD+103
    GameImage*       m_pBgImageMiddle;    // DWORD+104
    unsigned int     m_dwBgResource1;     // DWORD+105
    unsigned int     m_dwBgResource2;     // DWORD+106

    // ---- 鏡射 / 相機 / 螢幕（DWORD+4895..4943）--------------------------
    int              m_iTileDrawCount;    // DWORD+4895（=5 或 7，依解析度）
    int              m_iWideOffset;       // DWORD+4896：>800 寬螢幕時的 letterbox offset
    int              m_iScrollX;          // DWORD+4898
    int              m_iScrollY;          // DWORD+4899
    int              m_iBgScrollX;        // DWORD+4900
    int              m_iBgScrollY;        // DWORD+4901
    ClientCharacter* m_pViewChar;         // DWORD+4902
    unsigned short   m_wTileWidth2;       // WORD+9876（鏡射）
    unsigned short   m_wTileHeight2;      // WORD+9877（鏡射）
    unsigned short   m_wPortalCount2;     // WORD+9878（鏡射）
    int              m_iMapPixelWidth;    // DWORD+4933 = m_wTileWidth * 256
    int              m_iMapWidthClipped;  // DWORD+4934 = pixelWidth - g_Game_System_Info
    int              m_iMapPixelHeight;   // DWORD+4935 = m_wTileHeight * 256
    int              m_iMapHeightClipped; // DWORD+4936 = pixelHeight - nHeight
    int              m_iTotalTileCount;   // DWORD+4937 = tileW * tileH
    int              m_iHasDrawBG;        // DWORD+4940
    int              m_iHasBg1;           // DWORD+4941
    int              m_iHasBg2;           // DWORD+4942
    int              m_iHasBg3;           // DWORD+4943
    unsigned short   m_wTopOffsetForBg;   // WORD+9894
    int              m_iClipRange0;       // DWORD+4948
    int              m_iClipRange1;       // DWORD+4949
    int              m_iClipRange2;       // DWORD+4950

    // ---- 碰撞圖大小（mofclient.c WORD+9866 / WORD+9870）-----------------
    //   讀的是 m_iMapPixelWidth / m_iMapPixelHeight 的 low16，這裡命名為
    //   獨立欄位以便 CreateMap 寫入時保留語意。
    unsigned short   m_wCollisionWidth;   // WORD+9866 = (uint16) m_iMapPixelWidth
    unsigned short   m_wCollisionHeight;  // WORD+9870 = (uint16) m_iMapPixelHeight

    // ---- 雜項（DWORD+5047..5050 + 5052..5053）---------------------------
    unsigned int     m_dwMapItemEffect_1; // DWORD+5047：StartMapItemClimate 寫入
    unsigned int     m_dwMapItemEffect_2; // DWORD+5048
    unsigned int     m_dwMapItemEffect_3; // DWORD+5049
    float            m_fMonsterShake;     // float+5050：怪物頭頂抖動倍率
    std::list<CMap_Item_Climate*> m_lstMapItemClimate;  // mofclient.c +20208 雙向鏈結串列

    // ---- 路徑暫存區（mofclient.c +19804 / +19932 / +20060）-------------
    char             m_szMapFilePath[128]; // GetMapFileName 暫存
    char             m_szBGMFilePath[128]; // GetBGMFileName 暫存
    char             m_szAmbientPath[128]; // GetAmbientFileName 暫存

    // ---- 兩張背景 Image（mofclient.c 偏移 +28 / +208）------------------
    Image            m_image1;
    Image            m_image2;
};
