#include "Effect/CEffect_UI_WorldMap_Light.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x00532FC0
CEffect_UI_WorldMap_Light::CEffect_UI_WorldMap_Light()
    : m_bIsFinished(false), m_nDisableFlag(0)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 69, �ɮצW��: "Effect/map_face01.ea"
    CEAManager::GetInstance()->GetEAData(69, "Effect/map_face01.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ô`������ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // Play(..., 0, 1) -> �`������
}

// �����ϲ�Ķ�X: 0x00533060
CEffect_UI_WorldMap_Light::~CEffect_UI_WorldMap_Light()
{
}

// �����ϲ�Ķ�X: 0x00533070
void CEffect_UI_WorldMap_Light::SetEffect(float x, float y)
{
    // --- �֤��޿�G�����]�w CCAEffect ���ù��y�� ---
    // ��l�X: *((float *)this + 12) = a2;
    //         *((float *)this + 13) = a3;
    m_ccaEffect.SetPosition(x, y);
}

// �����ϲ�Ķ�X: 0x00533090
bool CEffect_UI_WorldMap_Light::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe���v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // ��^�ͩR�g���X�СA�ѥ~���޿�M�w��ɵ���
    return m_bIsFinished;
}

// �����ϲ�Ķ�X: 0x005330B0
void CEffect_UI_WorldMap_Light::Process()
{
    // �����N�B�z���ȩe�U�������� CCAEffect
    // �]���y�Фw�b SetEffect ���]�w�A�L�ݦA����s
    m_ccaEffect.Process();

    // UI �S���`�O�i��
    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x005330C0
void CEffect_UI_WorldMap_Light::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}

// �����ϲ�Ķ�X: 0x005330D0
void CEffect_UI_WorldMap_Light::SetDisable(int disableFlag)
{
    // �]�w�@�ӥ������X�СA��γ~�b���Ѫ��{���X�����Q����
    m_nDisableFlag = disableFlag;
}