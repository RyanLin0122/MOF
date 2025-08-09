#include "Effect/CEffect_Battle_MagicBook.h"
#include "Effect/CEAManager.h"
#include "Image/CDeviceManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"
#include <cmath>

// �����ϲ�Ķ�X: 0x0052EBA0
CEffect_Battle_MagicBook::CEffect_Battle_MagicBook()
    : m_pTargetCharacter(nullptr),
    m_fTotalDistance(0.0f),
    m_fTraveledDistance(0.0f),
    m_fAngle(0.0f),
    m_nHitInfoID(0)
{
    // CEffectBase �غc�禡�w�۰ʩI�s

    // ���J�S�Ī���ı�ʵe
    CEAManager::GetInstance()->GetEAData(6, "Effect/efn-firebids01.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // ��g���ʵe���`������

    // �]�w���ʭp�ɾ�����s�W�v (30 FPS)
    // ��l�X: *((_DWORD *)this + 38) = 995783694;
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
}

// �����ϲ�Ķ�X: 0x0052EC70
CEffect_Battle_MagicBook::~CEffect_Battle_MagicBook()
{
}

// �����ϲ�Ķ�X: 0x0052EC90
void CEffect_Battle_MagicBook::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, bool bFlip, int hitInfoID)
{
    if (!pCaster || !pTarget) return;

    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    m_fSpeed = 10.0f;

    // --- �B�J 1: ���`���J�S�ļƾ� ---
    CEAManager::GetInstance()->GetEAData(6, "MoFData/Effect/efn-firebids01.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true);

    // --- �B�J 2: ����мg�V�X�Ҧ� (����ץ�) ---
    // �L���ɮפ��w�q���O����A�ڭ̳��j���ϥΥ[�k�V�X
    m_ccaEffect.m_ucSrcBlend = D3DBLEND_SRCALPHA; // �Ȭ� 5
    m_ccaEffect.m_ucDestBlend = D3DBLEND_ONE;      // �Ȭ� 2

    // --- ���򪺪��z�p�⤣�� ---
    D3DXVECTOR2 startPos(static_cast<float>(pCaster->GetPosX()), static_cast<float>(pCaster->GetPosY() - 15));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY() - (pTarget->GetCharHeight() >> 2)));

    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;

    D3DXVECTOR2 vecDirection = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vecDirection);

    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x;
    m_fDirectionY = vecDirection.y;

    m_bIsFlip = (pCaster->GetPosX() > pTarget->GetPosX());

    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vecDirection);
    m_fAngle = acosf(dotProduct);
    if (vecDirection.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// �����ϲ�Ķ�X: 0x0052EF90
void CEffect_Battle_MagicBook::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID)
{
    if (!pStartPos || !pEndPos) return;

    m_nHitInfoID = hitInfoID;
    m_fSpeed = 10.0f; // �P�˴��ɳt��
    m_fCurrentPosX = pStartPos->x;
    m_fCurrentPosY = pStartPos->y;

    D3DXVECTOR2 vecDirection = *pEndPos - *pStartPos;
    m_fTotalDistance = D3DXVec2Length(&vecDirection);

    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x;
    m_fDirectionY = vecDirection.y;

    m_bIsFlip = (static_cast<int>(fFlip) != 0);

    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vecDirection);
    m_fAngle = acosf(dotProduct);
    if (vecDirection.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// �����ϲ�Ķ�X: 0x0052F1A0
bool CEffect_Battle_MagicBook::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount)) {
        // --- �P�� Bug �ץ� ---
        // ��ڲ��ʶZ���ݭn���W�t��
        float moveDistance = m_fSpeed * static_cast<float>(moveFrameCount);
        m_fTraveledDistance += moveDistance;

        if (m_fTraveledDistance >= m_fTotalDistance) {
            if (m_pTargetCharacter) {
                m_pTargetCharacter->SetHited(m_nHitInfoID, 16);
            }
            return true; // ��F�ؼСA��^ true �H�P���S��
        }
        else {
            // ��s��m
            m_fCurrentPosX += m_fDirectionX * moveDistance;
            m_fCurrentPosY += m_fDirectionY * moveDistance;
        }
    }
    return false;
}

// �����ϲ�Ķ�X: 0x0052F2A0
void CEffect_Battle_MagicBook::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetRotation(m_fAngle);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x0052F2F0
void CEffect_Battle_MagicBook::Draw()
{
    if (m_bIsVisible) {
        // --- ø�s�޿�²�� ---
        // �{�b���A�ݭn��ʳ]�w��V���A�A�]�� m_ccaEffect �����|�۰�
        // �ϥΧڭ̦b SetEffect ���ץ��L�����T�V�X�Ҧ��C
        m_ccaEffect.Draw();
    }
}