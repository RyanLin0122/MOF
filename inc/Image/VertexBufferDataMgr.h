#pragma once

#include <d3d9.h>
#include <cstdio> // �Ω� NULL

template<typename T>
void SafeRelease(T*& p) {
    if (p) {
        p->Release();
        p = nullptr;
    }
}

// �ھڤϽsĶ�{���X�����Ϊk�A�w�qVertexBufferData���c
// �j�p�� 0x10 (16�줸��)
struct VertexBufferData {
    VertexBufferData* pPrev;          // �첾 +0: ���V�e�@�Ӹ`�I
    VertexBufferData* pNext;          // �첾 +4: ���V��@�Ӹ`�I
    IDirect3DVertexBuffer9* pVertexBuffer; // �첾 +8: ���VD3D���I�w�İϪ���
    unsigned short capacity;          // �첾 +12: �w�İϮe�q
    unsigned char type;               // �첾 +14: ���I����
    char _padding;                    // �ɻ���16�줸��

    // �`�I���Ѻc�禡
    ~VertexBufferData() {
        // �b�`�I�Q�P���ɡA�۰������֦���D3D�귽
        SafeRelease(pVertexBuffer);
    }
};

/**
 * @class VertexBufferDataMgr
 * @brief �޲zVertexBufferData�`�I�����V�쵲��C�C
 *
 * �o�Ӻ޲z���t�d�إߡB�R���B�æbD3D�˸m�򥢮ɳB�z�Ҧ������I�w�İϸ귽�C
 */
class VertexBufferDataMgr {
public:
    /// @brief �غc�禡
    VertexBufferDataMgr();

    /// @brief �Ѻc�禡
    ~VertexBufferDataMgr();

    /// @brief �s�W�@�Ӹ`�I���쵲��C�����ê�^�C
    /// @return ���V�s�إߪ�VertexBufferData�`�I�����СC
    VertexBufferData* Add();

    /// @brief �q�쵲��C���R�����w���`�I�C
    /// @param pNode �n�R�����`�I���СC
    void Delete(VertexBufferData* pNode);

    /// @brief �R��������޲z�������Ҧ��`�I�M�귽�C
    void DeleteAll();

    /// @brief �B�zD3D�˸m�򥢨ƥ�C
    /// ���禡�|����Ҧ����I�w�İϡA���O�d�`�I���c�H�K���򭫫ءC
    void DeviceLostToRelease();

    /// @brief �B�zD3D�˸m���]�ƥ�C
    /// ���禡�|�ھڤ��e�x�s����T�A���s�إߩҦ����I�w�İϡC
    void DeviceLostToReLoad();

private:
    VertexBufferData* m_pHead; // �쵲��C���Y����
    VertexBufferData* m_pTail; // �쵲��C��������
    int m_nCount;              // �쵲��C�����`�I�ƶq
};