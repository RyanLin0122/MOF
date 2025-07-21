#pragma once

#include <d3d9.h>
#include <d3dx9tex.h> // �ϥ�D3DXCreateTextureFromFileExA�ݭn�����Y
#include <cstdio>      // �Ω� NULL

// �ھڤϽsĶ�{���X�����Ϊk�A�w�qTextureListData���c
// �j�p�� 0x10C (268�줸��)
struct TextureListData {
    TextureListData* pPrev;          // �첾 +0: ���V�e�@�Ӹ`�I
    TextureListData* pNext;          // �첾 +4: ���V��@�Ӹ`�I
    IDirect3DTexture9* pTexture;     // �첾 +8: ���VD3D���z����
    char szFileName[255];            // �첾 +12: �x�s���z���ɮ׸��|
    unsigned char flag;              // �첾 +267: �x�s���X��

    // �`�I���Ѻc�禡
    ~TextureListData() {
        // �b�`�I�Q�P���ɡA�۰������֦���D3D�귽
        if (pTexture)
        {
            pTexture->Release();
            pTexture = nullptr;
        }
    }
};

/**
 * @class TextureListDataMgr
 * @brief �޲zTextureListData�`�I�����V�쵲��C�C
 *
 * �o�Ӻ޲z���t�d���z���إߡ]�q�L�~���޿�^�B�R���B�æbD3D�˸m�򥢮ɳB�z�Ҧ����z�귽�C
 */
class TextureListDataMgr {
public:
    /// @brief �غc�禡
    TextureListDataMgr();

    /// @brief �Ѻc�禡
    ~TextureListDataMgr();

    /// @brief �s�W�@�Ӹ`�I���쵲��C�����ê�^�C
    /// @return ���V�s�إߪ�TextureListData�`�I�����СC
    TextureListData* Add();

    /// @brief �q�쵲��C���R�����w���`�I�C
    /// @param pNode �n�R�����`�I���СC
    void Delete(TextureListData* pNode);

    /// @brief �R��������޲z�������Ҧ��`�I�M�귽�C
    void DeleteAll();

    /// @brief �B�zD3D�˸m�򥢨ƥ�C
    /// ���禡�|����Ҧ����z�A���O�d�`�I���c�H�K���򭫫ءC
    void DeviceLostToRelease();

    /// @brief �B�zD3D�˸m���]�ƥ�C
    /// ���禡�|�ھڤ��e�x�s���ɮ׸��|�A���s���J�Ҧ����z�C
    void DeviceLostToReLoad();

private:
    TextureListData* m_pHead; // �쵲��C���Y����
    TextureListData* m_pTail; // �쵲��C��������
    int m_nCount;             // �쵲��C�����`�I�ƶq
};