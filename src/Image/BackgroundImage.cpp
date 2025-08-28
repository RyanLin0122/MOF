#include "Image/BackgroundImage.h"
#include "Image/CDeviceManager.h"
#include "Image/CDeviceResetManager.h"
#include <cstring> // for memcpy

// --- 外部依賴的全域變數 ---
extern LPDIRECT3DDEVICE9    Device;
extern bool                 DontDraw; // 全域的 "不要繪製" 旗標

BackgroundImage::BackgroundImage()
{
    // 將所有指標和數值成員初始化為 0 或 nullptr
    Reset();

    // 初始化本地頂點快取
    for (int i = 0; i < 4; ++i)
    {
        // 根據反編譯程式碼的邏輯初始化
        m_imageVertices[i] = { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
        m_giVertices[i] = GIVertex(); // 使用 GIVertex 的預設建構函式
    }
}

BackgroundImage::~BackgroundImage()
{
    // 在解構時釋放 D3D 資源
    if (m_pTexData) {
        CDeviceResetManager::GetInstance()->DeleteTexture(m_pTexData);
    }
    if (m_pVBData) {
        CDeviceResetManager::GetInstance()->DeleteVertexBuffer(m_pVBData);
    }
}

void BackgroundImage::Reset()
{
    // 釋放 D3D 資源
    if (m_pTexData) {
        CDeviceResetManager::GetInstance()->DeleteTexture(m_pTexData);
        m_pTexData = nullptr;
    }
    if (m_pVBData) {
        CDeviceResetManager::GetInstance()->DeleteVertexBuffer(m_pVBData);
        m_pVBData = nullptr;
    }

    // 重設所有數值成員
    m_fTexWidth = 0.0f;
    m_fTexHeight = 0.0f;
    m_fImgWidth = 0.0f;
    m_fImgHeight = 0.0f;
    m_fU_End = 0.0f;
    m_fV_End = 0.0f;
    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_fU_Start = 0.0f;
    m_fV_Start = 0.0f;
}

void BackgroundImage::CreateImage(const char* szFilename, float imgWidth, float imgHeight, float texWidth, float texHeight)
{
    Reset(); // 先清除舊資源

    m_pTexData = CDeviceResetManager::GetInstance()->CreateTexture(szFilename, 0);
    m_pVBData = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4, 2); // 類型 2: ImageVertex

    m_fImgWidth = imgWidth;
    m_fImgHeight = imgHeight;
    m_fTexWidth = texWidth;
    m_fTexHeight = texHeight;

    m_fU_Start = 0.0f;
    m_fU_End = m_fImgWidth / m_fTexWidth;
    m_fV_End = m_fImgHeight / m_fTexHeight;
    m_fV_Start = 1.0f - m_fV_End; // V 座標從底部開始計算
}

void BackgroundImage::CreateBlackBG(float x, float y, float width, float height)
{
    Reset(); // 先清除舊資源

    m_pVBData = CDeviceResetManager::GetInstance()->CreateVertexBuffer(4, 0); // 類型 0: GIVertex
    if (!m_pVBData) return;

    // 設定四個頂點的螢幕座標和顏色
    float left = x - 0.5f;
    float top = y - 0.5f;
    float right = x + width - 0.5f;
    float bottom = y + height - 0.5f;
    const DWORD blackColor = 0xFF000000;

    m_giVertices[0].position_x = left;
    m_giVertices[0].position_y = top;
    m_giVertices[0].diffuse_color = blackColor;

    m_giVertices[1].position_x = right;
    m_giVertices[1].position_y = top;
    m_giVertices[1].diffuse_color = blackColor;

    m_giVertices[2].position_x = right;
    m_giVertices[2].position_y = bottom;
    m_giVertices[2].diffuse_color = blackColor;

    m_giVertices[3].position_x = left;
    m_giVertices[3].position_y = bottom;
    m_giVertices[3].diffuse_color = blackColor;

    // 將頂點資料上傳到 GPU
    if (m_pVBData && m_pVBData->pVertexBuffer) {
        void* pV = nullptr;
        if (SUCCEEDED(m_pVBData->pVertexBuffer->Lock(0, 0, &pV, 0))) {
            memcpy(pV, m_giVertices, sizeof(m_giVertices));
            m_pVBData->pVertexBuffer->Unlock();
        }
    }
}

void BackgroundImage::SetPosition(float x, float y)
{
    m_fPosX = x;
    m_fPosY = y;
}

bool BackgroundImage::SetPositionUP(float delta)
{
    m_fV_Start -= (delta / m_fTexHeight);
    if (m_fV_Start < 0.0f) {
        m_fV_Start = 0.0f;
        return true; // 已達頂部
    }
    return false;
}

bool BackgroundImage::SetPositionDOWN(float delta)
{
    m_fV_Start += (delta / m_fTexHeight);
    float maxV = 1.0f - m_fV_End;
    if (m_fV_Start > maxV) {
        m_fV_Start = maxV;
        return true; // 已達底部
    }
    return false;
}

void BackgroundImage::Process()
{
    if (!m_pVBData) return;

    // 根據位置和 UV 起點計算最終的頂點資料
    float left = m_fPosX - 0.5f;
    float top = m_fPosY - 0.5f;
    float right = left + m_fImgWidth;
    float bottom = top + m_fImgHeight;

    m_imageVertices[0] = { left,  top,    0.5f, 1.0f, m_fU_Start, m_fV_Start };
    m_imageVertices[1] = { right, top,    0.5f, 1.0f, m_fU_End,   m_fV_Start };
    m_imageVertices[2] = { right, bottom, 0.5f, 1.0f, m_fU_End,   m_fV_End + m_fV_Start };
    m_imageVertices[3] = { left,  bottom, 0.5f, 1.0f, m_fU_Start, m_fV_End + m_fV_Start };

    // 將頂點資料上傳到 GPU
    if (m_pVBData && m_pVBData->pVertexBuffer) {
        void* pV = nullptr;
        if (SUCCEEDED(m_pVBData->pVertexBuffer->Lock(0, 0, &pV, 0))) {
            memcpy(pV, m_imageVertices, sizeof(m_imageVertices));
            m_pVBData->pVertexBuffer->Unlock();
        }
    }
}

void BackgroundImage::Render()
{
    if (DontDraw || !m_pVBData || !m_pTexData) return;

    CDeviceManager::GetInstance()->SetTexture(0, m_pTexData->pTexture);
    CDeviceManager::GetInstance()->SetStreamSource(0, m_pVBData->pVertexBuffer, 0, sizeof(ImageVertex));
    CDeviceManager::GetInstance()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1); // FVF: 0x104
    Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}

void BackgroundImage::RenderBlackBG()
{
    if (DontDraw || !m_pVBData) return;

    CDeviceManager::GetInstance()->SetTexture(0, nullptr); // 不使用紋理
    CDeviceManager::GetInstance()->SetStreamSource(0, m_pVBData->pVertexBuffer, 0, sizeof(GIVertex));
    // FVF 應為 D3DFVF_XYZRHW | D3DFVF_DIFFUSE (0x44) 或包含紋理座標的 0x144
    // 但為忠於原始碼，這裡使用它記載的 0x104
    CDeviceManager::GetInstance()->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
    Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
}