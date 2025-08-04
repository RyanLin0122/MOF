#include "Effect/CBulletArrow.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"

// �����ϲ�Ķ�X: 0x0052D570
CBulletArrow::CBulletArrow()
    : m_pArrowImage(nullptr)
{
    // CBulletBase ���غc�禡�|�Q�۰ʩI�s

    // ��l�X: *((_DWORD *)this + 7) = 256;
    m_dwAlpha = 256;
}

// �����ϲ�Ķ�X: 0x0052D5B0
CBulletArrow::~CBulletArrow()
{
}

// �����ϲ�Ķ�X: 0x0052D5C0
void CBulletArrow::Create(unsigned int dwOwnerID, D3DXVECTOR2* pStartPos, D3DXVECTOR2* pEndPos, float fSpeed)
{
    m_dwOwnerID = dwOwnerID;
    m_vecPos = *pStartPos;
    m_fSpeed = fSpeed;

    // �p���V�V�q�ó���
    m_vecDir = *pEndPos - *pStartPos;
    D3DXVec2Normalize(&m_vecDir, &m_vecDir);
}

// �����ϲ�Ķ�X: 0x0052D620
bool CBulletArrow::Process(float fElapsedTime)
{
    // --- �֤��޿� ---

    // 1. ��s��m
    // ��l�X: v10 = *((float *)this + 4) + *((float *)this + 2);
    //         v3 = *((float *)this + 5) + *((float *)this + 3);
    // �`�N�G��l�X���G�S���N�t�׭��W��V�A�o�i��O�@��bug��²�ơC
    // �@�ӧ󧹾㪺��{���ӬO: m_vecPos += m_vecDir * m_fSpeed * fElapsedTime;
    // �����F�����٭�A�ڭ̼�����欰�C
    m_vecPos += m_vecDir;

    // 2. ����ó]�w GameImage
    m_pArrowImage = cltImageManager::GetInstance()->GetGameImage(7, 0xB00001Cu, 0, 1);

    if (m_pArrowImage) {
        m_pArrowImage->SetBlockID(0); // �j��ϥβ�0�V
        m_pArrowImage->SetPosition(m_vecPos.x, m_vecPos.y);
        m_pArrowImage->SetAlpha(m_dwAlpha); // �]�w�z����
        m_pArrowImage->Process();
    }

    // 3. ��s�ͩR�g���p�ɾ�
    // ��l�X: v8 = *((_DWORD *)this + 7) - 1;
    m_dwAlpha--;

    // 4. �ˬd�ͩR�g���O�_����
    // ��l�X: return v8 == 0;
    return (m_dwAlpha <= 0);
}

// �����ϲ�Ķ�X: 0x0052D6E0
void CBulletArrow::Draw()
{
    if (m_pArrowImage && m_pArrowImage->IsInUse())
    {
        m_pArrowImage->Draw();
    }
}