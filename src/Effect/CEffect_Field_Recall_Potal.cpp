#include "Effect/CEffect_Field_Recall_Potal.h"
#include "Effect/CEAManager.h"
#include "Effect/cltMoFC_EffectKindInfo.h" // �ݭn�S�ĺ�����T�޲z��
#include "Effect/CEffectManager.h"
#include "global.h"


// ���M��l�X�S�����ѫغc/�Ѻc�禡�A�����F����ʦӲK�[
CEffect_Field_Recall_Potal::CEffect_Field_Recall_Potal()
    : m_bIsFinished(false)
{
}

CEffect_Field_Recall_Potal::~CEffect_Field_Recall_Potal()
{
}

// �����ϲ�Ķ�X: 0x005370C0
void CEffect_Field_Recall_Potal::SetEffect(char* szEffectName, float x, float y)
{
    // �B�J 1: �ھڦW�٬d��S�ĸ�T
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(szEffectName);
    if (!pKindInfo) {
        return; // �䤣��������S�ĩw�q
    }

    // �B�J 2: �c�ا��㪺�ɮ׸��|
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", pKindInfo->szFileName);

    // �B�J 3: ���J�S�ļƾڨö}�l����
    CEAManager::GetInstance()->GetEAData(pKindInfo->usKindID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �B�J 4: �]�w�S�Ħ�m�M��l���A
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_bIsFinished = false;
}

// �����ϲ�Ķ�X: 0x00537160
bool CEffect_Field_Recall_Potal::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe���v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // ��^�ͩR�g���X�СA�ѥ~���޿�M�w��ɵ���
    return m_bIsFinished;
}

// �����ϲ�Ķ�X: 0x00537180
void CEffect_Field_Recall_Potal::Process()
{
    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x005371C0
void CEffect_Field_Recall_Potal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}