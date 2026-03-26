#include "Logic/cltMatchManager.h"
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
// stMatchInfo
// ===========================================================================

stMatchInfo::stMatchInfo()
{
    roomID      = 0;
    userNum     = 0;
    maxUserNum  = 0;
    roomLevel   = 0;
    // GT: 使用 strcpy 初始化字串
    std::strcpy(masterName, "");
    std::strcpy(roomTitle, "");
    posX        = 0;
    posY        = 0;
    std::memset(tag, 0, sizeof(tag));
    betMoney    = 0;
    std::memset(infoText, 0, sizeof(infoText));
    textPosX    = 0;
    textPosY    = 0;
    nameScreenX = 0;
    nameScreenY = 0;
    animFrame   = 0;
    pFlagImage  = nullptr;
    pSubImage   = nullptr;
    // alphaBox 與 chatBallon 由各自的建構函式初始化
    alphaBox.NoneActive();
    chatBallon.NoneActive();
}

stMatchInfo::~stMatchInfo()
{
}

// ===========================================================================
// cltMatchManager
// ===========================================================================

cltMatchManager::cltMatchManager()
    : m_nRoomNum(0)
{
    std::memset(m_sortedPtrs, 0, sizeof(m_sortedPtrs));
}

cltMatchManager::~cltMatchManager()
{
}

// ---------------------------------------------------------------------------
// Poll — 收集所有活躍的房間，建立排序指標陣列
// ---------------------------------------------------------------------------
void cltMatchManager::Poll()
{
    m_nRoomNum = 0;
    std::memset(m_sortedPtrs, 0, sizeof(m_sortedPtrs));

    for (int i = 0; i < MAX_ROOMS; ++i)
    {
        if (m_matches[i].roomID != 0)
        {
            m_sortedPtrs[m_nRoomNum] = &m_matches[i];
            m_nRoomNum++;
        }
    }
}

// ---------------------------------------------------------------------------
// PrepareDrawing
// ---------------------------------------------------------------------------
void cltMatchManager::PrepareDrawing()
{
    for (int i = 1; i < MAX_ROOMS; ++i)    // GT: 從 m_matches[1] 起跑，迭代 20 筆 (1..20)
    {
        stMatchInfo& info = m_matches[i];
        if (info.roomID == 0)
            continue;

        // 動畫影格
        info.animFrame++;
        if (info.animFrame >= 12)
            info.animFrame = 0;

        // 旗幟圖片
        GameImage* flagImg = cltImageManager::GetInstance()->GetGameImage(
            6u, 0xD00008Au, 0, 1);
        info.pFlagImage = flagImg;
        if (!flagImg)
            continue;

        int screenX = info.posX - dword_A73088;
        int screenY = info.posY - dword_A7308C;

        flagImg->SetBlockID(info.animFrame);
        flagImg->m_bFlag_447       = true;
        flagImg->m_bFlag_446       = true;
        flagImg->m_bVertexAnimation = false;
        flagImg->SetPosition((float)(info.posX - dword_A73088),
                              (float)(info.posY - dword_A7308C));

        // 更新 alphaBox 位置
        info.alphaBox.SetPos(info.textPosX - dword_A73088 - 2,
                              info.textPosY - dword_A7308C - 2);
        info.alphaBox.PrepareDrawing();

        // 更新 chatBallon 位置
        info.chatBallon.SetPos(info.nameScreenX - dword_A73088,
                                info.nameScreenY - dword_A7308C);
        info.chatBallon.PrepareDrawing();

        // GT: 依 tag 欄位 (offset 68) 判斷是否顯示副圖片
        if (std::strcmp(info.tag, "") != 0)
        {
            GameImage* subImg = cltImageManager::GetInstance()->GetGameImage(
                6u, 0xD00008Au, 0, 1);
            info.pSubImage = subImg;
            if (subImg)
            {
                subImg->SetBlockID(13);
                subImg->m_bFlag_447       = true;
                subImg->m_bFlag_446       = true;
                subImg->m_bVertexAnimation = false;
                subImg->SetPosition(
                    (float)(info.textPosX - dword_A73088 - 18),
                    (float)(info.textPosY - dword_A7308C - 4));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void cltMatchManager::Draw(uint8_t index)
{
    // GT: 直接取 sortedPtrs，不做 index 範圍/空指標檢查
    stMatchInfo* pInfo = m_sortedPtrs[index];

    // 繪製旗幟
    if (pInfo->pFlagImage)
    {
        pInfo->pFlagImage->Draw();
        pInfo->alphaBox.Draw();
        pInfo->chatBallon.Draw();

        // 格式化資訊文字
        g_MoFFont.SetFont("MatchInfo");
        const char* suffix = g_DCTTextManager.GetText(3224);
        sprintf(pInfo->infoText,
                "%s : %i/%i : Lv.%i : %i%s",
                pInfo->masterName,
                (int)pInfo->userNum,
                (int)pInfo->maxUserNum,
                (int)pInfo->roomLevel,
                (int)pInfo->betMoney,
                suffix);

        g_MoFFont.SetTextLineA(
            pInfo->textPosX - dword_A73088,
            pInfo->textPosY - dword_A7308C,
            0xFFFFFFFF,
            pInfo->infoText,
            0, -1, -1);
    }

    // 副圖片 — GT: 比對 tag 欄位 (offset 68)
    if (std::strcmp(pInfo->tag, "") != 0)
    {
        if (pInfo->pSubImage)
            pInfo->pSubImage->Draw();
    }
}

// ---------------------------------------------------------------------------
// IsCreateMatchRoom — 檢查 (x,y) 附近是否已有房間
// ---------------------------------------------------------------------------
int cltMatchManager::IsCreateMatchRoom(int x, int y)
{
    int count = (int)m_nRoomNum;
    if (count <= 0)
        return 1;

    for (int i = 0; i < count; ++i)
    {
        stMatchInfo* p = m_sortedPtrs[i];
        if (!p || p->roomID == 0)
            continue;

        if (std::abs(p->posX - x) < 100)
        {
            if (std::abs(p->posY - y - 50) < 100)
                return 0;   // 太近，不可建立
        }
    }
    return 1;
}

// ---------------------------------------------------------------------------
// IsJoinMatchRoom — 檢查滑鼠點擊是否命中某房間
// ---------------------------------------------------------------------------
uint8_t cltMatchManager::IsJoinMatchRoom(int mouseX, int mouseY)
{
    if (m_nRoomNum == 0)
        return 0;

    for (int i = 0; i < (int)m_nRoomNum; ++i)
    {
        stMatchInfo* p = m_sortedPtrs[i];
        if (!p || p->roomID == 0)
            continue;

        if (std::abs(p->posX - dword_A73088 - mouseX) < 100)
        {
            if (std::abs(p->posY - dword_A7308C - mouseY - 50) < 100
                && p->userNum < p->maxUserNum)
            {
                return p->roomID;
            }
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// InitMatchInfo — 清空全部
// ---------------------------------------------------------------------------
void cltMatchManager::InitMatchInfo()
{
    for (unsigned int i = 0; i < MAX_ROOMS; ++i)
        DelMatchInfo((uint8_t)i);
}

// ---------------------------------------------------------------------------
// AddMatchInfo — 將傳入的 stMatchInfo 複製到對應 slot
// ---------------------------------------------------------------------------
void cltMatchManager::AddMatchInfo(stMatchInfo* pInfo)
{
    if (!pInfo)
        return;
    if (pInfo->roomID == 0)
        return;

    // GT: 不做 MAX_ROOMS 範圍檢查，由呼叫端保證
    uint8_t idx = pInfo->roomID;

    stMatchInfo& dst = m_matches[idx];

    // GT: 整筆複製基本欄位 + 控件狀態 (operator=)
    dst.roomID      = pInfo->roomID;
    dst.userNum     = pInfo->userNum;
    dst.maxUserNum  = pInfo->maxUserNum;
    dst.roomLevel   = pInfo->roomLevel;
    std::strcpy(dst.masterName, pInfo->masterName);
    std::strcpy(dst.roomTitle,  pInfo->roomTitle);
    dst.posX        = pInfo->posX;
    dst.posY        = pInfo->posY;
    std::memcpy(dst.tag, pInfo->tag, sizeof(dst.tag));
    dst.betMoney    = pInfo->betMoney;
    std::memcpy(dst.infoText, pInfo->infoText, sizeof(dst.infoText));
    dst.textPosX    = pInfo->textPosX;
    dst.textPosY    = pInfo->textPosY;
    dst.nameScreenX = pInfo->nameScreenX;
    dst.nameScreenY = pInfo->nameScreenY;
    dst.animFrame   = pInfo->animFrame;
    dst.pFlagImage  = pInfo->pFlagImage;
    dst.pSubImage   = pInfo->pSubImage;
    // GT: CControlBase::operator= + CControlChatBallon::operator= 複製控件狀態
    dst.alphaBox    = pInfo->alphaBox;
    dst.chatBallon  = pInfo->chatBallon;

    // 格式化資訊文字
    g_MoFFont.SetFont("MatchInfo");
    const char* suffix = g_DCTTextManager.GetText(3224);
    sprintf(dst.infoText,
            "%s : %i/%i : Lv.%i : %i%s",
            dst.masterName,
            (int)dst.userNum,
            (int)dst.maxUserNum,
            (int)dst.roomLevel,
            (int)dst.betMoney,
            suffix);

    // 計算文字長度，設定顯示座標
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

    // GT: 先呼叫 chatBallon.Create 設定外觀，再 SetString
    dst.chatBallon.Create(nullptr, 1, 0xFF000000u, 0);
    dst.chatBallon.SetString(dst.roomTitle, 0, 0, 0, 0, DirDown);
}

// ---------------------------------------------------------------------------
// DelMatchInfo
// ---------------------------------------------------------------------------
void cltMatchManager::DelMatchInfo(uint8_t index)
{
    // GT: 不做範圍檢查，由呼叫端保證
    stMatchInfo& dst = m_matches[index];
    dst.roomID      = 0;
    dst.userNum     = 0;
    dst.maxUserNum  = 0;
    dst.roomLevel   = 0;
    // GT: 使用 strcpy 清空字串（而非 memset）
    std::strcpy(dst.masterName, "");
    std::strcpy(dst.roomTitle,  "");
    dst.posX        = 0;
    dst.posY        = 0;
    dst.betMoney    = 0;
    dst.textPosX    = 0;
    dst.textPosY    = 0;
    dst.nameScreenX = 0;
    dst.nameScreenY = 0;
    dst.animFrame   = 0;
    dst.alphaBox.NoneActive();
    dst.chatBallon.NoneActive();
}

// ---------------------------------------------------------------------------
// UpdateMatchUserSize
// ---------------------------------------------------------------------------
void cltMatchManager::UpdateMatchUserSize(uint8_t roomID, uint8_t newUserNum)
{
    if (roomID < MAX_ROOMS)
        m_matches[roomID].userNum = newUserNum;
}

// ---------------------------------------------------------------------------
// GetMatchInfo
// ---------------------------------------------------------------------------
stMatchInfo* cltMatchManager::GetMatchInfo(uint8_t roomID)
{
    // GT: 只檢查 roomID != 0 和 slot 是否有效，不做 MAX_ROOMS 範圍檢查
    if (roomID == 0)
        return nullptr;
    if (m_matches[roomID].roomID == 0)
        return nullptr;
    return &m_matches[roomID];
}

// ---------------------------------------------------------------------------
// GetMatchInfoByIndex
// ---------------------------------------------------------------------------
stMatchInfo* cltMatchManager::GetMatchInfoByIndex(uint8_t index)
{
    if (index >= MAX_ROOMS)
        return nullptr;
    if (m_matches[index].roomID == 0)
        return nullptr;
    return &m_matches[index];
}

// ---------------------------------------------------------------------------
// GetMatchRoomNum
// ---------------------------------------------------------------------------
int cltMatchManager::GetMatchRoomNum()
{
    return (int)m_nRoomNum;
}
