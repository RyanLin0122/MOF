#include "Effect/CEffectUseHitMulti.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "Effect/CEffectManager.h" // ���F���k�Ы�
#include "global.h"
#include <cstdlib> // for rand()
#include <cmath>   // for sin(), cos()

// �����ϲ�Ķ�X: 0x0052FCD0
CEffectUseHitMulti::CEffectUseHitMulti()
{
    // CEffectBase �غc�禡�w�I�s
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f; // 1015580809 -> 0.01666f

    // ��l�Ʋɤl�}�C
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        m_Particles[i].pImage = nullptr;
        m_Particles[i].fAlpha = 250.0f; // 1133903872
        m_Particles[i].fScale = 0.0f;
        m_Particles[i].cFrameIndex = 0;
        m_Particles[i].ucOwnerDirection = 0;
        m_Particles[i].bIsActive = false; // ��l���D����
        // ... ��L��l�� ...
        m_Particles[i].fVelX = 1.0f; // 1065353216
        m_Particles[i].fVelY = 1.0f; // 1065353216
    }

    m_FrameSkip.m_fTimePerFrame = 1.0f / 75.0f; // 994482188 -> 0.01333f
}

CEffectUseHitMulti::~CEffectUseHitMulti()
{
}

// �����ϲ�Ķ�X: 0x0052FD80
// �����ϲ�Ķ�X: 0x0052FD80
void CEffectUseHitMulti::SetEffect2(float x, float y, unsigned char ucOwnerDirection, int effectType)
{
    // �B�J 1: �ھ� effectType �M�w�ɤl�ƶq�B�귽ID�M�S��欰
    m_bUseAdditiveBlend = true;
    m_nEffectType = effectType;

    switch (effectType)
    {
    case 1:
        m_uParticleCount = 8;
        m_dwResourceID = 184550915; // 0xB00059B
        // �� 60% �����v���k�Ыؤ@�� type 4 ���l�S��
        if ((rand() % 10) > 3) {
            CEffectUseHitMulti* pSubEffect = new CEffectUseHitMulti();
            if (pSubEffect) {
                pSubEffect->SetEffect2(x, y - 10.0f, ucOwnerDirection, 4);
                CEffectManager::GetInstance()->BulletAdd(pSubEffect);
            }
        }
        break;
    case 2:
        m_uParticleCount = 10;
        m_dwResourceID = 184550914; // 0xB00059A
        break;
    case 3:
        m_uParticleCount = 3;
        m_dwResourceID = 184550807; // 0xB000597
        m_bUseAdditiveBlend = false; // �������ϥδ��q Alpha �V�X
        break;
    case 4:
        m_uParticleCount = (rand() % 5) + 3; // �H�� 3 �� 7 �Ӳɤl
        m_dwResourceID = 184550917; // 0xB00059D
        break;
    default:
        m_uParticleCount = 0;
        m_dwResourceID = 0;
        break;
    }

    // �B�J 2: �M���ê�l�ƨC�@�Ӳɤl
    for (unsigned int i = 0; i < m_uParticleCount; ++i)
    {
        Particle& p = m_Particles[i];

        // 2a. �]�w�q�Ϊ�l���A
        p.bIsActive = true;
        p.fPosY = y - 30.0f;
        p.ucOwnerDirection = ucOwnerDirection;
        p.fRotation_Unused = 0.0f;

        // �ھڨ���¦V�]�w��¦ X �y��
        if (ucOwnerDirection == 1)      p.fPosX = x - 10.0f;
        else if (ucOwnerDirection == 0) p.fPosX = x + 10.0f;
        else                            p.fPosX = x;
        p.fPosX -= 8.0f;

        float angle = 0.0f;

        // 2b. �ھ� effectType ��C�Ӳɤl�i���H����
        switch (effectType)
        {
        case 1:
            p.cFrameIndex = 0;
            if (i >= 2) { // �e��Ӳɤl���j�A���򪺸��p
                p.fAlpha = (float)(rand() % 200 + 200);
                angle = (ucOwnerDirection == 1)
                    ? ((float)(rand() % 157) * 0.005f + 2.5933f)
                    : ((float)(rand() % 157) * 0.005f + 4.7333f);
                p.fSpeed = ((float)(rand() % 6) + 1.0f) * 0.1f;
            }
            else {
                p.fAlpha = (float)(rand() % 100 + 300);
                angle = (ucOwnerDirection == 1)
                    ? ((float)(68 - rand() % 20) * 0.005f + 2.5933f)
                    : ((float)(68 - rand() % 20) * 0.005f + 4.7333f);
                p.fSpeed = (float)(rand() % 5) * 0.1f + 0.5f;
            }
            p.fPosX += (float)(rand() % 30 - 15);
            p.fPosY += (float)(rand() % 30 - 15);
            break;

        case 2:
            p.fAlpha = (float)(rand() % 100 + 150) * 0.6667f;
            p.cFrameIndex = 0;
            p.fPosY -= 15.0f;
            angle = (float)(rand() % 628) * 0.01f;
            p.fPosX += (float)(rand() % 10 - 5);
            p.fPosY += (float)(rand() % 10 - 5);
            p.fSpeed = (i >= 3)
                ? ((float)(rand() % 10) * 0.1f + 0.4f)
                : ((float)(rand() % 3) * 0.1f + 0.02f);
            break;

        case 3:
            p.fAlpha = (float)(rand() % 100 + 200) * 0.5556f;
            p.cFrameIndex = 0;
            angle = (float)(rand() % 628) * 0.01f;
            p.fSpeed = ((float)(rand() % 10) + 1.0f) * 0.1f;
            break;

        case 4:
            p.fAlpha = (float)(rand() % 5 + 25);
            p.cFrameIndex = 4;
            angle = (ucOwnerDirection)
                ? ((float)(rand() % 157) * 0.005f + 4.7333f)
                : ((float)(rand() % 157) * 0.005f + 2.5933f);

            if (ucOwnerDirection == 1) p.fPosX = x - 60.0f;
            else if (ucOwnerDirection == 0) p.fPosX = x - 40.0f;

            p.fPosY += 10.0f;
            p.fSpeed = (float)(rand() % 8 + 1) * 0.9f;
            p.fAngle = (angle + 0.8f) * 57.295776f; // �����ਤ��
            break;
        }

        // 2c. �ھڨ��׭p��̲ת��t�צV�q
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        float vecX = cosA - sinA;
        float vecY = cosA + sinA;
        float length = sqrtf(vecX * vecX + vecY * vecY);
        if (length > 0.001f) {
            p.fVelX = vecX / length;
            p.fVelY = vecY / length;
        }
        else {
            p.fVelX = 0.0f;
            p.fVelY = 0.0f;
        }
    }
}

// �����ϲ�Ķ�X: 0x00530350
bool CEffectUseHitMulti::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }

    bool allParticlesInactive = true;
    for (unsigned int i = 0; i < m_uParticleCount; ++i) {
        Particle& p = m_Particles[i];
        if (p.bIsActive) {
            // ��s�ɤl���z
            switch (m_nEffectType) {
            case 1: case 3:
                p.fAlpha -= (float)frameCount * 1.8f;
                p.fScale -= (float)frameCount * 0.5f;
                break;
            case 2:
                p.fAlpha -= (float)frameCount;
                p.fScale -= (float)frameCount * 0.5f;
                break;
            case 4:
                p.fScale -= (float)frameCount * 1.5f;
                break;
            }
            p.fPosX += (float)frameCount * p.fVelX * p.fSpeed;
            p.fPosY += (float)frameCount * p.fVelY * p.fSpeed;

            // �ˬd�ͩR�g��
            if ((m_nEffectType == 4 && p.fScale <= 0.0f) || p.fAlpha <= 0.0f) {
                p.bIsActive = false;
            }
            else {
                allParticlesInactive = false;
            }
        }
    }

    return allParticlesInactive;
}

// �����ϲ�Ķ�X: 0x00530530
void CEffectUseHitMulti::Process()
{
    for (unsigned int i = 0; i < m_uParticleCount; ++i) {
        Particle& p = m_Particles[i];
        if (p.bIsActive) {
            float screenX = p.fPosX - static_cast<float>(g_Game_System_Info.ScreenX);
            p.bIsVisible = IsCliping(screenX, 0.0f);

            if (p.bIsVisible) {
                p.pImage = cltImageManager::GetInstance()->GetGameImage(7, m_dwResourceID, 0, 1);
                if (p.pImage) {
                    float screenY = p.fPosY - static_cast<float>(g_Game_System_Info.ScreenY);
                    p.pImage->SetPosition(screenX, screenY);
                    p.pImage->SetBlockID(p.cFrameIndex);

                    float clampedAlpha = (p.fAlpha > 255.0f) ? 255.0f : p.fAlpha;
                    p.pImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));

                    if (m_nEffectType == 4) p.pImage->SetRotation(static_cast<int>(p.fScale));
                    else p.pImage->SetColor(static_cast<unsigned int>(p.fScale));

                    p.pImage->Process();
                }
            }
        }
    }
}

// �����ϲ�Ķ�X: 0x005306A0
void CEffectUseHitMulti::Draw()
{
    // �ھگS�������]�w�V�X�Ҧ�
    if (m_bUseAdditiveBlend) {
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // 0xA -> 5
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); // 2
    }
    else {
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // 5
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); // 6
    }

    // ø�s�Ҧ����ʥB�i�����ɤl
    for (unsigned int i = 0; i < m_uParticleCount; ++i) {
        Particle& p = m_Particles[i];
        if (p.bIsActive && p.bIsVisible && p.pImage && p.pImage->IsInUse()) {
            p.pImage->Draw();
        }
    }

    // ��_�w�]�V�X�Ҧ�
    CDeviceManager::GetInstance()->ResetRenderState();
}