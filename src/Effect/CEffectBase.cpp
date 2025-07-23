#include "Effect/CEffectBase.h"

// ���]�������ܼơA�Ω� IsCliping �禡
// �o���ܼ����b�C���D�t�Τ��w�q�M��s
extern struct GameSystemInfo {
    int ScreenWidth;
    int ScreenHeight;
} g_Game_System_Info;


// �����ϲ�Ķ�X: 0x0053A2C0
CEffectBase::CEffectBase()
{
    // m_ccaEffect �|�b���B�۰ʩI�s��غc�禡�A������l�X�����G
    // CCAEffect::CCAEffect((CEffectBase *)((char *)this + 36));

    m_pOwner = nullptr;          // *((_DWORD *)this + 1) = 0;
    m_fCurrentPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fDirectionX = 0.0f;
    m_fDirectionY = 0.0f;
    m_fSpeed = 0.0f;             // *((_DWORD *)this + 6) = 0;
    m_bIsFlip = false;           // *((_BYTE *)this + 28) = 0;
    m_bIsVisible = false;
}

// �����ϲ�Ķ�X: 0x0053A300
CEffectBase::~CEffectBase()
{
    // *(_DWORD *)this = &CEffectBase::`vftable'; // �ѽsĶ���B�z
    // m_ccaEffect ���Ѻc�禡�|�b���B�۰ʳQ�I�s�A������l�X�����G
    // CCAEffect::~CCAEffect((CEffectBase *)((char *)this + 36));
}

/**
 * @brief �B�z�S�Ī��ͩR�g���C
 * �o�O�����O���w�]�欰�A������^ false�A��ܯS�ĥä������C
 * �l�����O�����мg���禡�H���Ѧۤv����������C
 */
 // �����ϲ�Ķ�X: 0x0053A310
bool CEffectBase::FrameProcess(float fElapsedTime)
{
    // �w�]���p�U�A�S�Ĥ��|�۰ʵ����C
    return false; // return 0;
}

/**
 * @brief �ˬd�S�ĬO�_�b�ù����i���d�򤺡A�[�W�@�ӽw�İϡC
 * �o�O���F�u�Ʈį�A�קK�B�z�Mø�s�����b�ù��~���S�ġC
 */
 // �����ϲ�Ķ�X: 0x0053A340
bool CEffectBase::IsCliping(float x, float y)
{
    // ��l�X���ϥ� 150.0 �@���ù��~���w�İ�
    const float fBuffer = 150.0f;

    // �ˬd X �y�ЬO�_�b [ -�w�İ�, �ù��e�� + �w�İ� ] ���d��
    if (x + fBuffer < 0.0f) {
        return false;
    }
    if (x > static_cast<float>(g_Game_System_Info.ScreenWidth) + fBuffer) {
        return false;
    }

    // ��l�X�S���ˬd Y �y�СA���@�ӧ��㪺��@�q�`�|�]�t��
    // if (y + fBuffer < 0.0f) return FALSE;
    // if (y > static_cast<float>(g_Game_System_Info.ScreenHeight) + fBuffer) return FALSE;

    return true;
}