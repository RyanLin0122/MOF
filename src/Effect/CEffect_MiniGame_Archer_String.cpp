#include "Effect/CEffect_MiniGame_Archer_String.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x005384D0
CEffect_MiniGame_Archer_String::CEffect_MiniGame_Archer_String()
{
    m_pImage = nullptr;
    m_cFrameIndex = 0;
    m_ucState = 0;
    m_fAlpha = 240.0f;        // 1132396544 (0x43700000)
    m_fScale = 80.0f;         // 1117782016 (0x42A00000)
    m_fInitialPosX = 0.0f;
    m_fCurrentPosY = 0.0f;

    // 1015580809 -> 1.0f / 60.0f
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
}

// �����ϲ�Ķ�X: 0x00538560
CEffect_MiniGame_Archer_String::~CEffect_MiniGame_Archer_String()
{
}

// �����ϲ�Ķ�X: 0x00538580
void CEffect_MiniGame_Archer_String::SetEffect(char cFrameIndex, float x, float y)
{
    m_cFrameIndex = cFrameIndex;
    m_fInitialPosX = x;
    m_fCurrentPosY = y;
}

// �����ϲ�Ķ�X: 0x005385B0
bool CEffect_MiniGame_Archer_String::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    if (m_ucState == 0) { // ���q 0: ��j
        m_fScale += fFrameCount * 5.0f;
        if (m_fScale >= 110.0f) {
            m_fScale = 110.0f;
            m_ucState = 1;
        }
    }
    else if (m_ucState == 1) { // ���q 1: �Y�p
        m_fScale -= fFrameCount;
        if (m_fScale <= 100.0f) {
            m_fScale = 100.0f;
            m_ucState = 2;
        }
    }
    else if (m_ucState == 2) { // ���q 2: �H�X�W�}
        m_fAlpha -= fFrameCount * 5.0f;
        m_fCurrentPosY -= fFrameCount;
    }

    return m_fAlpha < 0.0f;
}

// �����ϲ�Ķ�X: 0x005386E0
void CEffect_MiniGame_Archer_String::Process()
{
    // ���S�Ĥ��Ҽ{��v���y�СA�����b UI �hø�s

    // ��� GameImage�A�귽 ID: 0x1000009Cu
    m_pImage = cltImageManager::GetInstance()->GetGameImage(7, 0x1000009Cu, 0, 1);

    if (m_pImage) {
        // �ھڤ�r�Ϥ����e�׶i��~�����
        RECT rect;
        m_pImage->SetBlockID(m_cFrameIndex);
        m_pImage->GetBlockRect(&rect);
        float centeredX = m_fInitialPosX - static_cast<float>(rect.right - rect.left) * 0.5f;

        m_pImage->SetPosition(centeredX, m_fCurrentPosY);

        float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
        m_pImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));
        m_pImage->SetScale(static_cast<int>(m_fScale));

        m_pImage->Process();
    }
}

// �����ϲ�Ķ�X: 0x005387C0
void CEffect_MiniGame_Archer_String::Draw()
{
    if (m_pImage && m_pImage->IsInUse())
    {
        CDeviceManager::GetInstance()->ResetRenderState();
        m_pImage->Draw();
    }
}