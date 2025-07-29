#include "Effect/CEffect_Battle_BowShoot.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x0052D770
CEffect_Battle_BowShoot::CEffect_Battle_BowShoot()
    : m_pTargetCharacter(nullptr),
    m_fTotalDistance(0.0f),
    m_fTraveledDistance(0.0f),
    m_fAngle(0.0f),
    m_nHitInfoID(0)
{
    // CEffectBase �غc�禡�w�۰ʩI�s

    // ���J�b�ڪ���ı�ʵe
    CEAManager::GetInstance()->GetEAData(3, "Effect/efn_bowshoot.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // �b�ڭ���ʵe���`������

    // �]�w���ʭp�ɾ�����s�W�v (30 FPS)
    // ��l�X: *((_DWORD *)this + 38) = 995783694;
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
}

// �����ϲ�Ķ�X: 0x0052D850
CEffect_Battle_BowShoot::~CEffect_Battle_BowShoot()
{
}

// �����ϲ�Ķ�X: 0x0052D870
void CEffect_Battle_BowShoot::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, bool a4, int hitInfoID)
{
    if (!pCaster || !pTarget) return;

    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    m_fSpeed = 2.0f; // ��l�X�w�s�X

    D3DXVECTOR2 startPos(static_cast<float>(pCaster->GetPosX()), static_cast<float>(pCaster->GetPosY()));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY()));

    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;

    D3DXVECTOR2 vec = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vec) - 60.0f;
    if (m_fTotalDistance < 0.0f) m_fTotalDistance = 0.0f;

    D3DXVec2Normalize(&vec, &vec);
    m_fDirectionX = vec.x * m_fSpeed; // ��l�X�b���B�N�t�׭��J�F��V�V�q
    m_fDirectionY = vec.y * m_fSpeed;

    // �ھڬI�k�̻P�ؼЪ��۹��m�M�w�O�_½��
    m_bIsFlip = (pCaster->GetPosX() - pTarget->GetPosX()) > 0;

    // �p�⭸�樤��
    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vec);
    m_fAngle = acosf(dotProduct);

    // ��l�X�����@�ӽ������P�_�M�w���ץ��t�A�i²�Ƭ��ˬd Y ���q
    if (vec.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

/**
 * @brief �]�w�}�b�S�� (�y�Ъ���)�C
 * @param pStartPos ��g�����_�l�@�ɮy�СC
 * @param pEndPos ��g�����ؼХ@�ɮy�СC
 * @param fFlip ½��X�� (�b��l�X���� float�A���欰���� bool)�C
 * @param hitInfoID �����ؼЮɶǻ�����T ID�C
 * @note ���禡���޿��T�٭�� Effectall.c �� 0x0052DC00 ���P�W�禡�C
 */
void CEffect_Battle_BowShoot::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fFlip, int hitInfoID)
{
    if (!pStartPos || !pEndPos) return;

    // �B�J 1: �]�w��l�ݩ�
    m_nHitInfoID = hitInfoID;
    m_fSpeed = 2.0f; // �ѷӨ��⪩�����]�w�A�]�w�@�Ӱ�¦�t��

    // �]�w�S�Ī���l��m
    // ��l�X: *((_DWORD *)this + 2) = *(_DWORD *)a2;
    //         *((_DWORD *)this + 3) = *((_DWORD *)a2 + 1);
    m_fCurrentPosX = pStartPos->x;
    m_fCurrentPosY = pStartPos->y;

    // �B�J 2: �p�⭸��V�q�M�`�Z��
    // ��l�X: v19 = *(float *)a3 - *(float *)a2;
    //         v7 = *((float *)a3 + 1) - *((float *)a2 + 1);
    D3DXVECTOR2 vecDirection = *pEndPos - *pStartPos;

    // ��l�X: *((float *)this + 34) = sqrt(v8 * v8 + v9) - 60.0;
    m_fTotalDistance = D3DXVec2Length(&vecDirection) - 60.0f;
    if (m_fTotalDistance < 0.0f) {
        m_fTotalDistance = 0.0f;
    }

    // �B�J 3: �N�V�q���ƥH��o�¤�V�A�í��W�t��
    // ��l�X: D3DXVec2Normalize((char *)this + 16, (char *)this + 16);
    //         v20 = *((float *)this + 6) * *v6;
    D3DXVec2Normalize(&vecDirection, &vecDirection);
    m_fDirectionX = vecDirection.x * m_fSpeed;
    m_fDirectionY = vecDirection.y * m_fSpeed;

    // �B�J 4: �]�w½��X��
    // ��l�X: *((_BYTE *)this + 56) = LOBYTE(a4);
    m_bIsFlip = (static_cast<int>(fFlip) != 0);

    // �B�J 5: �p�⭸�樤�ץH�K���T��V
    // �o�q�޿�P���⪩���� SetEffect �����ۦP�A�Ω�Ͻb�ڵ�ı�W�¦V�ؼ�
    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f); // �ھ�½�બ�A�]�w�����ѦҦV�q

    // �p��ѦҦV�q�P�����V�V�q���I�n
    float dotProduct = D3DXVec2Dot(&refVec, &vecDirection);

    // �ϥΤϾl����ƭp�⧨���]���ס^
    m_fAngle = acosf(dotProduct);

    // �ھ� Y �b��V�M�w���ת����t
    // ��l�X�� v14 | v15 �O�@�ӽ����B�i��]�ϲ�Ķ�X�����P�_�A
    // ��ڥ��ت��O�P�_�V�q�O�b�ѦҦV�q�����ɰw�٬O�f�ɰw��V�C
    // �ˬd Y ���q�O�󪽱��B���Ī���{�C
    if (vecDirection.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// �����ϲ�Ķ�X: 0x0052DE10
bool CEffect_Battle_BowShoot::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount)) {
        float moveDistance = static_cast<float>(moveFrameCount); // �t�פw���J�V�q�A�o�̫Y�Ƭ�1
        m_fTraveledDistance += moveDistance;

        if (m_fTraveledDistance >= m_fTotalDistance) {
            if (m_pTargetCharacter) {
                m_pTargetCharacter->SetHited(m_nHitInfoID, 16);
            }
            return true;
        }
        else {
            m_fCurrentPosX += m_fDirectionX * moveDistance;
            m_fCurrentPosY += m_fDirectionY * moveDistance;
        }
    }
    return false;
}

// �����ϲ�Ķ�X: 0x0052DF10
void CEffect_Battle_BowShoot::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetRotation(m_fAngle);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x0052DF60
void CEffect_Battle_BowShoot::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}