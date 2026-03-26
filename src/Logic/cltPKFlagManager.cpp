#include "Logic/cltPKFlagManager.h"
#include "Logic/Map.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Font/MoFFont.h"
#include "Text/DCTTextManager.h"
#include "global.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>

// ===========================================================================
// stPKFlag
// ===========================================================================

stPKFlag::stPKFlag()
{
    handleID    = 0;
    posX        = 0;
    posY        = 0;
    _reserved1  = 0;
    userNum     = 0;
    maxUserNum  = 0;
    roomLevel   = 0;
    textPosX    = 0;
    textPosY    = 0;
    nameScreenX = 0;
    nameScreenY = 0;
    // GT: 使用 strcpy 初始化字串
    std::strcpy(masterName, "");
    std::strcpy(infoText, "");
    animFrame   = 0;
    pFlagImage  = nullptr;
    // alphaBox 由建構函式初始化
    alphaBox.NoneActive();
}

stPKFlag::~stPKFlag()
{
}

// ===========================================================================
// cltPKFlagManager
// ===========================================================================

cltPKFlagManager::cltPKFlagManager()
    : m_nRoomNum(0)
{
    std::memset(m_sortedPtrs, 0, sizeof(m_sortedPtrs));
}

cltPKFlagManager::~cltPKFlagManager()
{
    // GT: memset 整塊清零 (0xE100 bytes)，再由 eh vector destructor iterator 逐一解構
    std::memset(m_rooms, 0, sizeof(m_rooms));
    // C++ 編譯器會自動對 m_rooms[] 陣列元素呼叫 ~stPKFlag()
}

// ---------------------------------------------------------------------------
// FindRoomIndex (private helper)
// ---------------------------------------------------------------------------
int cltPKFlagManager::FindRoomIndex(unsigned int handleID)
{
    for (int i = 0; i < MAX_PK_ROOMS; ++i)
    {
        if (m_rooms[i].handleID == handleID)
            return i;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// AddPKRoom
// ---------------------------------------------------------------------------
void cltPKFlagManager::AddPKRoom(stPKFlag* pFlag)
{
    if (m_nRoomNum >= MAX_PK_ROOMS)
        return;

    // 尋找空 slot
    int idx = -1;
    for (int i = 0; i < MAX_PK_ROOMS; ++i)
    {
        if (m_rooms[i].handleID == 0)
        {
            idx = i;
            break;
        }
    }
    if (idx < 0)
        return;

    // GT: qmemcpy 整筆複製 stPKFlag (0x240 bytes)
    std::memcpy(&m_rooms[idx], pFlag, sizeof(stPKFlag));

    stPKFlag& dst = m_rooms[idx];

    // 格式化資訊文字
    g_MoFFont.SetFont("MatchInfo");
    const char* suffix = g_DCTTextManager.GetText(3224);
    sprintf(dst.infoText,
            "%s : %i/%i : Lv.%i : %s",
            dst.masterName,
            (int)dst.userNum,
            (int)dst.maxUserNum,
            (int)dst.roomLevel,
            suffix);

    // 計算文字長度
    int textWidth = 0, textHeight = 0;
    g_MoFFont.GetTextLength(&textWidth, &textHeight, "MatchInfo", dst.infoText);

    dst.textPosX = dst.posX - textWidth / 2 - 4;
    dst.textPosY = dst.posY - 102;

    // 設定 alphaBox（半透明綠色背景）
    float r = 0.0f;
    float g = 1.0f;
    float b = 0.0f;
    float a = 128.0f / 255.0f;

    int scrX = g_Map.MapXtoScreenX(dst.textPosX - 2);
    int scrY = g_Map.MapYtoScreenY(dst.textPosY - 2);
    dst.alphaBox.Create(scrX, scrY, (uint16_t)(textWidth + 25), 15,
                        r, g, b, a, nullptr);

    dst.nameScreenX = dst.posX;
    dst.nameScreenY = dst.posY - 110;
}

// ---------------------------------------------------------------------------
// DeletePKRoom
// ---------------------------------------------------------------------------
void cltPKFlagManager::DeletePKRoom(unsigned int handleID)
{
    int idx = FindRoomIndex(handleID);
    if (idx < 0)
        return;

    std::memset(&m_rooms[idx], 0, sizeof(stPKFlag));
}

// ---------------------------------------------------------------------------
// UpdatePKRoomInfo — 整筆覆寫
// ---------------------------------------------------------------------------
void cltPKFlagManager::UpdatePKRoomInfo(unsigned int handleID, stPKFlag* pNewInfo)
{
    int idx = FindRoomIndex(handleID);
    if (idx < 0)
        return;

    // GT: qmemcpy 整筆覆寫 stPKFlag (0x240 bytes)
    std::memcpy(&m_rooms[idx], pNewInfo, sizeof(stPKFlag));
    UpdatePKRoomInfoText(idx);
}

// ---------------------------------------------------------------------------
// SetMasterName
// ---------------------------------------------------------------------------
void cltPKFlagManager::SetMasterName(unsigned int handleID, char* name)
{
    int idx = FindRoomIndex(handleID);
    if (idx < 0)
        return;
    // GT: 使用 strcpy（非 strncpy）
    std::strcpy(m_rooms[idx].masterName, name);
    UpdatePKRoomInfoText(idx);
}

// ---------------------------------------------------------------------------
// SetUserNum
// ---------------------------------------------------------------------------
void cltPKFlagManager::SetUserNum(unsigned int handleID, uint8_t num)
{
    int idx = FindRoomIndex(handleID);
    if (idx < 0)
        return;
    m_rooms[idx].userNum = num;
    UpdatePKRoomInfoText(idx);
}

// ---------------------------------------------------------------------------
// SetMaxUserNum
// ---------------------------------------------------------------------------
void cltPKFlagManager::SetMaxUserNum(unsigned int handleID, uint8_t num)
{
    int idx = FindRoomIndex(handleID);
    if (idx < 0)
        return;
    m_rooms[idx].maxUserNum = num;
    UpdatePKRoomInfoText(idx);
}

// ---------------------------------------------------------------------------
// SetRoomLevel
// ---------------------------------------------------------------------------
void cltPKFlagManager::SetRoomLevel(unsigned int handleID, uint8_t level)
{
    int idx = FindRoomIndex(handleID);
    if (idx < 0)
        return;
    m_rooms[idx].roomLevel = level;
    UpdatePKRoomInfoText(idx);
}

// ---------------------------------------------------------------------------
// UpdatePKRoomInfoText — 重新格式化文字並更新 alphaBox
// ---------------------------------------------------------------------------
void cltPKFlagManager::UpdatePKRoomInfoText(int index)
{
    // GT: 不做範圍檢查，由呼叫端保證
    stPKFlag& room = m_rooms[index];

    g_MoFFont.SetFont("MatchInfo");
    const char* suffix = g_DCTTextManager.GetText(3224);
    sprintf(room.infoText,
            "%s : %i/%i : Lv.%i : %s",
            room.masterName,
            (int)room.userNum,
            (int)room.maxUserNum,
            (int)room.roomLevel,
            suffix);

    int textWidth = 0, textHeight = 0;
    g_MoFFont.GetTextLength(&textWidth, &textHeight, "MatchInfo", room.infoText);

    room.textPosX = room.posX - textWidth / 2 - 4;
    room.textPosY = room.posY - 102;

    // 更新 alphaBox
    float r = 0.0f;
    float g = 1.0f;
    float b = 0.0f;
    float a = 128.0f / 255.0f;

    int scrX = g_Map.MapXtoScreenX(room.textPosX - 2);
    int scrY = g_Map.MapYtoScreenY(room.textPosY - 2);
    room.alphaBox.Create(scrX, scrY, (uint16_t)(textWidth + 25), 15,
                         r, g, b, a, nullptr);

    room.nameScreenX = room.posX;
    room.nameScreenY = room.posY - 110;
}

// ---------------------------------------------------------------------------
// Poll — 收集活躍房間
// ---------------------------------------------------------------------------
void cltPKFlagManager::Poll()
{
    m_nRoomNum = 0;
    std::memset(m_sortedPtrs, 0, sizeof(m_sortedPtrs));

    for (int i = 0; i < MAX_PK_ROOMS; ++i)
    {
        if (m_rooms[i].handleID != 0)
        {
            m_sortedPtrs[m_nRoomNum] = &m_rooms[i];
            m_nRoomNum++;
        }
    }
}

// ---------------------------------------------------------------------------
// PrepareDrawing
// ---------------------------------------------------------------------------
void cltPKFlagManager::PrepareDrawing()
{
    for (int i = 0; i < MAX_PK_ROOMS; ++i)
    {
        stPKFlag& room = m_rooms[i];
        if (room.handleID == 0)
            continue;

        room.animFrame++;
        if (room.animFrame >= 12)
            room.animFrame = 0;

        GameImage* flagImg = cltImageManager::GetInstance()->GetGameImage(
            6u, 0xD00008Au, 0, 1);
        room.pFlagImage = flagImg;
        if (!flagImg)
            continue;

        int screenX = room.posX - dword_A73088;
        int screenY = room.posY - dword_A7308C;

        flagImg->SetBlockID(room.animFrame + 14);
        flagImg->m_bFlag_447       = true;
        flagImg->m_bFlag_446       = true;
        flagImg->m_bVertexAnimation = false;
        flagImg->SetPosition((float)screenX, (float)screenY);

        // 更新 alphaBox 位置
        room.alphaBox.SetPos(room.textPosX - dword_A73088 - 2,
                              room.textPosY - dword_A7308C - 2);
        room.alphaBox.PrepareDrawing();
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void cltPKFlagManager::Draw(uint8_t index)
{
    // GT: 直接取 sortedPtrs，只檢查 pFlagImage
    stPKFlag* pRoom = m_sortedPtrs[index];

    if (pRoom->pFlagImage)
    {
        pRoom->pFlagImage->Draw();
        pRoom->alphaBox.Draw();

        g_MoFFont.SetFont("MatchInfo");
        g_MoFFont.SetTextLineA(
            pRoom->textPosX - dword_A73088,
            pRoom->textPosY - dword_A7308C,
            0xFFFFFFFF,
            pRoom->infoText,
            0, -1, -1);
    }
}

// ---------------------------------------------------------------------------
// IsCreateMatchRoom — 檢查 (x,y) 附近是否已有 PK 房間
// ---------------------------------------------------------------------------
int cltPKFlagManager::IsCreateMatchRoom(int x, int y)
{
    if (m_nRoomNum <= 0)
        return 1;

    for (int i = 0; i < m_nRoomNum; ++i)
    {
        stPKFlag* p = m_sortedPtrs[i];
        if (!p || p->handleID == 0)
            continue;

        if (std::abs(p->posX - x) < 100)
        {
            if (std::abs(p->posY - y - 50) < 100)
                return 0;
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// IsJoinMatchRoom — 檢查點擊是否命中 PK 房間（回傳 handleID）
// ---------------------------------------------------------------------------
unsigned int cltPKFlagManager::IsJoinMatchRoom(int mouseX, int mouseY)
{
    if (m_nRoomNum <= 0)
        return 0;

    for (int i = 0; i < m_nRoomNum; ++i)
    {
        stPKFlag* p = m_sortedPtrs[i];
        if (!p || p->handleID == 0)
            continue;

        if (std::abs(p->posX - dword_A73088 - mouseX) < 100)
        {
            if (std::abs(p->posY - dword_A7308C - mouseY - 50) < 100
                && p->userNum < p->maxUserNum)
            {
                return p->handleID;
            }
        }
    }
    return 0;
}
