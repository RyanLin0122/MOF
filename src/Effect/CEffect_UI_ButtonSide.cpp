#include "Effect/CEffect_UI_ButtonSide.h"
#include "Effect/CEAManager.h"
#include "global.h"

// �����ϲ�Ķ�X: 0x005328B0
CEffect_UI_ButtonSide::CEffect_UI_ButtonSide()
    : m_bIsFinished(false)
{
    // CEffectBase ���غc�禡�|�Q�۰ʩI�s

    // �V CEAManager �ШD�S�ļƾ�
    // �S�� ID: 70, �ɮצW��: "Effect/efn_BtnSide.ea"
    CEAManager::GetInstance()->GetEAData(70, "Effect/efn_BtnSide.ea", &m_ccaEffect);

    // �]�w�v��ɶ��ô`������ʵe
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // Play(..., 0, 1) -> �`������
}

// �����ϲ�Ķ�X: 0x00532950
CEffect_UI_ButtonSide::~CEffect_UI_ButtonSide()
{
}

// �����ϲ�Ķ�X: 0x00532960
void CEffect_UI_ButtonSide::SetEffect(float x, float y)
{
    // --- �֤��޿�G�����]�w CCAEffect ���ù��y�� ---
    // ��l�X: *((float *)this + 12) = a2;
    //         *((float *)this + 13) = a3;
    m_ccaEffect.SetPosition(x, y);
}

// �����ϲ�Ķ�X: 0x00532980
bool CEffect_UI_ButtonSide::FrameProcess(float fElapsedTime)
{
    // ��s�����ʵe���v��
    m_ccaEffect.FrameProcess(fElapsedTime);

    // ��^�ͩR�g���X�СA�ѥ~���޿�M�w��ɵ���
    return m_bIsFinished;
}

// �����ϲ�Ķ�X: 0x005329A0
void CEffect_UI_ButtonSide::Process()
{
    // �����N�B�z���ȩe�U�������� CCAEffect
    // �]���y�Фw�b SetEffect ���]�w�A�L�ݦA����s
    m_ccaEffect.Process();

    // UI �S���`�O�i��
    m_bIsVisible = TRUE;
}

// �����ϲ�Ķ�X: 0x005329B0
void CEffect_UI_ButtonSide::Draw()
{
    // �����Nø�s���ȩe�U�������� CCAEffect ����
    m_ccaEffect.Draw();
}