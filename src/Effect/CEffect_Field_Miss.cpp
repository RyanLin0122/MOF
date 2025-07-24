#include "Effect/CEffect_Field_Miss.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>
#include <cmath>

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x00537A40
CEffect_Field_Miss::CEffect_Field_Miss()
{
    m_pMissImage = nullptr;
    m_fAlpha = 250.0f;        // 1137180672 (0x437A0000)
    m_fScale = 2.0f;          // 0x40000000 -> 2.0f, �i��O�@���Y����C���
    m_fRotation = 0.0f;
    m_cInitialFrame = rand() % 4; // ���M���ϥΡA����l�X�����欰
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fVelX = 0.0f;
    m_fVelY = 0.0f;
    m_fSpeedFactor = 0.0f;
    m_dwResourceID = 0;

    // ��l�X: *((_DWORD *)this + 43) = 993738471; (0x3B4CCCC7 -> ~0.03f)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 33.0f;
}

CEffect_Field_Miss::~CEffect_Field_Miss()
{
}

// �����ϲ�Ķ�X: 0x00537B10
void CEffect_Field_Miss::SetEffect(float x, float y)
{
    m_dwResourceID = 184550807; // 0xB000597
    m_fCurrentPosX = x - 35.0f;
    m_fCurrentPosY = y - 13.0f;

    // �H���ƪ�l���樤�שM�t��
    float angle = static_cast<float>(rand() % 628) * 0.01f;
    m_fVelX = -sinf(angle);
    m_fVelY = cosf(angle);
    m_fSpeedFactor = (static_cast<float>(rand() % 5 + 3) + 1.0f) * 0.01f;
}

// �����ϲ�Ķ�X: 0x00537BA0
bool CEffect_Field_Miss::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    // ��s���� (��l�X�� m_fRotation ��ڤW�O m_fScale)
    m_fScale += fFrameCount * 1.8f * 0.33333334f;

    // ��s�z����
    m_fAlpha -= fFrameCount * 1.5f;

    // ��s�C��� (��l�X�� m_fScale ��ڤW�O m_fColorValue)
    // m_fColorValue += fFrameCount * 1.5f;

    // ��s��m�A���ͺ}���ĪG
    m_fCurrentPosX += m_fVelX * m_fSpeedFactor * fFrameCount;
    m_fCurrentPosY += m_fVelY * m_fSpeedFactor * fFrameCount;

    return m_fAlpha < 0.0f;
}

// �����ϲ�Ķ�X: 0x00537CB0
void CEffect_Field_Miss::Process()
{
    float screenX = m_fCurrentPosX;
    // �ھڭ�l�X�A���S�Ħ��G���Ҽ{��v���y�СA�����ϥΥ@�ɮy�Ч@���ù��y��
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_pMissImage = cltImageManager::GetInstance()->GetGameImage(7, m_dwResourceID, 0, 1);

        if (m_pMissImage) {
            float screenY = m_fCurrentPosY;

            m_pMissImage->SetPosition(screenX, screenY);
            // ��l�X���N�H���� m_cInitialFrame �]���F BlockID�A����ڮĪG���� "MISS"
            m_pMissImage->SetBlockID(62);

            float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
            m_pMissImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));
            m_pMissImage->SetColor(static_cast<unsigned int>(m_fScale)); // ��l�X�N m_fScale �]�� Color
            m_pMissImage->SetRotation(static_cast<int>(m_fRotation)); // ��l�X�N m_fRotation �]�� Rotation

            m_pMissImage->Process();
        }
    }
}

// �����ϲ�Ķ�X: 0x00537DF0
void CEffect_Field_Miss::Draw()
{
    if (m_bIsVisible && m_pMissImage && m_pMissImage->IsInUse())
    {
        CDeviceManager::GetInstance()->ResetRenderState();
        m_pMissImage->Draw();
    }
}