#include "Effect/CEffect_Field_AttackMiss.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x005341C0
CEffect_Field_AttackMiss::CEffect_Field_AttackMiss()
{
    // CEffectBase �غc�禡�w�۰ʩI�s
    m_pMissImage = nullptr;
    m_ucState = 0;
    m_fAlpha = 240.0f;      // 1132396544 (0x43700000)
    m_fColorValue = 100.0f; // 1120403456 (0x42C80000)
    m_fInitialPosX = 0.0f;
    m_fCurrentPosY = 0.0f;

    // ��l�X: *((_DWORD *)this + 42) = 1015580809; (0x3C888889 -> ~1/60)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
}

// �����ϲ�Ķ�X: 0x00534250
CEffect_Field_AttackMiss::~CEffect_Field_AttackMiss()
{
}

// �����ϲ�Ķ�X: 0x00534270
void CEffect_Field_AttackMiss::SetEffect(float x, float y)
{
    // �]�w�S�Ī���l�@�ɮy��
    m_fInitialPosX = x;
    m_fCurrentPosY = y;
}

// �����ϲ�Ķ�X: 0x00534290
bool CEffect_Field_AttackMiss::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }

    if (m_ucState == 0) {
        // --- ���q 0: �H�J�P���d ---
        m_fAlpha -= static_cast<float>(frameCount * 2);
        if (m_fAlpha < 220.0f) {
            m_fAlpha = 220.0f;
            m_ucState = 1; // ������U�@���q
        }
    }
    else if (m_ucState == 1) {
        // --- ���q 1: �H�X�P�W�} ---
        m_fAlpha -= static_cast<float>(frameCount * 5);
        m_fCurrentPosY -= static_cast<float>(frameCount); // Y �y�д�֡A���ͤW�}�ĪG
    }

    // ��z���׭��� 0 �H�U�ɡA�S�ĥͩR�g������
    return m_fAlpha < 0.0f;
}

// �����ϲ�Ķ�X: 0x00534380
void CEffect_Field_AttackMiss::Process()
{
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // �q�������� "MISS" �Ϥ�
        // �귽 ID: 0xB0005A4u (�@�ɼƦr/��r����), �v�� ID: 62
        m_pMissImage = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);

        if (m_pMissImage) {
            float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

            m_pMissImage->SetPosition(screenX, screenY);
            m_pMissImage->SetBlockID(62); // "MISS" �r�˦b���ɤ�������

            // �N�p��X�� Alpha �M Color ���Ψ� GameImage
            float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
            m_pMissImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));
            m_pMissImage->SetColor(static_cast<unsigned int>(m_fColorValue));

            // ��s���I�w�İ�
            m_pMissImage->Process();
        }
    }
}

// �����ϲ�Ķ�X: 0x00534460
void CEffect_Field_AttackMiss::Draw()
{
    if (m_bIsVisible && m_pMissImage && m_pMissImage->IsInUse())
    {
        // �]�w�зǪ� Alpha �V�X�Ҧ�
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        m_pMissImage->Draw();
    }
}