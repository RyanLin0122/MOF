#include "UI/CControlText.h"
#include <string>
#include <cstring> // for strcpy, strcmp
#include <cstdio>  // for sprintf

// --- 模擬的外部依賴 ---
class DCTTextManager {
public:
    char* GetText(int id) { return (char*)""; }
    char* GetParsedText(int id, int, int) { return (char*)""; }
};
DCTTextManager g_DCTTextManager;

struct stFontInfo {
    int height;
    int weight;
    char fontName[128];
};

class MoFFont {
public:
    stFontInfo* GetFontInfo(const char* fontName) { return nullptr; }
    void SetFont(int height, const char* fontName, int weight) {}
    float* GetTextLength(float* pSize, const char* fontName, const char* text, int weight) { 
        if(pSize) { pSize[0] = 0; pSize[1] = 0; }
        return pSize;
    }
    void SetTextLineA(int x, int y, unsigned long color, const char* text, int align, int, int) {}
    void SetTextLineShadow(int x, int y, unsigned long color, const char* text, int align) {}
    void SetTextBoxA(stRect* rect, unsigned long color, const char* text, int lineSpace, int align, bool) {}
    int GetLineCountByWidth(unsigned short width, const char* text, const char* fontName, int fontHeight, int fontWeight) { return 1; }
};
MoFFont g_MoFFont;

void Numeric2MoneyByComma(unsigned int value, char* buffer, int size, int) {}
// --- 模擬結束 ---


/**
 * CControlText 建構函式
 * 初始化文字相關屬性。
 */
CControlText::CControlText()
{
    m_TextData = "";
    m_nFontHeight = 12;
    m_nFontWeight = 400; // Normal
    m_nTextAlignment = 0; // Left
    m_dwTextColor = 0xFF600A09; // 預設顏色
    m_dwTextShadowColor = 0;
    m_dwTextBackgroundColor = 0;
    m_nMultiLineSpace = 5;
    m_bWantSpaceFirstByte = false;
    m_ucLineCount = 1;
    m_bRecalculateLines = true; // 初始時需要計算
    memset(m_szFontName, 0, sizeof(m_szFontName));
    m_bIsEnable = false; // 文字預設不可互動
}

CControlText::~CControlText()
{
}

/**
 * 清除資料，不僅清除基底類別，也清空文字內容。
 */
void CControlText::ClearData()
{
    CControlBase::ClearData();
    ClearText();
}

/**
 * 設定文字內容。
 * @param szText 要顯示的文字字串。
 */
void CControlText::SetText(const char* szText)
{
    if (!szText)
    {
        ClearText();
        return;
    }

    if (m_TextData != szText)
    {
        m_bRecalculateLines = true; // 文字已變更，需要重新計算行數
        m_TextData = szText;
    }
}

void CControlText::SetText(int nDCTID)
{
    if (nDCTID >= 0)
    {
        SetText(g_DCTTextManager.GetText(nDCTID));
    }
}

void CControlText::SetParsedText(int nDCTID)
{
    if (nDCTID >= 0)
    {
        SetText(g_DCTTextManager.GetParsedText(nDCTID, 0, 0));
    }
}

/**
 * 清空文字內容並重設狀態。
 */
void CControlText::ClearText()
{
    if (!m_TextData.empty())
    {
        m_TextData.clear();
        m_bRecalculateLines = true;
    }
}

/**
 * 繪製文字。
 * 根據單行或多行模式、是否有陰影等屬性進行繪製。
 */
void CControlText::Draw()
{
    if (!m_bIsVisible || m_TextData.empty())
    {
        CControlBase::Draw();
        return;
    }

    float absPos[2];
    GetAbsPos(absPos);

    g_MoFFont.SetFont(m_nFontHeight, m_szFontName, m_nFontWeight);
    const char* szText = m_TextData.c_str();

    // 判斷是多行文字框還是單行文字
    bool isMultiLine = (m_usWidth > 0 && m_usHeight > 0);

    if (isMultiLine)
    {
        stRect rc = { absPos[0], absPos[1], absPos[0] + m_usWidth, absPos[1] + m_usHeight };
        
        // 繪製背景顏色
        if (m_dwTextBackgroundColor != 0)
        {
            // CControlAlphaBox m_bgBox; // 假設有一個背景方塊來繪製
            // m_bgBox.SetAttr(...)
            // m_bgBox.Draw();
        }

        // 繪製陰影/外框
        if (m_dwTextShadowColor != 0)
        {
            stRect shadowRc;
            // 透過8個方向的偏移來實現外框效果
            shadowRc = { rc.left - 1, rc.top, rc.right - 1, rc.bottom };
            g_MoFFont.SetTextBoxA(&shadowRc, m_dwTextShadowColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
            shadowRc = { rc.left - 1, rc.top - 1, rc.right - 1, rc.bottom - 1 };
            g_MoFFont.SetTextBoxA(&shadowRc, m_dwTextShadowColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
            shadowRc = { rc.left, rc.top - 1, rc.right, rc.bottom - 1 };
            g_MoFFont.SetTextBoxA(&shadowRc, m_dwTextShadowColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
            shadowRc = { rc.left + 1, rc.top - 1, rc.right + 1, rc.bottom - 1 };
            g_MoFFont.SetTextBoxA(&shadowRc, m_dwTextShadowColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
            shadowRc = { rc.left + 1, rc.top, rc.right + 1, rc.bottom };
            g_MoFFont.SetTextBoxA(&shadowRc, m_dwTextShadowColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
            shadowRc = { rc.left + 1, rc.top + 1, rc.right + 1, rc.bottom + 1 };
            g_MoFFont.SetTextBoxA(&shadowRc, m_dwTextShadowColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
            shadowRc = { rc.left, rc.top + 1, rc.right, rc.bottom + 1 };
            g_MoFFont.SetTextBoxA(&shadowRc, m_dwTextShadowColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
            shadowRc = { rc.left - 1, rc.top + 1, rc.right - 1, rc.bottom + 1 };
            g_MoFFont.SetTextBoxA(&shadowRc, m_dwTextShadowColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
        }

        // 繪製主要文字
        g_MoFFont.SetTextBoxA(&rc, m_dwTextColor, szText, m_nMultiLineSpace, m_nTextAlignment, m_bWantSpaceFirstByte);
    }
    else // 單行文字
    {
        if (m_dwTextShadowColor != 0)
        {
            g_MoFFont.SetTextLineShadow(absPos[0], absPos[1], m_dwTextShadowColor, szText, m_nTextAlignment);
        }
        g_MoFFont.SetTextLineA(absPos[0], absPos[1], m_dwTextColor, szText, m_nTextAlignment, -1, -1);
    }

    CControlBase::Draw();
}

/**
 * 獲取渲染後的文字寬度和高度。
 */
float* CControlText::GetTextLength(float* pSize)
{
    if (m_TextData.empty()) {
        pSize[0] = 0;
        pSize[1] = 0;
        return pSize;
    }
    return g_MoFFont.GetTextLength(pSize, m_szFontName, m_TextData.c_str(), m_nFontWeight);
}

/**
 * 獲取多行文字在指定寬度下計算出的總高度。
 */
int CControlText::GetCalcedTextBoxHeight(unsigned short usWidth)
{
    if (m_TextData.empty()) return 0;

    unsigned short width = (usWidth == 0) ? m_usWidth : usWidth;
    if (width == 0) return 0;
    
    return (m_nFontHeight + m_nMultiLineSpace) * GetMultiTextLineCount(width);
}

/**
 * 使用快取機制獲取多行文字的行數。
 */
unsigned char CControlText::GetMultiTextLineCount(unsigned short usWidth)
{
    if (m_TextData.empty()) return 0;
    
    // 如果文字沒有變更，直接返回快取的值
    if (!m_bRecalculateLines) return m_ucLineCount;

    // 否則，重新計算，更新快取，並清除髒標記
    m_bRecalculateLines = false;
    unsigned short width = (usWidth == 0) ? m_usWidth : usWidth;
    m_ucLineCount = g_MoFFont.GetLineCountByWidth(width, m_TextData.c_str(), m_szFontName, m_nFontHeight, m_nFontWeight);
    return m_ucLineCount;
}

// --- 其他 Set/Get 函式實作 ---
void CControlText::SetTextItoa(int nValue) { char buffer[32]; sprintf(buffer, "%d", nValue); SetText(buffer); }
void CControlText::SetTextMoney(unsigned int uiValue) { char buffer[32]; Numeric2MoneyByComma(uiValue, buffer, 32, 0); SetText(buffer); }
void CControlText::SetTextMoney(int nDCTID, unsigned int uiValue) { /* 實作 */ }
void CControlText::SetParsedTextMoney(int nDCTID, unsigned int uiValue) { /* 實作 */ }
const char* CControlText::GetText() const { return m_TextData.c_str(); }
void CControlText::SetFontHeight(int nHeight) { m_nFontHeight = nHeight; m_bRecalculateLines = true; }
void CControlText::SetFontWeight(int nWeight) { m_nFontWeight = nWeight; m_bRecalculateLines = true; }
void CControlText::SetControlSetFont(const char* szFontName) {
    stFontInfo* pFontInfo = g_MoFFont.GetFontInfo(szFontName);
    if (pFontInfo) {
        m_nFontHeight = pFontInfo->height;
        m_nFontWeight = pFontInfo->weight;
        strncpy(m_szFontName, pFontInfo->fontName, 255);
        m_szFontName[255] = '\0';
        m_bRecalculateLines = true;
    }
}
void CControlText::SetMultiLineSpace(int nSpace) { m_nMultiLineSpace = nSpace; m_bRecalculateLines = true; }
void CControlText::SetMultiLineSize(short usWidth, short usHeight) { m_usWidth = usWidth; m_usHeight = usHeight; m_bRecalculateLines = true; }
void CControlText::SetTextPosToParentCenter() { 
    CControlBase::SetCenterPos();
    m_nTextAlignment = 1; // 1=Center
    // 原始碼中有 Y-5 的微調
    m_nPosY -= 5;
}
int CControlText::GetFontHeight() const { return m_nFontHeight; }
const char* CControlText::GetFontFace() const { return m_szFontName; }
int CControlText::GetCharByteByLine(unsigned char* pLineBytes, int nMaxLines) { return 0; /* 需 MoFFont 完整實作 */ }
int CControlText::GetMultiLineSpace() const { return m_nMultiLineSpace; }
void CControlText::SetWantSpaceFirstByte(bool bWantSpace) { m_bWantSpaceFirstByte = bWantSpace; }
bool CControlText::IsStringData() const { return !m_TextData.empty(); }

/**
 * @brief 判斷一個點是否在矩形內 (windows.h PtInRect 的簡易本地實作)。
 */
bool PtInRect(const stRect* rc, stPoint pt)
{
    return (pt.x >= rc->left && pt.x < rc->right && pt.y >= rc->top && pt.y < rc->bottom);
}

/**
 * 判斷一個點是否在文字的渲染範圍內。
 * 這個版本是 CControlText 特有的，它基於文字的實際渲染大小，
 * 而不是 CControlBase 的 m_usWidth 和 m_usHeight。
 */
bool CControlText::PtInCtrl(stPoint pt)
{
    if (!m_bIsVisible || m_TextData.empty())
    {
        return false;
    }

    float textSize[2];
    GetTextLength(textSize);

    if (textSize[0] == 0 || textSize[1] == 0)
    {
        return false;
    }

    float absPos[2];
    GetAbsPos(absPos);

    stRect rc = { absPos[0], absPos[1], absPos[0] + textSize[0], absPos[1] + textSize[1] };

    return PtInRect(&rc, pt);
}

