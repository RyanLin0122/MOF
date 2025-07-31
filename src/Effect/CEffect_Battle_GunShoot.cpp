#include "Effect/CEffect_Battle_GunShoot.h"
#include "Effect/CEffectManager.h"
#include "Image/CDeviceManager.h"
#include "Image/cltImageManager.h"
#include "Character/ClientCharacter.h"
#include "Effect/CEffect_Field_Walkdust.h"
#include "Effect/CEffect_Battle_GunSpark.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x0052E090
CEffect_Battle_GunShoot::CEffect_Battle_GunShoot()
    : m_pTargetCharacter(nullptr),
    m_fTotalDistance(0.0f),
    m_fTraveledDistance(0.0f),
    m_nHitInfoID(0),
    m_dwCasterAccountID(0)
{
    // CEffectBase �غc�禡�w�۰ʩI�s

    // ��l�X: *((_DWORD *)this + 39) = 993397601; (0x3B360B61 -> 0.0271f ~ 1/36.8)
    // �]�w���ʭp�ɾ�����s�W�v
    m_MovementFrameSkip.m_fTimePerFrame = 1.0f / 37.0f;
}

CEffect_Battle_GunShoot::~CEffect_Battle_GunShoot()
{
}

// �����ϲ�Ķ�X: 0x0052E130
void CEffect_Battle_GunShoot::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short a4, int hitInfoID)
{
    if (!pCaster || !pTarget) return;

    m_pTargetCharacter = pTarget;
    m_nHitInfoID = hitInfoID;
    m_dwCasterAccountID = pCaster->GetAccountID();

    D3DXVECTOR2 startPos(static_cast<float>(pCaster->GetPosX()), static_cast<float>(pCaster->GetPosY()));
    D3DXVECTOR2 endPos(static_cast<float>(pTarget->GetPosX()), static_cast<float>(pTarget->GetPosY()));

    // --- Ĳ�o�}���ɪ����ݯS�� ---
    if (pCaster->GetActionSide() == 1) { // �¥k
        startPos.x -= 70.0f; // �վ�S�İ_�l��m�H�ǰt�j�f

        // �إ߷ϹЮĪG
        CEffect_Field_Walkdust* pDust = new CEffect_Field_Walkdust();
        pDust->SetEffect(startPos.x + 55.0f, startPos.y - 45.0f, false, 0, 0, 0, 0);
        CEffectManager::GetInstance()->BulletAdd(pDust);

        // �إߺj�f����
        CEffect_Battle_GunSpark* pSpark = new CEffect_Battle_GunSpark();
        pSpark->SetEffect(startPos.x, startPos.y, true);
        CEffectManager::GetInstance()->BulletAdd(pSpark);

    }
    else { // �¥�
        startPos.x += 5.0f;

        CEffect_Field_Walkdust* pDust = new CEffect_Field_Walkdust();
        pDust->SetEffect(startPos.x + 50.0f, startPos.y - 45.0f, false, 0, 0, 0, 0);
        CEffectManager::GetInstance()->BulletAdd(pDust);

        CEffect_Battle_GunSpark* pSpark = new CEffect_Battle_GunSpark();
        pSpark->SetEffect(startPos.x, startPos.y, false);
        CEffectManager::GetInstance()->BulletAdd(pSpark);
    }

    // --- �p�⭸����| ---
    m_fCurrentPosX = startPos.x;
    m_fCurrentPosY = startPos.y;

    D3DXVECTOR2 vec = endPos - startPos;
    m_fTotalDistance = D3DXVec2Length(&vec) - 60.0f; // ���e�@�I�Z��Ĳ�o�R��

    D3DXVec2Normalize(&vec, &vec);
    m_fDirectionX = vec.x;
    m_fDirectionY = vec.y;

    m_fSpeed = 10.0f; // �]�w����t��
}

// �t�@�� SetEffect �h���A���M���b CEffectManager �������ϥΡA���s�b���l�X��
void CEffect_Battle_GunShoot::SetEffect(D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, bool bFlip, unsigned short a5, int hitInfoID)
{
    // ... �������V�q�M�Z���p���޿� ...
}

// �����ϲ�Ķ�X: 0x0052E520
bool CEffect_Battle_GunShoot::FrameProcess(float fElapsedTime)
{
    int moveFrameCount = 0;
    if (m_MovementFrameSkip.Update(fElapsedTime, moveFrameCount)) {
        float moveDistance = m_fSpeed * static_cast<float>(moveFrameCount);
        m_fTraveledDistance += moveDistance;

        if (m_fTraveledDistance >= m_fTotalDistance) {
            if (m_pTargetCharacter) {
                m_pTargetCharacter->SetHited(m_nHitInfoID, 16);
            }
            return true; // ��F�ؼСA�S�ĵ���
        }
        else {
            m_fCurrentPosX += m_fDirectionX * moveDistance;
            m_fCurrentPosY += m_fDirectionY * moveDistance;
        }
    }
    return false; // �~�򭸦�
}

/**
 * @brief �ǳ�ø�s�e���޿��s�A�֤ߤu�@�O����ó]�w�l�u�Ϲ������A�C
 * @note ���禡��T�٭�� Effectall.c �� 0x0052E610 ���޿�C
 */
void CEffect_Battle_GunShoot::Process()
{
    // �q���������Χ�s�l�u�� GameImage ���
    // �귽 ID 0xC000171u �O�w�s�X��
    m_pBulletImage = cltImageManager::GetInstance()->GetGameImage(7, 0xC000171u, 0, 1);

    if (m_pBulletImage)
    {
        // �N�@�ɮy���ഫ���ù��y��
        // ��l�X: v4 = *((float *)this + 2) - (double)dword_A73088;
        //         v3 = *((float *)this + 3) - (double)dword_A7308C - 60.0;
        float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY) - 60.0f; // Y�b��60�������B�~����

        // --- �}�l�]�w GameImage ���Ҧ��ݩ� ---

        // �]�w��m
        m_pBulletImage->SetPosition(screenX, screenY);

        // �]�w�ʵe�v�� ID
        // ��l�X: *((_WORD *)v2 + 186) = *((_WORD *)this + 82); (this+164 -> m_usUnk_a4)
        m_pBulletImage->SetBlockID(m_usUnk_a4);

        // �]�w�z����
        // ��l�X: *(_DWORD *)(v5 + 380) = 255;
        m_pBulletImage->SetAlpha(255);

        // �]�w�C��
        // ��l�X: *(_DWORD *)(v6 + 376) = 100;
        m_pBulletImage->SetColor(100);

        // �ǻ��I�k�̪� AccountID�A�i��Ω� Shader
        // ��l�X: *(_DWORD *)(*((_DWORD *)this + 33) + 392) = *((_DWORD *)this + 42);
        // m_pBulletImage->m_dwSomeShaderData = m_dwCasterAccountID; // ���] GameImage ��������

        // ��s���I�w�İ�
        m_pBulletImage->Process();
    }
}

/**
 * @brief ø�s�l�u�Ϲ��C
 * @note ���禡��T�٭�� Effectall.c �� 0x0052E6D0 ���޿�C
 */
void CEffect_Battle_GunShoot::Draw()
{
    if (m_pBulletImage && m_pBulletImage->IsInUse())
    {
        // �]�w�S�ıM�Ϊ� Alpha �V�X�Ҧ�
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);  // 5
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); // 6

        // ø�s�Ϲ�
        m_pBulletImage->Draw();

        // �`�N�G��l�X�b Draw ����S����_ RenderState�A
        // �o�q�`�� CEffectManager::Draw ������ ResetRenderState �Τ@�B�z�C
    }
}