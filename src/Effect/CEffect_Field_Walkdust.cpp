#include "Effect/CEffect_Field_Walkdust.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib> // for rand()

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x005371D0
CEffect_Field_Walkdust::CEffect_Field_Walkdust() : CEffectBase() // ���T�I�s�����O�غc�禡
{
    m_pEffectImage = nullptr;
    m_fAlpha = 250.0f;       // 1137180672
    m_fScale = 50.0f;        // 1092616192
    m_fRotation = 0.0f;
    m_cInitialFrame = rand() % 4; // �H����� 0-3 ����l�v��
    m_bIsFlip = false;
    m_dwResourceID = 0;
    m_fScaleX_Rate = 1.0f;   // 1065353216
    m_fAlpha_Rate = 1.0f;    // 1065353216
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_bMoveRight = false;
    m_bIsVisible = FALSE;

    // ��l�X: *((_DWORD *)this + 43) = 993738471; (0x3B4CCCC7 -> ~0.03f)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 33.0f;
}

CEffect_Field_Walkdust::~CEffect_Field_Walkdust()
{
    // GameImage �� cltImageManager �޲z�A���B���� delete
}

void CEffect_Field_Walkdust::SetEffect(float x, float y, bool bFlip, unsigned int resourceID, int a6, char frameID, bool bMoveRight)
{
    // --- ����ץ� ---
    // �N�良�w�q���� m_bFlipX ����ȡA
    // �אּ��q CEffectBase �~�ӨӪ����� m_bIsFlip ��ȡC
    m_bIsFlip = bFlip;

    m_fCurrentPosY = y - 13.0f;

    // ... �禡����l�������� ...
    if (a6 != 0) {
        m_fScaleX_Rate = 0.0f;
        m_cInitialFrame = frameID;
        m_fAlpha_Rate = 0.5f;
    }

    if (bFlip) {
        m_fCurrentPosX = x + 7.0f;
    }
    else {
        m_fCurrentPosX = x - 35.0f;
    }

    m_dwResourceID = resourceID;
    if (resourceID == 0) {
        m_dwResourceID = 184550807; //0xB000597
    }

    m_bMoveRight = bMoveRight;
}

// �����ϲ�Ķ�X: 0x00537330
bool CEffect_Field_Walkdust::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false; // �p�G�S���v����i�A�h���������
    }

    // ��s����
    m_fRotation += static_cast<float>(frameCount) * 0.5f;

    // ��s�Y��
    m_fScale += static_cast<float>(frameCount) * m_fScaleX_Rate;

    // ��s�z����
    m_fAlpha -= static_cast<float>(frameCount) * m_fAlpha_Rate;

    // ��s Y �y�� (�W��)
    m_fCurrentPosY -= static_cast<float>(frameCount) * 0.1f;

    // �ˬd�ͩR�g���O�_����
    if (m_fAlpha < 0.0f) {
        return true;
    }

    // �p�G�]�w�F�����}��
    if (m_bMoveRight) {
        m_fCurrentPosX += 0.25f;
    }

    return false;
}

// �����ϲ�Ķ�X: 0x00537430
void CEffect_Field_Walkdust::Process()
{
    float screenX = m_fCurrentPosX;
    if (!m_bMoveRight) {
        screenX -= static_cast<float>(g_Game_System_Info.ScreenWidth);
    }

    // --- ����ץ� ---
    // �]���{�b�~�Ӧ� CEffectBase�A�i�H�����I�s IsCliping �����禡�C
    m_bIsVisible = IsCliping(screenX, 0.0f);
    if (!m_bIsVisible) return;

    // ��� GameImage
    m_pEffectImage = cltImageManager::GetInstance()->GetGameImage(7, m_dwResourceID, 0, 1);

    if (m_pEffectImage) {
        float screenY = m_bMoveRight ? m_fCurrentPosY : m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

        m_pEffectImage->SetPosition(screenX, screenY);
        m_pEffectImage->SetBlockID(m_cInitialFrame);

        float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
        m_pEffectImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));

        m_pEffectImage->SetColor(static_cast<unsigned int>(m_fScale));
        m_pEffectImage->SetRotation(static_cast<int>(m_fRotation));

        m_pEffectImage->Process();
    }
}

// �����ϲ�Ķ�X: 0x005375B0
void CEffect_Field_Walkdust::Draw()
{
    if (m_bIsVisible && m_pEffectImage && m_pEffectImage->IsInUse())
    {
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        m_pEffectImage->Draw();
    }
}