#include "UI/CControlAlphaBox.h"
#include "Image/CDeviceResetManager.h"
#include "Image/CDeviceManager.h"
#include <d3dx9.h>
#include <d3d9.h> // 假設使用 D3D9

/**
 * CControlAlphaBox 建構函式
 * 初始化頂點資料和成員變數。
 */
CControlAlphaBox::CControlAlphaBox()
    : m_pVertexBuffer(nullptr), m_bIsCreated(false)
{
    // 初始化頂點預設值
    for (int i = 0; i < 4; ++i)
    {
        m_vertices[i].x = 0.0f;
        m_vertices[i].y = 0.0f;
        m_vertices[i].z = 0.5f;   // 深度值
        m_vertices[i].rhw = 1.0f; // 倒數 W，用於 2D 渲染
    }
    SetColor(1.0f, 1.0f, 1.0f, 1.0f); // 預設為不透明白色
}

/**
 * CControlAlphaBox 解構函式
 * 釋放 DirectX Vertex Buffer 資源。
 */
CControlAlphaBox::~CControlAlphaBox()
{
    if (m_pVertexBuffer)
    {
        CDeviceResetManager::GetInstance()->DeleteVertexBuffer(m_pVertexBuffer);
        m_pVertexBuffer = nullptr;
    }
}

/**
 * 創建控制項並初始化繪圖資源。
 */
void CControlAlphaBox::Create(int nPosX, int nPosY, unsigned short usWidth, unsigned short usHeight, CControlBase* pParent)
{
    if (!m_bIsCreated)
    {
        m_bIsCreated = true;
        // 創建一個可容納 4 個頂點的 Vertex Buffer
        m_pVertexBuffer = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4, sizeof(D3DVertex));
        if (m_pVertexBuffer)
        {
            CControlBase::Create(nPosX, nPosY, usWidth, usHeight, pParent);
        }
    }
}

void CControlAlphaBox::Create(CControlBase* pParent)
{
    if (!m_bIsCreated)
    {
        m_bIsCreated = true;
        m_pVertexBuffer = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4, sizeof(D3DVertex));
        if (m_pVertexBuffer)
        {
            CControlBase::Create(pParent);
        }
    }
}

void CControlAlphaBox::Create(int nPosX, int nPosY, unsigned short usWidth, unsigned short usHeight, float r, float g, float b, float a, CControlBase* pParent)
{
    if (!m_bIsCreated)
    {
        m_bIsCreated = true;
        m_pVertexBuffer = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4, sizeof(D3DVertex));
        if (m_pVertexBuffer)
        {
            SetColor(r, g, b, a);
            CControlBase::Create(nPosX, nPosY, usWidth, usHeight, pParent);
        }
    }
}


/**
 * 準備繪製。
 * 計算方塊的四個頂點在螢幕上的最終座標，並將這些資料寫入 Vertex Buffer。
 */
void CControlAlphaBox::PrepareDrawing()
{
    // 假設 byte_21CB35D 是一個全域的繪製開關
    // if (byte_21CB35D) return;

    if (m_pVertexBuffer /*&& m_pVertexBuffer->pVB*/)
    {
        int absPos[2];
        GetAbsPos((float*)absPos);
        
        float left = static_cast<float>(absPos[0]) - 0.5f;
        float top = static_cast<float>(absPos[1]) - 0.5f;
        float right = left + static_cast<float>(m_usWidth);
        float bottom = top + static_cast<float>(m_usHeight);

        // 更新頂點座標
        m_vertices[0].x = left;  m_vertices[0].y = top;
        m_vertices[1].x = right; m_vertices[1].y = top;
        m_vertices[2].x = right; m_vertices[2].y = bottom;
        m_vertices[3].x = left;  m_vertices[3].y = bottom;
        
        // 將頂點資料鎖定並複製到硬體緩衝區
        void* pVertices = nullptr;
        // if (SUCCEEDED(m_pVertexBuffer->pVB->Lock(0, sizeof(m_vertices), &pVertices, 0)))
        // {
        //     memcpy(pVertices, m_vertices, sizeof(m_vertices));
        //     m_pVertexBuffer->pVB->Unlock();
        // }
    }
    // 呼叫基底類別的函式以準備子控制項
    CControlBase::PrepareDrawing();
}

/**
 * 執行繪製。
 * 設定渲染狀態，並命令 GPU 繪製方塊。
 */
void CControlAlphaBox::Draw()
{
    // if (byte_21CB35D) return;

    if (m_pVertexBuffer /*&& m_pVertexBuffer->pVB && Device*/)
    {
        // 1. 設定渲染狀態：關閉貼圖
        CDeviceManager::GetInstance()->SetTexture(0, nullptr);
        
        // 2. 綁定 Vertex Buffer
        // CDeviceManager::SetStreamSource(0, m_pVertexBuffer->pVB, 0, sizeof(D3DVertex));
        
        // 3. 設定頂點格式 (Flexible Vertex Format)
        // D3DFVF_XYZRHW | D3DFVF_DIFFUSE 的值為 0x44
        // CDeviceManager::SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
        
        // 4. 發出繪製命令，使用 Triangle Fan 繪製一個矩形 (2個三角形)
        // Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
    }
    // 呼叫基底類別的函式以繪製子控制項
    CControlBase::Draw();
}

/**
 * 將 RGBA 浮點數顏色轉換為 ARGB 的 DWORD 格式。
 */
unsigned long FloatToDWORDColor(float r, float g, float b, float a)
{
    auto ToByte = [](float val) -> unsigned char {
        if (val >= 1.0f) return 255;
        if (val <= 0.0f) return 0;
        return static_cast<unsigned char>(val * 255.0f + 0.5f);
    };
    return (ToByte(a) << 24) | (ToByte(r) << 16) | (ToByte(g) << 8) | ToByte(b);
}

/**
 * 設定方塊的單一顏色。
 */
void CControlAlphaBox::SetColor(float r, float g, float b, float a)
{
    unsigned long color = FloatToDWORDColor(r, g, b, a);
    m_vertices[0].color = color;
    m_vertices[1].color = color;
    m_vertices[2].color = color;
    m_vertices[3].color = color;
}

/**
 * 設定方塊的四角漸層顏色。
 */
void CControlAlphaBox::SetColor(float r1, float g1, float b1, float a1, float r2, float g2, float b2, float a2, float r3, float g3, float b3, float a3, float r4, float g4, float b4, float a4)
{
    m_vertices[0].color = FloatToDWORDColor(r1, g1, b1, a1);
    m_vertices[1].color = FloatToDWORDColor(r2, g2, b2, a2);
    m_vertices[2].color = FloatToDWORDColor(r3, g3, b3, a3);
    m_vertices[3].color = FloatToDWORDColor(r4, g4, b4, a4);
}

/**
 * 單獨設定透明度。
 * @param alpha 0 (完全透明) 到 255 (完全不透明)。
 */
void CControlAlphaBox::SetAlpha(unsigned char alpha)
{
    for (int i = 0; i < 4; ++i)
    {
        // 清除原有的 Alpha 值，並設定新的 Alpha 值
        m_vertices[i].color = (m_vertices[i].color & 0x00FFFFFF) | (alpha << 24);
    }
}

/**
 * 快速設定所有屬性。
 */
void CControlAlphaBox::SetAttr(int nPosX, int nPosY, short usWidth, short usHeight, float r, float g, float b, float a)
{
    SetAbsPos(nPosX, nPosY);
    m_usWidth = usWidth;
    m_usHeight = usHeight;
    SetColor(r, g, b, a);
}

/**
 * 將方塊大小設定為與父物件相同。
 */
void CControlAlphaBox::SetRectInParent()
{
    if(m_pParent)
    {
        m_usWidth = m_pParent->GetWidth();
        m_usHeight = m_pParent->GetHeight();
    }
}
