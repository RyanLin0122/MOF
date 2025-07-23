#include "Effect/CEffect_Skill_Type_ShootUnit.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: CEffect_Battle_BowShoot::CEffect_Battle_BowShoot (���c����)
CEffect_Skill_Type_ShootUnit::CEffect_Skill_Type_ShootUnit()
    : m_pTargetCharacter(nullptr),
    m_fTotalDistance(0.0f),
    m_fTraveledDistance(0.0f),
    m_fAngle(0.0f),
    m_nHitInfoID(0)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    m_fSpeed = 10.0f;

    // --- ����ץ� ---
    // ��l�ƥΩ󲾰ʪ� FrameSkip�C
    // ��l�X (0x0052D770 @ CEffect_Battle_BowShoot) ���A���p�ɾ����C�v��ɶ��Q�]�� 0x3D088889
    // �o�ӤQ���i��ȹ������B�I�Ƭ��� 0.03333f�A�Y 1.0f / 30.0f�C
    // �o��ܪ��z��s���W�v�O�T�w�� 30 FPS�C
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 30.0f;
}

CEffect_Skill_Type_ShootUnit::~CEffect_Skill_Type_ShootUnit()
{
}

void CEffect_Skill_Type_ShootUnit::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName, int hitInfoID)
{
    // ... (���禡���e���ܡA�O�����) ...
    if (!pCaster || !pTarget) return;
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true);
    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    D3DXVECTOR2 startPos(static_cast<float>(pCaster->GetPosX()), static_cast<float>(pCaster->GetPosY() - 25));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY() - 15));
    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;
    D3DXVECTOR2 vec = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vec);
    D3DXVec2Normalize(&vec, &vec);
    m_fDirectionX = vec.x;
    m_fDirectionY = vec.y;
    m_bIsFlip = (pCaster->GetActionSide() == 1);
    D3DXVECTOR2 refVec(m_bIsFlip ? -1.0f : 1.0f, 0.0f);
    float dotProduct = D3DXVec2Dot(&refVec, &vec);
    m_fAngle = acosf(dotProduct);
    if (vec.y < 0) {
        m_fAngle = -m_fAngle;
    }
}

// �����ϲ�Ķ�X: 0x0052DE10 (CEffect_Battle_BowShoot::FrameProcess)
bool CEffect_Skill_Type_ShootUnit::FrameProcess(float fElapsedTime)
{
    // �B�J 1: ��s��������ı�ʵe (���y�U�N�B�b�ڱ��൥)
    m_ccaEffect.FrameProcess(fElapsedTime);

    // �B�J 2: �ϥαM�������ʭp�ɾ��ӭp�⪫�z��s
    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount))
    {
        // �ھڭp��X���v��ƨӧ�s��m
        // �o�T�O�F�Y�ϹC���V�v�i�ʡA��g��������t�פ]�O����w
        float moveDistance = m_fSpeed * static_cast<float>(moveFrameCount);
        m_fTraveledDistance += moveDistance;

        // �B�J 3: �ˬd�O�_��F���I
        if (m_fTraveledDistance >= m_fTotalDistance) {
            // �w��F�A�q���ؼШ���Q����
            if (m_pTargetCharacter) {
                // ��l�X: *(_BYTE *)(32 * m_nHitInfoID + pTarget + 8099) = 16;
                m_pTargetCharacter->SetHited(m_nHitInfoID, 16); // ���]���禡
            }
            return true; // ��^ true�A��ܯS�ĥͩR�g������
        }
        else {
            // �|����F�A��s��e�@�ɮy��
            m_fCurrentPosX += m_fDirectionX * moveDistance;
            m_fCurrentPosY += m_fDirectionY * moveDistance;
        }
    }

    return false; // �S���~��s�b
}

// �����ϲ�Ķ�X: 0x0052DF10 (CEffect_Battle_BowShoot::Process)
void CEffect_Skill_Type_ShootUnit::Process()
{
    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, screenY);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetRotation(m_fAngle);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x0052DF60 (CEffect_Battle_BowShoot::Draw)
void CEffect_Skill_Type_ShootUnit::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}