#include "MiniGame/BoardMgr.h"

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

BoardMgr::BoardMgr()
    : m_x(0.0f)
    , m_yCurrent(0.0f)
    , m_yInitial(0.0f)
    , m_initialBreakCount(0)
    , m_breakCount(0)
    , m_lastBreakResult(false)
    , m_currentIdx(0)
    , m_dwordPad117(false)
    , m_pLeftPole(nullptr)
    , m_pRightPole(nullptr)
    , m_bFinalBreakTriggered(false)
    , m_finalTimer(0.0f)
{
}

BoardMgr::~BoardMgr() = default;

void BoardMgr::InitBoardMgr(float x, float yTop)
{
    m_x = x;
    m_yInitial = yTop;
    m_yCurrent = yTop;

    // mofclient.c 從陣列頂端（index 9）往下填入每塊木板，y 差距 30 像素。
    // 我們以等價的順序填入：m_boards[9] 對應最頂端，m_boards[0] 對應最底。
    for (int i = 0; i < 10; ++i)
    {
        float y = m_yCurrent - static_cast<float>(i) * 30.0f;
        m_boards[9 - i].InitAni_Board(m_x, y);
    }

    m_initialBreakCount = 0;
    m_breakCount = 0;
    m_lastBreakResult = false;
    m_currentIdx = 0;
    m_dwordPad117 = false;
    m_bFinalBreakTriggered = false;
    m_finalTimer = 0.0f;
}

int BoardMgr::SetBreakBoard(int count)
{
    // mofclient.c：寫入剩餘塊數，並立刻播放第一塊的破裂動畫。
    m_initialBreakCount = count;
    m_breakCount = count;
    if (count)
    {
        if (count == 1)
        {
            m_boards[m_currentIdx].Play(11);
            m_bFinalBreakTriggered = true;
            m_breakCount -= 1;
        }
        else
        {
            m_boards[m_currentIdx].Play(12);
            --m_breakCount;
        }
    }
    return 0;
}

void BoardMgr::SetPositionY(float deltaY)
{
    float newY = m_yCurrent + deltaY;
    m_yCurrent = newY;
    if (newY < m_yInitial)
        m_yCurrent = m_yInitial;

    for (int i = 0; i < 10; ++i)
    {
        float y = m_yCurrent - static_cast<float>(i) * 30.0f;
        m_boards[9 - i].SetPosition(m_x, y);
    }
}

char BoardMgr::Process(float dt)
{
    // mofclient.c 原始迴圈是以 v3 遞增 44 byte 往下掃，
    // 我們以等價的 index 迴圈處理。
    for (int i = 0; i < 10; ++i)
    {
        char result = m_boards[i].Process(dt);
        m_lastBreakResult = (result != 0);
        if (m_breakCount && result)
        {
            ++m_currentIdx;
            if (m_breakCount == 1)
            {
                m_boards[m_currentIdx].Play(11);
                m_currentIdx = 0;
                m_bFinalBreakTriggered = true;
            }
            else
            {
                m_boards[m_currentIdx].Play(12);
            }
            --m_breakCount;
        }
    }

    // 左右支柱（GameImage block 6 / 7 of 資源 0x20000039）。
    cltImageManager* pImgMgr = cltImageManager::GetInstance();
    GameImage* pLeft = pImgMgr->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pLeftPole = pLeft;
    if (pLeft)
    {
        pLeft->SetBlockID(6);
        pLeft->m_bFlag_446 = true;
        pLeft->m_fPosX = m_x - 215.0f;
        pLeft->m_bFlag_447 = true;
        pLeft->m_fPosY = m_yCurrent + 15.0f;
        pLeft->m_bFlag_447 = true;
        pLeft->Process();
    }

    GameImage* pRight = pImgMgr->GetGameImage(9u, 0x20000039u, 0, 1);
    m_pRightPole = pRight;
    if (pRight)
    {
        pRight->SetBlockID(7);
        pRight->m_bFlag_446 = true;
        pRight->m_fPosX = m_x + 112.0f;
        pRight->m_bFlag_447 = true;
        pRight->m_fPosY = m_yCurrent + 15.0f;
        pRight->m_bFlag_447 = true;
    }

    if (!m_bFinalBreakTriggered)
        return 0;

    m_finalTimer += dt;
    if (m_finalTimer <= 1.0f)
        return 0;

    m_bFinalBreakTriggered = false;
    return 1;
}

void BoardMgr::Render()
{
    for (int i = 9; i >= 0; --i)
        m_boards[i].Render();

    if (m_pLeftPole && m_pLeftPole->m_pGIData)
        m_pLeftPole->Draw();
    if (m_pRightPole && m_pRightPole->m_pGIData)
        m_pRightPole->Draw();
}
