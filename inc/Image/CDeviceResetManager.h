#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Image/ImageResourceListDataMgr.h" // �]�t�ڭ̤��e�٭쪺 ImageResource �޲z��
#include <d3dx9tex.h>

//-- ���U���O�M���c��²�Ʃw�q (Stub) -------------------------------------------
// �o�ǬO�ھ� CDeviceResetManager.cpp ���Ϊk���_�X��²�ƪ��w�q

// ���]�� VertexBufferData ���c
struct VertexBufferData {
    VertexBufferData* pPrev;
    VertexBufferData* pNext;
    IDirect3DVertexBuffer9* pVertexBuffer; // �qCreateVertexBuffer���_�A�����q��+8
    unsigned short capacity;               // �����q��+12
    unsigned char type;                    // �����q��+14
};

// ���]�� VertexBufferDataMgr ���O
class VertexBufferDataMgr {
public:
    VertexBufferData* Add() { /* ��@�Ӹ`�ٲ� */ return nullptr; }
    void Delete(VertexBufferData* pNode) { /* ��@�Ӹ`�ٲ� */ }
};

// ���]�� TextureListData ���c
struct TextureListData {
    TextureListData* pPrev;
    TextureListData* pNext;
    IDirect3DTexture9* pTexture;           // �qCreateTexture���_�A�����q��+8
    char szFileName[256];                  // �x�s�ɮ׸��|
    unsigned char flag;                    // �x�s�X��
};

// ���]�� TextureListDataMgr ���O
class TextureListDataMgr {
public:
    TextureListData* Add() { /* ��@�Ӹ`�ٲ� */ return nullptr; }
    void Delete(TextureListData* pNode) { /* ��@�Ӹ`�ٲ� */ }
};

//--------------------------------------------------------------------------------

/// @class CDeviceResetManager
/// @brief �����޲z�] Direct3D �˸m���] (Device Reset) �ӻݭn���s�إߪ��귽�C
///
/// �o�����O�ʸˤF�� Vertex Buffers�B�Ϥ��귽 (ImageResource) �M���z (Texture) ���޲z�A
/// �óB�z�˸m�� (Device Lost) �M���]�ɪ������޿�C
class CDeviceResetManager {
public:
    static CDeviceResetManager* GetInstance();
    /// @brief �Ѻc�禡
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
    /// @param flag �ǻ��� LoadGI/LoadGIInPack ���X�� (��l�X���� a3)�C
    /// @param packerType �ǻ��� LoadGI/LoadGIInPack ���X�� (��l�X���� a4)�C
    /// @param a5 �ǻ��� LoadGIInPack ���Ѽ� (��l�X���� a5)�A�q�`�O�ʸ��������C
    /// @return ���V�s�إߪ� ImageResourceListData �`�I�����СC
    ImageResourceListData* CreateImageResource(const char* pFileName, char flag, unsigned char packerType, int a5);

    /// @brief �R���@�ӹϤ��귽�C
    /// @param pImageNode �n�R�����Ϥ��귽�`�I�C
    void DeleteImageResource(ImageResourceListData* pImageNode);

    /// @brief �q�ɮ׫إߤ@�ӯ��z�C
    /// @param pFileName ���z�ɮת����|�C
    /// @param flag ��l�X�����X�� (a3)�C
    /// @return ���V�s�إߪ� TextureListData �`�I�����СC
    TextureListData* CreateTexture(const char* pFileName, unsigned char flag);

    /// @brief �R���@�ӯ��z�C
    /// @param pTextureNode �n�R�������z�`�I�C
    void DeleteTexture(TextureListData* pTextureNode);

    /// @brief ���o�@�ɪ� ID3DXSprite ����A�p�G���s�b�h�إߤ@�ӡC
    /// @return ���V ID3DXSprite ���󪺫��СC
    ID3DXSprite* GetSpriteObject();

    /// @brief �B�z�˸m���]�C
    /// ��D�j�鰻����˸m�ݭn���]�ɩI�s���禡�C
    /// @param hresult �Ӧ� Present() �Ψ�L D3D �I�s����^�X�C
    /// @return �p�G�˸m���A���`�Τw���\���]�A��^ true�C�p�G���]���ѡA��^ false�C
    bool ResetToDevice(long hresult);

private:
    // �p���غc�禡
    CDeviceResetManager();

    // ���V�ߤ@��Ҫ��R�A����
    static CDeviceResetManager* s_pInstance;
    // �ھڤϽsĶ�{���X���غc�禡�M�����첾�A�٭�X�H�U����
    VertexBufferDataMgr      m_vertexBufferMgr;      // �����q: 0
    ImageResourceListDataMgr m_imageResourceMgr;     // �����q: 12
    TextureListDataMgr       m_textureMgr;           // �����q: 24
    ID3DXSprite* m_pSprite;              // �����q: 36
};