#include "Effect/CEffect_Field_DamageNumber.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>
#include <cmath>

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x00534CA0
CEffect_Field_DamageNumber::CEffect_Field_DamageNumber()
{
    m_pNumberImages = nullptr;
    m_cDigitCount = 0;
    m_ucState = 0;
    m_fAlpha = 240.0f;        // 1132396544 (0x43700000)
    m_fScale = 80.0f;         // 1117782016 (0x42A00000)
    memset(m_ucDigitFrames, 0, sizeof(m_ucDigitFrames));
    m_fInitialPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fTotalWidth = 0.0f;

    // 1015580809 -> 1.0f / 60.0f
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
}

// �����ϲ�Ķ�X: 0x00534D40
CEffect_Field_DamageNumber::~CEffect_Field_DamageNumber()
{
    delete[] m_pNumberImages;
}

// �����ϲ�Ķ�X: 0x00534D80
void CEffect_Field_DamageNumber::SetEffect(int damageValue, float x, float y, int type)
{
    char first_digit_frame, plus_frame, minus_frame;

    if (type == 0) { // �Ǫ��缾�a (����Ʀr)
        first_digit_frame = 48;
        minus_frame = 59;
        plus_frame = 58;
    }
    else { // ���a��Ǫ� (�զ�Ʀr)
        first_digit_frame = 0;
        minus_frame = 11;
        plus_frame = 10;
    }

    int absValue = abs(damageValue);

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
    m_ucDigitFrames[m_cDigitCount++] = (damageValue >= 0) ? plus_frame : minus_frame;

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
            m_fTotalWidth += static_cast<float>(rect.right - rect.left - 5); // ��l�X�� -5 �����Z�վ�
        }
        cltImageManager::GetInstance()->ReleaseGameImage(pTempImage);
    }
    m_fTotalWidth *= 0.5f; // ���b�e
}

// �����ϲ�Ķ�X: 0x00534F00
bool CEffect_Field_DamageNumber::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    switch (m_ucState)
    {
    case 0: // ���q 0: ��j
        m_fScale += fFrameCount * 45.0f;
        if (m_fScale >= 210.0f) {
            m_fScale = 210.0f;
            m_ucState = 1;
        }
        m_fCurrentPosY -= fFrameCount * 2.0f;
        break;

    case 1: // ���q 1: �Y�p
        m_fScale -= fFrameCount * 10.0f;
        if (m_fScale <= 80.0f) {
            m_fScale = 80.0f;
            m_ucState = 2;
        }
        break;

    case 2: // ���q 2: �H�X�W�}
        m_fAlpha -= fFrameCount * 5.0f;
        m_fCurrentPosY -= fFrameCount * 1.5f;
        break;
    }

    return m_fAlpha < 0.0f;
}

// �����ϲ�Ķ�X: 0x00535040
void CEffect_Field_DamageNumber::Process()
{
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);
        float currentX = screenX - m_fTotalWidth;

        // �q�᩹�e�B�z�Ʀr (�q�Ӧ�ƶ}�l)
        for (int i = 0; i < m_cDigitCount; ++i) {
            m_pNumberImages[i] = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
            if (m_pNumberImages[i]) {
                unsigned char frameID = m_ucDigitFrames[i];
                m_pNumberImages[i]->SetBlockID(frameID);
                m_pNumberImages[i]->SetPosition(currentX, screenY);
                m_pNumberImages[i]->SetAlpha(static_cast<unsigned int>(m_fAlpha));
                m_pNumberImages[i]->SetScale(static_cast<int>(m_fScale));

                RECT rect;
                m_pNumberImages[i]->GetBlockRect(&rect);
                // �ھڨC�ӼƦr���e�רӽվ�U�@�ӼƦr����m
                currentX += static_cast<float>(rect.right - rect.left - 5) * (m_fScale / 100.f);
            }
        }
    }
}

// �����ϲ�Ķ�X: 0x005351A0
void CEffect_Field_DamageNumber::Draw()
{
    if (!m_bIsVisible || !m_pNumberImages) return;

    CDeviceManager::GetInstance()->ResetRenderState();

    // �q�᩹�eø�s (���e���b�U�h)
    for (int i = m_cDigitCount - 1; i >= 0; --i) {
        if (m_pNumberImages[i] && m_pNumberImages[i]->IsInUse()) {
            m_pNumberImages[i]->Draw();
        }
    }
}