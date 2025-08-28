#include "Image/CDeviceResetManager.h"
#include <string>

// --- 假設的外部全域變數 ---
// 這些變數是在原始程式的其他地方定義的，但CDeviceResetManager依賴它們。
extern IDirect3DDevice9* Device;          // 全域 Direct3D 裝置指標
extern D3DPRESENT_PARAMETERS g_d3dpp;           // 全域 D3D 呈現參數
extern int IsInMemory;              // 控制是從獨立檔案還是封裝檔載入 (IsInMemory)
extern bool IsDialogBoxMode;     // 控制是否設定 DialogBoxMode (IsDialogBoxMode)

// --- 假設的頂點FVF定義 ---
// 這些是根據CreateVertexBuffer中的邏輯推斷的
namespace AlphaBoxVertex { const unsigned int FVF = D3DFVF_DIFFUSE | D3DFVF_XYZRHW; }
namespace ImageVertex { const unsigned int FVF = D3DFVF_TEX1 | D3DFVF_XYZRHW; }
namespace GIVertex { const unsigned int FVF = D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_XYZRHW; }

// --- 靜態成員初始化 ---
CDeviceResetManager* CDeviceResetManager::s_pInstance = nullptr;

// 對應反編譯碼: 0x00544D50
CDeviceResetManager::CDeviceResetManager() {
    // 建構函式會自動呼叫成員變數的建構函式
    // m_vertexBufferMgr, m_imageResourceMgr, m_textureMgr
    m_pSprite = nullptr; // 將 Sprite 指標初始化為空
}

// 對應反編譯碼: 0x00544D80
CDeviceResetManager::~CDeviceResetManager() {
    // 釋放 Sprite 物件
    SafeRelease(m_pSprite);
    // m_vertexBufferMgr, m_imageResourceMgr, m_textureMgr 的解構函式會被自動呼叫
}

// 靜態 GetInstance 方法的實現
CDeviceResetManager* CDeviceResetManager::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) CDeviceResetManager();
    }
    return s_pInstance;
}

// 對應反編譯碼: 0x00544E00
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
        return nullptr; // 不支援的類型
    }

    hr = Device->CreateVertexBuffer(vertexSize * capacity, 8 /*D3DUSAGE_WRITEONLY*/, fvf, D3DPOOL_MANAGED, &pNewNode->pVertexBuffer, nullptr);

    if (SUCCEEDED(hr)) {
        pNewNode->capacity = capacity;
        pNewNode->type = type;
        return pNewNode;
    }

    m_vertexBufferMgr.Delete(pNewNode); // 建立失敗則刪除節點
    return nullptr;
}

// 對應反編譯碼: 0x00544F40
void CDeviceResetManager::DeleteVertexBuffer(VertexBufferData* pBufferData) {
    m_vertexBufferMgr.Delete(pBufferData);
}

// 對應反編譯碼: 0x00544F50
ImageResourceListData* CDeviceResetManager::CreateImageResource(const char* pFileName, char flag, unsigned char packerType, int a5) {
    ImageResourceListData* pNewNode = m_imageResourceMgr.Add();
    if (!pNewNode) return nullptr;

    // 複製檔案名稱和旗標
    strcpy_s(pNewNode->m_szFileName, sizeof(pNewNode->m_szFileName), pFileName);
    pNewNode->m_cFlag = flag;
    pNewNode->m_ucPackerType = packerType;

    bool bSuccess = false;
    // 根據全域旗標決定載入方式
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

// 對應反編譯碼: 0x00544FE0
void CDeviceResetManager::DeleteImageResource(ImageResourceListData* pImageNode) {
    m_imageResourceMgr.Delete(pImageNode);
}

// 對應反編譯碼: 0x00544FF0
TextureListData* CDeviceResetManager::CreateTexture(const char* pFileName, unsigned char flag) {
    TextureListData* pNewNode = m_textureMgr.Add();
    if (!pNewNode) return nullptr;

    strcpy_s(pNewNode->szFileName, sizeof(pNewNode->szFileName), pFileName);
    pNewNode->flag = flag;

    // 呼叫D3DX函式從檔案建立紋理
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

// 對應反編譯碼: 0x00545060
void CDeviceResetManager::DeleteTexture(TextureListData* pTextureNode) {
    m_textureMgr.Delete(pTextureNode);
}

// 對應反編譯碼: 0x00545070
ID3DXSprite* CDeviceResetManager::GetSpriteObject() {
    if (!m_pSprite) {
        if (FAILED(D3DXCreateSprite(Device, &m_pSprite))) {
            m_pSprite = nullptr; // 確保建立失敗時指標為空
        }
    }
    return m_pSprite;
}

// 對應反編譯碼: 0x005450A0
bool CDeviceResetManager::ResetToDevice(long hresult) {
    // 檢查是否需要重設裝置。D3DERR_DEVICENOTRESET 的值為 -2005530519。
    if (hresult >= 0 || Device->TestCooperativeLevel() != D3DERR_DEVICENOTRESET) {
        return true;
    }

    // --- 裝置遺失(Lost)階段 ---
    if (m_pSprite) {
        m_pSprite->OnLostDevice();
    }

    // --- 重設(Reset)階段 ---
    if (FAILED(Device->Reset(&g_d3dpp))) {
        return false; // 重設失敗
    }

    // --- 重設成功後階段 ---
    if (m_pSprite) {
        m_pSprite->OnResetDevice();
    }

    // 重新設定裝置的各種渲染狀態
    Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // 原始碼設為1(D3DCULL_CW)，但D3DCULL_NONE(1)才是常見值
    Device->SetRenderState(D3DRS_LIGHTING, FALSE);
    Device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
    Device->SetRenderState(D3DRS_DITHERENABLE, FALSE);
    Device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    Device->SetRenderState(D3DRS_FOGENABLE, FALSE);

    Device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
    Device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL); // 原始碼設為6(D3DCMP_NOTEQUAL)
    Device->SetRenderState(D3DRS_ALPHAREF, 0);
    Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA); // 5
    Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA); // 6

    // 設定紋理狀態
    Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE); // 4
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); // 2
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); // 0, DIFFUSE

    // 設定採樣器狀態
    Device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP); // 3
    Device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP); // 3
    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT); // 1, 不是常見的 D3DTEXF_LINEAR
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT); // 1
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);  // 0, 不是常見的 D3DTEXF_LINEAR

    // 根據旗標設定對話方塊模式
    if (!IsDialogBoxMode) { // IsDialogBoxMode
        Device->SetDialogBoxMode(TRUE);
    }

    return true;
}