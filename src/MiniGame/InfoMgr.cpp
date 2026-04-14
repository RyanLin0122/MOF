#include "MiniGame/InfoMgr.h"

#include "global.h"
#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

InfoMgr::InfoMgr()
    : m_pBgLeft(nullptr)
    , m_pBgRight(nullptr)
    , m_bActive(1)
    , m_scoreNum{}
    , m_timeNum()
    , m_screenX((g_Game_System_Info.ScreenWidth - 800) / 2)
    , m_screenY((g_Game_System_Info.ScreenHeight - 600) / 2)
    , m_scoreX{}
    , m_scoreY{}
    , m_scoreActive{}
{
    // mofclient.c：前 5 個分數顯示 X 分別為 screenX + 27, +67, +107, +147, +187
    float sx = static_cast<float>(m_screenX);
    float sy = static_cast<float>(m_screenY);
    m_scoreX[0] = sx + 27.0f;
    m_scoreX[1] = sx + 67.0f;
    m_scoreX[2] = sx + 107.0f;
    m_scoreX[3] = sx + 147.0f;
    m_scoreX[4] = sx + 187.0f;
    // 總分 X
    m_scoreX[5] = sx + 245.0f;

    // 所有分數 Y 都在 screenY + 60
    for (int i = 0; i < 6; ++i)
        m_scoreY[i] = sy + 60.0f;

    // 初始化 6 個 score 數字顯示器 —— 對應 mofclient.c 的 eh vector ctor。
    for (int i = 0; i < 6; ++i)
        m_scoreNum[i].InitMini_IMG_Number(0x22000016u, 0u, 0xAu);

    // 時間顯示器 (block base = 0x1B，10 個影格)
    m_timeNum.InitMini_IMG_Number(0x22000007u, 0x1Bu, 0x24u);
}

InfoMgr::~InfoMgr() = default;

void InfoMgr::InitInfoMgr()
{
    // mofclient.c：先把「總分」與「時間」顯示歸零並 Process 一次。
    m_scoreNum[5].SetNumber(0, m_scoreX[5], m_scoreY[5]);
    m_scoreNum[5].Process();

    float tx = static_cast<float>(m_screenX) + 765.0f;
    float ty = static_cast<float>(m_screenY) + 55.0f;
    m_timeNum.SetNumber(10, tx, ty);
    m_timeNum.Process();

    for (int i = 0; i < 5; ++i)
        m_scoreActive[i] = 0;
}

void InfoMgr::SetScore(int slot, int value)
{
    m_scoreActive[slot] = 1;
    m_scoreNum[slot].SetNumber(value, m_scoreX[slot], m_scoreY[slot]);
}

void InfoMgr::SetTime(int seconds)
{
    float tx = static_cast<float>(m_screenX) + 765.0f;
    float ty = static_cast<float>(m_screenY) + 55.0f;
    m_timeNum.SetNumber(seconds, tx, ty);
}

void InfoMgr::Process(float /*dt*/)
{
    if (!m_bActive)
        return;

    // 背景左 / 右 (GameImage block 8, 9 of res 0x20000039)
    GameImage* pLeft = cltImageManager::GetInstance()->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pBgLeft = pLeft;
    if (pLeft)
    {
        pLeft->SetBlockID(8);
        pLeft->m_bFlag_446 = true;
        pLeft->m_fPosX = static_cast<float>(m_screenX);
        pLeft->m_bFlag_447 = true;
        pLeft->m_fPosY = static_cast<float>(m_screenY) + 50.0f;
        pLeft->Process();
    }

    GameImage* pRight = cltImageManager::GetInstance()->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pBgRight = pRight;
    if (pRight)
    {
        pRight->SetBlockID(9);
        pRight->m_bFlag_446 = true;
        pRight->m_fPosX = static_cast<float>(m_screenX) + 659.0f;
        pRight->m_bFlag_447 = true;
        pRight->m_fPosY = static_cast<float>(m_screenY) + 62.0f;
        pRight->Process();
    }

    // 5 個單局分數
    for (int i = 0; i < 5; ++i)
        if (m_scoreActive[i])
            m_scoreNum[i].Process();

    // 總分、時間
    m_scoreNum[5].Process();
    m_timeNum.Process();
}

void InfoMgr::Render()
{
    if (!m_bActive)
        return;

    if (m_pBgLeft && m_pBgLeft->m_pGIData)
        m_pBgLeft->Draw();
    if (m_pBgRight && m_pBgRight->m_pGIData)
        m_pBgRight->Draw();

    for (int i = 0; i < 5; ++i)
        if (m_scoreActive[i])
            m_scoreNum[i].Render();

    m_scoreNum[5].Render();
    m_timeNum.Render();
}
