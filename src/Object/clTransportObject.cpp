#include "Object/clTransportObject.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <cstring>
#include <cmath>

extern int dword_A73088;
extern int dword_A7308C;

// 前向宣告：clTransportKindInfo / clClientTransportKindInfo 尚未還原為獨立模組。
// 當這些類別被還原後，應替換為正確的 #include。

// clTransportAniInfo 方法存根 (未還原)
// struct clTransportAniInfo {
//     void GetFrameInfo(unsigned int action, unsigned short curFrame,
//                       unsigned int* outRes, unsigned short* outFrame);
//     unsigned short GetTotalFrameNum(unsigned int action);
// };

// -------------------------------------------------------------------------
// Constructor — ground truth 004EFBE0
// -------------------------------------------------------------------------
clTransportObject::clTransportObject()
    : m_pImageUp(nullptr)      // DWORD 1
    , m_pImageDown(nullptr)    // DWORD 2
    , m_pOwner(nullptr)        // DWORD 3
    , m_pAniInfoUp(nullptr)    // DWORD 4
    , m_pAniInfoDown(nullptr)  // DWORD 5
    , m_pKindInfo(nullptr)     // DWORD 6
    , m_pCCA(nullptr)          // DWORD 7
    , _reserved8(0)            // DWORD 8
    , m_nActive(0)             // DWORD 9
    , m_wTotalFrameUp(0)
    , m_wTotalFrameDown(0)
    , m_wCurFrameUp(0)
    , m_wCurFrameDown(0)
    , m_nPosX(0)
    , m_nPosY(0)
    , m_dwActionState(0)
{
}

// -------------------------------------------------------------------------
// Destructor — ground truth 004EFC30
// -------------------------------------------------------------------------
clTransportObject::~clTransportObject()
{
    Release();
}

// -------------------------------------------------------------------------
// Release — ground truth 004EFC40
// 重設 DWORD 1,2,3,4,5,6,9 = 0
// -------------------------------------------------------------------------
void clTransportObject::Release()
{
    m_nActive = 0;       // DWORD 9
    m_pOwner = nullptr;  // DWORD 3
    m_pKindInfo = nullptr;  // DWORD 6
    m_pAniInfoUp = nullptr;  // DWORD 4
    m_pAniInfoDown = nullptr;  // DWORD 5
    m_pImageUp = nullptr;  // DWORD 1
    m_pImageDown = nullptr;  // DWORD 2
}

// -------------------------------------------------------------------------
// InitTransport — ground truth 004EFC60
// -------------------------------------------------------------------------
void clTransportObject::InitTransport(ClientCharacter* pOwner, CCA* pCCA, std::uint16_t transportKind)
{
    if (!pOwner)
        return;

    m_pOwner = pOwner;
    m_pCCA = pCCA;

    // Ground truth:
    // m_pKindInfo = clTransportKindInfo::GetTransportKindInfo(&g_clTransportKindInfo, transportKind);
    // if (m_pKindInfo) {
    //     m_pAniInfoUp = clClientTransportKindInfo::GetTransportAniInfoUp(&g_clTransportKindInfo, transportKind);
    //     m_pAniInfoDown = clClientTransportKindInfo::GetTransportAniInfoDown(&g_clTransportKindInfo, transportKind);
    //     if (m_pAniInfoUp && m_pAniInfoDown) {
    //         cltMyCharData::GetMyAccount(&g_clMyCharData);
    //     }
    // }
    // TODO: 當 clTransportKindInfo / clClientTransportKindInfo 被還原後啟用上述程式碼
    m_pKindInfo = nullptr;
    m_pAniInfoUp = nullptr;
    m_pAniInfoDown = nullptr;
}

// -------------------------------------------------------------------------
// SetActive — ground truth 004EFCD0
// Ground truth: 直接讀取 owner 的 action state (offset 9684)，不做 null check
// -------------------------------------------------------------------------
void clTransportObject::SetActive(int active)
{
    m_nActive = active;
    // Ground truth 不檢查 m_pOwner 是否為 null，直接讀取
    if (m_pOwner)
        SetActionState(m_pOwner->GetActionState());
}

// -------------------------------------------------------------------------
// GetActive — ground truth 004EFCF0
// -------------------------------------------------------------------------
int clTransportObject::GetActive()
{
    return m_nActive;
}

// -------------------------------------------------------------------------
// 位置更新輔助函式 — ground truth TransportMove/Stop/Hitted 三者邏輯完全相同
// Ground truth: 若 CCA 存在，座標來自 CCA float offset 128/132
//               否則使用 owner 的 m_iPosX/m_iPosY (offset 4384/4388)
// -------------------------------------------------------------------------
void clTransportObject::UpdatePosition()
{
    if (!m_pOwner)
        return;

    int ownerPosX = m_pOwner->m_iPosX;
    int ownerPosY = m_pOwner->m_iPosY;

    if (m_pCCA)
    {
        // Ground truth: 使用 CCA 的 float 座標 (offset 128, 132)
        float ccaPosX = *(float*)((char*)m_pCCA + 128);
        float ccaPosY = *(float*)((char*)m_pCCA + 132);
        m_nPosX = (int)(std::int64_t)ccaPosX;
        m_nPosY = (int)(std::int64_t)ccaPosY;
    }
    else
    {
        m_nPosX = ownerPosX;
        m_nPosY = ownerPosY;
    }
}

// -------------------------------------------------------------------------
// TransportMove / TransportStop / TransportHitted — ground truth 004EFF80/004EFFD0/004F0020
// -------------------------------------------------------------------------
void clTransportObject::TransportMove()
{
    UpdatePosition();
}

void clTransportObject::TransportStop()
{
    UpdatePosition();
}

void clTransportObject::TransportHitted()
{
    UpdatePosition();
}

// -------------------------------------------------------------------------
// PrepareDrawing — ground truth 004EFD00
// -------------------------------------------------------------------------
void clTransportObject::PrepareDrawing(int param)
{
    if (!m_nActive)
        return;
    if (!m_pOwner)
        return;

    unsigned int resUp = 0, resDown = 0;
    std::uint16_t frameUp = 0, frameDown = 0;

    // 根據 owner 的 action state 更新位置
    unsigned int actionState = m_pOwner->GetActionState();
    if (actionState == 0)
    {
        TransportStop();
    }
    else if (actionState == 1)
    {
        TransportMove();
    }
    else if (actionState == 4)
    {
        TransportHitted();
    }

    // 決定繪製格
    DecideDrawFrame(&resUp, &resDown, &frameUp, &frameDown);

    // 取得上下層圖像
    m_pImageUp = cltImageManager::GetInstance()->GetGameImage(1u, resUp, 0, 1);
    m_pImageDown = cltImageManager::GetInstance()->GetGameImage(1u, resDown, 0, 1);

    if (!m_pImageUp || !m_pImageDown)
        return;

    // 計算螢幕座標
    // Ground truth: 若 CCA 存在，直接使用 m_nPosX/m_nPosY (已由 UpdatePosition 設為 CCA 座標)
    //               否則減去相機偏移
    int screenX = m_nPosX;
    int screenY = m_nPosY;
    if (!m_pCCA)
    {
        screenX = m_nPosX - dword_A73088;
        screenY = m_nPosY - dword_A7308C;
    }

    float fScreenX = (float)screenX;
    float fScreenY = (float)screenY;

    // 設定上層圖像
    m_pImageUp->m_bFlag_447 = true;
    m_pImageUp->SetBlockID(frameUp);
    m_pImageUp->m_bFlag_446 = true;
    m_pImageUp->SetPosition(fScreenX, fScreenY);
    m_pImageUp->m_bVertexAnimation = false;
    // Ground truth: flip = (owner LR_Flag == 0)
    m_pImageUp->m_bFlipX = (m_pOwner->m_dwLR_Flag == 0);

    // 設定下層圖像
    m_pImageDown->SetPosition(fScreenX, fScreenY);
    m_pImageDown->SetBlockID(frameDown);
    m_pImageDown->m_bFlag_447 = true;
    m_pImageDown->m_bFlag_446 = true;
    m_pImageDown->m_bVertexAnimation = false;
    m_pImageDown->m_bFlipX = (m_pOwner->m_dwLR_Flag == 0);

    // Ground truth: 上層圖像 hotspot/offset 修正
    // 從 GIData 讀取 block 資料，依 block 尺寸/偏移修正 fAngleY (offset +352)
    if (m_pImageUp->m_pGIData)
    {
        int* pBlockData = (int*)(*((int*)m_pImageUp->m_pGIData + 8));  // GIData DWORD+8 = block array ptr
        if (pBlockData)
        {
            // Ground truth: block stride = 52 bytes = 13 DWORDs
            // block[frameUp].offset24 = DWORD at 52*frameUp + 24
            // block[frameUp].offset32 = DWORD at 52*frameUp + 32
            int blockOff24 = *(int*)((char*)pBlockData + 52 * frameUp + 24);
            int blockOff32 = *(int*)((char*)pBlockData + 52 * frameUp + 32);
            if ((double)blockOff24 < fabs((double)blockOff32))
            {
                *(float*)((char*)m_pImageUp + 352) = (float)(-(blockOff32 + blockOff24));
            }
        }
        m_pImageUp->m_bDrawPart2 = true;
    }

    // Ground truth: 下層圖像 hotspot/offset 修正 (公式不同)
    if (m_pImageDown->m_pGIData)
    {
        int* pBlockData = (int*)(*((int*)m_pImageDown->m_pGIData + 8));
        if (pBlockData)
        {
            int blockOff24 = *(int*)((char*)pBlockData + 52 * frameDown + 24);
            int blockOff32 = *(int*)((char*)pBlockData + 52 * frameDown + 32);
            if ((double)blockOff24 < fabs((double)blockOff32))
            {
                *(float*)((char*)m_pImageDown + 352) = 15.0f - (float)(blockOff32 + blockOff24);
            }
        }
        m_pImageDown->m_bDrawPart2 = true;
    }
}

// -------------------------------------------------------------------------
// DrawUp / DrawDown — ground truth 004EFF40 / 004EFF60
// -------------------------------------------------------------------------
void clTransportObject::DrawUp(int param)
{
    if (m_nActive && m_pImageUp)
        m_pImageUp->Draw();
}

void clTransportObject::DrawDown(int param)
{
    if (m_nActive && m_pImageDown)
        m_pImageDown->Draw();
}

// -------------------------------------------------------------------------
// DecideDrawFrame — ground truth 004F0070
// 根據 m_dwActionState 呼叫 clTransportAniInfo::GetFrameInfo
// -------------------------------------------------------------------------
void clTransportObject::DecideDrawFrame(unsigned int* outResUp, unsigned int* outResDown,
                                         std::uint16_t* outFrameUp, std::uint16_t* outFrameDown)
{
    if (!m_pAniInfoUp || !m_pAniInfoDown)
        return;

    // Ground truth: 根據 m_dwActionState 決定 action 參數
    // action 0 = Stop, 1 = Move, 4 = Hitted
    unsigned int action;
    switch (m_dwActionState)
    {
    case 0:  action = 0; break;
    case 1:  action = 1; break;
    case 4:  action = 4; break;
    default: return;
    }

    // Ground truth:
    // clTransportAniInfo::GetFrameInfo(m_pAniInfoUp, action, m_wCurFrameUp, outResUp, outFrameUp);
    // clTransportAniInfo::GetFrameInfo(m_pAniInfoDown, action, m_wCurFrameDown, outResDown, outFrameDown);
    // TODO: 當 clTransportAniInfo 被還原後取消註解
    (void)action;
}

// -------------------------------------------------------------------------
// Poll — ground truth 004F0130
// -------------------------------------------------------------------------
void clTransportObject::Poll()
{
    if (!m_nActive)
        return;

    ++m_wCurFrameUp;
    ++m_wCurFrameDown;

    if (m_wCurFrameUp >= m_wTotalFrameUp)
        m_wCurFrameUp = 0;
    if (m_wCurFrameDown >= m_wTotalFrameDown)
        m_wCurFrameDown = 0;
}

// -------------------------------------------------------------------------
// SetActionState — ground truth 004F0160
// -------------------------------------------------------------------------
void clTransportObject::SetActionState(unsigned int actionState)
{
    m_dwActionState = actionState;
    m_wTotalFrameUp = 0;
    m_wTotalFrameDown = 0;

    // Ground truth: 根據 actionState 呼叫 GetTotalFrameNum
    if (m_pAniInfoUp && m_pAniInfoDown)
    {
        // switch (actionState) {
        // case 0:
        //     m_wTotalFrameUp = m_pAniInfoUp->GetTotalFrameNum(0);
        //     m_wTotalFrameDown = m_pAniInfoDown->GetTotalFrameNum(0);
        //     break;
        // case 1:
        //     m_wTotalFrameUp = m_pAniInfoUp->GetTotalFrameNum(1);
        //     m_wTotalFrameDown = m_pAniInfoDown->GetTotalFrameNum(1);
        //     break;
        // case 4:
        //     m_wTotalFrameUp = m_pAniInfoUp->GetTotalFrameNum(4);
        //     m_wTotalFrameDown = m_pAniInfoDown->GetTotalFrameNum(4);
        //     break;
        // }
        // TODO: 當 clTransportAniInfo 被還原後取消註解
    }

    m_wCurFrameUp = 0;
    m_wCurFrameDown = 0;
}
