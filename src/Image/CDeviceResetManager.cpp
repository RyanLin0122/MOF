#include "Image/CDeviceResetManager.h"
#include "Image/GIVertex.h" // ���] GIVertex �Ψ�L���I�榡�� FVF �w�q�b��

// ���]���~�������ܼ�
extern LPDIRECT3DDEVICE9 Device;            // ���� Direct3D �˸m����
extern int IsInMemory;                     // ����O�q�W���ɮ��٬O�ʸ��ɸ��J
extern bool IsDialogBoxMode = 0;                  // ����O�_�]�w DialogBoxMode
extern D3DPRESENT_PARAMETERS g_d3dpp; // �ŧi g_d3dpp �O�b�O�B�w�q��

// ���U�禡�A�Ω�w���a���� COM ����
template<typename T>
void SafeRelease(T*& p) {
    if (p) {
        p->Release();
        p = nullptr;
    }
}

// ��l���R�A��������
CDeviceResetManager* CDeviceResetManager::s_pInstance = nullptr;

// �R�A GetInstance ��k����{
CDeviceResetManager* CDeviceResetManager::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) CDeviceResetManager();
    }
    return s_pInstance;
}

CDeviceResetManager::CDeviceResetManager() {
    // �غc�禡�|�۰ʩI�s�����ܼƪ��غc�禡
    // �N Sprite ���Ъ�l�Ƭ���
    m_pSprite = nullptr;
}

CDeviceResetManager::~CDeviceResetManager() {
    // ���� Sprite ����
    SafeRelease(m_pSprite);
    // �����޲z�����Ѻc�禡�|�Q�۰ʩI�s
}

VertexBufferData* CDeviceResetManager::CreateVertexBuffer(unsigned short capacity, unsigned char type) {
    // �ھڤϽsĶ�{���X�A�o�̬ٲ��F�� m_vertexBufferMgr.Add() ���I�s�A
    // �]���ڭ̨S���䧹���{�C�H�U�{���X���b�i�ܨ�֤��޿�C
    VertexBufferData* pNewNode = m_vertexBufferMgr.Add();
    if (!pNewNode) return nullptr;

    HRESULT hr = E_FAIL;

    // �ھ�������ܤ��P�����I�j�p�MFVF�ӫإ�VertexBuffer
    switch (type) {
    case 1: // AlphaBoxVertex
        //hr = Device->CreateVertexBuffer(20 * capacity, 8, AlphaBoxVertex::FVF, D3DPOOL_MANAGED, &pNewNode->pVertexBuffer, nullptr);
        break;
    case 2: // ImageVertex
        //hr = Device->CreateVertexBuffer(24 * capacity, 8, ImageVertex::FVF, D3DPOOL_MANAGED, &pNewNode->pVertexBuffer, nullptr);
        break;
    case 0:
    case 3: // GIVertex
        //hr = Device->CreateVertexBuffer(28 * capacity, 8, GIVertex::FVF, D3DPOOL_MANAGED, &pNewNode->pVertexBuffer, nullptr);
        break;
    default:
        // ���䴩������
        m_vertexBufferMgr.Delete(pNewNode);
        return nullptr;
    }

    if (SUCCEEDED(hr)) {
        pNewNode->capacity = capacity;
        pNewNode->type = type;
        return pNewNode;
    }

    m_vertexBufferMgr.Delete(pNewNode);
    return nullptr;
}

void CDeviceResetManager::DeleteVertexBuffer(VertexBufferData* pBufferData) {
    m_vertexBufferMgr.Delete(pBufferData); //
}

ImageResourceListData* CDeviceResetManager::CreateImageResource(const char* pFileName, char flag, unsigned char packerType, int a5) {
    ImageResourceListData* pNewNode = m_imageResourceMgr.Add(); //
    if (!pNewNode) return nullptr;

    // �ƻs�ɮצW�٩M�X��
    strcpy_s(pNewNode->m_szFileName, sizeof(pNewNode->m_szFileName), pFileName);
    pNewNode->m_cFlag = flag;
    pNewNode->m_ucPackerType = packerType;

    bool bSuccess = false;
    // �ھڥ���X�ШM�w���J�覡
    if (IsInMemory) {
        bSuccess = pNewNode->m_Resource.LoadGIInPack(pNewNode->m_szFileName, a5, packerType); //
    }
    else {
        bSuccess = pNewNode->m_Resource.LoadGI(pNewNode->m_szFileName, packerType); //
    }

    if (!bSuccess) {
        m_imageResourceMgr.Delete(pNewNode);
        return nullptr;
    }

    return pNewNode;
}

void CDeviceResetManager::DeleteImageResource(ImageResourceListData* pImageNode) {
    m_imageResourceMgr.Delete(pImageNode); //
}

TextureListData* CDeviceResetManager::CreateTexture(const char* pFileName, unsigned char flag) {
    TextureListData* pNewNode = m_textureMgr.Add(); //
    if (!pNewNode) return nullptr;

    strcpy_s(pNewNode->szFileName, sizeof(pNewNode->szFileName), pFileName); //
    pNewNode->flag = flag; //

    // �I�sD3DX�禡�q�ɮ׫إ߯��z
    HRESULT hr = D3DXCreateTextureFromFileExA(
        Device, pFileName, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT, 0,
        D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
        0xFFFF00FF, nullptr, nullptr, &pNewNode->pTexture);

    if (FAILED(hr)) {
        m_textureMgr.Delete(pNewNode);
        return nullptr;
    }

    return pNewNode;
}

void CDeviceResetManager::DeleteTexture(TextureListData* pTextureNode) {
    m_textureMgr.Delete(pTextureNode); //
}

ID3DXSprite* CDeviceResetManager::GetSpriteObject() {
    // �p�G Sprite ����|���إߡA�h�إߥ�
    if (!m_pSprite) {
        D3DXCreateSprite(Device, &m_pSprite); //
    }
    return m_pSprite;
}

bool CDeviceResetManager::ResetToDevice(long hresult) {
    // �ˬd�O�_�ݭn���]�˸m�CD3DERR_DEVICENOTRESET ���Ȭ� -2005530519�C
    if (hresult >= 0 || Device->TestCooperativeLevel() != D3DERR_DEVICENOTRESET) {
        return true;
    }

    // �˸m�򥢡A�q�� Sprite ����
    if (m_pSprite) {
        m_pSprite->OnLostDevice();
    }

    // ���]�˸m
    if (FAILED(Device->Reset(&g_d3dpp))) {
        return false;
    }

    // �˸m�w���]�A�q�� Sprite ����
    if (m_pSprite) {
        m_pSprite->OnResetDevice();
    }

    // ���s�]�w�˸m���U�ش�V���A
    Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    Device->SetRenderState(D3DRS_DITHERENABLE, FALSE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    Device->SetRenderState(D3DRS_FOGENABLE, FALSE);
    Device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
    Device->SetRenderState(D3DRS_ALPHAREF, 0);
    Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

    // �]�w���z���A
    Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    // �]�w�ļ˾����A
    Device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    Device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

    // �ھںX�г]�w��ܤ���Ҧ�
    if (!IsDialogBoxMode) {
        Device->SetDialogBoxMode(TRUE);
    }

    return true;
}