#pragma once
#include <cstdint>
#include "Info/cltClimateKindInfo.h"
#include "Info/cltItemKindInfo.h"
#include "Info/cltResurrectInfo.h"
#include "global.h"

// 地圖類型，對應 MapInfo.txt 第10欄
enum eMapCaps {
    MAP_CAPS_NONE = 0,
    MAP_CAPS_FIELD = 1,
    MAP_CAPS_VILLAGE = 2,
    MAP_CAPS_DUNGEON = 3,
    MAP_CAPS_FFTV = 4,
    MAP_CAPS_BUNKER = 5,
    MAP_CAPS_PRISON = 6,
    MAP_CAPS_POSEIDOS = 7,
    MAP_CAPS_POSEIDOSSTART = 8,
    MAP_CAPS_FIELDSTART = 9, // 也用於 INDUN, CIRCLEROOM
    MAP_CAPS_FFTV1 = 11,
    MAP_CAPS_FFTV2 = 12,
    MAP_CAPS_LUNDWEL = 15,
    MAP_CAPS_LUNDWELSTART = 16,
    MAP_CAPS_WEDDINGHALL = 17,
    MAP_CAPS_SKYCASTLE = 18,
    MAP_CAPS_SKYCASTLESTART = 19,
};

// 地圖地塊類型，對應 MapInfo.txt 第24欄
enum eMapTile {
    MAP_TILE_NONE = 0,
    MAP_TILE_DARK_PILLAR = 1,
    MAP_TILE_LAWN = 2,
    MAP_TILE_GOLDYLAN = 3,
    MAP_TILE_CREMBILL = 4,
    MAP_TILE_GLENN_WOOD = 5,
    MAP_TILE_LIBITOWN = 6,
    MAP_TILE_GREENSTAR = 7,
    MAP_TILE_PAPIRION = 8,
    MAP_TILE_CORULMATIN = 9,
    MAP_TILE_WESTENPA = 10,
    MAP_TILE_NORTHENPA = 11,
    MAP_TILE_EASTNPA = 12,
    MAP_TILE_NABAHORN = 13,
    MAP_TILE_PAPERONPLAIN = 14,
    MAP_TILE_LESTIN = 15,
    MAP_TILE_INDUN = 16,
    MAP_TILE_CIRCLEROOM = 17,
};

// MapInfo.txt 中的單筆地圖資料結構 (大小: 376 bytes)
#pragma pack(push, 1)
struct stMapInfo {                          // Offset: 項目名稱
    unsigned short m_wID;                   // 0: 地圖 ID (Map ID)
    unsigned short m_wFileName;             // 2: 檔案名稱
    unsigned short m_wRegionCode;           // 4: 區域代碼
    unsigned short m_wExtRegionCode;        // 6: 擴展區域代碼

    unsigned int   m_dwFixedBG;             // 8: 固定背景
    unsigned int   m_dwMovingBG;            // 12: 移動背景
    unsigned int   m_dwMiddleBG;            // 16: 中層背景
    unsigned short m_wRegionType;           // 20: 區域分類
    char           _padding22[2];
    unsigned int   m_BG;                    // 24: 是否有背景 (Has Background)

    char m_szBGM1[64];                      // 28: 背景音樂1 (BGM1)
    char m_szBGM2[64];                      // 92: 背景音樂2 (BGM2)
    char m_szAmbientSoundFile[64];          // 156: 環境音效 (Ambient Sound)
    unsigned int m_dwWorldMapPosX;          // 220: 世界地圖 X 座標 
    unsigned int m_dwWorldMapPosY;          // 224: 世界地圖 Y 座標 
    unsigned int m_dwTeleportX;             // 228: 瞬間移動 X 座標 
    unsigned int m_dwTeleportY;             // 232: 瞬間移動 Y 座標 
    unsigned int m_dwDragonMapPosX;         // 236: 飛龍地圖 X 座標 
    unsigned int m_dwDragonMapPosY;         // 240: 飛龍地圖 Y 座標 
    unsigned int m_dwShipMovePosX;          // 244: 船隻移動 X 座標 
    unsigned int m_dwShipMovePosY;          // 248: 船隻移動 Y 座標 
    unsigned char  m_byAreaType;            // 252: 區域類型 
    unsigned char  m_byteContinentType;     // 253: 大陸分類 
    char           _padding254[2];

    unsigned int   m_dwResourceID;          // 256: 資源 ID 
    unsigned short m_wBlockID;              // 260: 區塊 ID 
    unsigned short m_wRegionNameCode;       // 262: 區域名稱代碼 
    unsigned short m_wResurrectPos;         // 264: 重生位置
    unsigned short m_wTeleportDestX;        // 266: 瞬移到達 X 座標 
    unsigned short m_wTeleportDestY;        // 268: 瞬移到達 Y 座標 
    unsigned short m_wShipDragonDestX;      // 270: 船／飛龍到達 X 座標 
    unsigned short m_wShipDragonDestY;      // 272: 船／飛龍到達 Y 座標 
    char           _padding274[2];
    unsigned int   m_dwMiniMapResourceFile; // 276: 小地圖資源檔案 
    unsigned short m_wMiniMapBlockID;       // 280: 小地圖區塊 ID 
    char           _padding282[2];
    unsigned int   m_dwSecretSpace;         // 284: 秘密空間 
    unsigned short m_wLevelRequired;        // 288: 等級限制 
    char           _padding290[2];
    unsigned int   m_dwIsMarket;            // 292: 是否為市場 
    unsigned int   m_dwIsBattleArea;        // 296: 是否為可戰鬥區域 
    unsigned short m_wClimate;              // 300: 天氣 (Climate)

    unsigned char  m_byIsPKArea;            // 302: 是否為對抗戰地區 
    char           _padding303;
    unsigned int   m_dwCanOpenPK;           // 304: 是否可開啟對抗戰 
    unsigned int   m_dwAreaDomination;      // 308: 區域制霸
    unsigned int   m_dwPKLobby;             // 312: 是否可在 PK 地圖（等待室） 
    unsigned int   m_dwPKField;             // 316: 是否可在 PK 地圖（狩獵場） 
    unsigned int   m_dwMonsterAttBoost;     // 320: 怪物強化（攻擊力 %） 
    unsigned int   m_dwMonsterDefBoost;     // 324: 怪物強化（防禦力） 
    unsigned int   m_dwMonsterExpBoost;     // 328: 怪物強化（經驗值 %） 

    unsigned int   m_dwDropRate1;           // 332: 掉落機率 (Drop Rate)
    unsigned short m_wDropItemID1[5];       // 336: 掉落物品ID 1-5 (Drop Item ID 1-5)
    unsigned short m_wDropItemMin1;         // 346: 最小數量 (Min Amount)
    unsigned short m_wDropItemMax1;         // 348: 最大數量 (Max Amount)
    unsigned char  m_byteDropItemCount1;    // 350: 掉落物品數量 (Number of Drop Items)
    char           _padding351;
    unsigned int   m_dwDropRate2;           // 352: 掉落機率 (Drop Rate)
    unsigned short m_wDropItemID2[5];       // 356: 掉落物品ID 6-10 (Drop Item ID 6-10)
    unsigned short m_wDropItemMin2;         // 366: 最小數量 (Min Amount)
    unsigned short m_wDropItemMax2;         // 368: 最大數量 (Max Amount)
    unsigned char  m_byteDropItemCount2;    // 370: 掉落物品數量 (Number of Drop Items)
    char           _padding371;
    unsigned short m_wDungeonNameCode;      // 372: 地下城名稱代碼 (Dungeon Name Code)
    char           _padding374[2];
};
#pragma pack(pop)

static_assert(sizeof(stMapInfo) == 376, "stMapInfo size must be 376 bytes");


class cltMapInfo : cltClimateKindInfo {
public:
    cltMapInfo();
    virtual ~cltMapInfo();

    // 初始化，讀取指定的 MapInfo.txt 檔案
    bool Initialize(char* filename);
    // 釋放所有已分配的記憶體
    void Free();

    // 獲取地圖總數
    unsigned short GetTotalMapNum() const;
    // 獲取PK地圖總數
    unsigned short GetTotalPKMapNum() const;

    // 根據索引獲取地圖資訊
    stMapInfo* GetMapInfoByIndex(int index);
    // 根據地圖ID獲取地圖資訊
    stMapInfo* GetMapInfoByID(unsigned short map_id);
    // 根據地圖名稱獲取地圖資訊
    stMapInfo* GetMapInfoByMapName(char* map_name);

    // 根據地下城名稱代碼，獲取符合條件的地圖數量，並將ID存入 m_DungeonMapKinds
    int GetMapKindCountByDungeonNameCode(int dungeon_name_code);

    // 各種屬性獲取函數
    unsigned short GetMapCaps(unsigned short map_id);
    bool GetDrawBG(unsigned short map_id);
    unsigned int GetMiniMapPosX(unsigned short map_id);
    unsigned int GetMiniMapPosY(unsigned short map_id);
    unsigned short GetWarpX(unsigned short map_id);
    unsigned short GetWarpY(unsigned short map_id);
    unsigned short GetTeleportX(unsigned short map_id);
    unsigned short GetTeleportY(unsigned short map_id);
    unsigned short GetAreaVorF(unsigned short map_id);
    unsigned int GetRestrictionLevel(unsigned short map_id);
    unsigned short GetMapID(unsigned char match_map_id);
    unsigned short GetProvideMapID();
    bool IsProvideMatch(unsigned short map_id);
    bool IsMatchMap(unsigned short map_id);

    // 靜態工具函數
    static void InitializeStaticVariable(DCTTextManager* text_manager);
    static char* TranslateMapID(unsigned short map_id);
    static unsigned short TranslateKindCode(char* a1);
    static bool CanUseReturnItem(unsigned short map_id);
    static bool CanUseTeleportItem(unsigned short map_id);
    static bool CanMoveTeleportItem(unsigned short map_id);
    static bool CanMoveTeleportDragon(unsigned short map_id);
    static bool CanUserTakeShip(unsigned short map_id);

private:
    // 將所有PK地圖的ID收集到 m_pPKMapKindArray
    void SetPKMapKindAll();

    // --- 成員變數 ---
    stMapInfo* m_pMapInfoArray;       // 指向所有地圖資訊的陣列
    unsigned short m_wTotalMapNum;    // 地圖總數量
    unsigned short m_wPKMapCount;     // PK地圖的數量
    unsigned short* m_pPKMapKindArray; // 指向所有PK地圖ID的陣列

    // 用於 GetMapKindCountByDungeonNameCode 的結果緩存
    unsigned short m_DungeonMapKinds[50];

    // --- 靜態成員變數 ---
    static DCTTextManager* m_pclTextManager;
    static unsigned int* m_dwMapTypeAtb; // 地圖類型屬性表
};
