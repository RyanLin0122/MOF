#include "Effect/CEffect_Battle_Hit_Normal.h"
#include "Effect/CEAManager.h"
#include "global.h" // ���]�Ω���������ܼ� g_Game_System_Info

// ���]�������ܼ�
extern GameSystemInfo g_Game_System_Info;

// �����ϲ�Ķ�X: 0x0052E960
CEffect_Battle_Hit_Normal::CEffect_Battle_Hit_Normal()
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 10, �ɮצW��: "Effect/efn_hit.ea"
    CEAManager::GetInstance()->GetEAData(10, "Effect/efn_hit.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ü���ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // ����Ĥ@�Ӱʵe�ǦC�A�åB���`��
}

// �����ϲ�Ķ�X: 0x0052E9F0
CEffect_Battle_Hit_Normal::~CEffect_Battle_Hit_Normal()
{
}

// �����ϲ�Ķ�X: 0x0052EA00
void CEffect_Battle_Hit_Normal::SetEffect(float x, float y)
{
    // �]�w�S�Ī���l�@�ɮy��
    // ��l�X: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// �����ϲ�Ķ�X: 0x0052EA20
bool CEffect_Battle_Hit_Normal::FrameProcess(float fElapsedTime)
{
    // �N�ͩR�g���޲z�����ȧ����e�U�������� CCAEffect ����C
    // �� "efn_hit.ea" �ʵe���񧹲��ɡA���禡�|�^�� true�C
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// �����ϲ�Ķ�X: 0x0052EA30
void CEffect_Battle_Hit_Normal::Process()
{
    // �N�@�ɮy���ഫ���ù��y��
    // ��l�X: v3 = *((float *)this + 2) - (double)dword_A73088;
    float screenX = m_fCurrentPosX;// - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY;// -static_cast<float>(g_Game_System_Info.ScreenHeight);

    // �i����ŧP�_
    // ��l�X: v2 = CEffectBase::IsCliping(this, v3, 0.0); *((_DWORD *)this + 8) = v2;
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // ��s���� CCAEffect �����A
        // ��l�X: *((float *)this + 12) = ...; *((float *)this + 13) = ...;
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// �����ϲ�Ķ�X: 0x0052EA70
void CEffect_Battle_Hit_Normal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}