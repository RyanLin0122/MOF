#include "Effect/CBulletBase.h"

// �����ϲ�Ķ�X: 0x0052D700
CBulletBase::CBulletBase()
    : m_pVftable(nullptr),
    m_dwOwnerID(0),
    m_fSpeed(0.0f)
{
    // m_vecPos �M m_vecDir �|�Q�w�]��l��
}

// �����ϲ�Ķ�X: 0x0052D740
CBulletBase::~CBulletBase()
{
    // �����O���Ѻc�禡
}

// �����ϲ�Ķ�X: 0x0052D750
bool CBulletBase::Process(float fElapsedTime)
{
    // �����O���w�]�欰�O�����򳣤������åB���ä��������C
    // �l�����O�����мg���禡�ӹ�{�ۤv�����ʩM�ͩR�g���޿�C
    return false;
}