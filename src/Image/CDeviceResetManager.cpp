#include "Image/CDeviceResetManager.h"
#include "Image/GIVertex.h" // 假設 GIVertex 及其他頂點格式的 FVF 定義在此

// 假設的外部全域變數
extern LPDIRECT3DDEVICE9 Device;            // 全域 Direct3D 裝置指標
extern int IsInMemory;                     // 控制是從獨立檔案還是封裝檔載入
extern bool IsDialogBoxMode = 0;                  // 控制是否設定 DialogBoxMode
extern D3DPRESENT_PARAMETERS g_d3dpp; // 宣告 g_d3dpp 是在別處定義的

// 輔助函式，用於安全地釋放 COM 物件
template<typename T>
void SafeRelease(T*& p) {
    if (p) {
        p->Release();
        p = nullptr;
    }
}

// 初始化靜態成員指標
CDeviceResetManager* CDeviceResetManager::s_pInstance = nullptr;

// 靜態 GetInstance 方法的實現
CDeviceResetManager* CDeviceResetManager::GetInstance() {
    if (s_pInstance == nullptr) {
        s_pInstance = new (std::nothrow) CDeviceResetManager();
    }
    return s_pInstance;
}

CDeviceResetManager::CDeviceResetManager() {
    // 建構函式會自動呼叫成員變數的建構函式
    // 將 Sprite 指標初始化為空
    m_pSprite = nullptr;
}

CDeviceResetManager::~CDeviceResetManager() {
    // 釋放 Sprite 物件
    SafeRelease(m_pSprite);
    // 成員管理器的解構函式會被自動呼叫
}

VertexBufferData* CDeviceResetManager::CreateVertexBuffer(unsigned short capacity, unsigned char type) {
    // 根據反編譯程式碼，這裡省略了對 m_vertexBufferMgr.Add() 的呼叫，
    // 因為我們沒有其完整實現。以下程式碼旨在展示其核心邏輯。
    VertexBufferData* pNewNode = m_vertexBufferMgr.Add();
    if (!pNewNode) return nullptr;

    HRESULT hr = E_FAIL;

    // 根據類型選擇不同的頂點大小和FVF來建立VertexBuffer
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
        // 不支援的類型
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

    // 複製檔案名稱和旗標
    strcpy_s(pNewNode->m_szFileName, sizeof(pNewNode->m_szFileName), pFileName);
    pNewNode->m_cFlag = flag;
    pNewNode->m_ucPackerType = packerType;

    bool bSuccess = false;
    // 根據全域旗標決定載入方式
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

void CDeviceResetManager::DeleteTexture(TextureListData* pTextureNode) {
    m_textureMgr.Delete(pTextureNode); //
}

ID3DXSprite* CDeviceResetManager::GetSpriteObject() {
    // 如果 Sprite 物件尚未建立，則建立它
    if (!m_pSprite) {
        D3DXCreateSprite(Device, &m_pSprite); //
    }
    return m_pSprite;
}

bool CDeviceResetManager::ResetToDevice(long hresult) {
    // 檢查是否需要重設裝置。D3DERR_DEVICENOTRESET 的值為 -2005530519。
    if (hresult >= 0 || Device->TestCooperativeLevel() != D3DERR_DEVICENOTRESET) {
        return true;
    }

    // 裝置遺失，通知 Sprite 物件
    if (m_pSprite) {
        m_pSprite->OnLostDevice();
    }

    // 重設裝置
    if (FAILED(Device->Reset(&g_d3dpp))) {
        return false;
    }

    // 裝置已重設，通知 Sprite 物件
    if (m_pSprite) {
        m_pSprite->OnResetDevice();
    }

    // 重新設定裝置的各種渲染狀態
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

    // 設定紋理狀態
    Device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

    // 設定採樣器狀態
    Device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    Device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    Device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    Device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    Device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

    // 根據旗標設定對話方塊模式
    if (!IsDialogBoxMode) {
        Device->SetDialogBoxMode(TRUE);
    }

    return true;
}