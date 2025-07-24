#include "Effect/CEffect_Field_Pet_ItemPick.h"
#include "Effect/CEffect_Field_Pet_ItemPick_Sub.h" // �]�t�l�S�ĩw�q
#include "Character/ClientCharacter.h"
#include "Effect/CEffectManager.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;
extern CEffectManager g_EffectManager;
extern ClientCharacterManager g_ClientCharMgr;
extern int SETTING_FRAME; // ���]���@�ӱ`�ơA�Ҧp 5

// �����ϲ�Ķ�X: 0x005366C0
CEffect_Field_Pet_ItemPick::CEffect_Field_Pet_ItemPick()
{
    m_pOwnerPet = nullptr;
    m_dwOwnerAccountID = 0;
    m_dwTotalFrame = 0;
    m_ucState = 0;
    m_fSpeedFactor = 1.0f;
    m_nSubEffectTimer = 0;
    m_FrameSkip.m_fTimePerFrame = 1.0f / 75.0f; // 1001786209 -> ~0.01333f
}

CEffect_Field_Pet_ItemPick::~CEffect_Field_Pet_ItemPick() {}

// �����ϲ�Ķ�X: 0x00536700
void CEffect_Field_Pet_ItemPick::SetEffect(D3DXVECTOR2* pStartPosition, unsigned int dwOwnerAccountID)
{
    m_fCurrentPosX = pStartPosition->x;
    m_fCurrentPosY = pStartPosition->y - 50.0f; // ���~�q�a���W��@�I�}�l
    m_dwOwnerAccountID = dwOwnerAccountID;
    m_pOwnerPet = nullptr; // ��l�ɥ��]����
}

// �����ϲ�Ķ�X: 0x00536730
bool CEffect_Field_Pet_ItemPick::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    // ����d������
    // ��l�X���O��������a�A����d���A�o��²��
    m_pOwnerPet = g_ClientCharMgr.GetCharByAccount(m_dwOwnerAccountID);
    if (!m_pOwnerPet) return true; // �p�G�d�������A�S�ĵ���

    m_dwTotalFrame += frameCount;

    // �]�w�ؼ��I���d������m
    m_fTargetPosX = static_cast<float>(m_pOwnerPet->GetPetPosX());
    m_fTargetPosY = static_cast<float>(m_pOwnerPet->GetPetPosY() - 20);

    D3DXVECTOR2 vecToTarget(m_fTargetPosX - m_fCurrentPosX, m_fTargetPosY - m_fCurrentPosY);

    // �p�G�Z��������A�h��������ò��ͧ����S��
    if (D3DXVec2LengthSq(&vecToTarget) < 15.0f * 15.0f) {
        CEffect_Field_Pet_ItemPick_Sub_Complete* pComplete = new CEffect_Field_Pet_ItemPick_Sub_Complete();
        pComplete->SetEffect(m_fTargetPosX, m_fTargetPosY + 20.0f, false);
        g_EffectManager.BulletAdd(pComplete);
        return true;
    }

    // �W�ɾP��
    if (m_dwTotalFrame > 2000) {
        // �P�˲��ͧ����S��
        CEffect_Field_Pet_ItemPick_Sub_Complete* pComplete = new CEffect_Field_Pet_ItemPick_Sub_Complete();
        pComplete->SetEffect(m_fTargetPosX, m_fTargetPosY + 20.0f, false);
        g_EffectManager.BulletAdd(pComplete);
        return true;
    }

    D3DXVec2Normalize(&vecToTarget, &vecToTarget);

    // --- ���A����������޿� ---
    switch (m_ucState) {
    case 0:
        m_fCurrentPosY -= fFrameCount * 0.3f;
        m_fCurrentPosX += vecToTarget.x * fFrameCount;
        m_fCurrentPosY += vecToTarget.y * fFrameCount;
        if (m_dwTotalFrame > 70) { m_ucState = 1; m_dwTotalFrame = 70; }
        break;
    case 1:
        m_fCurrentPosY -= fFrameCount * 0.2f;
        m_fCurrentPosX += vecToTarget.x * fFrameCount;
        m_fCurrentPosY += vecToTarget.y * fFrameCount;
        if (m_dwTotalFrame > 140) { m_ucState = 2; m_dwTotalFrame = 140; }
        break;
        // ... (��L���A�������޿�A���O�L�ճt�שM�p�ɾ�) ...
    case 4:
        m_fCurrentPosX += vecToTarget.x * m_fSpeedFactor * fFrameCount;
        m_fCurrentPosY += vecToTarget.y * m_fSpeedFactor * fFrameCount;
        m_fSpeedFactor += fFrameCount * 0.01f;
        break;
    }

    return false;
}

// �����ϲ�Ķ�X: 0x00536C80
void CEffect_Field_Pet_ItemPick::Process()
{
    m_nSubEffectTimer++;
    if (m_nSubEffectTimer >= SETTING_FRAME) {
        m_nSubEffectTimer = 0;

        float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
        m_bIsVisible = IsCliping(screenX, 0.0f);

        if (m_bIsVisible) {
            CEffect_Field_Pet_ItemPick_Sub_Light* pLight = new CEffect_Field_Pet_ItemPick_Sub_Light();
            pLight->SetEffect(m_fCurrentPosX, m_fCurrentPosY, false);
            g_EffectManager.BulletAdd(pLight);
        }
    }
}

void CEffect_Field_Pet_ItemPick::Draw()
{
    // �D����O���i�����A���ݭnø�s����F��C
}