#include "Image/GameImage.h"
#include "Image/CDeviceResetManager.h" // 假設全域 g_Device_Reset_Manager 在此
#include "Image/CDeviceManager.h"      // 假設全域 g_clDeviceManager 在此
#include "Image/ResourceMgr.h"
#include "Image/ImageResourceListDataMgr.h" // 需要 ImageResourceListData 的完整定義
#include <cstring>               // 為了 memset
#include <algorithm>             // 為了 std::swap

// --- 外部依賴的全域變數 ---
extern LPDIRECT3DDEVICE9 Device;

// GameImage.c::Process 中引用的全域變數
extern int g_Game_System_Info;
extern int nHeight;
extern bool DontDraw;

GameImage::GameImage() {
    // 這裡的初始化順序和數值完全參照 GameImage.c 的建構函式
    m_pVBData = nullptr;
    m_pGIData = nullptr;

    // GIVertex m_Vertices[8] 會自動呼叫其預設建構函式，對應反編譯程式碼中的迴圈
    // 初始化所有狀態變數為預設值
    ResetGI();
}

GameImage::~GameImage() {
    // 釋放頂點緩衝區
    if (m_pVBData) {
        CDeviceResetManager::GetInstance()->DeleteVertexBuffer(m_pVBData);
        m_pVBData = nullptr;
    }
    // 確保資源指標為空，但實際的 Release 是由 ReleaseGIData 處理
    m_pGIData = nullptr;
}

void GameImage::CreateVertexBuffer() {
    // 建立一個可容納8個 GIVertex 的頂點緩衝區 (type 0)
    m_pVBData = CDeviceResetManager::GetInstance()->CreateVertexBuffer(8, 0);
}

void GameImage::ResetGI() {
    // 釋放D3D資源
    if (m_pVBData) {
        CDeviceResetManager::GetInstance()->DeleteVertexBuffer(m_pVBData);
        m_pVBData = nullptr;
    }
    if (m_pGIData) {
        // 在 cltImageManager 的 Free 方法中，ResetGI 被呼叫，此時不應釋放資源引用
        m_pGIData = nullptr;
    }

    // 將所有成員變數重設為建構時的初始狀態
    memset(m_Vertices, 0, sizeof(m_Vertices));
    memset(m_baseVertices, 0, sizeof(m_baseVertices));
    memset(m_transformedVertices, 0, sizeof(m_transformedVertices));

    m_fPosX = 0.0f;
    m_fPosY = 0.0f;
    m_fDrawWidth = 0.0f;
    m_fDrawHeight = 0.0f;
    m_fAngleX = 0.0f;
    m_fAngleY = 0.0f;
    m_fHotspotX = 0.0f;
    m_fHotspotY = 0.0f;
    m_fHotspotWidth = 25.0f; // 1103626240
    m_fHotspotHeight = -20.0f; // -1063256064
    m_wBlockID = 0;
    m_nScale = 100;
    m_dwAlpha = 255;
    m_dwColor = 255;
    m_nRotation = 0;
    m_bFlipX = false;
    m_bFlipY = false;
    m_dwColorOp = 0;
    m_fCenterX = 0.0f;
    m_dwGroupID = 0;
    m_dwResourceID = 0;
    m_fScaleX = 1.0f;
    m_fScaleY = 1.0f;
    m_bDrawPart1 = true;
    m_bDrawPart2 = false;
    m_bIsProcessed = false;
    m_bVertexAnimation = false;
    m_bFlag_445 = false;
    m_bFlag_446 = false;
    m_bFlag_447 = false;
    m_bFlag_448 = false;
    m_bFlag_449 = false;
    m_bFlag_450 = false;
    m_bFlag_451 = false;
    m_OverwriteColor[0] = 1.0f;
    m_OverwriteColor[1] = 1.0f;
    m_OverwriteColor[2] = 1.0f;
    m_OverwriteColor[3] = 1.0f;
    m_bUseOverwriteColor = false;
}

void GameImage::GetGIData(unsigned int dwGroupID, unsigned int dwResourceID, int a4, int a5) {
    // 如果請求的是同一個資源，且資源已存在，則不重新獲取
    if (m_dwResourceID == dwResourceID && m_pGIData != nullptr) {
        return;
    }

    // 呼叫全域資源管理器獲取資源
    ResourceMgr::eResourceType resourceType = static_cast<ResourceMgr::eResourceType>(dwGroupID);
    m_pGIData = ResourceMgr::GetInstance()->GetImageResource(resourceType, dwResourceID, a4, a5);
    m_dwGroupID = dwGroupID;
    m_dwResourceID = dwResourceID;
}

void GameImage::ReleaseGIData() {
    if (m_dwResourceID != 0) {
        ResourceMgr::eResourceType resourceType = static_cast<ResourceMgr::eResourceType>(m_dwGroupID);
        // 呼叫全域資源管理器釋放對資源的引用
        ResourceMgr::GetInstance()->ReleaseImageResource(resourceType, m_dwResourceID);

        // 將自身狀態重設為初始值，以便被物件池回收
        // 此處的重設邏輯是參照 GameImage.c::ReleaseGIData 的反編譯程式碼
        m_pGIData = nullptr;
        m_dwResourceID = 0;
        m_dwGroupID = 0;
        m_wBlockID = 0;
        m_nScale = 100;
        m_dwAlpha = 255;
        m_dwColor = 255;
        m_nRotation = 0;
        m_bFlipX = false;
        m_bFlipY = false;
        m_fScaleX = 1.0f;
        m_fScaleY = 1.0f;
        m_bDrawPart1 = true;
        m_bDrawPart2 = false;
    }
}

bool GameImage::Process() {
    if (!m_pGIData) { // 檢查資源是否有效
        return true;
    }

    m_pGIData->m_Resource.LoadTexture(); // 確保紋理已載入到 VRAM
    m_bIsProcessed = true; // 先假設能處理成功

    if (m_wBlockID >= m_pGIData->m_Resource.m_animationFrameCount) { // 檢查動畫影格ID是否有效
        m_bIsProcessed = false;
        return false;
    }

    AnimationFrameData* pFrame = &m_pGIData->m_Resource.m_pAnimationFrames[m_wBlockID]; // 獲取當前影格的資訊
    if (!pFrame) return false;

    // ----- 根據 GameImage.c::Process 的邏輯進行完整的頂點計算 -----

    // 可見性初步判斷 (這部分邏輯與螢幕邊界有關，直接還原)
    if (!((float)pFrame->width * m_fScaleX + m_fPosX >= 0.0f && (float)(g_Game_System_Info + pFrame->width) >= m_fPosX)) {
        m_bIsProcessed = false;
        return false;
    }
    if (!((float)pFrame->height * m_fScaleY + m_fPosY >= 0.0f && m_fPosY <= (float)(nHeight + pFrame->height) + m_fPosY)) {
        m_bIsProcessed = false;
        return false;
    }

    // 計算圖片影格的半寬和半高
    float halfWidth = (float)pFrame->width * 0.5f;
    float halfHeight = (float)pFrame->height * 0.5f;

    // 設置四個角落的基礎頂點座標 (以(0,0)為中心)
    m_baseVertices[0] = { -halfWidth, -halfHeight, 0.0f };
    m_baseVertices[1] = { halfWidth, -halfHeight, 0.0f };
    m_baseVertices[2] = { halfWidth,  halfHeight, 0.0f };
    m_baseVertices[3] = { -halfWidth,  halfHeight, 0.0f };

    // 設置四個角落的UV座標
    m_Vertices[0].texture_u = pFrame->u1;
    m_Vertices[0].texture_v = pFrame->v1;
    m_Vertices[1].texture_u = pFrame->u2;
    m_Vertices[1].texture_v = pFrame->v1;
    m_Vertices[2].texture_u = pFrame->u2;
    m_Vertices[2].texture_v = pFrame->v2;
    m_Vertices[3].texture_u = pFrame->u1;
    m_Vertices[3].texture_v = pFrame->v2;

    // 應用整體縮放 (m_nScale)，這是一個非線性的縮放計算
    float finalScale = (float)m_nScale * 0.01f;
    if (finalScale > 1.0f) {
        finalScale = (finalScale - 1.0f) * 0.5f + 1.0f;
    }
    else if (finalScale < 0.0f) {
        finalScale = 0.0f;
    }

    // 將縮放應用到基礎頂點，得到變換後頂點
    for (int i = 0; i < 4; ++i) {
        m_transformedVertices[i] = m_baseVertices[i] * finalScale;
    }

    // 應用獨立軸縮放 (m_fScaleX, m_fScaleY)，這部分邏輯是在已縮放的基礎上做調整
    float adjustX = (float)pFrame->width * (m_fScaleX - 1.0f);
    m_transformedVertices[1].x += adjustX;
    m_transformedVertices[2].x += adjustX;
    float adjustY = (float)pFrame->height * (m_fScaleY - 1.0f);
    m_transformedVertices[2].y += adjustY;
    m_transformedVertices[3].y += adjustY;

    // 應用旋轉 (m_nRotation)
    if (m_nRotation != 0) {
        D3DXMATRIX matRotation;
        D3DXMatrixRotationZ(&matRotation, (float)m_nRotation * 0.017453292f); // 0.017453292f ≈ PI / 180
        for (int i = 0; i < 4; ++i) {
            D3DXVec3TransformCoord(&m_transformedVertices[i], &m_transformedVertices[i], &matRotation);
        }
    }

    // 計算最終螢幕座標，包含影格本身的偏移
    float finalPosX = m_fPosX + pFrame->offsetX + halfWidth;
    float finalPosY = m_fPosY + pFrame->offsetY + halfHeight;
    for (int i = 0; i < 4; ++i) {
        m_Vertices[i].position_x = m_transformedVertices[i].x + finalPosX - 0.5f;
        m_Vertices[i].position_y = m_transformedVertices[i].y + finalPosY - 0.5f;
        m_Vertices[i].position_z_or_rhw = 0.5f;
        m_Vertices[i].rhw_value = 1.0f;
    }

    // 計算頂點顏色
    DWORD finalColor;
    if (m_dwColorOp == 1) { // 紅色模式
        finalColor = D3DCOLOR_ARGB(m_dwAlpha, m_dwAlpha, 0, 0);
    }
    else { // 正常/灰階模式
        finalColor = D3DCOLOR_ARGB(m_dwAlpha, m_dwColor, m_dwColor, m_dwColor);
    }
    for (int i = 0; i < 4; ++i) {
        m_Vertices[i].diffuse_color = finalColor;
    }

    // 處理翻轉 (透過交換UV座標實現)
    if (m_bFlipX) {
        std::swap(m_Vertices[0].texture_u, m_Vertices[1].texture_u);
        std::swap(m_Vertices[3].texture_u, m_Vertices[2].texture_u);
        // 還原原始碼中對X座標的修正
        float flipAdjust = (float)(pFrame->width + 2 * pFrame->offsetX);
        for (int i = 0; i < 4; ++i) m_Vertices[i].position_x -= flipAdjust;
    }
    if (m_bFlipY) {
        std::swap(m_Vertices[0].texture_v, m_Vertices[3].texture_v);
        std::swap(m_Vertices[1].texture_v, m_Vertices[2].texture_v);
    }

    // 將計算好的頂點資料上傳到 GPU
    if (m_pVBData && m_pVBData->pVertexBuffer) {
        void* pV = nullptr;
        HRESULT hr = m_pVBData->pVertexBuffer->Lock(0, sizeof(m_Vertices), &pV, D3DLOCK_DISCARD);
        if (SUCCEEDED(hr) && pV) {
            memcpy(pV, m_Vertices, sizeof(m_Vertices));
            m_pVBData->pVertexBuffer->Unlock();
        }
        else {
            m_bIsProcessed = false;
        }
    }
    else {
        m_bIsProcessed = false;
    }

    return m_bIsProcessed;
}

bool GameImage::Draw() {
    if (!m_bIsProcessed || !m_pGIData || !m_pGIData->m_Resource.m_pTexture) { // 如果頂點尚未處理，或資源無效，則不繪製
        return false;
    }

    // 設定渲染狀態
    CDeviceManager::GetInstance()->SetTexture(0, m_pGIData->m_Resource.m_pTexture);
    CDeviceManager::GetInstance()->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1); // FVF: 0x144
    CDeviceManager::GetInstance()->SetStreamSource(0, m_pVBData->pVertexBuffer, 0, sizeof(GIVertex));

    // 根據旗標繪製不同的部分
    if (m_bDrawPart2 && !DontDraw) {
        Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 4, 2); // 繪製頂點 4-7
    }
    if (m_bDrawPart1 && !DontDraw) {
        Device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2); // 繪製頂點 0-3
    }

    return true;
}

void GameImage::GetBlockRect(RECT* pOutRect) const
{
    // 檢查輸出指標和資源指標是否有效
    if (!pOutRect) return;

    if (!m_pGIData) { // 如果沒有綁定資源，返回全零的 RECT。
        pOutRect->left = 0;
        pOutRect->top = 0;
        pOutRect->right = 0;
        pOutRect->bottom = 0;
        return;
    }

    // 檢查動畫影格 ID 是否在有效範圍內
    if (m_wBlockID >= m_pGIData->m_Resource.m_animationFrameCount) {
        pOutRect->left = 0;
        pOutRect->top = 0;
        pOutRect->right = 0;
        pOutRect->bottom = 0;
        return;
    }

    // 根據 m_wBlockID 獲取對應的影格資訊
    AnimationFrameData* pFrame = &m_pGIData->m_Resource.m_pAnimationFrames[m_wBlockID];
    if (!pFrame) {
        pOutRect->left = 0;
        pOutRect->top = 0;
        pOutRect->right = 0;
        pOutRect->bottom = 0;
        return;
    }

    // 從影格資訊中讀取座標和尺寸，並計算 RECT
    pOutRect->left = pFrame->offsetX;
    pOutRect->top = pFrame->offsetY;
    pOutRect->right = pFrame->offsetX + pFrame->width;
    pOutRect->bottom = pFrame->offsetY + pFrame->height;
}

void GameImage::VertexAnimationCalculator(const GIVertex* pSourceVertices)
{
    if (!pSourceVertices) return;

    // 設定旗標，表示頂點由外部動畫控制
    m_bVertexAnimation = true;

    // 將外部傳入的8個頂點資料 (224 bytes) 複製到內部的頂點陣列中
    memcpy(m_Vertices, pSourceVertices, sizeof(m_Vertices));
}

void GameImage::SetDefaultTextureColor()
{
    // 將顏色覆蓋值重設為不影響原色的白色 (1.0f, 1.0f, 1.0f, 1.0f)。
    m_OverwriteColor[0] = 1.0f; // R
    m_OverwriteColor[1] = 1.0f; // G
    m_OverwriteColor[2] = 1.0f; // B
    m_OverwriteColor[3] = 1.0f; // A
}

void GameImage::SetOverWriteTextureColor(DWORD color)
{
    if (color == 0) {
        // 如果傳入0，則恢復預設顏色
        SetDefaultTextureColor();
    }
    else {
        // 將 32-bit ARGB 顏色轉換為 0.0f-1.0f 的浮點數格式
        // 原始碼的BYTE2對應D3DCOLOR的R, BYTE1對應G, LOBYTE對應B
        const float f = 1.0f / 255.0f;
        m_OverwriteColor[0] = (float)((color >> 16) & 0xFF) * f; // R
        m_OverwriteColor[1] = (float)((color >> 8) & 0xFF) * f; // G
        m_OverwriteColor[2] = (float)(color & 0xFF) * f;         // B
        m_OverwriteColor[3] = (float)((color >> 24) & 0xFF) * f; // A
    }
}

void GameImage::GetOverWriteTextureColor(float* pOutColor) const
{
    if (!pOutColor) return;

    // 將內部的顏色覆蓋值複製到輸出指標
    memcpy(pOutColor, m_OverwriteColor, sizeof(m_OverwriteColor));
}

void GameImage::SetOverWriteTextureColorDraw(bool bEnable)
{
    // 設定是否啟用顏色覆蓋模式的旗標
    m_bUseOverwriteColor = bEnable;
}

bool GameImage::IsOverWriteTextureColorDraw() const
{
    // 返回是否啟用顏色覆蓋模式的旗標狀態
    return m_bUseOverwriteColor;
}