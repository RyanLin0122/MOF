#include "Image/Image.h"
#include "Image/CDeviceManager.h"       // 假設 CDeviceManager 的定義在此

// --- 外部依賴的全域變數 ---
extern LPDIRECT3DDEVICE9    Device;
extern bool                 DontDraw; // 全域的 "不要繪製" 旗標

Image::Image()
{
    // 初始化指標和旗標
    m_pVBData = nullptr;
    m_pTexData = nullptr;
    m_bIsCreated = false;

    // 初始化成員變數
    m_nGridX = 0;
    m_fFrameWidth = 0.0f;
    m_fFrameHeight = 0.0f;
    m_fGridWidth = 0.0f;
    m_fGridHeight = 0.0f;
    m_nWord_28 = 0;
    g_Device_Reset_Manager = CDeviceResetManager::GetInstance();
    // 初始化頂點陣列和基礎頂點陣列
    for (int i = 0; i < 4; ++i)
    {
        m_vertices[i] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
        m_baseVertices[i] = { 0.0f, 0.0f, 0.0f };
    }
}

Image::~Image()
{
    // 在解構時呼叫 Free 來釋放資源
    this->Free();
}

bool Image::CreateImage(const char* szFilename, int texWidth, int texHeight, int frameWidth, int frameHeight)
{
    // 設定尺寸和格線資訊
    m_fFrameWidth = (float)frameWidth;
    m_fFrameHeight = (float)frameHeight;
    m_nGridX = texWidth / frameWidth;
    m_fGridWidth = (float)frameWidth / (float)texWidth;
    m_fGridHeight = (float)frameHeight / (float)texHeight;

    // 建立頂點緩衝區和紋理
    m_pVBData = g_Device_Reset_Manager->CreateVertexBuffer(4, 2); // 類型 2 對應 ImageVertex
    m_pTexData = g_Device_Reset_Manager->CreateTexture(szFilename, 0);

    if (!m_pVBData || !m_pTexData)
    {
        Free(); // 如果任一項建立失敗，則清理所有資源
        return false;
    }

    // 計算用於旋轉和定位的基礎頂點（以 (0,0) 為中心）
    float halfW = m_fFrameWidth * 0.5f;
    float halfH = m_fFrameHeight * 0.5f;
    m_baseVertices[0] = { -halfW, -halfH, 0.0f }; // 左上
    m_baseVertices[1] = { halfW, -halfH, 0.0f }; // 右上
    m_baseVertices[2] = { halfW,  halfH, 0.0f }; // 右下
    m_baseVertices[3] = { -halfW,  halfH, 0.0f }; // 左下

    m_bIsCreated = true;
    return true;
}

void Image::Free()
{
    // 使用 CDeviceResetManager 來安全地刪除資源
    if (m_pTexData)
    {
        g_Device_Reset_Manager->DeleteTexture(m_pTexData);
        m_pTexData = nullptr;
    }
    if (m_pVBData)
    {
        g_Device_Reset_Manager->DeleteVertexBuffer(m_pVBData);
        m_pVBData = nullptr;
    }
    m_bIsCreated = false;
}

void Image::SetImage(float x, float y, unsigned short frameID)
{
    if (!m_bIsCreated) return;

    // 1. 計算螢幕座標
    // 注意：原始碼中有 -0.5 的偏移，這通常是為了對齊像素中心
    m_vertices[0].x = x - 0.5f;
    m_vertices[0].y = y - 0.5f;
    m_vertices[1].x = x + m_fFrameWidth - 0.5f;
    m_vertices[1].y = y - 0.5f;
    m_vertices[2].x = x + m_fFrameWidth - 0.5f;
    m_vertices[2].y = y + m_fFrameHeight - 0.5f;
    m_vertices[3].x = x - 0.5f;
    m_vertices[3].y = y + m_fFrameHeight - 0.5f;

    // 2. 計算紋理 (UV) 座標
    float tx = (float)(frameID % m_nGridX) * m_fGridWidth;
    float ty = (float)(frameID / m_nGridX) * m_fGridHeight;
    m_vertices[0].u = tx;
    m_vertices[0].v = ty;
    m_vertices[1].u = tx + m_fGridWidth;
    m_vertices[1].v = ty;
    m_vertices[2].u = tx + m_fGridWidth;
    m_vertices[2].v = ty + m_fGridHeight;
    m_vertices[3].u = tx;
    m_vertices[3].v = ty + m_fGridHeight;

    // 3. 將頂點資料更新到硬體緩衝區
    this->Process();
}

void Image::SetPosition(float x, float y)
{
    // 將基礎頂點根據指定位置進行平移，得到最終的螢幕頂點座標
    // 同樣，-0.5f 是為了像素對齊
    for (int i = 0; i < 4; ++i)
    {
        m_vertices[i].x = x + m_baseVertices[i].x - 0.5f;
        m_vertices[i].y = y + m_baseVertices[i].y - 0.5f;
    }
}

void Image::SetAngle(float radians)
{
    D3DXMATRIX matRotation;
    D3DXMatrixRotationZ(&matRotation, radians);

    // 使用旋轉矩陣變換基礎頂點
    // 這裡不能直接修改 m_baseVertices，否則會累積旋轉
    // 原始碼的邏輯是直接變換 m_baseVertices，這可能是一個 bug 或特定設計
    // 此處為忠實還原
    for (int i = 0; i < 4; ++i)
    {
        D3DXVec3TransformCoord(&m_baseVertices[i], &m_baseVertices[i], &matRotation);
    }
}

void Image::DrawImage()
{
    if (!m_bIsCreated || DontDraw || !m_pVBData || !m_pVBData->pVertexBuffer) return;

    // 使用自身的頂點緩衝區來繪製
    this->DrawImage(m_pVBData->pVertexBuffer); // 假設 pVertexBuffer 在 VertexBufferData 中
}

void Image::DrawImage(IDirect3DVertexBuffer9* pVB)
{
    // 如果資源未建立、全域旗標為 true 或沒有紋理，則不執行
    if (!m_bIsCreated || !m_pTexData || !m_pTexData->pTexture) return;

    // 使用全域的 CDeviceManager 來設定渲染狀態並繪製
    // 這還原了反編譯程式碼中的呼叫流程

    // 1. 設定要使用的紋理
    CDeviceManager::GetInstance()->SetTexture(0, m_pTexData->pTexture);

    // 2. 設定頂點串流來源
    CDeviceManager::GetInstance()->SetStreamSource(0, pVB, 0, sizeof(ImageVertex));

    // 3. 設定頂點格式 (FVF)
    // 0x104 代表 D3DFVF_XYZRHW | D3DFVF_TEX1
    CDeviceManager::GetInstance()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

    // 4. 呼叫 Direct3D 的 DrawPrimitive 函式進行繪製
    // D3DPT_TRIANGLEFAN 會使用 4 個頂點繪製一個由兩個三角形組成的矩形
    Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}

void Image::Process()
{
    // 如果資源未建立、全域旗標為 true 或沒有頂點緩衝區，則不執行
    if (!m_bIsCreated || DontDraw || !m_pVBData || !m_pVBData->pVertexBuffer) return;

    // 鎖定頂點緩衝區
    void* pV = nullptr;
    HRESULT hr = m_pVBData->pVertexBuffer->Lock(0, sizeof(m_vertices), &pV, 0);

    if (SUCCEEDED(hr) && pV != nullptr)
    {
        // 將目前的頂點資料 (m_vertices) 複製到硬體緩衝區中
        memcpy(pV, m_vertices, sizeof(m_vertices));
        // 解鎖緩衝區
        m_pVBData->pVertexBuffer->Unlock();
    }
}

void Image::SetLRReverse()
{
    // 這個函式透過交換左右兩側的基礎頂點來實現水平翻轉。
    // 交換 左上(0) 和 右上(1)
    std::swap(m_baseVertices[0], m_baseVertices[1]);
    // 交換 左下(3) 和 右下(2)
    std::swap(m_baseVertices[3], m_baseVertices[2]);
}

void Image::SetBlockID(unsigned short blockID)
{
    if (!m_bIsCreated || m_nGridX == 0) return;

    // 根據區塊 ID (影格編號) 計算紋理的 UV 座標
    float tx = (float)(blockID % m_nGridX) * m_fGridWidth;
    float ty = (float)(blockID / m_nGridX) * m_fGridHeight;

    // 將計算出的 UV 座標設定到最終的頂點資料中
    m_vertices[0].u = tx;
    m_vertices[0].v = ty;
    m_vertices[1].u = tx + m_fGridWidth;
    m_vertices[1].v = ty;
    m_vertices[2].u = tx + m_fGridWidth;
    m_vertices[2].v = ty + m_fGridHeight;
    m_vertices[3].u = tx;
    m_vertices[3].v = ty + m_fGridHeight;
}

void Image::GetVerTextInfo(int* gridX, float* width, float* height, float* gridWidth, float* gridHeight)
{
    // 檢查傳入的指標是否有效，避免程式崩潰
    if (gridX)      *gridX = m_nGridX;
    if (width)      *width = m_fFrameWidth;
    if (height)     *height = m_fFrameHeight;
    if (gridWidth)  *gridWidth = m_fGridWidth;
    if (gridHeight) *gridHeight = m_fGridHeight;
}