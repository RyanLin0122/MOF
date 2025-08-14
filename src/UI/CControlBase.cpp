#include "UI/CControlBase.h"

// 由於原始碼依賴，我們需要一個 stToolTipData 的空實作來連結
// 實際上這部分的程式碼位於 ToolTip.cpp
stToolTipData::stToolTipData() {}
stToolTipData::~stToolTipData() {}
void stToolTipData::Init() {}
void stToolTipData::SetStringType(char* text, int data) {}
void stToolTipData::SetKindType(int type, short usData, int unk, int data, char uiType, short slotIndex, void* extraData) {}

/**
 * CControlBase 建構函式
 * 初始化所有成員變數為預設值。
 */
CControlBase::CControlBase()
{
    m_pParent = nullptr;
    m_pPrev = nullptr;
    m_pNext = nullptr;
    m_pChildHead = nullptr;
    m_pChildTail = nullptr;
    m_nPosX = 0;
    m_nPosY = 0;
    m_usWidth = 0;
    m_usHeight = 0;
    m_fScaleX = 1.0f;
    m_fScaleY = 1.0f;
    m_bIsActive = true;
    m_bIsVisible = false;
    m_bAbsorbClick = false;
    m_bIsEnable = true;
    m_bPassKeyInputToParent = false;
    m_bHasToolTip = false; // 實際由 stToolTipData 內容決定
    m_nArrayIndex = 0;
    m_bIsCenterOrigin = false;
    m_nControlID = 0;
}

/**
 * CControlBase 解構函式
 * 由於是基底類別，設為虛擬函式。
 */
CControlBase::~CControlBase()
{
    // m_ToolTipData 會自動呼叫其解構函式
}

/**
 * 創建控制項並設定父物件。
 * @param pParent 父控制項指標。
 */
void CControlBase::Create(CControlBase* pParent)
{
    if (pParent && !m_pParent)
    {
        m_pParent = pParent;
        pParent->AddChild(this);
    }
}

/**
 * 創建控制項，設定位置和父物件。
 */
void CControlBase::Create(int nPosX, int nPosY, CControlBase* pParent)
{
    Create(pParent);
    SetPos(nPosX, nPosY);
}

/**
 * 創建控制項，設定位置、大小和父物件。
 */
void CControlBase::Create(int nPosX, int nPosY, unsigned short usWidth, unsigned short usHeight, CControlBase* pParent)
{
    m_usWidth = usWidth;
    m_usHeight = usHeight;
    Create(nPosX, nPosY, pParent);
}

/**
 * 顯示控制項。
 */
void CControlBase::Show()
{
    m_bIsVisible = true;
    OnShow(); // 呼叫虛擬的 OnShow 事件
}

/**
 * 隱藏控制項。
 */
void CControlBase::Hide()
{
    m_bIsVisible = false;
    OnHide(); // 呼叫虛擬的 OnHide 事件
}

/**
 * 顯示所有子控制項。
 */
void CControlBase::ShowChildren()
{
    for (CControlBase* pChild = m_pChildHead; pChild != nullptr; pChild = pChild->m_pNext)
    {
        pChild->Show();
    }
}

/**
 * 隱藏所有子控制項。
 */
void CControlBase::HideChildren()
{
    for (CControlBase* pChild = m_pChildHead; pChild != nullptr; pChild = pChild->m_pNext)
    {
        pChild->Hide();
    }
}

/**
 * 新增一個子控制項到鏈結串列的尾部。
 * @param pChild 要新增的子控制項指標。
 */
void CControlBase::AddChild(CControlBase* pChild)
{
    if (m_pChildTail) // 如果已有子控制項 (尾部存在)
    {
        m_pChildTail->m_pNext = pChild;
        pChild->m_pPrev = m_pChildTail;
        m_pChildTail = pChild;
    }
    else // 如果是第一個子控制項
    {
        m_pChildHead = pChild;
        m_pChildTail = pChild;
    }
}

/**
 * 刪除最後一個子控制項。
 */
void CControlBase::DelLastChild()
{
    if (!m_pChildTail) return;

    if (m_pChildTail == m_pChildHead) // 如果只有一個子控制項
    {
        m_pChildHead = nullptr;
        m_pChildTail = nullptr;
    }
    else // 不只一個子控制項
    {
        CControlBase* pNewTail = m_pChildTail->m_pPrev;
        m_pChildTail = pNewTail;
        if (pNewTail)
        {
            pNewTail->m_pNext = nullptr;
        }
    }
}

/**
 * 準備繪製所有可見的子控制項。
 */
void CControlBase::PrepareDrawing()
{
    for (CControlBase* pChild = m_pChildHead; pChild != nullptr; pChild = pChild->m_pNext)
    {
        if (pChild->m_bIsVisible)
        {
            pChild->PrepareDrawing();
        }
    }
}

/**
 * 繪製所有可見的子控制項。
 */
void CControlBase::Draw()
{
    for (CControlBase* pChild = m_pChildHead; pChild != nullptr; pChild = pChild->m_pNext)
    {
        if (pChild->m_bIsVisible)
        {
            pChild->Draw();
        }
    }
}

/**
 * 獲取絕對座標。
 * @param pPoint 用於接收座標的陣列指標。
 * @return 傳入的 pPoint 指標。
 */
float* CControlBase::GetAbsPos(float* pPoint)
{
    int nAbsX = m_nPosX;
    int nAbsY = m_nPosY;

    // 遞迴地加上所有父控制項的相對座標
    for (CControlBase* pParent = m_pParent; pParent != nullptr; pParent = pParent->m_pParent)
    {
        nAbsX += pParent->m_nPosX;
        nAbsY += pParent->m_nPosY;
    }

    pPoint[0] = nAbsX;
    pPoint[1] = nAbsY;
    return pPoint;
}

int CControlBase::GetAbsX()
{
    int pos[2];
    GetAbsPos((float*)pos);
    return pos[0];
}

int CControlBase::GetAbsY()
{
    int pos[2];
    GetAbsPos((float*)pos);
    return pos[1];
}

/**
 * 設定絕對座標。
 * @param nAbsX 絕對 X 座標。
 * @param nAbsY 絕對 Y 座標。
 */
void CControlBase::SetAbsPos(int nAbsX, int nAbsY)
{
    // 透過減去所有父控制項的相對座標來計算自己應有的相對座標
    for (CControlBase* pParent = m_pParent; pParent != nullptr; pParent = pParent->m_pParent)
    {
        nAbsX -= pParent->m_nPosX;
        nAbsY -= pParent->m_nPosY;
    }
    m_nPosX = nAbsX;
    m_nPosY = nAbsY;
}

/**
 * 判斷一個點是否在控制項的矩形範圍內。
 * @param pt 要測試的點。
 * @return 如果在範圍內返回 true，否則返回 false。
 */
bool CControlBase::PtInCtrl(stPoint pt)
{
    if (!m_bIsVisible || m_usWidth == 0 || m_usHeight == 0)
    {
        return false;
    }

    int absPos[2];
    GetAbsPos((float*)absPos);
    int nLeft = absPos[0];
    int nTop = absPos[1];
    
    // 如果原點在中心，需調整左上角座標
    if (m_bIsCenterOrigin)
    {
        nLeft -= static_cast<int>(m_usWidth * 0.5f);
        nTop -= static_cast<int>(m_usHeight * 0.5f);
    }
    
    int nRight = nLeft + static_cast<int>(m_usWidth * m_fScaleX);
    int nBottom = nTop + static_cast<int>(m_usHeight * m_fScaleY);

    return (pt.x >= nLeft && pt.x <= nRight && pt.y >= nTop && pt.y <= nBottom);
}

/**
 * 從後向前尋找被點擊的子控制項（後加入的子控制項會先被檢查）。
 * @param pt 點擊的座標。
 * @return 返回被點擊的控制項指標，如果沒有則返回自己。
 */
CControlBase* CControlBase::FindClickedChild(stPoint pt)
{
    // 從尾部（最上層）開始遍歷子控制項
    for (CControlBase* pChild = m_pChildTail; pChild != nullptr; pChild = pChild->m_pPrev)
    {
        // 如果子控制項可見且啟用
        if (pChild->m_bIsEnable && pChild->m_bIsVisible && pChild->PtInCtrl(pt))
        {
            // 如果它不吸收點擊且有子控制項，則遞迴尋找
            if (!pChild->m_bAbsorbClick && pChild->m_pChildHead)
            {
                return pChild->FindClickedChild(pt);
            }
            // 否則，就是它被點擊了
            return pChild;
        }
    }
    // 如果沒有子控制項被點擊，則返回自己
    return this;
}

// 其他函式的實作...
/**
 * 處理鍵盤與滑鼠輸入事件。
 * 根據 m_bPassKeyInputToParent 旗標決定是否將事件「冒泡」傳遞給父控制項。
 * @return 根據處理情況，可能返回 nullptr、父控制項指標或父控制項處理後的回傳值。
 */
int* CControlBase::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    // 根據反編譯碼，回傳值 result 初始設為父物件指標
    CControlBase* pResult = m_pParent;

    if (m_pParent)
    {
        if (m_bPassKeyInputToParent)
        {
            // 如果旗標為 true，則呼叫父物件的函式，並用其回傳值更新 pResult
            pResult = (CControlBase*)m_pParent->ControlKeyInputProcess(msg, key, x, y, a6, a7);
        }
        // 如果旗標為 false，則不執行任何操作，pResult 保持為 m_pParent 的值
    }
    // 如果 m_pParent 為 nullptr，pResult 從一開始就是 nullptr

    return (int*)pResult;
}


void CControlBase::InitLogIn() {
    if (m_bHasToolTip) { // 這裡的 m_bHasToolTip 是一個推斷
        m_ToolTipData.Init();
    }
    for (CControlBase* pChild = m_pChildHead; pChild != nullptr; pChild = pChild->m_pNext)
    {
        pChild->InitLogIn();
    }
}
void CControlBase::ClearData() {
    for (CControlBase* pChild = m_pChildHead; pChild != nullptr; pChild = pChild->m_pNext)
    {
        pChild->ClearData();
    }
}
/**
 * @brief 設定控制項相對於其父物件的位置。
 * @param nPosX X 座標。
 * @param nPosY Y 座標。
 */
void CControlBase::SetPos(int nPosX, int nPosY)
{
    m_nPosX = nPosX;
    m_nPosY = nPosY;
}

void CControlBase::SetPos(stPoint pos) { SetPos(pos.x, pos.y); }
void CControlBase::SetX(int nPosX) { m_nPosX = nPosX; }
void CControlBase::SetY(int nPosY) { m_nPosY = nPosY; }
int CControlBase::GetX() const { return m_nPosX; }
int CControlBase::GetY() const { return m_nPosY; }
unsigned short CControlBase::GetWidth() const { return m_usWidth; }
unsigned short CControlBase::GetHeight() const { return m_usHeight; }
void CControlBase::SetAbsPos(stPoint pos) { SetAbsPos(pos.x, pos.y); }
void CControlBase::SetAbsX(int nAbsX) {
    int parentAbsX = 0;
    if (m_pParent) {
        parentAbsX = m_pParent->GetAbsX();
    }
    m_nPosX = nAbsX - parentAbsX;
}
void CControlBase::SetAbsY(int nAbsY) {
    int parentAbsY = 0;
    if (m_pParent) {
        parentAbsY = m_pParent->GetAbsY();
    }
    m_nPosY = nAbsY - parentAbsY;
}
void CControlBase::SetCenterPos(int nAbsCenterX, int nAbsCenterY) {
    int newAbsX = nAbsCenterX - static_cast<int>(m_usWidth * 0.5f);
    int newAbsY = nAbsCenterY - static_cast<int>(m_usHeight * 0.5f);

    if(m_bIsCenterOrigin)
    {
        newAbsX += static_cast<int>(m_usWidth * 0.5f);
        newAbsY += m_usHeight;
    }
    SetAbsPos(newAbsX, newAbsY);
}
void CControlBase::SetCenterPos() { /* 根據需要實作 */ }
void CControlBase::SetCenterXToParent() { /* 根據需要實作 */ }
void CControlBase::SetCenterYToParent() { /* 根據需要實作 */ }
void CControlBase::SetWindowCenterAbsPos() { /* 根據需要實作 */ }
void CControlBase::Active() { m_bIsActive = true; }
void CControlBase::NoneActive() { m_bIsActive = false; }
bool CControlBase::IsActive() const { return m_bIsActive; }
void CControlBase::SetToolTipData(short usData, int type, int unk, int data, char uiType, short slotIndex, void* extraData) {
    m_ToolTipData.SetKindType(type, usData, unk, data, uiType, slotIndex, extraData);
}
void CControlBase::SetToolTipDataString(char* text, int data) {
    m_ToolTipData.SetStringType(text, data);
}
void CControlBase::SetToolTipDataDesc(unsigned short descriptionID) {
    m_ToolTipData.m_nToolTipType = 17; // 根據反編譯碼的硬編碼值
    m_ToolTipData.m_usData = descriptionID;
}
void CControlBase::SetArrayIndex(int nIndex) { m_nArrayIndex = nIndex; }
CControlBase* CControlBase::GetChild(int nIndex) { /* 根據需要實作 */ return nullptr; }
void CControlBase::MoveWindow(int nDeltaX, int nDeltaY) {
    m_nPosX += nDeltaX;
    m_nPosY += nDeltaY;
}
void CControlBase::SetChildPosMove(int nDeltaX, int nDeltaY) {
    for (CControlBase* pChild = m_pChildHead; pChild != nullptr; pChild = pChild->m_pNext)
    {
        pChild->MoveWindow(nDeltaX, nDeltaY);
    }
}
