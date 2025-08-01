#include "Effect/CEffect_Skill_Resurrection.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00533C90
CEffect_Skill_Resurrection::CEffect_Skill_Resurrection()
    : m_fInitialPosX(0.0f), m_fInitialPosY(0.0f)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    // �S�ĸ귽�O�b SetEffect ���ʺA���J��
}

// �����ϲ�Ķ�X: 0x00533CD0
CEffect_Skill_Resurrection::~CEffect_Skill_Resurrection()
{
}

// �����ϲ�Ķ�X: 0x00533CE0
void CEffect_Skill_Resurrection::SetEffect(float x, float y, unsigned char skillLevel)
{
    const char* szFileName = nullptr;
    int effectID = 0;

    // �ھڧޯ൥�ſ�ܤ��P���S�ĸ귽
    if (skillLevel == 1) {
        effectID = 72;
        szFileName = "Effect/efn_resurrectionLv2.ea";
    }
    else if (skillLevel == 2) {
        effectID = 73;
        szFileName = "Effect/efn_resurrectionLv3.ea";
    }
    else { // �w�]������ 0
        effectID = 71;
        szFileName = "Effect/efn_Resurrection.ea";
    }

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �x�s��m�ַ�
    m_fInitialPosX = x;
    m_fInitialPosY = y;
}

// �����ϲ�Ķ�X: 0x00533D60
bool CEffect_Skill_Resurrection::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00533D70
void CEffect_Skill_Resurrection::Process()
{
    // --- �֤��޿�G�ϥ��x�s����l��m ---
    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // �i����ŧP�_
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x00533DC0
void CEffect_Skill_Resurrection::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}