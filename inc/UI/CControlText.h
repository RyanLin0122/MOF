#ifndef CCONTROLTEXT_H
#define CCONTROLTEXT_H

#include "UI/CControlBase.h"
#include <string>

// 前向宣告，代表外部的字型管理器和字型資訊結構
class MoFFont;
struct stFontInfo;

/**
 * @class CControlText
 * @brief 用於顯示文字的 UI 控制項。
 *
 * 繼承自 CControlBase，提供豐富的文字樣式設定，包括字體、顏色、陰影、
 * 多行顯示和自動換行等功能。
 */
class CControlText : public CControlBase
{
public:
    // 建構函式與解構函式
    CControlText();
    virtual ~CControlText();

    // --- 覆寫的虛擬函式 ---
    virtual void ClearData() override;
    virtual void Draw() override;
    
    // --- 文字內容設定 ---
    void SetText(const char* szText);
    void SetText(int nDCTID); // 從文字管理器設定
    void SetParsedText(int nDCTID); // 從文字管理器設定並解析特殊碼
    void ClearText();

    // --- 各種格式化設定文字 ---
    void SetTextItoa(int nValue);
    void SetTextMoney(unsigned int uiValue);
    void SetTextMoney(int nDCTID, unsigned int uiValue);
    void SetParsedTextMoney(int nDCTID, unsigned int uiValue);

    // --- 字型與樣式設定 ---
    void SetFontHeight(int nHeight);
    void SetFontWeight(int nWeight);
    void SetControlSetFont(const char* szFontName); // 從預設集中設定字型
    void SetTextColor(unsigned long dwColor);
    void SetTextShadowColor(unsigned long dwColor);
    void SetTextBackgroundColor(unsigned long dwColor);

    // --- 排版設定 ---
    void SetMultiLineSpace(int nSpace);
    void SetMultiLineSize(short usWidth, short usHeight);
    void SetTextAlignment(int nAlignment);
    void SetWantSpaceFirstByte(bool bWantSpace); // 多行文字，首字元前是否留空
    void SetTextPosToParentCenter();

    // --- 資訊獲取 ---
    const char* GetText() const;
    float* GetTextLength(float* pSize);
    int GetCalcedTextBoxHeight(unsigned short usWidth = 0);
    int GetFontHeight() const;
    const char* GetFontFace() const;
    bool IsStringData() const;
    int GetCharByteByLine(unsigned char* pLineBytes, int nMaxLines);
    int GetMultiLineSpace() const;
    unsigned char GetMultiTextLineCount(unsigned short usWidth = 0);
    bool PtInCtrl(stPoint pt);
protected:
    // --- 成員變數 ---
    std::string m_TextData;         // 儲存文字內容
    int m_nFontHeight;              // 字體高度
    int m_nFontWeight;              // 字體粗細 (e.g., 400=Normal, 700=Bold)
    int m_nTextAlignment;           // 文字對齊方式 (0=Left, 1=Center, 2=Right)
    unsigned long m_dwTextColor;    // 文字顏色
    unsigned long m_dwTextShadowColor; // 文字陰影/外框顏色
    unsigned long m_dwTextBackgroundColor; // 文字背景顏色
    int m_nMultiLineSpace;          // 多行文字的行距
    bool m_bWantSpaceFirstByte;     // 多行文字首行首字元是否留空
    
    unsigned char m_ucLineCount;    // 快取的行數
    bool m_bRecalculateLines;       // 是否需要重新計算行數的髒標記 (Dirty Flag)

    char m_szFontName[256];         // 字體名稱
};

#endif // CCONTROLTEXT_H
