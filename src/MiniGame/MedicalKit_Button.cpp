#include "MiniGame/MedicalKit_Button.h"

#include <cstring>

#include "Image/cltImageManager.h"
#include "Image/GameImage.h"

namespace {
inline float bitsToFloat(unsigned int bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof(f));
    return f;
}
}

MedicalKit_Button::MedicalKit_Button()
{
    // mofclient.c：先清掉 m_ani[0..1]
    std::memset(m_ani, 0, sizeof(m_ani));

    // FrameSkip 預設 ~1/60（由 FrameSkip ctor 完成），再於 ctor 末尾覆寫。
    m_bSelected     = 0;
    m_buttonIdx     = 0;
    m_pImage        = nullptr;
    m_fX            = 0.0f;
    m_fY            = 0.0f;
    m_fAnchor       = 0.0f;
    m_state         = 0;
    // mofclient.c 在 ctor 末尾把 threshold 改寫成 0x3AA3D70A
    m_FrameSkip.m_fTimePerFrame = bitsToFloat(983815946u);   // ≈ 0.00125
}

MedicalKit_Button::~MedicalKit_Button() = default;

void MedicalKit_Button::CreateMedicalKit_Button(Mini_AniInfo* aniInfo, int buttonIdx)
{
    m_ani[0]    = aniInfo[0];
    m_ani[1]    = aniInfo[1];
    m_buttonIdx = buttonIdx;
}

void MedicalKit_Button::OpenMedicalKit(float x, float y)
{
    m_fX = x;
    int idx = m_buttonIdx;
    m_fY = y;
    switch (idx)
    {
        case 0:
            m_bSelected = 0;
            m_state     = 0;
            m_fAnchor   = y - 85.0f;
            break;
        case 1:
            m_bSelected = 0;
            m_state     = 0;
            m_fAnchor   = x - 70.0f;
            break;
        case 2:
            m_bSelected = 0;
            m_state     = 0;
            m_fAnchor   = y + 65.0f;
            break;
        case 3:
            m_fAnchor   = x + 68.0f;
            m_bSelected = 0;
            m_state     = 0;
            break;
        default:
            m_bSelected = 0;
            m_state     = 0;
            break;
    }
}

void MedicalKit_Button::CloseMedicalKit()
{
    m_state     = 2;
    m_FrameSkip.m_fTimePerFrame = bitsToFloat(981668463u);  // ≈ 0.001
}

void MedicalKit_Button::SetSelect(bool selected)
{
    m_bSelected = selected ? 1 : 0;
}

int MedicalKit_Button::Process(float dt)
{
    cltImageManager* pMgr = cltImageManager::GetInstance();

    // 取對應的圖：未選中 m_ani[0]，選中 m_ani[1]
    // mofclient.c 直接寫 pImg 欄位（無 null guard）。
    GameImage* pImg = pMgr->GetGameImage(9u, m_ani[m_bSelected].resourceID, 0, 1);
    m_pImage = pImg;
    std::uint16_t blockID = m_ani[m_bSelected].blockID;
    pImg->m_bFlag_446 = true;
    pImg->m_wBlockID  = blockID;

    // ----- 開啟階段 -----
    if (m_state == 0)
    {
        switch (m_buttonIdx)
        {
            case 0:
            {
                float v = dt + m_FrameSkip.m_fAccumulatedTime;
                bool below = v < m_FrameSkip.m_fTimePerFrame;
                m_FrameSkip.m_fAccumulatedTime = v;
                int frames = 0;
                if (!below)
                {
                    long long n = static_cast<long long>(v / m_FrameSkip.m_fTimePerFrame);
                    if (n)
                        m_FrameSkip.m_fAccumulatedTime = v - static_cast<float>(static_cast<int>(n)) * m_FrameSkip.m_fTimePerFrame;
                    frames = static_cast<int>(n);
                }
                m_fY -= static_cast<float>(frames) * 0.5f;
                if (m_fY < m_fAnchor)
                {
                    float anchor = m_fAnchor;
                    m_state  = 1;
                    m_fY     = anchor;
                    m_fAnchor = anchor + 85.0f;
                }
                break;
            }
            case 1:
            {
                float v = dt + m_FrameSkip.m_fAccumulatedTime;
                bool below = v < m_FrameSkip.m_fTimePerFrame;
                m_FrameSkip.m_fAccumulatedTime = v;
                int frames = 0;
                if (!below)
                {
                    long long n = static_cast<long long>(v / m_FrameSkip.m_fTimePerFrame);
                    if (n)
                        m_FrameSkip.m_fAccumulatedTime = v - static_cast<float>(static_cast<int>(n)) * m_FrameSkip.m_fTimePerFrame;
                    frames = static_cast<int>(n);
                }
                m_fX -= static_cast<float>(frames) * 0.5f;
                if (m_fX < m_fAnchor)
                {
                    float anchor = m_fAnchor;
                    m_state  = 1;
                    m_fX     = anchor;
                    m_fAnchor = anchor + 70.0f;
                }
                break;
            }
            case 2:
            {
                float v = dt + m_FrameSkip.m_fAccumulatedTime;
                bool below = v < m_FrameSkip.m_fTimePerFrame;
                m_FrameSkip.m_fAccumulatedTime = v;
                int frames = 0;
                if (!below)
                {
                    long long n = static_cast<long long>(v / m_FrameSkip.m_fTimePerFrame);
                    if (n)
                        m_FrameSkip.m_fAccumulatedTime = v - static_cast<float>(static_cast<int>(n)) * m_FrameSkip.m_fTimePerFrame;
                    frames = static_cast<int>(n);
                }
                m_fY += static_cast<float>(frames) * 0.5f;
                if (m_fY > m_fAnchor)
                {
                    m_fY = m_fAnchor;
                    m_fAnchor = m_fAnchor - 70.0f;
                    m_state   = 1;
                }
                break;
            }
            case 3:
            {
                float v = dt + m_FrameSkip.m_fAccumulatedTime;
                bool below = v < m_FrameSkip.m_fTimePerFrame;
                m_FrameSkip.m_fAccumulatedTime = v;
                int frames = 0;
                if (!below)
                {
                    long long n = static_cast<long long>(v / m_FrameSkip.m_fTimePerFrame);
                    if (n)
                        m_FrameSkip.m_fAccumulatedTime = v - static_cast<float>(static_cast<int>(n)) * m_FrameSkip.m_fTimePerFrame;
                    frames = static_cast<int>(n);
                }
                m_fX += static_cast<float>(frames) * 0.5f;
                if (m_fX > m_fAnchor)
                {
                    m_fX = m_fAnchor;
                    m_fAnchor = m_fAnchor - 70.0f;
                    m_state   = 1;
                }
                break;
            }
            default:
                break;
        }
    }

    // ----- 關閉階段 -----
    if (m_state == 2)
    {
        switch (m_buttonIdx)
        {
            case 0:
            {
                float v = dt + m_FrameSkip.m_fAccumulatedTime;
                bool below = v < m_FrameSkip.m_fTimePerFrame;
                m_FrameSkip.m_fAccumulatedTime = v;
                int frames = 0;
                if (!below)
                {
                    long long n = static_cast<long long>(v / m_FrameSkip.m_fTimePerFrame);
                    if (n)
                        m_FrameSkip.m_fAccumulatedTime = v - static_cast<float>(static_cast<int>(n)) * m_FrameSkip.m_fTimePerFrame;
                    frames = static_cast<int>(n);
                }
                m_fY += static_cast<float>(frames) * 0.5f;
                if (m_fY > m_fAnchor)
                {
                    m_fY    = m_fAnchor;
                    m_state = 3;
                }
                break;
            }
            case 1:
            {
                float v = dt + m_FrameSkip.m_fAccumulatedTime;
                bool below = v < m_FrameSkip.m_fTimePerFrame;
                m_FrameSkip.m_fAccumulatedTime = v;
                int frames = 0;
                if (!below)
                {
                    long long n = static_cast<long long>(v / m_FrameSkip.m_fTimePerFrame);
                    if (n)
                        m_FrameSkip.m_fAccumulatedTime = v - static_cast<float>(static_cast<int>(n)) * m_FrameSkip.m_fTimePerFrame;
                    frames = static_cast<int>(n);
                }
                m_fX += static_cast<float>(frames) * 0.5f;
                if (m_fX > m_fAnchor)
                {
                    m_fX    = m_fAnchor;
                    m_state = 3;
                }
                break;
            }
            case 2:
            {
                float v = dt + m_FrameSkip.m_fAccumulatedTime;
                bool below = v < m_FrameSkip.m_fTimePerFrame;
                m_FrameSkip.m_fAccumulatedTime = v;
                int frames = 0;
                if (!below)
                {
                    long long n = static_cast<long long>(v / m_FrameSkip.m_fTimePerFrame);
                    if (n)
                        m_FrameSkip.m_fAccumulatedTime = v - static_cast<float>(static_cast<int>(n)) * m_FrameSkip.m_fTimePerFrame;
                    frames = static_cast<int>(n);
                }
                m_fY -= static_cast<float>(frames) * 0.5f;
                if (m_fY < m_fAnchor)
                {
                    m_fY    = m_fAnchor;
                    m_state = 3;
                }
                break;
            }
            case 3:
            {
                float v = dt + m_FrameSkip.m_fAccumulatedTime;
                bool below = v < m_FrameSkip.m_fTimePerFrame;
                m_FrameSkip.m_fAccumulatedTime = v;
                int frames = 0;
                if (!below)
                {
                    long long n = static_cast<long long>(v / m_FrameSkip.m_fTimePerFrame);
                    if (n)
                        m_FrameSkip.m_fAccumulatedTime = v - static_cast<float>(static_cast<int>(n)) * m_FrameSkip.m_fTimePerFrame;
                    frames = static_cast<int>(n);
                }
                m_fX -= static_cast<float>(frames) * 0.5f;
                if (m_fX < m_fAnchor)
                {
                    m_fX    = m_fAnchor;
                    m_state = 3;
                }
                break;
            }
            default:
                break;
        }
    }

    // mofclient.c：直接寫 pImg 欄位並呼叫 Process（無 null guard）。
    m_pImage->m_fPosX = m_fX;
    m_pImage->m_bFlag_447 = true;
    m_pImage->m_fPosY = m_fY;
    m_pImage->m_bFlag_447 = true;
    m_pImage->Process();
    return m_state;
}

void MedicalKit_Button::Render()
{
    GameImage* pImg = m_pImage;
    if (pImg)
    {
        if (pImg->m_pGIData)
            pImg->Draw();
    }
}
