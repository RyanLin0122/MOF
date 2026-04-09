#pragma once
#include <cstdint>
#include <cstring>
#include <map>
#include <windows.h>

// ============================================================================
// stPlayerInfo — PK 房間內的玩家資訊 (548 bytes)
// 從 mofclient.c 反推的記憶體佈局
// ============================================================================
struct stPlayerInfo {
    unsigned int accountID;      // offset 0   (DWORD[0])
    unsigned int hostAccountID;  // offset 4   (DWORD[1])
    unsigned short level;        // offset 8   (WORD[4])
    unsigned short _pad0;        // offset 10
    int state;                   // offset 12  (DWORD[3])  — defaults -1
    int charKind;                // offset 16  (DWORD[4])
    int classType;               // offset 20  (DWORD[5])  — defaults -1
    int supportTeam;             // offset 24  (DWORD[6])  — defaults -2
    int unk1;                    // offset 28  (DWORD[7])
    int unk2;                    // offset 32  (DWORD[8])
    char name[256];              // offset 36
    char supportName[256];       // offset 292

    stPlayerInfo();
    void Init();
    void* operator=(void* a2);
};

// ============================================================================
// stPKRoomInfo — PK 房間資訊 (7144 bytes = 568 header + 12*548 players)
// 從 mofclient.c 反推的記憶體佈局
// ============================================================================
struct stPKRoomInfo {
    unsigned int handleID;       // offset 0   (DWORD[0])
    unsigned int hostAccountID;  // offset 4   (DWORD[1])
    int roomNum;                 // offset 8   (DWORD[2])
    int unk1;                    // offset 12  (DWORD[3])
    unsigned short unkShort;     // offset 16  (WORD[8])
    unsigned short _pad0;        // offset 18
    int gameLevel;               // offset 20  (DWORD[5])  — defaults -1
    int roomType;                // offset 24  (DWORD[6])  — defaults -1
    int unk3;                    // offset 28  (DWORD[7])  — defaults -1
    int lockState;               // offset 32  (DWORD[8])  — defaults -1
    int unk4;                    // offset 36  (DWORD[9])
    int unk5;                    // offset 40  (DWORD[10])
    int unk6;                    // offset 44  (DWORD[11])
    int costType;                // offset 48  (DWORD[12]) — defaults -2
    char isStarted;              // offset 52  (BYTE[52])
    char unk7;                   // offset 53  (BYTE[53])
    char roomName[256];          // offset 54
    char password[256];          // offset 310
    unsigned short _pad1;        // offset 566
    stPlayerInfo players[12];    // offset 568

    void Init();
    char* operator=(char* a2);
};

// ============================================================================
// cltPKManager — PK 房間管理器
// 從 mofclient.c 反推
// ============================================================================
static constexpr int MAX_PK_PLAYERS = 12;

class cltPKManager {
public:
    cltPKManager();
    virtual ~cltPKManager();

    void Release();
    void Init();

    // 房間列表管理
    void AddPKRoomInfoByLobby(stPKRoomInfo* pInfo);
    int  GetGameLevelByHandle(unsigned int handleID);
    void SetPKRoomInfoCreate(stPKRoomInfo* pInfo);
    int  CopyPKRoomInfoFromList(int gameLevel, int index);
    void DestroyRoom(unsigned int handleID);
    void UpdatePKRoomInfo(stPKRoomInfo* pInfo);
    int  GetPKRoomNumByGameLevel(int gameLevel);

    // 隊伍查詢
    int  GetTeamByAccount(unsigned int accountID);
    BOOL IsMyTeam(unsigned int accountID);
    int  GetTeamByIndex(int index);

    // 房間加入檢查
    int  CheckQuickJoinRoom(int gameLevel);
    int  CheckPlayerNum();
    int  CheckClickJoinRoom();

    // 玩家索引查詢
    int  GetIndexbyAccount(unsigned int accountID);
    int  GetMyIndex();
    BOOL IsMyIndex(int index);
    int  GetIndexByCharName(char* name);
    int  GetSpaceIndexbyTeam(int team);

    // 玩家資訊存取
    int  GetPKRoomAccountNum();
    int  GetAccountNumByTeam(int team);
    int  GetReadyNumByTeam(int team);
    int  GetPlayerNum();
    int  IsAllReady();
    BOOL CheckTeamPlayerNum();
    int  GetObserverNum();

    // 支援隊伍
    BOOL IsSupportTeamPlayer(int index);
    void ProcessTeamMoveObserver();

    // 名字存取
    char* GetNameByIndex(int index);
    void  ResetSupportNameByIndex(int index);
    void  SetSupportNameByIndex(int index, char* name);
    char* GetSupportNameByIndex(int index);

    // 支援隊伍存取
    void SetSupportTeamByIndex(int index, int team);
    int  GetSupportTeamByIndex(int index);

    // 狀態存取
    void SetStateByIndex(int index, int state);
    int  GetStateByIndex(int index);

    // 職業類型存取
    void SetClassTypeByIndex(int index, int classType);
    int  GetClassTypeByIndex(int index);

    // 玩家管理
    void AddPlayer(int index, unsigned int accountID, char* name,
                   int charKind, unsigned short level, int classType,
                   int state, unsigned int hostAccountID);

    // 費用
    int  GetCost(int level, int team);

    // 公開成員 — 提供給外部直接存取 (對齊 GT 用法)
    stPKRoomInfo m_currentRoom;

private:
    // 5 個 map，依 gameLevel 分類 (0~4)
    std::map<unsigned long, stPKRoomInfo*> m_mapRooms[5];

    // 迭代器狀態 (跨方法共用)
    std::map<unsigned long, stPKRoomInfo*>::iterator m_iter;
    int m_iterMapIdx;

    // offset 7232: 額外資料區
    char m_extraData[48];

    // offset 7280+
    int m_gameLevelSetting;      // DWORD[1820], defaults -1
    int m_roomTypeSetting;       // DWORD[1821], defaults -1
    int m_unk_7288;              // DWORD[1822], defaults -1
    unsigned short m_unkShort;   // WORD[3646], defaults 0
    char m_unkName1[256];        // offset 7294
    char m_unkName2[256];        // offset 7550
};
