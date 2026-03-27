#pragma once

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdarg>
#include <cwchar>
#include <windows.h>

#include "UI/CControlBase.h"

// 前置宣告
struct stFontInfo;

class CControlText : public CControlBase
{
public:
    CControlText();
    virtual ~CControlText();

    // ---- 文字 ----
    void SetText(char* text);
    void SetText(const char* text);
    void SetText(int stringId);
    void SetParsedText(int stringId);
    void SetTextItoa(int value);
    void SetTextMoney(unsigned int value);
    void SetTextMoney(int textIdFmt, unsigned int value);
    void SetParsedTextMoney(int textIdFmt, unsigned int value);
    void SetTextFmtW(const wchar_t* format, ...);

    char* GetText();
    const char* GetText() const;
    void ClearText();

    // ---- 外觀設定 ----
    void SetFontHeight(int h);
    void SetFontWeight(int w);
    int  GetFontWeight() const { return m_FontWeight; }
    void SetControlSetFont(const char* fontKey);
    int  GetFontHeight();
    char* GetFontFace();

    void SetTextColor(DWORD color) { m_TextColor = color; }
    void SetShadowColor(DWORD color) { m_ShadowColor = color; }
    void SetAlignment(int alignment) { m_isCentered = alignment; }

    void GetTextPixelSize(int* pWidth, int* pHeight);

    void SetMultiLineSpace(int space);
    int  GetMultiLineSpace();

    void SetMultiLineSize(short width, int height);

    int  GetCalcedTextBoxHeight(unsigned short width);

    void SetWantSpaceFirstByte(int v);

    BOOL IsStringData();

    int  GetCharByteByLine(unsigned char* lineBreakBytes, int maxLines);

    // 取得文字尺寸（回傳 packed DWORD: LOWORD=width, HIWORD=height）
    DWORD* GetTextLength(DWORD* outSize);

    // 取得多行數量（快取版）
    unsigned char GetMultiTextLineCount(unsigned short width);

    // 位置輔助
    void SetTextPosToParentCenter();

    // ---- 繪製 ----
    virtual void Draw() override;

    // 命中測試
    BOOL PtInCtrl(int x, int y);

    // 清除資料
    void ClearData();

    // ---- 成員變數（對齊反編譯偏移） ----
    // std::string 在 offset +120 (this+30/31/32/33)
    std::string m_Text;                     // this[31] = ptr, this[32] = size, this[33] = capacity

    int   m_FontHeight{ 12 };               // this[34]
    int   m_FontWeight{ 400 };              // this[35]
    int   m_isCentered{ 0 };               // this[36]
    DWORD m_TextColor{ 0xFF600A09 };        // this[37] = -10483191
    DWORD m_ShadowColor{ 0 };              // this[38]
    DWORD m_OutlineColor{ 0 };             // this[39]
    int   m_LineSpacing{ 5 };              // this[40]
    int   m_WantSpaceFirstByte{ 0 };       // this[41]

    // 多行相關（對齊 this[8]，即 m_usWidth/m_usHeight）
    // m_usWidth 和 m_usHeight 繼承自 CControlBase

    // 字型名稱（ANSI）offset +176，256 bytes
    char  m_FontFaceA[256]{ 0 };           // (char*)this + 176

    // 行數快取
    int           m_LineCountDirty{ 1 };   // this[43]
    unsigned char m_CachedLineCount{ 1 };  // byte at this+168
};

// 工具函式
void Numeric2MoneyByComma(unsigned int value, char* out, int outSize, int flagUnused);
