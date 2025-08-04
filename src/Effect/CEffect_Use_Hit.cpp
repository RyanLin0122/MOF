#include "Effect/CEffect_Use_Hit.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>

// �����ϲ�Ķ�X: 0x0052F870
CEffect_Use_Hit::CEffect_Use_Hit()
{
    m_pImage = nullptr;
    m_fAlpha = 250.0f;        // 1133903872 (0x437A0000)
    m_fScale = 80.0f;         // 1117782016 (0x42A00000)
    m_fRotation = 0.0f;
    m_cCurrentFrame = 0;
    m_cDirectionType = 0;
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fScaleX_Rate = 1.0f;    // 1065353216 (0x3F800000)
    m_fAlpha_Rate = 1.0f;     // 1065353216 (0x3F800000)
    m_dwResourceID = 0;
    m_fFrameCounter = 0.0f;

    // ��l�X: *((_DWORD *)this + 43) = 993738471; (0x3B4CCCC7 -> ~0.03f)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 33.0f;
}

// �����ϲ�Ķ�X: 0x0052F920
CEffect_Use_Hit::~CEffect_Use_Hit()
{
}

// �����ϲ�Ķ�X: 0x0052F940
void CEffect_Use_Hit::SetEffect(float x, float y, unsigned char directionType)
{
    m_fFrameCounter = 0.0f;
    m_cDirectionType = directionType;
    m_fCurrentPosY = y - 55.0f;
    m_dwResourceID = 184550916; // 0xB0005A4

    switch (directionType)
    {
    case 1:
        m_fCurrentPosX = x - 15.0f;
        break;
    case 0:
        m_fCurrentPosX = x - 30.0f;
        break;
    case 2:
        m_fCurrentPosX = x - 20.0f;
        break;
    default:
        // �b��l�X���A�p�G directionType ���O 0, 1, 2�A
        // m_fCurrentPosX �|�ϥΤ@�ӥ���l�ƪ��ȡA�M��A�� 8�C
        // �o�i��O�@�� bug�A�Ϊ̨̿��S�w���I�s���ǡC
        // ���F�w���A�ڭ̵��@�ӹw�]�ȡC
        m_fCurrentPosX = x - 8.0f;
        break;
    }
}

// �����ϲ�Ķ�X: 0x0052F9D0
bool CEffect_Use_Hit::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    // ��s���� (�b��l�X���A�������Q�Χ@�Y��)
    m_fScale += fFrameCount * 0.5f;

    // ��s�z����
    m_fAlpha -= fFrameCount * m_fAlpha_Rate * 1.4f;

    // ��s�B�I�Ƽv��p�ƾ�
    m_fFrameCounter += fFrameCount * m_fScaleX_Rate * 0.04f;

    // �p�G�p�ƾ��W�L3�A��ܰʵe�ǦC�����A�B�p�G�z���פw�ܧC�h�P��
    if (m_fFrameCounter >= 3.0f) {
        m_fFrameCounter = 3.0f;
        if (m_fAlpha < 45.0f) {
            return true;
        }
    }

    // ��s�n��ܪ���Ƽv��
    m_cCurrentFrame = static_cast<char>(m_fFrameCounter);

    return m_fAlpha < 0.0f;
}

// �����ϲ�Ķ�X: 0x0052FAF0
void CEffect_Use_Hit::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_pImage = cltImageManager::GetInstance()->GetGameImage(7, m_dwResourceID, 0, 1);

        if (m_pImage) {
            float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

            m_pImage->SetPosition(screenX, screenY);
            m_pImage->SetBlockID(m_cCurrentFrame);

            float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
            m_pImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));
            // ��l�X�N m_fAlpha �]�]�w���F Color
            m_pImage->SetColor(static_cast<unsigned int>(clampedAlpha));
            m_pImage->SetScale(static_cast<int>(m_fScale));

            m_pImage->Process();
        }
    }
}

// �����ϲ�Ķ�X: 0x0052FC40
void CEffect_Use_Hit::Draw()
{
    if (m_bIsVisible && m_pImage && m_pImage->IsInUse())
    {
        // �]�w�[�G�V�X�Ҧ�
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // 0xA -> 5
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); // 2

        m_pImage->Draw();

        // ��_�w�]�V�X�Ҧ�
        CDeviceManager::GetInstance()->ResetRenderState();
    }
}