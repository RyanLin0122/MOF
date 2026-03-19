#include "Object/cltMoF_SpiritObject.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Character/ClientCharacter.h"
#include "Character/ClientCharacterManager.h"
#include "System/CSpiritSystem.h"
#include "System/cltLevelSystem.h"
#include "System/cltClassSystem.h"
#include "global.h"
#include <cstring>
#include <cstdlib>
#include <cmath>

extern int dword_A73088;
extern int dword_A7308C;


// CSpiritSpeechMgr — 精靈對話管理器 (尚未還原)
class CSpiritSpeechMgr {
public:
    char* GetSpiritSpeechScript() { return (char*)""; }
};
static CSpiritSpeechMgr g_clSpiritSpeechMgr;

// -------------------------------------------------------------------------
// Constructor
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
    , m_nPatrolDir(1)
    , m_nOffsetX(0)
    , m_nCharHeight(0)
    , m_nAcceleration(0)
    , m_fTraceScale(1.0f)  // 1065353216 = 1.0f in IEEE 754
    , m_nTraceFlag(1)
    , m_fPatrolScale(0.0f)
    , m_nPatrolReturn(0)
{
    // CControlChatBallon 由其自己的建構函式初始化
}

// -------------------------------------------------------------------------
// Destructor
// -------------------------------------------------------------------------
cltMoF_SpiritObject::~cltMoF_SpiritObject()
{
    // CControlChatBallon 由其自己的解構函式清理
}

// -------------------------------------------------------------------------
// SetActive — 設定精靈啟動狀態
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::SetActive(ClientCharacter* pChar, int active)
{
    // Ground truth: 檢查角色的 accountID 是否匹配
    // *((_DWORD *)a2 + 114) == *(_DWORD *)((char *)&loc_43E090 + *((_DWORD *)this + 15))
    // 這裡簡化為直接設定
    if (pChar)
    {
        m_nOffsetX = 0;
        m_nActive = active;
        m_nPatrolDir = 1;
        m_fTraceScale = 1.0f;
        m_nTraceFlag = 1;
    }
}

// -------------------------------------------------------------------------
// GetActive
// -------------------------------------------------------------------------
int cltMoF_SpiritObject::GetActive()
{
    return m_nActive;
}

// -------------------------------------------------------------------------
// Initialize — 初始化精靈物件
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::Initialize(ClientCharacterManager* pMgr)
{
    m_pCharMgr = pMgr;
    m_wBlockCount = 0;

    // 重設聊天氣泡
    m_ChatBallon.SetString((char*)"", 0, 0, 0, 0, (Direction)(DirLeft | DirRight));

    m_nOffsetX = 0;
    m_nPatrolDir = 1;
    m_fTraceScale = 1.0f;
    m_nTraceFlag = 1;
    m_nActive = 0;
}

// -------------------------------------------------------------------------
// GetFront — 檢查巡邏方向是否為正向
// -------------------------------------------------------------------------
int cltMoF_SpiritObject::GetFront()
{
    return (m_nPatrolDir > 0) ? 1 : 0;
}

// -------------------------------------------------------------------------
// SetChar — 設定精靈所屬角色和精靈資訊
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::SetChar(ClientCharacter* pChar, std::uint16_t param)
{
    if (!pChar)
        return;

    m_pOwnerChar = pChar;
    m_wCharKind = param;
    m_byLevel = (std::uint8_t)g_clLevelSystem.GetLevel();
    m_wClassCode = g_clClassSystem.GetClass();

    m_nDirection = pChar->m_dwLR_Flag;
    m_nActive = 0;

    // 根據等級取得精靈資訊
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
// UpdateSpirit — 更新精靈資訊 (當等級變化時)
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::UpdateSpirit(std::uint8_t level)
{
    if (!m_nActive)
    {
        m_byLevel = level;
        return;
    }

    m_byLevel = level;
    m_nActive = 0;

    std::uint16_t spiritKind = g_clSpiritSystem.GetSpiritKind(level);
    stSpiritInfo* pInfo = g_clSpiritSystem.GetSpiritInfo(spiritKind);

    if (pInfo)
    {
        m_dwResourceID = pInfo->dwResourceId;
        m_dwStartBlockID = pInfo->wStartBlockId;
        m_wBlockCount = pInfo->wBlockCount;

        m_nActive = 1;
        if (m_pOwnerChar)
            m_nDirection = m_pOwnerChar->m_dwLR_Flag;
        m_fTraceScale = 1.0f;
        m_nTraceFlag = 1;
        m_dwCurrentBlock = 0;
    }
}

// -------------------------------------------------------------------------
// Poll — 更新聊天氣泡內容
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
// PrepareDrawing — 準備精靈繪製
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::PrepareDrawing(int showSpeech)
{
    if (!m_nActive)
        return;

    // 變身時不顯示精靈
    // if (ClientCharacterManager::GetMyTransformationed(&g_ClientCharMgr))
    //     return;

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

    // 方向
    m_pGameImage->m_bFlipX = (m_nDirection != 0);

    // 推進動畫
    m_fFrameAccum += 0.2f;
    m_dwCurrentBlock = m_dwStartBlockID + (int)(std::int64_t)m_fFrameAccum;

    // 聊天氣泡位置
    if (m_nShowSpeech)
    {
        if (m_pGameImage->m_pGIData && m_pOwnerChar)
        {
            // 設定聊天氣泡位置
            int ballonX = m_nOffsetX + m_pOwnerChar->m_iPosX - dword_A73088;
            int ballonY = (int)(m_fPosY - (float)dword_A7308C);
            // Ground truth: 使用 block 資訊計算 Y 偏移
            m_ChatBallon.SetPos(ballonX, ballonY);
            m_ChatBallon.Show();
        }
    }
}

// -------------------------------------------------------------------------
// Draw — 繪製精靈
// -------------------------------------------------------------------------
void cltMoF_SpiritObject::Draw()
{
    if (!m_nActive)
        return;

    // 變身時不繪製
    // if (ClientCharacterManager::GetMyTransformationed(&g_ClientCharMgr))
    //     return;

    if (m_pGameImage)
        m_pGameImage->Draw();

    if (m_nShowSpeech)
        m_ChatBallon.Draw();
}

// -------------------------------------------------------------------------
// MoveTrace — 追蹤角色移動
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
            return;
        }
    }

    // 追蹤模式：使用縮放因子接近角色
    m_nTraceFlag = 0;

    if (m_fTraceScale > 0.0f)
    {
        m_nOffsetX = (int)(std::int64_t)((m_fPosX - (float)m_pOwnerChar->m_iPosX) * m_fTraceScale);
        m_fTraceScale -= 0.02f;
    }

    // 更新面向方向
    int lr = m_pOwnerChar->m_dwLR_Flag;
    m_nDirection = (lr == 0) ? 1 : 0;

    // 根據角色面向調整 X 偏移
    if (lr)
    {
        if (m_nOffsetX < 35)
        {
            m_nOffsetX += m_nAcceleration;
            if (m_nOffsetX >= 35)
                m_nOffsetX = 35;
        }
    }
    else
    {
        if (m_nOffsetX > -35)
        {
            m_nOffsetX -= m_nAcceleration;
            if (m_nOffsetX <= -35)
                m_nOffsetX = -35;
        }
    }

    m_nAcceleration += 2;

    // 更新世界座標
    m_fPosX = (float)(m_nOffsetX + m_pOwnerChar->m_iPosX);
    m_fPosY = (float)(m_pOwnerChar->m_iPosY - m_nCharHeight);
}

// -------------------------------------------------------------------------
// MovePatrol — 巡邏移動
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
                m_nDirection = (m_pOwnerChar->m_dwLR_Flag == 0) ? 1 : 0;
            }
            return;
        }
    }

    // 巡邏：左右來回移動
    m_nPatrolReturn = 0;
    m_fTraceScale = 1.0f;
    m_nTraceFlag = 1;
    m_nDirection = 1;

    int newOffset = m_nPatrolDir + m_nOffsetX;
    m_nOffsetX = newOffset;

    if (abs(newOffset) > 45)
    {
        m_nOffsetX = (newOffset <= 0) ? -45 : 45;
        m_nPatrolDir = -m_nPatrolDir;
    }

    if (m_nPatrolDir < 0)
        m_nDirection = 0;

    m_nAcceleration = 1;
    m_fPosX = (float)(m_nOffsetX + m_pOwnerChar->m_iPosX);
    m_fPosY = (float)(m_pOwnerChar->m_iPosY - m_nCharHeight);
}
