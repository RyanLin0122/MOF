#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Image/VertexBufferDataMgr.h"     // �]�t���e�٭쪺 VertexBuffer �޲z��
#include "Image/ImageResourceListDataMgr.h"// �]�t ImageResource �޲z��
#include "Image/TextureListDataMgr.h"      // �]�t���e�٭쪺 Texture �޲z��

/**
 * @class CDeviceResetManager
 * @brief �����޲zD3D�귽�A�óB�z�˸m��(Lost)�P���](Reset)���֤��޿�C
 *
 * �o�����O�ϥγ�ҼҦ�(Singleton)�A�T�O�{�����u���@�ӹ�ҡC
 * ���ʸˤF�� Vertex Buffers�BImage Resources �M Textures ���ͩR�g���޲z�C
 */
class CDeviceResetManager {
public:
    /// @brief ���o�ߤ@�����O��ҡC
    static CDeviceResetManager* GetInstance();

    /// @brief �Ѻc�禡�C
    ~CDeviceResetManager();

    /// @brief �إߤ@�ӫ��w���������I�w�İϡC
    /// @param capacity ���I�w�İϯ�e�Ǫ����I�ƶq�C
    /// @param type ���I���� (0/3: GIVertex, 1: AlphaBoxVertex, 2: ImageVertex)�C
    /// @return ���V�s�إߪ� VertexBufferData �`�I�����СC
    VertexBufferData* CreateVertexBuffer(unsigned short capacity, unsigned char type);

    /// @brief �R���@�ӳ��I�w�İϡC
    /// @param pBufferData �n�R�������I�w�İϸ`�I�C
    void DeleteVertexBuffer(VertexBufferData* pBufferData);

    /// @brief �إߤ@�ӹϤ��귽�C
    /// @param pFileName �Ϥ��ɮת����|�Φb�ʸ��ɤ����W�١C
    /// @param flag �ǻ��� LoadGI/LoadGIInPack ���X�СC
    /// @param packerType �ǻ��� LoadGI/LoadGIInPack �����]�������C
    /// @param a5 �ǻ��� LoadGIInPack ���Ѽ�(�q�`�O�ʸ�������)�C
    /// @return ���V�s�إߪ� ImageResourceListData �`�I�����СC
    ImageResourceListData* CreateImageResource(const char* pFileName, char flag, unsigned char packerType, int a5);

    /// @brief �R���@�ӹϤ��귽�C
    /// @param pImageNode �n�R�����Ϥ��귽�`�I�C
    void DeleteImageResource(ImageResourceListData* pImageNode);

    /// @brief �q�ɮ׫إߤ@�ӯ��z�C
    /// @param pFileName ���z�ɮת����|�C
    /// @param flag ��l�X�����X�СC
    /// @return ���V�s�إߪ� TextureListData �`�I�����СC
    TextureListData* CreateTexture(const char* pFileName, unsigned char flag);

    /// @brief �R���@�ӯ��z�C
    /// @param pTextureNode �n�R�������z�`�I�C
    void DeleteTexture(TextureListData* pTextureNode);

    /// @brief ���o�@�ɪ� ID3DXSprite ����A�p�G���s�b�h�إߤ@�ӡC
    /// @return ���V ID3DXSprite ���󪺫��СC
    ID3DXSprite* GetSpriteObject();

    /// @brief �B�z�˸m���]�C
    /// @param hresult �Ӧ� Present() �Ψ�L D3D �I�s����^�X�C
    /// @return �p�G�˸m���A���`�Τw���\���]�A��^ true�C
    bool ResetToDevice(long hresult);

private:
    /// @brief �p���غc�禡�A����~�������إߡC
    CDeviceResetManager();

    // �R�������غc�禡�M��ȹB��l�A�T�O��Ҫ��ߤ@��
    CDeviceResetManager(const CDeviceResetManager&) = delete;
    CDeviceResetManager& operator=(const CDeviceResetManager&) = delete;

private:
    // ���V�ߤ@��Ҫ��R�A����
    static CDeviceResetManager* s_pInstance;

    // �����ܼ� (���ǩM�j�p�P�ϽsĶ�X�@�P)
    VertexBufferDataMgr      m_vertexBufferMgr;      // �첾: +0
    ImageResourceListDataMgr m_imageResourceMgr;     // �첾: +12
    TextureListDataMgr       m_textureMgr;           // �첾: +24
    ID3DXSprite* m_pSprite;              // �첾: +36
};