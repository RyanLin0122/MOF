#include "Effect/CEffect_Battle_StaffShoot.h"
#include "Effect/CEAManager.h"
#include "global.h"
#include <cmath>

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x0052F420
CEffect_Battle_StaffShoot::CEffect_Battle_StaffShoot()
    : m_fTraveledDistance(0.0f), m_fTotalDistance(0.0f)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 11, �ɮצW��: "Effect/efn_staffshoot.ea"
    CEAManager::GetInstance()->GetEAData(11, "Effect/efn_staffshoot.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // �ʵe�u����@��
}

// �����ϲ�Ķ�X: 0x0052F4C0
CEffect_Battle_StaffShoot::~CEffect_Battle_StaffShoot()
{
}

// �����ϲ�Ķ�X: 0x0052F4D0
void CEffect_Battle_StaffShoot::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, bool bFlip)
{
    if (!pStartPos || !pEndPos) return;

    // �B�J 1: �]�w��l��m
    m_fCurrentPosX = pStartPos->x;
    m_fCurrentPosY = pStartPos->y;

    // �B�J 2: �p�⭸��V�q
    D3DXVECTOR2 vecDirection = *pEndPos - *pStartPos;

    // �B�J 3: �p���`�Z���M�t��
    m_fTotalDistance = D3DXVec2Length(&vecDirection);
    m_fSpeed = m_fTotalDistance * 0.02f; // �t�׬O�`�Z���� 2%

    // �B�J 4: �N�V�q���ƥH��o�¤�V
    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x;
    m_fDirectionY = vecDirection.y;

    // �B�J 5: �]�w½��
    m_bIsFlip = bFlip;
}

// �����ϲ�Ķ�X: 0x0052F570
bool CEffect_Battle_StaffShoot::FrameProcess(float fElapsedTime)
{
    // ��s��ı�ʵe���v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // �ˬd�O�_�w��F�ζW�L�ؼжZ��
    // ��l�X: return *((float *)this + 33) > (double)*((float *)this + 34);
    return m_fTraveledDistance > m_fTotalDistance;
}

// �����ϲ�Ķ�X: 0x0052F5A0
void CEffect_Battle_StaffShoot::Process()
{
    // �B�J 1: ��s��m
    // ��l�X: v5 = *((float *)this + 4) + *((float *)this + 2);
    //         v2 = *((float *)this + 5) + *((float *)this + 3);
    // �`�N�G��l�X���N�t�׭��J�F��V�V�q�A�ڭ̪������N�b Process �����γt��
    m_fCurrentPosX += m_fDirectionX * m_fSpeed;
    m_fCurrentPosY += m_fDirectionY * m_fSpeed;

    // �B�J 2: �֭p�w����Z��
    // ��l�X: *((float *)this + 33) = *((float *)this + 6) + *((float *)this + 33);
    m_fTraveledDistance += m_fSpeed;

    // �B�J 3: �ഫ���ù��y�Шöi����ŧP�_
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x0052F620
void CEffect_Battle_StaffShoot::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}