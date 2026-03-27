#pragma once
#include <cstdint>
#include <windows.h>

/**
 * 對齊反編譯：CTextBoxScroll
 * 文字捲動框：將文字按寬度斷行，支援逐行捲動顯示
 * 內含 TextBoxScrollLineInfoListMgr 管理行資訊鏈結串列
 */

// 行資訊節點（對齊反編譯：TBSInfo，20 bytes）
struct TBSInfo
{
    TBSInfo* pPrev{ nullptr };   // +0
    TBSInfo* pNext{ nullptr };   // +4
    char*    pText{ nullptr };   // +8（動態分配的行文字）
    float    fWidth{ 0.0f };     // +12
    DWORD    dwColor{ 0 };       // +16

    ~TBSInfo();
};

// 行資訊鏈結串列管理器
class TextBoxScrollLineInfoListMgr
{
public:
    TextBoxScrollLineInfoListMgr();
    ~TextBoxScrollLineInfoListMgr();

    void Add(const char* lineText, float width, DWORD color);
    void DeleteAll();

    TBSInfo* GetHead() const { return m_pHead; }
    int      GetCount() const { return m_nCount; }

private:
    TBSInfo* m_pHead{ nullptr };   // +0
    TBSInfo* m_pTail{ nullptr };   // +4
    int      m_nCount{ 0 };        // +8
};

// CTextBoxScroll 主類別
class CTextBoxScroll
{
public:
    CTextBoxScroll();
    ~CTextBoxScroll();

    void SetTextBoxScroll(int cHeight, const char* pszFaceName, const char* lpString,
                          unsigned short a5, unsigned short a6, int a7,
                          int cWeight, int a9, int a10);
    void Reset();
    void CreateTextBoxScroll(int cHeight, const char* pszFaceName, const char* lpString,
                              unsigned short a5, unsigned short a6, int a7,
                              int cWeight, int a9, int a10);
    void SetLine(int lineIndex);
    void TextBoxDraw(int x, int y, unsigned int color, int lineSpacing,
                     char alignment, int fontWeight);

    // 取得總行數和可見行數（供 CControlScrollText 使用）
    int GetTotalLineCount() const { return m_nTotalLines; }
    int GetVisibleLineCount() const { return m_nVisibleLines; }

private:
    // 字型名稱緩衝區（offset 0, 56 bytes = 14 DWORDs）
    char m_szFontFace[56]{ 0 };

    // 字型高度 this[13]
    int m_nFontHeight{ 0 };

    // 行資訊管理器 offset +56 → this[14]
    TextBoxScrollLineInfoListMgr m_LineMgr;

    // this[17..22]
    int m_nTotalLines{ 0 };     // this[17] 總行數
    int m_nVisibleLines{ 0 };   // this[18] 可見行數
    int m_nWidth{ 0 };          // this[19]
    int m_nHeight{ 0 };         // this[20]
    int m_nCurrentLine{ 0 };    // this[21] 目前顯示起始行
    int m_nLineHeight{ 0 };     // this[22] 行高（字高 + 間距）
};
