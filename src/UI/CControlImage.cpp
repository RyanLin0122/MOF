#include "UI/CControlImage.h"
#include "Image/cltImageManager.h"
#include <windows.h> // 為了 timeGetTime


// 將浮點顏色轉換為 DWORD
unsigned long FloatsToDWORDColor(float r, float g, float b, float a) {
    auto ToByte = [](float val) -> unsigned char {
        if (val >= 1.0f) return 255;
        if (val <= 0.0f) return 0;
        return static_cast<unsigned char>(val * 255.0f + 0.5f);
    };
    return (ToByte(a) << 24) | (ToByte(r) << 16) | (ToByte(g) << 8) | ToByte(b);
}
// --- 模擬結束 ---

/**
 * CControlImage 建構函式
 * 初始化所有圖片相關的成員變數。
 */
CControlImage::CControlImage()
{
    m_nImageType = 5; // 預設類型
    m_nImageGIID = 0;
    m_usImageBlockID = 0xFFFF; // -1 代表無效
    m_pGameImage = nullptr;
    m_nAlpha = 255;
    m_nAngle = 0;
    m_nRenderType = 3;
    m_bIsImageLoaded = false;
    m_cOutputPercentTop = 0;
    m_nShadeAlpha = 0;
    m_dwShadeStartTime = timeGetTime();
    m_nShadeMode = 1;
    m_nShadeDirection = 1;
    m_fRed = 1.0f;
    m_fGreen = 1.0f;
    m_fBlue = 1.0f;
    m_fAlphaTint = 1.0f;
    m_bIsMouseOver = false;
}

/**
 * CControlImage 解構函式
 */
CControlImage::~CControlImage()
{
}

/**
 * 清除資料，重設圖片 ID。
 */
void CControlImage::ClearData()
{
    CControlBase::ClearData();
    m_nImageGIID = 0;
    m_usImageBlockID = 0xFFFF;
}

/**
 * 創建控制項，並設定淡入效果。
 */
void CControlImage::Create(CControlBase* pParent)
{
    CControlBase::Create(pParent);
    m_nShadeAlpha = 0;
    SetAlpha(0); // 初始透明度為0，配合淡入效果
}

void CControlImage::Create(int nPosX, int nPosY, CControlBase* pParent)
{
    CControlBase::Create(nPosX, nPosY, pParent);
    m_nShadeAlpha = 0;
    SetAlpha(0);
}

/**
 * 創建控制項，同時設定圖片資源。
 */
void CControlImage::Create(int nPosX, int nPosY, unsigned int uiGIID, unsigned short usBlockID, CControlBase* pParent)
{
    m_usImageBlockID = usBlockID;
    m_nImageGIID = uiGIID;
    SetImageSize();
    CControlBase::Create(nPosX, nPosY, pParent);
    m_nShadeAlpha = 0;
    SetAlpha(0);
}

/**
 * 設定圖片資源 ID，並更新控制項大小以符合圖片。
 */
void CControlImage::SetImage(unsigned int uiGIID, unsigned short usBlockID)
{
    m_nImageGIID = uiGIID;
    m_usImageBlockID = usBlockID;
    SetImageSize();
}

void CControlImage::SetImageID(unsigned int uiImageType, unsigned int uiGIID, unsigned short usBlockID)
{
    if (m_nImageType != uiImageType || m_nImageGIID != uiGIID || m_usImageBlockID != usBlockID)
    {
        m_nImageType = uiImageType;
        m_nImageGIID = uiGIID;
        m_usImageBlockID = usBlockID;
        SetImageSize();
    }
}

void CControlImage::SetImageID(unsigned int uiGIID, unsigned short usBlockID)
{
    if (m_nImageGIID != uiGIID || m_usImageBlockID != usBlockID)
    {
        m_nImageGIID = uiGIID;
        m_usImageBlockID = usBlockID;
        SetImageSize();
    }
}

/**
 * 根據圖片資源的實際大小，設定控制項的寬和高。
 */
void CControlImage::SetImageSize()
{
    if (m_usImageBlockID == 0xFFFF)
    {
        m_usWidth = 0;
        m_usHeight = 0;
        return;
    }
    
    // 透過圖片管理器獲取圖片資訊
    GameImage* pImage = cltImageManager::GetInstance()->GetGameImage(m_nImageType, m_nImageGIID, 0, true);
    if (pImage)
    {
        // 從圖片資源中讀取寬高 (此為模擬)
        // 原始碼中會從 pImage->pBlockInfo->pBlock[m_usImageBlockID] 讀取
        m_usWidth = 32;  // 假設寬度
        m_usHeight = 32; // 假設高度
        cltImageManager::GetInstance()->ReleaseGameImage(pImage);
    }
}

/**
 * 準備繪製。
 * 載入圖片資源、計算淡入效果的透明度、設定渲染屬性。
 */
void CControlImage::PrepareDrawing()
{
    // 步驟 1: 初始化並進行有效性檢查
    m_bIsImageLoaded = false;
    if (!m_bIsVisible || m_usImageBlockID == 0xFFFF)
    {
        // 如果不可見或沒有設定圖片，則無需準備，直接處理子物件
        CControlBase::PrepareDrawing();
        return;
    }

    // 步驟 2: 獲取底層的 GameImage 資源物件
    m_pGameImage = cltImageManager::GetInstance()->GetGameImage(m_nImageType, m_nImageGIID, 0, true);
    if (!m_pGameImage)
    {
        // 如果獲取資源失敗，也無法繪製
        CControlBase::PrepareDrawing();
        return;
    }

    // 標記為已準備好繪製
    m_bIsImageLoaded = true;

    // 步驟 3: 處理淡入/淡出效果
    if (m_nShadeMode == 1) // 1 代表淡入模式
    {
        if (m_nShadeAlpha < 255)
        {
            // 根據時間計算當前的透明度，實現平滑的淡入動畫
            float fElapsedTime = (float)(timeGetTime() - m_dwShadeStartTime);
            // 這裡的計算方式與反編譯碼的邏輯一致
            float fAlphaIncrement = (m_nShadeDirection * (fElapsedTime / 1000.0f) * 30.0f * 255.0f) / (float)60;
            m_nShadeAlpha += static_cast<int>(fAlphaIncrement);

            if (m_nShadeAlpha > 255) m_nShadeAlpha = 255;
            if (m_nShadeAlpha < 0)   m_nShadeAlpha = 0;

            m_dwShadeStartTime = timeGetTime();
        }
    }

    // 步驟 4: 將 CControlImage 的狀態轉移到底層 GameImage 物件

    // 4.1 獲取絕對座標
    int absPos[2];
    GetAbsPos((float*)absPos);

    // 4.2 設定 GameImage 的屬性
    m_pGameImage->SetPosition(static_cast<float>(absPos[0]), static_cast<float>(absPos[1]));
    m_pGameImage->SetScaleXY(m_fScaleX, m_fScaleY);
    m_pGameImage->SetRotation(m_nAngle);
    m_pGameImage->SetBlockID(m_usImageBlockID);

    // 根據是否有淡入效果，選擇使用 m_nShadeAlpha 還是 m_nAlpha
    m_pGameImage->SetAlpha(m_nShadeMode ? m_nShadeAlpha : m_nAlpha);

    // (在您提供的反編譯碼中未見，但一個完整的系統通常會有)
    // m_pGameImage->SetFlipX( ... );
    // m_pGameImage->SetFlipY( ... );

    // 步驟 5: 呼叫 GameImage 的 Process 函式
    // 這一步會使用上面設定好的所有屬性，來計算最終要渲染的頂點座標和顏色
    m_pGameImage->Process();

    // 步驟 6: 遞迴呼叫父類別的 PrepareDrawing，以處理所有子控制項
    CControlBase::PrepareDrawing();
}


/**
 * 執行繪製。
 */
void CControlImage::Draw()
{
    if (m_bIsVisible && m_bIsImageLoaded && m_pGameImage)
    {
        // 設定顏色濾鏡
        m_pGameImage->SetOverWriteTextureColor(FloatsToDWORDColor(m_fRed, m_fGreen, m_fBlue, m_fAlphaTint));
        
        // 呼叫底層繪圖函式
        m_pGameImage->Draw();
        
        // 恢復預設顏色
        m_pGameImage->SetDefaultTextureColor();
        
        CControlBase::Draw();
    }
}

void CControlImage::Hide()
{
    if (m_nShadeMode) {
        m_nShadeAlpha = 0; // 重設淡入動畫
        m_dwShadeStartTime = timeGetTime();
    }
    CControlBase::Hide();
}

// ... 其他 Set/Get 函式實作 ...
void CControlImage::SetImageID(int type, int giid, short blockid) { SetImageID((unsigned int)type, (unsigned int)giid, (unsigned short)blockid); }
void CControlImage::SetGIID(unsigned int uiGIID) { m_nImageGIID = uiGIID; }
void CControlImage::SetBlockID(unsigned short usBlockID) { m_usImageBlockID = usBlockID; }
void CControlImage::SetAlpha(int nAlpha) { m_nAlpha = nAlpha; }
void CControlImage::SetAngle(int nAngle) { m_nAngle = nAngle; }
void CControlImage::SetShadeMode(int nMode) { m_nShadeMode = nMode; }
void CControlImage::SetOutputImagePercentFromTop(unsigned char ucPercent) { m_cOutputPercentTop = ucPercent; }
int* CControlImage::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) {
    stPoint pt = { (long)x, (long)y };
    m_bIsMouseOver = PtInCtrl(pt);
    return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}
