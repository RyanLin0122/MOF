#include "Effect/CEffect_MapEffect.h"
#include "Effect/CEAManager.h"
#include "Effect/CEffectManager.h"
#include "Effect/cltMoFC_EffectKindInfo.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;
extern cltMoFC_EffectKindInfo g_clEffectKindInfo;

// �����ϲ�Ķ�X: 0x00537780
CEffect_MapEffect::CEffect_MapEffect()
    : m_pStateOwner(nullptr), m_wRequiredStateID(0)
{
}

// �����ϲ�Ķ�X: 0x005377C0
CEffect_MapEffect::~CEffect_MapEffect()
{
}

// �����ϲ�Ķ�X: 0x005377D0
void CEffect_MapEffect::SetEffect(char* szEffectName, unsigned short requiredStateID, ClientCharacter* pStateOwner, int x, int y)
{
    // �z�L�W������S�� Kind ID
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(szEffectName);
    if (pKindInfo) {
        // �I�s�t�@�Ӧh�������ӧ����]�w
        SetEffect(pKindInfo->usKindID, requiredStateID, pStateOwner, x, y);
    }
}

// �����ϲ�Ķ�X: 0x00537810
void CEffect_MapEffect::SetEffect(unsigned short effectKindID, unsigned short requiredStateID, ClientCharacter* pStateOwner, int x, int y)
{
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(effectKindID);
    if (!pKindInfo) return;

    // �c���ɮ׸��|�ø��J�ƾ�
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", pKindInfo->szFileName);
    CEAManager::GetInstance()->GetEAData(effectKindID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // ����ʯS�����`������

    // �x�s���A�M��m��T
    m_wRequiredStateID = requiredStateID;
    m_pStateOwner = pStateOwner;
    m_fCurrentPosX = static_cast<float>(x);
    m_fCurrentPosY = static_cast<float>(y);
}


// �����ϲ�Ķ�X: 0x005378B0
bool CEffect_MapEffect::FrameProcess(float fElapsedTime)
{
    if (!m_pStateOwner) {
        return true; // �p�G���A�֦��̤��s�b�A�h�S�ĥߧY����
    }

    // �ˬd�֦��̪����A ID �O�_�P�n�D�� ID �ǰt
    // ��l�X: *(_WORD *)(*((_DWORD *)this + 33) + 592) == *((_WORD *)this + 68)
    if (m_pStateOwner->GetMapID() == m_wRequiredStateID) {
        // ���A�ǰt�A��s�ʵe�����S���~��s�b
        m_ccaEffect.FrameProcess(fElapsedTime);
        return false;
    }
    else {
        // ���A���ǰt�A�S�ĵ���
        return true;
    }
}

// �����ϲ�Ķ�X: 0x005378E0
void CEffect_MapEffect::Process()
{
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00537920
void CEffect_MapEffect::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}