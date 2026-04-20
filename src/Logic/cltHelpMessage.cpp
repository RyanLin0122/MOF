#include "Logic/cltHelpMessage.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"
#include "Image/CDeviceManager.h"
#include "Util/cltTimer.h"
#include "global.h"
#include <windows.h>

// ---------------------------------------------------------------------------
// cltHelpMessage
// ---------------------------------------------------------------------------

cltHelpMessage::cltHelpMessage()
    : m_bActive(0)
    , m_bDisabling(0)
    , m_bCanShow(1)
    , m_dwResourceID(0)
    , m_wBlockID(0)
    , m_nAlpha(0)
    , m_pBgImage(nullptr)
    , m_pCloseImage(nullptr)
    , m_hTimer(0)
    , m_btnExit()
    , m_bBtnCreated(1)
{
}

cltHelpMessage::~cltHelpMessage()
{
}

// ---------------------------------------------------------------------------
// Initialize — 根據 helpKind 設定資源 ID 與 block，建立計時器和按鈕
// ---------------------------------------------------------------------------
void cltHelpMessage::Initialize(int helpKind)
{
    if (!m_bCanShow)
        return;

    m_dwResourceID = 0;

    switch (helpKind)
    {
    case 0:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 0;
        break;
    case 1:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 3;
        break;
    case 2:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 7;
        break;
    case 5:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 16;
        break;
    case 6:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 17;
        break;
    case 7:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 19;
        break;
    case 8:
        m_dwResourceID = 0x2000005Eu;
        m_wBlockID = 2;
        break;
    case 11:
        m_dwResourceID = 0x2000005Eu;
        m_wBlockID = 0;
        break;
    case 12:
        m_dwResourceID = 0x2000005Eu;
        m_wBlockID = 1;
        break;
    case 13:
        m_dwResourceID = 0x20000076u;
        m_wBlockID = 0;
        break;
    case 14:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 1;
        break;
    case 15:
    case 22:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 2;
        break;
    case 16:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 15;
        break;
    case 17:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 5;
        break;
    case 18:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 8;
        break;
    case 19:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 9;
        break;
    case 20:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 10;
        break;
    case 21:
        m_dwResourceID = 0x2000005Du;
        m_wBlockID = 11;
        break;
    default:
        break;
    }

    if (m_dwResourceID)
    {
        m_bActive  = 1;
        m_bDisabling = 0;
        m_nAlpha   = 255;

        // 釋放前一個計時器
        if (m_hTimer)
            g_clTimerManager.ReleaseTimer(m_hTimer);
        m_hTimer = 0;

        // 建立 60 秒 (0xEA60 ms) 計時器
        m_hTimer = g_clTimerManager.CreateTimer(
            0xEA60u,
            reinterpret_cast<std::uintptr_t>(this),
            0, 1, 0, 0,
            reinterpret_cast<cltTimer::TimerCallback>(cltHelpMessage::OnTimer_TimeOutView),
            0, 0);

        // 只在第一次建立按鈕
        if (m_bBtnCreated)
        {
            m_btnExit.CreateBtn(
                g_Game_System_Info.ScreenWidth - 156,
                263,
                6u,
                0x20000064u, 0,
                0x20000064u, 1,
                0x20000064u, 2,
                0x20000064u, 3,
                reinterpret_cast<void (*)(std::uintptr_t)>(cltHelpMessage::OnBtn_Exit),
                reinterpret_cast<std::uintptr_t>(this),
                0);
            m_bBtnCreated = 0;
        }

        m_btnExit.SetActive(1);
        OutputDebugStringA("cltHelpMessage::Initialize()\n");
    }
    else
    {
        m_bActive = 0;
    }
}

// ---------------------------------------------------------------------------
// 靜態回呼
// ---------------------------------------------------------------------------
void cltHelpMessage::OnTimer_TimeOutView(unsigned int /*timerID*/, cltHelpMessage* pThis)
{
    pThis->SetDisable();
}

void cltHelpMessage::OnBtn_Exit(cltHelpMessage* pThis)
{
    pThis->Free();
}

// ---------------------------------------------------------------------------
// SetDisable — 開始淡出
// ---------------------------------------------------------------------------
void cltHelpMessage::SetDisable()
{
    m_bDisabling = 1;
    m_bCanShow   = 0;
    m_hTimer     = 0;
}

// ---------------------------------------------------------------------------
// SetActive — 恢復顯示
// ---------------------------------------------------------------------------
void cltHelpMessage::SetActive()
{
    m_bDisabling = 0;
    m_bCanShow   = 1;
}

// ---------------------------------------------------------------------------
// IsShow — 設定是否允許顯示
// ---------------------------------------------------------------------------
void cltHelpMessage::IsShow(int helpState)
{
    m_bCanShow = helpState;
}

// ---------------------------------------------------------------------------
// Free — 完全釋放
// ---------------------------------------------------------------------------
void cltHelpMessage::Free()
{
    unsigned int hTimer = m_hTimer;

    m_bActive    = 0;
    m_bDisabling = 0;
    m_pBgImage   = nullptr;
    m_wBlockID   = 0;
    m_nAlpha     = 0;

    if (hTimer)
        g_clTimerManager.ReleaseTimer(hTimer);
    m_hTimer = 0;

    m_btnExit.SetActive(0);
    dword_AFD344 = 0;
}

// ---------------------------------------------------------------------------
// CloseHelpBox
// ---------------------------------------------------------------------------
int cltHelpMessage::CloseHelpBox()
{
    if (!m_bActive)
        return 0;
    Free();
    dword_AFD344 = 0;
    return 1;
}

// ---------------------------------------------------------------------------
// Poll
// ---------------------------------------------------------------------------
void cltHelpMessage::Poll()
{
    if (m_bActive && m_bCanShow)
    {
        if (m_bDisabling)
        {
            m_nAlpha -= 3;
            if (m_nAlpha < 0)
                Free();
        }
        m_btnExit.Poll();
    }
}

// ---------------------------------------------------------------------------
// PrepareDrawing
// ---------------------------------------------------------------------------
void cltHelpMessage::PrepareDrawing()
{
    if (!m_bActive)
        return;
    if (!m_bCanShow)
        return;

    // 背景圖片
    GameImage* bg = cltImageManager::GetInstance()->GetGameImage(
        6u, m_dwResourceID, 0, 1);
    m_pBgImage = bg;

    int xPos = g_Game_System_Info.ScreenWidth - 250;
    bg->SetPosition((float)xPos, 70.0f);
    bg->SetBlockID(m_wBlockID);
    bg->m_bFlag_447        = true;
    bg->m_bFlag_446        = true;
    bg->m_bVertexAnimation = false;

    bg->SetAlpha((unsigned int)m_nAlpha);
    bg->m_bFlag_450        = true;
    bg->m_bVertexAnimation = false;

    // 關閉按鈕底圖
    GameImage* closeImg = cltImageManager::GetInstance()->GetGameImage(
        6u, 0x2000005Fu, 0, 1);
    m_pCloseImage = closeImg;

    int xPos2 = g_Game_System_Info.ScreenWidth - 250;
    closeImg->SetPosition((float)xPos2, 260.0f);
    closeImg->SetBlockID(2);
    closeImg->m_bFlag_447        = true;
    closeImg->m_bFlag_446        = true;
    closeImg->m_bVertexAnimation = false;

    closeImg->SetAlpha((unsigned int)m_nAlpha);
    closeImg->m_bFlag_450        = true;
    closeImg->m_bVertexAnimation = false;

    m_btnExit.PrepareDrawing();
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------
void cltHelpMessage::Draw()
{
    if (!m_bActive || !m_bCanShow)
        return;

    CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, 5u);   // D3DBLEND_SRCALPHA
    CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, 6u);   // D3DBLEND_INVSRCALPHA

    if (m_pBgImage)
        m_pBgImage->Draw();
    if (m_pCloseImage)
        m_pCloseImage->Draw();

    m_btnExit.Draw();
}

// ---------------------------------------------------------------------------
// OnHSPopupMessage — 全域輔助
// ---------------------------------------------------------------------------
void OnHSPopupMessage(int helpKind)
{
    g_clHelpMessage.Initialize(helpKind);
}
