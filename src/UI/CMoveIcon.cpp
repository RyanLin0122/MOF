#include "UI/CMoveIcon.h"

// --- 模擬的外部依賴 ---
class CUIBase {};
class CUIManager {
public:
    CUIBase* GetUIWindow(int nUITypeID) { return nullptr; }
};
CUIManager* g_UIMgr = nullptr;
// --- 模擬結束 ---

/**
 * CMoveIcon 建構函式
 * 初始化所有成員變數，並設定其內部的文字控制項。
 */
CMoveIcon::CMoveIcon()
{
    m_pOwner = nullptr;
    m_pTarget = nullptr;
    m_bIsStarted = false;

    InitData();

    // 將文字控制項作為自己的子物件，並設定樣式
    m_Text.Create(this);
    m_Text.SetControlSetFont("MoveIconText");
    m_Text.SetMultiLineSpace(0);
    m_Text.SetMultiLineSize(36, 36);
    //m_Text.SetTextAlignment(1); // 居中
    //m_Text.SetTextShadowColor(0xFF000000);
}

CMoveIcon::~CMoveIcon()
{
}

/**
 * 初始化/重設所有資料。
 */
void CMoveIcon::InitData()
{
    m_bAutoTerminate = true;
    m_cDataCount = 0;
    m_pTarget = nullptr;
    
    for (int i = 0; i < 64; ++i)
    {
        m_Data[i].m_dwData = 0;
        m_Data[i].m_bIsSet = false;
    }
    m_Text.ClearText();
}

/**
 * 開始拖放操作。
 */
bool CMoveIcon::Start(CUIBase* pOwner, bool bAutoTerminate)
{
    if (m_bIsStarted || m_bIsVisible)
    {
        return false;
    }
    
    m_bIsStarted = true;
    m_pOwner = pOwner;
    InitData();
    
    if (!bAutoTerminate)
    {
        SetNotAutoTerminate();
    }
    m_bIsEnable = false; // 移動圖示本身不應接收點擊事件
    return true;
}

bool CMoveIcon::Start(CUIBase* pOwner, unsigned int uiGIID, unsigned short usBlockID, int nPosX, int nPosY)
{
    if (Start(pOwner, true))
    {
        SetImageID(uiGIID, usBlockID);
        SetCenterPos(nPosX, nPosY);
        Show();
        return true;
    }
    return false;
}

bool CMoveIcon::Start(CUIBase* pOwner, CControlImage* pSrcImage, int nPosX, int nPosY)
{
    if (Start(pOwner, true))
    {
        // 假設 CControlImage 有 GetImageID 等函式
       // SetImageID(pSrcImage->GetImageType(), pSrcImage->GetGIID(), pSrcImage->GetBlockID());
        //m_nTooltipIndex = pSrcImage->GetTooltipIndex(); // 繼承 Tooltip
        SetCenterPos(nPosX, nPosY);
        Show();
        return true;
    }
    return false;
}


/**
 * 將圖示移動到新位置。
 * @param nPosX 通常是滑鼠的 X 座標。
 * @param nPosY 通常是滑鼠的 Y 座標。
 */
void CMoveIcon::Move(int nPosX, int nPosY)
{
    if (m_bIsVisible)
    {
        SetCenterPos(nPosX, nPosY);
    }
}

/**
 * 放下圖示，完成拖放。
 * @param pTarget 放下圖示時，滑鼠下方的UI元件。
 */
bool CMoveIcon::Put(CUIBase* pTarget)
{
    if (!m_bIsStarted) return false;

    if (m_bAutoTerminate)
    {
        m_usImageBlockID = 0xFFFF; // 設為無效
        m_pTarget = pTarget;
        m_usWidth = 0;
        m_usHeight = 0;
        m_nShadeAlpha = 0;
        Hide();
        // 此處不直接呼叫 Terminate，而是讓 AutoTerminate 處理
        // 可能是為了讓外部有機會在 terminate 前做最後的檢查
        return AutoTerminate();
    }
    return false;
}

bool CMoveIcon::Put(int nUITypeID)
{
    CUIBase* pTargetUI = g_UIMgr->GetUIWindow(nUITypeID);
    return Put(pTargetUI);
}

/**
 * 結束拖放，並通知來源和目標物件。
 */
void CMoveIcon::Terminate()
{
    if (m_bIsStarted)
    {
        m_bIsStarted = false;
        Hide();

        // 通知來源物件，拖放已結束 (透過虛擬函式呼叫)
        if (m_pOwner)
        {
            // m_pOwner->OnMoveIconDrop(this); // 假設的事件處理函式
        }

        // 如果目標和來源不同，也通知目標物件
        if (m_pTarget && m_pTarget != m_pOwner)
        {
            // m_pTarget->OnMoveIconDrop(this);
        }

        InitData();
    }
}

/**
 * 如果設定為自動終止，則執行 Terminate。
 */
bool CMoveIcon::AutoTerminate()
{
    if (!m_bIsStarted || !m_bAutoTerminate)
    {
        return false;
    }
    Terminate();
    return true;
}

/**
 * 附加資料。
 */
void CMoveIcon::AddData(unsigned char ucIndex, unsigned int dwData)
{
    if (m_bIsStarted && !m_Data[ucIndex].m_bIsSet)
    {
        m_Data[ucIndex].m_bIsSet = true;
        m_Data[ucIndex].m_dwData = dwData;
        m_cDataCount++;
    }
}

/**
 * 覆蓋資料。
 */
void CMoveIcon::OverlapData(unsigned char ucIndex, unsigned int dwData)
{
    if (m_bIsStarted)
    {
        m_Data[ucIndex].m_bIsSet = true;
        m_Data[ucIndex].m_dwData = dwData;
    }
}

/**
 * 獲取資料。
 */
unsigned int CMoveIcon::GetData(unsigned char ucIndex)
{
    if (!m_bIsStarted || !m_Data[ucIndex].m_bIsSet)
    {
        return 0;
    }
    // 原始碼中有一個 ClientCharacterManager::ResetMoveTarget() 的呼叫
    return m_Data[ucIndex].m_dwData;
}


// --- 其他函式 ---
bool CMoveIcon::IsStart() const { return m_bIsStarted; }
bool CMoveIcon::IsSetData(unsigned char ucIndex) const { return m_Data[ucIndex].m_bIsSet; }
bool CMoveIcon::IsDownUIType(int nUITypeID) {
    if (!m_bIsStarted || !m_pOwner) return false;
    // return m_pOwner->GetTypeID() == nUITypeID; // 假設 CUIBase 有 GetTypeID
    return false;
}
bool CMoveIcon::IsDownUIType(CUIBase* pUI) { 
    // if (pUI) return IsDownUIType(pUI->GetTypeID());
    return false; 
}
void CMoveIcon::SetNotAutoTerminate() { m_bAutoTerminate = false; }
CControlText* CMoveIcon::GetTextControl() { return &m_Text; }
