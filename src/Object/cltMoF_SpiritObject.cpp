#include "Object/cltMoF_SpiritObject.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "System/CSpiritSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltClassSystem.h"
#include "global.h"
#include "Logic/CSpiritSpeechMgr.h"
#include <cstring>
#include <cstdlib>
#include <cmath>

extern int dword_A73088;
extern int dword_A7308C;

// -------------------------------------------------------------------------
// Constructor — ground truth 004E81A0
// -------------------------------------------------------------------------
cltMoF_SpiritObject::cltMoF_SpiritObject()
    : m_nActive(0)
    , m_wClassCode(0)
    , m_wCharKind(0)
    , m_byLevel(0)
    , m_fPosX(0.0f)
    , m_fPosY(0.0f)
    , m_dwResourceID(0)
    , m_dwStartBlockID(0)
    , m_dwCurrentBlock(0)
    , m_wBlockCount(0)
    , m_fFrameAccum(0.0f)
    , m_nDirection(0)
    , m_nShowSpeech(0)
    , m_pGameImage(nullptr)
    , m_pOwnerChar(nullptr)
    , m_pCharMgr(nullptr)
    , m_nPatrolDir(1)       // DWORD 683 = 1
    , m_nOffsetX(0)         // DWORD 684 = 0
    , m_nCharHeight(0)      // DWORD 685
    , m_nAcceleration(0)    // DWORD 686
    , m_fTraceScale(1.0f)   // DWORD 687 = 1065353216 (1.0f)
    , m_nTraceFlag(1)       // DWORD 688 = 1
    , m_fPatrolScale(0.0f)  // DWORD 689
    , m_nPatrolReturn(0)    // DWORD 690
{
}

// -------------------------------------------------------------------------
// Destructor — ground truth 004E8210
// -------------------------------------------------------------------------
cltMoF_SpiritObject::~cltMoF_SpiritObject()
{
}

// -------------------------------------------------------------------------
// SetActive — ground truth 004E8220
// Ground truth: 檢查角色的 accountID 是否與玩家帳號匹配
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::SetActive(ClientCharacter* pChar, int active)
{
    if (!pChar)
        return;

    // Ground truth: *((_DWORD *)a2 + 114) == *(_DWORD *)((char *)&loc_43E090 + *((_DWORD *)this + 15))
    // 比較角色的 accountID (offset 456) 與玩家帳號
    if (pChar->m_dwAccountID != g_dwMyAccountID)
        return;

    m_nOffsetX = 0;
    m_nActive = active;
    m_nPatrolDir = 1;
    m_fTraceScale = 1.0f;
    m_nTraceFlag = 1;
}

// -------------------------------------------------------------------------
// GetActive — ground truth 004E8270
// -------------------------------------------------------------------------
int cltMoF_SpiritObject::GetActive()
{
    return m_nActive;
}

// -------------------------------------------------------------------------
// Initialize — ground truth 004E8280
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::Initialize(ClientCharacterManager* pMgr)
{
    m_pCharMgr = pMgr;
    m_wBlockCount = 0;

    // Ground truth: 使用 CControlChatBallon vtable offset 80 方法
    // (*(void (__thiscall **)(char *, _DWORD, _DWORD, int, _DWORD))(v3 + 80))((char *)this + 64, 0, 0, -1, 0);
    // 這是 CControlChatBallon 的某個初始化方法，接受 4 個 int 參數
    // 目前以 SetString 近似
    m_ChatBallon.SetString((char*)"", 0, 0, 0, 0, (Direction)(DirLeft | DirRight));

    m_nOffsetX = 0;
    m_nPatrolDir = 1;
    m_fTraceScale = 1.0f;
    m_nTraceFlag = 1;
    m_nActive = 0;
}

// -------------------------------------------------------------------------
// GetFront — ground truth 004E82E0
// -------------------------------------------------------------------------
int cltMoF_SpiritObject::GetFront()
{
    return (m_nPatrolDir > 0) ? 1 : 0;
}

// -------------------------------------------------------------------------
// SetChar — ground truth 004E82F0
// Ground truth: 檢查角色 accountID，設定方向為 owner offset 572 的值
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::SetChar(ClientCharacter* pChar, std::uint16_t param)
{
    // Ground truth: 先檢查 accountID 匹配，再檢查 pChar 非空
    if (!pChar)
        return;
    if (pChar->m_dwAccountID != g_dwMyAccountID)
        return;

    m_pOwnerChar = pChar;
    m_wCharKind = param;
    m_byLevel = (std::uint8_t)g_clLevelSystem.GetLevel();
    m_wClassCode = g_clClassSystem.GetClass();

    // Ground truth: m_nDirection = *(DWORD*)(owner + 572) = m_dwLR_Flag
    m_nDirection = pChar->m_dwLR_Flag;
    m_nActive = 0;

    std::uint16_t spiritKind = g_clSpiritSystem.GetSpiritKind(m_byLevel);
    stSpiritInfo* pInfo = g_clSpiritSystem.GetSpiritInfo(spiritKind);

    if (pInfo)
    {
        m_dwResourceID = pInfo->dwResourceId;
        m_dwStartBlockID = pInfo->wStartBlockId;
        m_wBlockCount = pInfo->wBlockCount;
        m_dwCurrentBlock = pInfo->wStartBlockId;

        m_nActive = 1;

        std::uint16_t charHeight = pChar->GetCharHeight();
        m_fTraceScale = 1.0f;
        m_nCharHeight = charHeight - 25;
        m_nTraceFlag = 1;
        m_fPosX = (float)pChar->m_iPosX;

        UpdateSpirit(m_byLevel);
    }
}

// -------------------------------------------------------------------------
// UpdateSpirit — ground truth 004E83E0
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::UpdateSpirit(std::uint8_t level)
{
    m_byLevel = level;

    if (!m_nActive)
        return;

    m_nActive = 0;

    std::uint16_t spiritKind = g_clSpiritSystem.GetSpiritKind(level);
    stSpiritInfo* pInfo = g_clSpiritSystem.GetSpiritInfo(spiritKind);

    if (pInfo)
    {
        m_dwResourceID = pInfo->dwResourceId;
        m_dwStartBlockID = pInfo->wStartBlockId;
        m_wBlockCount = pInfo->wBlockCount;

        m_nActive = 1;
        // Ground truth: direction = owner offset 572
        if (m_pOwnerChar)
            m_nDirection = m_pOwnerChar->m_dwLR_Flag;
        m_fTraceScale = 1.0f;
        m_nTraceFlag = 1;
        // Ground truth: currentBlock = 0
        m_dwCurrentBlock = 0;
    }
}

// -------------------------------------------------------------------------
// Poll — ground truth 004E8470
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::Poll()
{
    if (m_nShowSpeech)
    {
        char* script = g_clSpiritSpeechMgr.GetSpiritSpeechScript();
        m_ChatBallon.SetString(script, 0, 0, 0, 0, (Direction)(DirLeft | DirRight));
    }
    else
    {
        m_ChatBallon.SetString((char*)"", 0, 0, 0, 0, (Direction)(DirLeft | DirRight));
    }
}

// -------------------------------------------------------------------------
// PrepareDrawing — ground truth 004E84B0
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::PrepareDrawing(int showSpeech)
{
    if (!m_nActive)
        return;

    // Ground truth: 變身時不顯示精靈
    if (g_ClientCharMgr.GetMyTransformationed())
        return;

    m_nShowSpeech = showSpeech;

    if (!m_pOwnerChar)
        return;

    // 根據動作狀態選擇移動模式
    if (m_pOwnerChar->GetActionState() != 0)
        MoveTrace();
    else
        MovePatrol();

    // 檢查 block 範圍
    if (m_dwCurrentBlock >= m_dwStartBlockID + m_wBlockCount)
    {
        m_dwCurrentBlock = m_dwStartBlockID;
        m_fFrameAccum = 0.0f;
    }

    // 取得圖像
    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(0xCu, m_dwResourceID, 0, 1);
    if (!m_pGameImage)
        return;

    // 設定圖像屬性
    float screenX = m_fPosX - (float)dword_A73088;
    float screenY = m_fPosY - (float)dword_A7308C;

    m_pGameImage->SetBlockID((std::uint16_t)m_dwCurrentBlock);
    m_pGameImage->m_bFlag_447 = true;
    m_pGameImage->m_bFlag_446 = true;
    m_pGameImage->m_bVertexAnimation = false;
    m_pGameImage->SetPosition(screenX, screenY);

    // Ground truth: image flip = this->direction (直接賦值，不是布林轉換)
    // *(DWORD*)(image + 392) = m_nDirection
    m_pGameImage->m_bFlipX = m_nDirection;

    // 推進動畫
    int startBlock = m_dwStartBlockID;
    m_fFrameAccum += 0.2f;
    m_dwCurrentBlock = startBlock + (int)(std::int64_t)m_fFrameAccum;

    // 聊天氣泡位置
    if (m_nShowSpeech)
    {
        if (m_pGameImage->m_pGIData && m_pOwnerChar)
        {
            // Ground truth: 從 GIData 讀 block 高度偏移
            // v10 = *(DWORD*)(pGameImage + 8) = GIData
            // v11 = owner
            // v12 = *(int*)(*(DWORD*)(v10 + 32) + 52 * startBlock + 32) = block offset32
            // abs(v12) 用於計算 Y 偏移
            int ballonX = m_nOffsetX + m_pOwnerChar->m_iPosX - dword_A73088;

            // 計算 Y 偏移：從 GIData 的 block 資料讀取高度
            int blockHeightOffset = 0;
            int* pGIDataDwords = (int*)m_pGameImage->m_pGIData;
            if (pGIDataDwords)
            {
                int* pBlockArray = (int*)(*(pGIDataDwords + 8));
                if (pBlockArray)
                {
                    int blockOff32 = *(int*)((char*)pBlockArray + 52 * startBlock + 32);
                    // Ground truth: abs via ((HIDWORD(v12) ^ v12) - HIDWORD(v12))
                    blockHeightOffset = abs(blockOff32);
                }
            }

            int ballonY = (int)(m_fPosY - (float)dword_A7308C - (float)blockHeightOffset);

            // Ground truth: 使用 vtable offset 84 的 SetPos 方法
            m_ChatBallon.SetPos(ballonX, ballonY);
            // Ground truth: 使用 vtable offset 24 的 Show 方法
            m_ChatBallon.Show();
        }
    }
}

// -------------------------------------------------------------------------
// Draw — ground truth 004E8620
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::Draw()
{
    if (!m_nActive)
        return;

    // Ground truth: 變身時不繪製
    if (g_ClientCharMgr.GetMyTransformationed())
        return;

    if (m_pGameImage)
        m_pGameImage->Draw();

    // Ground truth: 使用 vtable offset 28 的 Draw 方法
    if (m_nShowSpeech)
        m_ChatBallon.Draw();
}

// -------------------------------------------------------------------------
// MoveTrace — ground truth 004E8660
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::MoveTrace()
{
    // 重設巡邏相關狀態
    m_fPatrolScale = 1.0f;
    m_nPatrolReturn = 1;

    if (m_nTraceFlag)
    {
        if (!m_pOwnerChar)
            return;

        // 如果距離角色夠近，直接跟隨
        float dist = m_fPosX - (float)m_pOwnerChar->m_iPosX;
        if ((int)abs((int)(std::int64_t)dist) < 85)
        {
            m_fPosY = (float)(m_pOwnerChar->m_iPosY - m_nCharHeight);
            m_nOffsetX = (int)(std::int64_t)(m_fPosX - (float)m_pOwnerChar->m_iPosX);
            // Ground truth: 近距離 return 時不更新方向 — 保持現有方向
            return;
        }
    }

    // 追蹤模式
    m_nTraceFlag = 0;

    if (m_fTraceScale > 0.0f)
    {
        m_nOffsetX = (int)(std::int64_t)((m_fPosX - (float)m_pOwnerChar->m_iPosX) * m_fTraceScale);
        m_fTraceScale -= 0.02f;
    }

    // 更新面向方向
    int lr = m_pOwnerChar->m_dwLR_Flag;
    // Ground truth: direction = (lr == 0) ? 1 : 0
    m_nDirection = (lr == 0) ? 1 : 0;

    // 根據角色面向調整 X 偏移
    int offsetX = m_nOffsetX;
    if (lr)
    {
        if (offsetX < 35)
        {
            offsetX += m_nAcceleration;
            m_nOffsetX = offsetX;
            if (offsetX >= 35)
                offsetX = 35;
            m_nOffsetX = offsetX;
        }
    }
    else
    {
        if (offsetX > -35)
        {
            offsetX -= m_nAcceleration;
            m_nOffsetX = offsetX;
            if (offsetX <= -35)
                offsetX = -35;
            m_nOffsetX = offsetX;
        }
    }

    m_nAcceleration += 2;

    // 更新世界座標
    m_fPosX = (float)(m_nOffsetX + m_pOwnerChar->m_iPosX);
    m_fPosY = (float)(m_pOwnerChar->m_iPosY - m_nCharHeight);
}

// -------------------------------------------------------------------------
// MovePatrol — ground truth 004E87C0
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::MovePatrol()
{
    if (!m_pOwnerChar)
        return;

    if (m_nPatrolReturn)
    {
        // 返回角色身邊
        float dist = m_fPosX - (float)m_pOwnerChar->m_iPosX;

        if ((int)abs((int)(std::int64_t)dist) > 45)
        {
            if (m_fPatrolScale <= 0.0f)
            {
                m_nPatrolReturn = 0;
            }
            else
            {
                m_nOffsetX = (int)(std::int64_t)(dist * m_fPatrolScale);
                m_fPatrolScale -= 0.02f;
                m_fPosX = (float)(m_nOffsetX + m_pOwnerChar->m_iPosX);
                m_fPosY = (float)(m_pOwnerChar->m_iPosY - m_nCharHeight);
                // Ground truth: direction = (owner LR == 0) ? 1 : 0
                m_nDirection = (m_pOwnerChar->m_dwLR_Flag == 0) ? 1 : 0;
            }
            return;
        }
    }

    // 巡邏：左右來回移動
    m_nPatrolReturn = 0;
    m_fTraceScale = 1.0f;
    m_nTraceFlag = 1;
    // Ground truth: direction = 1
    m_nDirection = 1;

    int newOffset = m_nPatrolDir + m_nOffsetX;
    m_nOffsetX = newOffset;

    if (abs(newOffset) > 45)
    {
        m_nOffsetX = (newOffset <= 0) ? -45 : 45;
        m_nPatrolDir = -m_nPatrolDir;
    }

    // Ground truth: 若巡邏方向為負，direction = 0
    if (m_nPatrolDir < 0)
        m_nDirection = 0;

    m_nAcceleration = 1;
    m_fPosX = (float)(m_nOffsetX + m_pOwnerChar->m_iPosX);
    m_fPosY = (float)(m_pOwnerChar->m_iPosY - m_nCharHeight);
}
