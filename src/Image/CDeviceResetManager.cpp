#include "Image/CDeviceResetManager.h"
#include <string>

// --- ���]���~�������ܼ� ---
// �o���ܼƬO�b��l�{������L�a��w�q���A��CDeviceResetManager�̿०�̡C
extern IDirect3DDevice9* Device;          // ���� Direct3D �˸m����
extern D3DPRESENT_PARAMETERS g_d3dpp;           // ���� D3D �e�{�Ѽ�
extern int IsInMemory;              // ����O�q�W���ɮ��٬O�ʸ��ɸ��J (IsInMemory)
extern bool IsDialogBoxMode;     // ����O�_�]�w DialogBoxMode (IsDialogBoxMode)

// --- ���]�����IFVF�w�q ---
// �o�ǬO�ھ�CreateVertexBuffer�����޿���_��
namespace AlphaBoxVertex { const unsigned int FVF = D3DFVF_DIFFUSE | D3DFVF_XYZRHW; }
namespace ImageVertex { const unsigned int FVF = D3DFVF_TEX1 | D3DFVF_XYZRHW; }
namespace GIVertex { const unsigned int FVF = D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW; }

// --- �R�A������l�� ---
CDeviceResetManager* CDeviceResetManager::s_pInstance = nullptr;

// �����ϽsĶ�X: 0x00544D50
CDeviceResetManager::CDeviceResetManager() {
    // �غc�禡�|�۰ʩI�s�����ܼƪ��غc�禡
    // m_vertexBufferMgr, m_imageResourceMgr, m_textureMgr
    m_pSprite = nullptr; // �N Sprite ���Ъ�l�Ƭ���
}

// �����ϽsĶ�X: 0x00544D80
CDeviceResetManager::~CDeviceResetManager() {
    // ���� Sprite ����
    SafeRelease(m_pSprite);
    // m_vertexBufferMgr, m_imageResourceMgr, m_textureMgr ���Ѻc�禡�|�Q�۰ʩI�s
}

// �R�A GetInstance ��k����{
CDeviceResetManager* CDeviceResetManager::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) CDeviceResetManager();
    }
    return s_pInstance;
}

// �����ϽsĶ�X: 0x00544E00
VertexBufferData* CDeviceResetManager::CreateVertexBuffer(unsigned short capacity, unsigned char type) {
    VertexBufferData* pNewNode = m_vertexBufferMgr.Add();
    if (!pNewNode) return nullptr;

    HRESULT hr = E_FAIL;
    unsigned int vertexSize = 0;
    unsigned int fvf = 0;

    switch (type) {
    case 1: // AlphaBoxVertex
        vertexSize = 20;
        fvf = AlphaBoxVertex::FVF;
        break;
    case 2: // ImageVertex
        vertexSize = 24;
        fvf = ImageVertex::FVF;
        break;
    case 0:
    case 3: // GIVertex
        vertexSize = 28;
        fvf = GIVertex::FVF;
        break;
    default:
        m_vertexBufferMgr.Delete(pNewNode);
        return nullptr; // ���䴩������
    }

    hr = Device->CreateVertexBuffer(vertexSize * capacity, 8 /*D3DUSAGE_WRITEONLY*/, fvf, D3DPOOL_MANAGED, &pNewNode->pVertexBuffer, nullptr);

    if (SUCCEEDED(hr)) {
        pNewNode->capacity = capacity;
        pNewNode->type = type;
        return pNewNode;
    }

    m_vertexBufferMgr.Delete(pNewNode); // �إߥ��ѫh�R���`�I
    return nullptr;
}

// �����ϽsĶ�X: 0x00544F40
void CDeviceResetManager::DeleteVertexBuffer(VertexBufferData* pBufferData) {
    m_vertexBufferMgr.Delete(pBufferData);
}

// �����ϽsĶ�X: 0x00544F50
ImageResourceListData* CDeviceResetManager::CreateImageResource(const char* pFileName, char flag, unsigned char packerType, int a5) {
    ImageResourceListData* pNewNode = m_imageResourceMgr.Add();
    if (!pNewNode) return nullptr;

    // �ƻs�ɮצW�٩M�X��
    strcpy_s(pNewNode->m_szFileName, sizeof(pNewNode->m_szFileName), pFileName);
    pNewNode->m_cFlag = flag;
    pNewNode->m_ucPackerType = packerType;

    bool bSuccess = false;
    // �ھڥ���X�ШM�w���J�覡
    if (IsInMemory) { // IsInMemory
        bSuccess = pNewNode->m_Resource.LoadGIInPack(pNewNode->m_szFileName, a5, packerType);
    }
    else {
        bSuccess = pNewNode->m_Resource.LoadGI(pNewNode->m_szFileName, packerType);
    }

    if (!bSuccess) {
        m_imageResourceMgr.Delete(pNewNode);
        return nullptr;
    }

    return pNewNode;
}

// �����ϽsĶ�X: 0x00544FE0
void CDeviceResetManager::DeleteImageResource(ImageResourceListData* pImageNode) {
    m_imageResourceMgr.Delete(pImageNode);
}

// �����ϽsĶ�X: 0x00544FF0
TextureListData* CDeviceResetManager::CreateTexture(const char* pFileName, unsigned char flag) {
    TextureListData* pNewNode = m_textureMgr.Add();
    if (!pNewNode) return nullptr;

    strcpy_s(pNewNode->szFileName, sizeof(pNewNode->szFileName), pFileName);
    pNewNode->flag = flag;

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

// �����ϽsĶ�X: 0x00545060
void CDeviceResetManager::DeleteTexture(TextureListData* pTextureNode) {
    m_textureMgr.Delete(pTextureNode);
}

// �����ϽsĶ�X: 0x00545070
ID3DXSprite* CDeviceResetManager::GetSpriteObject() {
    if (!m_pSprite) {
        if (FAILED(D3DXCreateSprite(Device, &m_pSprite))) {
            m_pSprite = nullptr; // �T�O�إߥ��Ѯɫ��Ь���
        }
    }
    return m_pSprite;
}

// �����ϽsĶ�X: 0x005450A0
bool CDeviceResetManager::ResetToDevice(long hresult) {
    // �ˬd�O�_�ݭn���]�˸m�CD3DERR_DEVICENOTRESET ���Ȭ� -2005530519�C
    if (hresult >= 0 || Device->TestCooperativeLevel() != D3DERR_DEVICENOTRESET) {
        return true;
    }

    // --- �˸m��(Lost)���q ---
    if (m_pSprite) {
        m_pSprite->OnLostDevice();
    }

    // --- ���](Reset)���q ---
    if (FAILED(Device->Reset(&g_d3dpp))) {
        return false; // ���]����
    }

    // --- ���]���\�ᶥ�q ---
    if (m_pSprite) {
        m_pSprite->OnResetDevice();
    }

    // ���s�]�w�˸m���U�ش�V���A
    Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // ��l�X�]��1(D3DCULL_CW)�A��D3DCULL_NONE(1)�~�O�`����
    Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    Device->SetRenderState(D3DRS_DITHERENABLE, FALSE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    Device->SetRenderState(D3DRS_FOGENABLE, FALSE);

    Device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL); // ��l�X�]��6(D3DCMP_NOTEQUAL)
    Device->SetRenderState(D3DRS_ALPHAREF, 0);
    Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // 5
    Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); // 6

    // �]�w���z���A
    Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE); // 4
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); // 2
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); // 0, DIFFUSE

    // �]�w�ļ˾����A
    Device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP); // 3
    Device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP); // 3
    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT); // 1, ���O�`���� D3DTEXF_LINEAR
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT); // 1
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);  // 0, ���O�`���� D3DTEXF_LINEAR

    // �ھںX�г]�w��ܤ���Ҧ�
    if (!IsDialogBoxMode) { // IsDialogBoxMode
        Device->SetDialogBoxMode(TRUE);
    }

    return true;
}