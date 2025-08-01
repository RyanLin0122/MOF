#include "Effect/CEffect_Skill_Trap_Explosion.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x005340B0
CEffect_Skill_Trap_Explosion::CEffect_Skill_Trap_Explosion()
    : m_fInitialPosX(0.0f), m_fInitialPosY(0.0f)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s
    // �S�ĸ귽�O�b SetEffect ���ʺA���J��
}

// �����ϲ�Ķ�X: 0x005340F0
CEffect_Skill_Trap_Explosion::~CEffect_Skill_Trap_Explosion()
{
}

// �����ϲ�Ķ�X: 0x00534100
void CEffect_Skill_Trap_Explosion::SetEffect(float x, float y, unsigned char skillLevel)
{
    const char* szFileName = nullptr;
    int effectID = 0;

    // �ھڧޯ൥�ſ�ܤ��P���S�ĸ귽
    if (skillLevel == 1) {
        effectID = 90;
        szFileName = "Effect/efn_Trap_ExplosionLv2.ea";
    }
    else if (skillLevel == 2) {
        effectID = 91;
        szFileName = "Effect/efn_Trap_ExplosionLv3.ea";
    }
    else { // �w�]������ 0
        effectID = 89;
        szFileName = "Effect/efn_Trap_Explosion.ea";
    }

    // ����ó]�w�S�ļƾ�
    CEAManager::GetInstance()->GetEAData(effectID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A���`��

    // �x�s��m�ַ�
    m_fInitialPosX = x;
    m_fInitialPosY = y;
}

// �����ϲ�Ķ�X: 0x00534180
bool CEffect_Skill_Trap_Explosion::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x00534190
void CEffect_Skill_Trap_Explosion::Process()
{
    // --- �֤��޿�G�ϥ��x�s����l��m ---
    // ��l�X: v1[3] = v1[24] - (double)dword_A73088;
    //         v1[4] = v1[25] - (double)dword_A7308C;
    // �o�̪� v1[24] �M v1[25] ��ڤW�O m_fInitialPosX �M m_fInitialPosY
    // �����F�M���_���A�ڭ̤������ާ@ CCAEffect ����������

    // �N�@�ɮy���ഫ���ù��y��
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // ��s���� CCAEffect �����A
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // ���ŧP�_�b CCAEffect::Process ���������A�o�̥u�ݳ]�w�i����
    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x005341B0
void CEffect_Skill_Trap_Explosion::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}