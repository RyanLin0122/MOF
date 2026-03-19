#include "Object/clTransportObject.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <cstring>
#include <cmath>

extern int dword_A73088;
extern int dword_A7308C;

// clTransportAniInfo 和 stTransportKindInfo 尚未還原為獨立模組。
// 這裡提供最小定義讓程式碼可以編譯。
// 當這些類別被還原後，應替換為正確的 #include。

// -------------------------------------------------------------------------
// Constructor
// -------------------------------------------------------------------------
clTransportObject::clTransportObject()
    : m_pImageUp(nullptr)
    , m_pImageDown(nullptr)
    , m_pOwner(nullptr)
    , m_pAniInfoUp(nullptr)
    , m_pAniInfoDown(nullptr)
    , m_pKindInfo(nullptr)
    , m_pCCA(nullptr)
    , _reserved8(0)
    , m_nActive(0)
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
// Destructor
// -------------------------------------------------------------------------
clTransportObject::~clTransportObject()
{
    Release();
}

// -------------------------------------------------------------------------
// Release — 重設所有成員
// -------------------------------------------------------------------------
void clTransportObject::Release()
{
    m_nActive = 0;
    m_pOwner = nullptr;
    m_pKindInfo = nullptr;
    m_pAniInfoUp = nullptr;
    m_pAniInfoDown = nullptr;
    m_pImageUp = nullptr;
    m_pImageDown = nullptr;
}

// -------------------------------------------------------------------------
// InitTransport — 初始化交通工具
// Ground truth 呼叫:
//   clTransportKindInfo::GetTransportKindInfo
//   clClientTransportKindInfo::GetTransportAniInfoUp
//   clClientTransportKindInfo::GetTransportAniInfoDown
//   cltMyCharData::GetMyAccount
// -------------------------------------------------------------------------
void clTransportObject::InitTransport(ClientCharacter* pOwner, CCA* pCCA, std::uint16_t transportKind)
{
    if (!pOwner)
        return;

    m_pOwner = pOwner;
    m_pCCA = pCCA;

    // TODO: 當 clTransportKindInfo / clClientTransportKindInfo 被還原後啟用:
    // m_pKindInfo = clTransportKindInfo::GetTransportKindInfo(&g_clTransportKindInfo, transportKind);
    // if (m_pKindInfo) {
    //     m_pAniInfoUp = clClientTransportKindInfo::GetTransportAniInfoUp(&g_clTransportKindInfo, transportKind);
    //     m_pAniInfoDown = clClientTransportKindInfo::GetTransportAniInfoDown(&g_clTransportKindInfo, transportKind);
    //     if (m_pAniInfoUp && m_pAniInfoDown) {
    //         cltMyCharData::GetMyAccount(&g_clMyCharData);
    //     }
    // }
    m_pKindInfo = nullptr;
    m_pAniInfoUp = nullptr;
    m_pAniInfoDown = nullptr;
}

// -------------------------------------------------------------------------
// SetActive / GetActive
// -------------------------------------------------------------------------
void clTransportObject::SetActive(int active)
{
    m_nActive = active;
    if (m_pOwner)
        SetActionState(m_pOwner->GetActionState());
}

int clTransportObject::GetActive()
{
    return m_nActive;
}

// -------------------------------------------------------------------------
// PrepareDrawing — 準備繪製交通工具
// -------------------------------------------------------------------------
void clTransportObject::PrepareDrawing(int param)
{
    if (!m_nActive)
        return;
    if (!m_pOwner)
        return;

    unsigned int resUp = 0, resDown = 0;
    std::uint16_t frameUp = 0, frameDown = 0;

    // 根據動作狀態更新位置
    unsigned int actionState = m_pOwner->GetActionState();
    switch (actionState)
    {
    case 0:
        TransportStop();
        break;
    case 1:
        TransportMove();
        break;
    case 4:
        TransportHitted();
        break;
    }

    // 決定繪製格
    DecideDrawFrame(&resUp, &resDown, &frameUp, &frameDown);

    // 取得上下層圖像
    m_pImageUp = cltImageManager::GetInstance()->GetGameImage(1u, resUp, 0, 1);
    m_pImageDown = cltImageManager::GetInstance()->GetGameImage(1u, resDown, 0, 1);

    if (!m_pImageUp || !m_pImageDown)
        return;

    // 計算螢幕座標
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
    m_pImageUp->m_bFlipX = (m_pOwner->m_dwLR_Flag == 0);

    // 設定下層圖像
    m_pImageDown->SetPosition(fScreenX, fScreenY);
    m_pImageDown->SetBlockID(frameDown);
    m_pImageDown->m_bFlag_447 = true;
    m_pImageDown->m_bFlag_446 = true;
    m_pImageDown->m_bVertexAnimation = false;
    m_pImageDown->m_bFlipX = (m_pOwner->m_dwLR_Flag == 0);

    // 上層圖像: 調整 hotspot 和啟用繪製
    if (m_pImageUp->m_pGIData)
    {
        // Ground truth: 根據 block 資訊調整 fAngleY
        // 具體邏輯依賴 ImageResourceListData 的內部佈局
        m_pImageUp->m_bDrawPart2 = true;
    }

    // 下層圖像: 調整 hotspot 和啟用繪製
    if (m_pImageDown->m_pGIData)
    {
        m_pImageDown->m_bDrawPart2 = true;
    }
}

// -------------------------------------------------------------------------
// DrawUp / DrawDown — 繪製上下層圖像
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
// TransportMove / TransportStop / TransportHitted — 位置更新
// Ground truth: 三者邏輯完全相同
// -------------------------------------------------------------------------
static void UpdateTransportPosition(clTransportObject* self,
                                     int& posX, int& posY,
                                     ClientCharacter* pOwner, CCA* pCCA)
{
    if (pCCA)
    {
        // Ground truth: 使用 CCA 的 float 座標 (offset 128, 132)
        // posX = (int)(*(float*)((char*)pCCA + 128));
        // posY = (int)(*(float*)((char*)pCCA + 132));
        // CCA 類別尚未完整定義，先使用角色座標
        posX = pOwner->m_iPosX;
        posY = pOwner->m_iPosY;
    }
    else
    {
        posX = pOwner->m_iPosX;
        posY = pOwner->m_iPosY;
    }
}

void clTransportObject::TransportMove()
{
    if (m_pOwner)
        UpdateTransportPosition(this, m_nPosX, m_nPosY, m_pOwner, m_pCCA);
}

void clTransportObject::TransportStop()
{
    if (m_pOwner)
        UpdateTransportPosition(this, m_nPosX, m_nPosY, m_pOwner, m_pCCA);
}

void clTransportObject::TransportHitted()
{
    if (m_pOwner)
        UpdateTransportPosition(this, m_nPosX, m_nPosY, m_pOwner, m_pCCA);
}

// -------------------------------------------------------------------------
// DecideDrawFrame — 根據動作狀態決定繪製資源和幀
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
    default: action = 0; break;
    }

    // TODO: 當 clTransportAniInfo 被還原後啟用:
    // m_pAniInfoUp->GetFrameInfo(action, m_wCurFrameUp, outResUp, outFrameUp);
    // m_pAniInfoDown->GetFrameInfo(action, m_wCurFrameDown, outResDown, outFrameDown);
}

// -------------------------------------------------------------------------
// Poll — 推進動畫幀
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
// SetActionState — 設定動作狀態並重算幀數
// -------------------------------------------------------------------------
void clTransportObject::SetActionState(unsigned int actionState)
{
    m_dwActionState = actionState;
    m_wTotalFrameUp = 0;
    m_wTotalFrameDown = 0;

    if (m_pAniInfoUp && m_pAniInfoDown)
    {
        // TODO: 當 clTransportAniInfo 被還原後啟用:
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
    }

    m_wCurFrameUp = 0;
    m_wCurFrameDown = 0;
}
