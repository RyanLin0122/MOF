#include "Effect/CEffect_Portal.h"
#include "Effect/CEAManager.h"
#include "Effect/CEffectmanager.h"
#include "Effect/cltMoFC_EffectKindInfo.h" // �ݭn�S�ĺ�����T�޲z��
#include "global.h"

CEffect_Portal::CEffect_Portal()
    : m_bIsFinished(false)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
}

CEffect_Portal::~CEffect_Portal()
{
}

// �����ϲ�Ķ�X: 0x00537930
void CEffect_Portal::SetEffect(char* szEffectName, float x, float y)
{
    // �B�J 1: �ھڦW�٬d��S�ĸ�T
    stEffectKindInfo* pKindInfo = CEffectManager::GetInstance()->g_clEffectKindInfo.GetEffectKindInfo(szEffectName);
    if (!pKindInfo) {
        m_bIsFinished = true; // �p�G�䤣��귽�A�����аO������
        return;
    }

    // �B�J 2: �c�ا��㪺�ɮ׸��|
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", pKindInfo->szFileName);

    // �B�J 3: ���J�S�ļƾڨö}�l����
    CEAManager::GetInstance()->GetEAData(pKindInfo->usKindID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // �ǰe���q�`�O�`������

    // �B�J 4: �]�w�S�Ħ�m�M��l���A
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
    m_bIsFinished = false;
}

// �����ϲ�Ķ�X: 0x005379D0
bool CEffect_Portal::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe���v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // ��^�ͩR�g���X�СA�ѥ~���޿�M�w��ɵ���
    return m_bIsFinished;
}

// �����ϲ�Ķ�X: 0x005379F0
void CEffect_Portal::Process()
{
    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00537A30
void CEffect_Portal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}