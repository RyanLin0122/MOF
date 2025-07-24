#include "Effect/CEffect_Field_ExpNumber.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>
#include <cmath>

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x00535210
CEffect_Field_ExpNumber::CEffect_Field_ExpNumber()
{
    m_pNumberImages = nullptr;
    m_cDigitCount = 0;
    m_ucState = 0;
    m_fAlpha = 255.0f;        // 1132396544 (0x437F0000)
    m_fScale = 80.0f;         // 1117782016 (0x42A00000), ���M���ϥΡA������l��
    memset(m_ucDigitFrames, 0, sizeof(m_ucDigitFrames));
    m_fInitialPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fTotalWidth = 0.0f;

    // 1015580809 -> 1.0f / 60.0f
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
}

// �����ϲ�Ķ�X: 0x005352B0
CEffect_Field_ExpNumber::~CEffect_Field_ExpNumber()
{
    delete[] m_pNumberImages;
}

// �����ϲ�Ķ�X: 0x005352F0
void CEffect_Field_ExpNumber::SetEffect(int expValue, float x, float y)
{
    // �g��ȼƦr�ϥι��ɤ��q���� 12 �}�l�����Ʀr
    const char first_digit_frame = 12;
    const char plus_frame = 22;
    const char minus_frame = 23;

    int absValue = abs(expValue);

    if (absValue == 0) {
        m_ucDigitFrames[m_cDigitCount++] = first_digit_frame;
    }
    else {
        int tempValue = absValue;
        while (tempValue > 0) {
            m_ucDigitFrames[m_cDigitCount++] = (tempValue % 10) + first_digit_frame;
            tempValue /= 10;
        }
    }
    m_ucDigitFrames[m_cDigitCount++] = (expValue >= 0) ? plus_frame : minus_frame;

    m_pNumberImages = new (std::nothrow) GameImage * [m_cDigitCount];

    m_fInitialPosX = x;
    m_fCurrentPosY = y;

    // �w���p���`�e�ץH�i��~��
    GameImage* pTempImage = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
    if (pTempImage) {
        for (int i = 0; i < m_cDigitCount; ++i) {
            RECT rect;
            pTempImage->SetBlockID(m_ucDigitFrames[i]);
            pTempImage->GetBlockRect(&rect);
            m_fTotalWidth += static_cast<float>(rect.right - rect.left);
        }
        cltImageManager::GetInstance()->ReleaseGameImage(pTempImage);
    }
    m_fTotalWidth *= 0.5f; // ���b�e
}

// �����ϲ�Ķ�X: 0x00535430
bool CEffect_Field_ExpNumber::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    if (m_ucState == 0) { // ���q 0: �H�J�P���d
        m_fAlpha -= fFrameCount * 2.0f;
        if (m_fAlpha < 220.0f) {
            m_fAlpha = 220.0f;
            m_ucState = 1;
        }
        m_fCurrentPosY -= fFrameCount * 0.5f;
    }
    else if (m_ucState == 1) { // ���q 1: �H�X�P�W�}
        m_fAlpha -= fFrameCount * 5.0f;
        m_fCurrentPosY -= fFrameCount * 1.0f;
    }

    return m_fAlpha < 0.0f;
}

// �����ϲ�Ķ�X: 0x00535530
void CEffect_Field_ExpNumber::Process()
{
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);
        float currentX = screenX - m_fTotalWidth;

        // �q�᩹�e�B�z�Ʀr (�q�Ӧ�ƶ}�l)
        for (int i = 0; i < m_cDigitCount; ++i) {
            m_pNumberImages[i] = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
            if (m_pNumberImages[i]) {
                unsigned char frameID = m_ucDigitFrames[i];
                m_pNumberImages[i]->SetBlockID(frameID);
                m_pNumberImages[i]->SetPosition(currentX, screenY);

                float clampedAlpha = (m_fAlpha > 254.0f) ? 254.0f : m_fAlpha;
                m_pNumberImages[i]->SetAlpha(static_cast<unsigned int>(clampedAlpha));

                // �g��ȼƦr�S���Y��ʵe�A�C��T�w

                RECT rect;
                m_pNumberImages[i]->GetBlockRect(&rect);
                currentX += static_cast<float>(rect.right - rect.left - 6); // ��l�X�� -6 �����Z�վ�
            }
        }
    }
}

// �����ϲ�Ķ�X: 0x00535680
void CEffect_Field_ExpNumber::Draw()
{
    if (!m_bIsVisible || !m_pNumberImages) return;

    CDeviceManager::GetInstance()->ResetRenderState();

    // �q�᩹�eø�s (���e���b�U�h�A�Y�Ӧ��)
    for (int i = 0; i < m_cDigitCount; ++i) {
        if (m_pNumberImages[i] && m_pNumberImages[i]->IsInUse()) {
            m_pNumberImages[i]->Draw();
        }
    }
}