#include "Logic/cltPKManager.h"
#include "Logic/cltMyCharData.h"
#include "global.h"
#include <cstring>
#include <cstdlib>

// ============================================================================
// stPlayerInfo
// ============================================================================

stPlayerInfo::stPlayerInfo()
{
    accountID = 0;
    hostAccountID = 0;
    level = 0;
    _pad0 = 0;
    state = -1;
    charKind = 0;
    classType = -1;
    supportTeam = -2;
    unk1 = 0;
    unk2 = 0;
    std::memset(name, 0, 256);
    std::memset(supportName, 0, 256);
}

void stPlayerInfo::Init()
{
    state = -1;
    classType = -1;
    accountID = 0;
    hostAccountID = 0;
    level = 0;
    charKind = 0;
    supportTeam = -2;
    unk2 = 0;
    unk1 = 0;
    std::memset(name, 0, 256);
    std::memset(supportName, 0, 256);
}

void* stPlayerInfo::operator=(void* a2)
{
    if (this != a2)
    {
        stPlayerInfo* src = (stPlayerInfo*)a2;
        accountID = src->accountID;
        hostAccountID = src->hostAccountID;
        level = src->level;
        state = src->state;
        charKind = src->charKind;
        classType = src->classType;
        unk1 = src->unk1;
        unk2 = src->unk2;
        supportTeam = src->supportTeam;
        std::strcpy(name, src->name);
        std::strcpy(supportName, src->supportName);
    }
    return this;
}

// ============================================================================
// stPKRoomInfo
// ============================================================================

void stPKRoomInfo::Init()
{
    handleID = 0;
    hostAccountID = 0;
    unkShort = 0;
    gameLevel = -1;
    roomType = -1;
    unk3 = -1;
    lockState = -1;
    unk4 = 0;
    unk5 = 0;
    unk6 = 0;
    isStarted = 0;
    roomNum = 0;
    unk1 = 0;
    costType = -2;
    unk7 = 0;
    std::memset(roomName, 0, 256);
    std::memset(password, 0, 256);
    for (int i = 0; i < 12; i++)
    {
        players[i].Init();
    }
}

char* stPKRoomInfo::operator=(char* a2)
{
    stPKRoomInfo* src = (stPKRoomInfo*)a2;
    if ((char*)this == a2)
        return (char*)this;

    handleID = src->handleID;
    hostAccountID = src->hostAccountID;
    unkShort = src->unkShort;
    gameLevel = src->gameLevel;
    roomType = src->roomType;
    unk3 = src->unk3;
    lockState = src->lockState;
    unk4 = src->unk4;
    unk5 = src->unk5;
    unk6 = src->unk6;
    isStarted = src->isStarted;
    roomNum = src->roomNum;
    unk1 = src->unk1;
    costType = src->costType;
    unk7 = src->unk7;
    std::strcpy(roomName, src->roomName);
    std::strcpy(password, src->password);
    for (int i = 0; i < 12; i++)
    {
        players[i] = (stPlayerInfo*)&src->players[i];
    }
    return (char*)this;
}

// ============================================================================
// cltPKManager
// ============================================================================

cltPKManager::cltPKManager()
{
    // maps are default-constructed by std::map

    // Initialize embedded stPKRoomInfo players
    for (int i = 0; i < 12; i++)
    {
        // stPlayerInfo constructor is called automatically
    }

    // Initialize current room fields
    m_currentRoom.handleID = 0;
    m_currentRoom.hostAccountID = 0;
    m_currentRoom.unkShort = 0;
    m_currentRoom.gameLevel = -1;
    m_currentRoom.roomType = -1;
    m_currentRoom.unk3 = -1;
    m_currentRoom.lockState = -1;
    m_currentRoom.unk4 = 0;
    m_currentRoom.unk5 = 0;
    m_currentRoom.unk6 = 0;
    m_currentRoom.isStarted = 0;
    m_currentRoom.roomNum = 0;
    m_currentRoom.unk1 = 0;
    m_currentRoom.costType = -2;
    m_currentRoom.unk7 = 0;
    std::memset(m_currentRoom.roomName, 0, 256);
    std::memset(m_currentRoom.password, 0, 256);
    for (int i = 0; i < 12; i++)
    {
        m_currentRoom.players[i].accountID = 0;
        m_currentRoom.players[i].hostAccountID = 0;
        m_currentRoom.players[i].level = 0;
        m_currentRoom.players[i].state = -1;
        m_currentRoom.players[i].charKind = 0;
        m_currentRoom.players[i].classType = -1;
        m_currentRoom.players[i].supportTeam = -2;
        m_currentRoom.players[i].unk1 = 0;
        m_currentRoom.players[i].unk2 = 0;
        std::memset(m_currentRoom.players[i].name, 0, 256);
        std::memset(m_currentRoom.players[i].supportName, 0, 256);
    }

    // Initialize extra fields
    std::memset(m_extraData, 0, 48);
    m_gameLevelSetting = -1;
    m_roomTypeSetting = -1;
    m_unk_7288 = -1;
    m_unkShort = 0;
    std::memset(m_unkName1, 0, 256);
    std::memset(m_unkName2, 0, 256);

    m_iter = m_mapRooms[0].end();
}

cltPKManager::~cltPKManager()
{
    // Reset current room fields (GT: destructor resets before Release)
    m_currentRoom.handleID = 0;
    m_currentRoom.hostAccountID = 0;
    m_currentRoom.unkShort = 0;
    m_currentRoom.gameLevel = -1;
    m_currentRoom.roomType = -1;
    m_currentRoom.unk3 = -1;
    m_currentRoom.lockState = -1;
    m_currentRoom.unk4 = 0;
    m_currentRoom.unk5 = 0;
    m_currentRoom.unk6 = 0;
    m_currentRoom.isStarted = 0;
    m_currentRoom.roomNum = 0;
    m_currentRoom.unk1 = 0;
    m_currentRoom.costType = -2;
    m_currentRoom.unk7 = 0;
    std::memset(m_currentRoom.roomName, 0, 256);
    std::memset(m_currentRoom.password, 0, 256);
    for (int i = 0; i < 12; i++)
    {
        m_currentRoom.players[i].accountID = 0;
        m_currentRoom.players[i].hostAccountID = 0;
        m_currentRoom.players[i].level = 0;
        m_currentRoom.players[i].state = -1;
        m_currentRoom.players[i].charKind = 0;
        m_currentRoom.players[i].classType = -1;
        m_currentRoom.players[i].supportTeam = -2;
        m_currentRoom.players[i].unk1 = 0;
        m_currentRoom.players[i].unk2 = 0;
        std::memset(m_currentRoom.players[i].name, 0, 256);
        std::memset(m_currentRoom.players[i].supportName, 0, 256);
    }
    std::memset(m_extraData, 0, 48);

    Release();
    // maps are cleaned up by std::map destructor after Release deletes the pointers
}

void cltPKManager::Release()
{
    // Delete all stPKRoomInfo* in each map and clear
    for (int i = 0; i < 5; i++)
    {
        for (auto it = m_mapRooms[i].begin(); it != m_mapRooms[i].end(); ++it)
        {
            if (it->second)
            {
                delete it->second;
            }
        }
        m_mapRooms[i].clear();
    }

    // Reset current room fields
    m_currentRoom.handleID = 0;
    m_currentRoom.hostAccountID = 0;
    m_currentRoom.unkShort = 0;
    m_currentRoom.gameLevel = -1;
    m_currentRoom.roomType = -1;
    m_currentRoom.unk3 = -1;
    m_currentRoom.lockState = -1;
    m_currentRoom.unk4 = 0;
    m_currentRoom.unk5 = 0;
    m_currentRoom.unk6 = 0;
    m_currentRoom.isStarted = 0;
    m_currentRoom.roomNum = 0;
    m_currentRoom.unk1 = 0;
    m_currentRoom.costType = -2;
    m_currentRoom.unk7 = 0;
    std::memset(m_currentRoom.roomName, 0, 256);
    std::memset(m_currentRoom.password, 0, 256);
    for (int i = 0; i < 12; i++)
    {
        m_currentRoom.players[i].accountID = 0;
        m_currentRoom.players[i].hostAccountID = 0;
        m_currentRoom.players[i].level = 0;
        m_currentRoom.players[i].state = -1;
        m_currentRoom.players[i].charKind = 0;
        m_currentRoom.players[i].classType = -1;
        m_currentRoom.players[i].supportTeam = -2;
        m_currentRoom.players[i].unk1 = 0;
        m_currentRoom.players[i].unk2 = 0;
        std::memset(m_currentRoom.players[i].name, 0, 256);
        std::memset(m_currentRoom.players[i].supportName, 0, 256);
    }
    std::memset(m_extraData, 0, 48);
}

void cltPKManager::Init()
{
    m_gameLevelSetting = -1;
    m_roomTypeSetting = -1;
    m_unk_7288 = -1;
    m_unkShort = 0;
    std::memset(m_unkName1, 0, 256);
    std::memset(m_unkName2, 0, 256);
    std::memset(m_extraData, 0, 48);

    // Reset current room
    m_currentRoom.handleID = 0;
    m_currentRoom.hostAccountID = 0;
    m_currentRoom.unkShort = 0;
    m_currentRoom.gameLevel = -1;
    m_currentRoom.roomType = -1;
    m_currentRoom.unk3 = -1;
    m_currentRoom.lockState = -1;
    m_currentRoom.unk4 = 0;
    m_currentRoom.unk5 = 0;
    m_currentRoom.unk6 = 0;
    m_currentRoom.isStarted = 0;
    m_currentRoom.roomNum = 0;
    m_currentRoom.unk1 = 0;
    m_currentRoom.costType = -2;
    m_currentRoom.unk7 = 0;
    std::memset(m_currentRoom.roomName, 0, 256);
    std::memset(m_currentRoom.password, 0, 256);
    for (int i = 0; i < 12; i++)
    {
        m_currentRoom.players[i].accountID = 0;
        m_currentRoom.players[i].hostAccountID = 0;
        m_currentRoom.players[i].level = 0;
        m_currentRoom.players[i].state = -1;
        m_currentRoom.players[i].charKind = 0;
        m_currentRoom.players[i].classType = -1;
        m_currentRoom.players[i].supportTeam = -2;
        m_currentRoom.players[i].unk1 = 0;
        m_currentRoom.players[i].unk2 = 0;
        std::memset(m_currentRoom.players[i].name, 0, 256);
        std::memset(m_currentRoom.players[i].supportName, 0, 256);
    }
}

// ---- 房間列表管理 ----

void cltPKManager::AddPKRoomInfoByLobby(stPKRoomInfo* pInfo)
{
    int level = pInfo->gameLevel;
    if (level >= 0 && level <= 4)
    {
        m_mapRooms[level][pInfo->handleID] = pInfo;
    }
}

int cltPKManager::GetGameLevelByHandle(unsigned int handleID)
{
    // GT (mofclient.c:199444) 對每個 map 呼叫 find/lower_bound，
    // 並將結果寫回 m_iter (offset +84) 即使 find 失敗也寫入。
    for (int i = 0; i < 5; i++)
    {
        auto it = m_mapRooms[i].find(handleID);
        m_iter = it;
        if (it != m_mapRooms[i].end())
            return i;
    }
    return -1;
}

void cltPKManager::SetPKRoomInfoCreate(stPKRoomInfo* pInfo)
{
    // GT (mofclient.c:199519) 只做單純的 self-assignment 檢查：
    //   if ( (char*)this + 88 != a2 ) { ...copy... }
    if (pInfo == &m_currentRoom)
        return;

    m_currentRoom.handleID = pInfo->handleID;
    m_currentRoom.hostAccountID = pInfo->hostAccountID;
    m_currentRoom.unkShort = pInfo->unkShort;
    m_currentRoom.gameLevel = pInfo->gameLevel;
    m_currentRoom.roomType = pInfo->roomType;
    m_currentRoom.unk3 = pInfo->unk3;
    m_currentRoom.lockState = pInfo->lockState;
    m_currentRoom.unk4 = pInfo->unk4;
    m_currentRoom.unk5 = pInfo->unk5;
    m_currentRoom.unk6 = pInfo->unk6;
    m_currentRoom.isStarted = pInfo->isStarted;
    m_currentRoom.roomNum = pInfo->roomNum;
    m_currentRoom.unk1 = pInfo->unk1;
    m_currentRoom.costType = pInfo->costType;
    m_currentRoom.unk7 = pInfo->unk7;
    std::strcpy(m_currentRoom.roomName, pInfo->roomName);
    std::strcpy(m_currentRoom.password, pInfo->password);
    for (int i = 0; i < 12; i++)
    {
        m_currentRoom.players[i] = (stPlayerInfo*)&pInfo->players[i];
    }
}

int cltPKManager::CopyPKRoomInfoFromList(int a2, int a3)
{
    // Check bounds
    if (a2 < 0 || a2 > 4)
        return 0;

    if ((int)m_mapRooms[a2].size() <= a3)
        return 0;

    // Get begin iterator and advance to position a3
    m_iter = m_mapRooms[a2].begin();

    for (int i = 0; i < a3; i++)
    {
        ++m_iter;
    }

    if (m_iter == m_mapRooms[a2].end())
        return 0;

    stPKRoomInfo* pSrc = m_iter->second;

    if (a2 == 0 || a2 == 1)
    {
        // Full reset then copy (GT: clears all fields, then copies)
        m_currentRoom.gameLevel = -1;
        m_currentRoom.roomType = -1;
        m_currentRoom.unk3 = -1;
        m_currentRoom.lockState = -1;
        m_currentRoom.handleID = 0;
        m_currentRoom.hostAccountID = 0;
        m_currentRoom.unkShort = 0;
        m_currentRoom.unk4 = 0;
        m_currentRoom.unk5 = 0;
        m_currentRoom.unk6 = 0;
        m_currentRoom.isStarted = 0;
        m_currentRoom.roomNum = 0;
        m_currentRoom.unk1 = 0;
        m_currentRoom.costType = -2;
        m_currentRoom.unk7 = 0;
        std::memset(m_currentRoom.roomName, 0, 256);
        std::memset(m_currentRoom.password, 0, 256);
        for (int i = 0; i < 12; i++)
            m_currentRoom.players[i].Init();

        // Copy from source
        if (pSrc != &m_currentRoom)
        {
            m_currentRoom.handleID = pSrc->handleID;
            m_currentRoom.hostAccountID = pSrc->hostAccountID;
            m_currentRoom.unkShort = pSrc->unkShort;
            m_currentRoom.gameLevel = pSrc->gameLevel;
            m_currentRoom.roomType = pSrc->roomType;
            m_currentRoom.unk3 = pSrc->unk3;
            m_currentRoom.lockState = pSrc->lockState;
            m_currentRoom.unk4 = pSrc->unk4;
            m_currentRoom.unk5 = pSrc->unk5;
            m_currentRoom.unk6 = pSrc->unk6;
            m_currentRoom.isStarted = pSrc->isStarted;
            m_currentRoom.roomNum = pSrc->roomNum;
            m_currentRoom.unk1 = pSrc->unk1;
            m_currentRoom.costType = pSrc->costType;
            m_currentRoom.unk7 = pSrc->unk7;
            std::strcpy(m_currentRoom.roomName, pSrc->roomName);
            std::strcpy(m_currentRoom.password, pSrc->password);
            for (int i = 0; i < 12; i++)
                m_currentRoom.players[i] = (stPlayerInfo*)&pSrc->players[i];
        }
        return 1;
    }
    else if (a2 == 2)
    {
        m_currentRoom.operator=((char*)pSrc);
        return 1;
    }
    else if (a2 == 3 || a2 == 4)
    {
        // Reset then copy (GT: Init then operator=)
        m_currentRoom.Init();
        m_currentRoom.operator=((char*)pSrc);
        return 1;
    }

    return 0;
}

void cltPKManager::DestroyRoom(unsigned int handleID)
{
    if (!handleID)
        return;

    for (int i = 0; i < 5; i++)
    {
        auto it = m_mapRooms[i].find(handleID);
        if (it != m_mapRooms[i].end())
        {
            if (it->second)
                delete it->second;
            m_mapRooms[i].erase(it);
            return;
        }
    }
}

void cltPKManager::UpdatePKRoomInfo(stPKRoomInfo* pInfo)
{
    // GT (mofclient.c:200037) 流程：
    //   m_currentRoom.gameLevel = m_gameLevelSetting;
    //   依序在 map0..map4 呼叫 find/insert/operator[]，將結果寫回 m_iter；
    //   找到 → 對既有 entry 呼叫 operator= 進行 in-place 更新。
    //   全部都找不到 → 不做任何事 (GT 沒有「建立新房間」的 fallback)。
    if (!pInfo)
        return;

    m_currentRoom.gameLevel = m_gameLevelSetting;

    unsigned long handleID = pInfo->handleID;

    for (int i = 0; i < 5; i++)
    {
        auto it = m_mapRooms[i].find(handleID);
        m_iter = it;
        if (it != m_mapRooms[i].end())
        {
            // Found — update the existing room (skip self-assignment)
            if (it->second != pInfo)
                it->second->operator=((char*)pInfo);
            return;
        }
    }
    // Not found in any map: GT 不會自行建立新房間，這裡也不做。
}

int cltPKManager::GetPKRoomNumByGameLevel(int a2)
{
    switch (a2)
    {
    case 0: return (int)m_mapRooms[0].size();
    case 1: return (int)m_mapRooms[1].size();
    case 2: return (int)m_mapRooms[2].size();
    case 3: return (int)m_mapRooms[3].size();
    case 4: return (int)m_mapRooms[4].size();
    }
    return 0;
}

// ---- 隊伍查詢 ----

int cltPKManager::GetTeamByAccount(unsigned int accountID)
{
    if (m_currentRoom.roomType == 4)
    {
        for (int i = 0; i < 12; i++)
        {
            if (m_currentRoom.players[i].accountID == accountID)
            {
                if (i >= 8)
                    return -1;
                return m_currentRoom.players[i].accountID
                       != cltMyCharData::GetMyAccount((cltMyCharData*)&g_clMyCharData) ? 1 : 0;
            }
        }
        return -2;
    }
    else
    {
        for (int i = 0; i < 12; i++)
        {
            if (m_currentRoom.players[i].accountID == accountID)
            {
                if (i < 4)
                    return 0;
                if (i < 8)
                    return 1;
                return -1;
            }
        }
        return -2;
    }
}

BOOL cltPKManager::IsMyTeam(unsigned int accountID)
{
    unsigned int myAccount = cltMyCharData::GetMyAccount((cltMyCharData*)&g_clMyCharData);
    int myTeam = GetTeamByAccount(myAccount);
    return myTeam == GetTeamByAccount(accountID);
}

int cltPKManager::GetTeamByIndex(int index)
{
    if (m_currentRoom.roomType == 4)
    {
        if (index < 0)
            return -2;
        if (index < 8)
            return 0;
    }
    else
    {
        if (index < 0)
            return -2;
        if (index < 4)
            return 0;
        if (index < 8)
            return 1;
    }
    if (index < 12)
        return -1;
    return -2;
}

// ---- 房間加入檢查 ----

int cltPKManager::CheckQuickJoinRoom(int gameLevel)
{
    int mapIdx;
    if (!m_gameLevelSetting)
        mapIdx = 0;
    else if (gameLevel >= 1 && gameLevel <= 4)
        mapIdx = gameLevel;
    else
        return 0;

    m_iter = m_mapRooms[mapIdx].begin();

    if (m_iter == m_mapRooms[mapIdx].end())
        return 0;

    while (!CheckPlayerNum())
    {
        ++m_iter;
        if (m_iter == m_mapRooms[mapIdx].end())
            return 0;
    }

    // Found a room with space — copy into m_currentRoom
    stPKRoomInfo* pSrc = m_iter->second;
    if (pSrc != &m_currentRoom)
    {
        m_currentRoom.handleID = pSrc->handleID;
        m_currentRoom.hostAccountID = pSrc->hostAccountID;
        m_currentRoom.unkShort = pSrc->unkShort;
        m_currentRoom.gameLevel = pSrc->gameLevel;
        m_currentRoom.roomType = pSrc->roomType;
        m_currentRoom.unk3 = pSrc->unk3;
        m_currentRoom.lockState = pSrc->lockState;
        m_currentRoom.unk4 = pSrc->unk4;
        m_currentRoom.unk5 = pSrc->unk5;
        m_currentRoom.unk6 = pSrc->unk6;
        m_currentRoom.isStarted = pSrc->isStarted;
        m_currentRoom.roomNum = pSrc->roomNum;
        m_currentRoom.unk1 = pSrc->unk1;
        m_currentRoom.costType = pSrc->costType;
        m_currentRoom.unk7 = pSrc->unk7;
        std::strcpy(m_currentRoom.roomName, pSrc->roomName);
        std::strcpy(m_currentRoom.password, pSrc->password);
        for (int i = 0; i < 12; i++)
            m_currentRoom.players[i] = (stPlayerInfo*)&pSrc->players[i];
    }
    return 1;
}

int cltPKManager::CheckPlayerNum()
{
    // Read room from current iterator
    stPKRoomInfo* pRoom = m_iter->second;

    int count = 0;
    for (int i = 0; i < 12; i++)
    {
        if (pRoom->players[i].accountID)
            ++count;
    }

    int roomType = pRoom->roomType;
    int maxPlayers;
    switch (roomType)
    {
    case 0:  maxPlayers = 6;  break;
    case 1:  maxPlayers = 8;  break;
    case 2:  maxPlayers = 10; break;
    case 3:
    case 4:  maxPlayers = 12; break;
    default: maxPlayers = 6;  break;
    }

    if (count >= maxPlayers)
        return 0;

    // Check if room is locked or started
    if (pRoom->lockState == 1 || pRoom->isStarted == 1)
        return 0;

    return 1;
}

int cltPKManager::CheckClickJoinRoom()
{
    int count = 0;
    for (int i = 0; i < 12; i++)
    {
        if (m_currentRoom.players[i].accountID)
            ++count;
    }

    if (m_currentRoom.lockState == 1)
        return 0;

    int roomType = m_currentRoom.roomType;
    if (roomType == 0)
    {
        if (count < 6)
            return 1;
        return 0;
    }
    else if (roomType == 1)
    {
        if (count >= 8)
            return 0;
    }
    else if (roomType == 2)
    {
        if (count >= 10)
            return 0;
    }
    else if (roomType == 3 || roomType == 4)
    {
        if (count >= 12)
            return 0;
    }
    return 1;
}

// ---- 玩家索引查詢 ----

int cltPKManager::GetIndexbyAccount(unsigned int accountID)
{
    for (int i = 0; i < 12; i++)
    {
        if (m_currentRoom.players[i].accountID == accountID)
            return i;
    }
    return -1;
}

int cltPKManager::GetMyIndex()
{
    unsigned int myAccount = cltMyCharData::GetMyAccount((cltMyCharData*)&g_clMyCharData);
    for (int i = 0; i < 12; i++)
    {
        if (m_currentRoom.players[i].accountID == myAccount)
            return i;
    }
    return -1;
}

BOOL cltPKManager::IsMyIndex(int index)
{
    return m_currentRoom.players[index].accountID
           == cltMyCharData::GetMyAccount((cltMyCharData*)&g_clMyCharData);
}

int cltPKManager::GetIndexByCharName(char* lpString1)
{
    for (int i = 0; i < 12; i++)
    {
        if (lstrcmpA(lpString1, m_currentRoom.players[i].name) == 0)
            return i;
    }
    return -1;
}

int cltPKManager::GetSpaceIndexbyTeam(int team)
{
    for (int i = 0; i < 12; i++)
    {
        if (GetTeamByIndex(i) == team && !m_currentRoom.players[i].accountID)
            return i;
    }
    return -1;
}

// ---- 玩家資訊存取 ----

int cltPKManager::GetPKRoomAccountNum()
{
    int count = 0;
    for (int i = 0; i < 12; i++)
    {
        if (m_currentRoom.players[i].accountID)
            ++count;
    }
    return count;
}

int cltPKManager::GetAccountNumByTeam(int team)
{
    if (team < 0)
        return 0;

    int count = 0;

    if (m_currentRoom.roomType == 4)
    {
        // Free-for-all: count first 8 players
        for (int i = 0; i < 8; i++)
        {
            if (m_currentRoom.players[i].accountID)
                ++count;
        }
    }
    else
    {
        // Team mode: count 4 players per team
        int start = 4 * team;
        int end = start + 4;
        for (int i = start; i < end; i++)
        {
            if (m_currentRoom.players[i].accountID)
                ++count;
        }
    }
    return count;
}

int cltPKManager::GetReadyNumByTeam(int team)
{
    int countTeam0 = 0;
    int countTeam1 = 0;
    int countObserver = 0;

    for (int i = 0; i < 12; i++)
    {
        if (m_currentRoom.players[i].accountID
            && (m_currentRoom.players[i].accountID == m_currentRoom.hostAccountID
                || m_currentRoom.players[i].state == 1))
        {
            int t = GetTeamByIndex(i);
            if (t == 0)
                ++countTeam0;
            else if (t == 1)
                ++countTeam1;
            else
                ++countObserver;
        }
    }

    if (m_currentRoom.roomType == 4)
    {
        if (team == 0)
            return countTeam0 + countTeam1;
        return countObserver;
    }

    if (team == 0)
        return countTeam0;
    if (team == 1)
        return countTeam1;
    return countObserver;
}

int cltPKManager::GetPlayerNum()
{
    int count = 0;
    for (int i = 0; i < 8; i++)
    {
        if (m_currentRoom.players[i].accountID)
            ++count;
    }
    return count;
}

int cltPKManager::IsAllReady()
{
    for (int i = 0; i < 12; i++)
    {
        if (m_currentRoom.players[i].accountID
            && m_currentRoom.players[i].accountID != m_currentRoom.hostAccountID
            && m_currentRoom.players[i].state != 1)
        {
            return 0;
        }
    }
    return 1;
}

BOOL cltPKManager::CheckTeamPlayerNum()
{
    if (m_unk_7288 == 4 && GetPlayerNum() > 1)
        return 1;

    int countTeam0 = 0;
    int countTeam1 = 0;

    for (int i = 0; i < 4; i++)
    {
        if (m_currentRoom.players[i].accountID)
            ++countTeam0;
    }
    for (int i = 4; i < 8; i++)
    {
        if (m_currentRoom.players[i].accountID)
            ++countTeam1;
    }

    return countTeam0 == countTeam1;
}

int cltPKManager::GetObserverNum()
{
    int count = 0;
    for (int i = 8; i < 12; i++)
    {
        if (m_currentRoom.players[i].accountID)
            ++count;
    }
    return count;
}

// ---- 支援隊伍 ----

BOOL cltPKManager::IsSupportTeamPlayer(int index)
{
    if (m_currentRoom.roomType != 4)
    {
        return GetAccountNumByTeam(m_currentRoom.players[index].supportTeam) != 0;
    }

    // Free-for-all mode: check if any of the first 8 players' names
    // match this player's supportName
    for (int i = 0; i < 8; i++)
    {
        if (m_currentRoom.players[i].accountID
            && std::strcmp(m_currentRoom.players[index].supportName,
                           m_currentRoom.players[i].name) == 0)
        {
            return 1;
        }
    }
    return 0;
}

void cltPKManager::ProcessTeamMoveObserver()
{
    for (int i = 8; i < 12; i++)
    {
        if (!IsSupportTeamPlayer(i))
        {
            m_currentRoom.players[i].state = 0;
            m_currentRoom.players[i].supportTeam = -2;
            m_currentRoom.players[i].supportName[0] = 0;
        }
    }
}

// ---- 名字存取 ----

char* cltPKManager::GetNameByIndex(int index)
{
    return m_currentRoom.players[index].name;
}

void cltPKManager::ResetSupportNameByIndex(int index)
{
    std::memset(m_currentRoom.players[index].supportName, 0, 256);
}

void cltPKManager::SetSupportNameByIndex(int index, char* name)
{
    std::strcpy(m_currentRoom.players[index].supportName, name);
}

char* cltPKManager::GetSupportNameByIndex(int index)
{
    return m_currentRoom.players[index].supportName;
}

// ---- 支援隊伍存取 ----

void cltPKManager::SetSupportTeamByIndex(int index, int team)
{
    m_currentRoom.players[index].supportTeam = team;
}

int cltPKManager::GetSupportTeamByIndex(int index)
{
    return m_currentRoom.players[index].supportTeam;
}

// ---- 狀態存取 ----

void cltPKManager::SetStateByIndex(int index, int state)
{
    m_currentRoom.players[index].state = state;
}

int cltPKManager::GetStateByIndex(int index)
{
    return m_currentRoom.players[index].state;
}

// ---- 職業類型存取 ----

void cltPKManager::SetClassTypeByIndex(int index, int classType)
{
    m_currentRoom.players[index].classType = classType;
}

int cltPKManager::GetClassTypeByIndex(int index)
{
    return m_currentRoom.players[index].classType;
}

// ---- 玩家管理 ----

void cltPKManager::AddPlayer(int index, unsigned int accountID, char* name,
                             int charKind, unsigned short level, int classType,
                             int state, unsigned int hostAccountID)
{
    m_currentRoom.players[index].accountID = accountID;
    m_currentRoom.players[index].charKind = charKind;
    m_currentRoom.players[index].level = level;
    m_currentRoom.players[index].classType = classType;
    m_currentRoom.players[index].state = state;
    std::strcpy(m_currentRoom.players[index].name, name);
    m_currentRoom.hostAccountID = hostAccountID;
}

// ---- 費用 ----

int cltPKManager::GetCost(int level, int team)
{
    int result = 0;
    switch (level)
    {
    case 0:
        result = 500;
        if (team != -1)
            result = 2000;
        break;
    case 1:
        result = 1000;
        if (team != -1)
            result = 3000;
        break;
    case 2:
        result = 2000;
        if (team != -1)
            result = 4000;
        break;
    case 3:
        result = 5000;
        if (team != -1)
            result = 10000;
        break;
    case 4:
        result = 500;
        if (team != -1)
            result = 1000;
        break;
    default:
        break;
    }
    return result;
}
