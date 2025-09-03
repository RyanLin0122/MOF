#pragma once

#include <string>
#include <cstdint>
#include <windows.h>

#include "UI/CControlBase.h"
#include "Font/MoFFont.h"
#include "Text/DCTTextManager.h"
#include "global.h"

// 前置宣告
struct stFontInfo;

class CControlText : public CControlBase
{
public:
    CControlText();
    virtual ~CControlText();

    // ---- 文字 ----
    void SetText(const char* text);
    void SetText(int stringId);
    void SetText(int stringId, int number);
    void SetTextFmtW(const wchar_t* fmt, ...);
    void SetParsedText(int stringId);
    void SetTextItoa(int value);
    void SetTextMoney(unsigned int value);
    void SetTextMoney(int textIdFmt, unsigned int value);
    void SetParsedTextMoney(int textIdFmt, unsigned int value);

    const char* GetText() const { return m_Text.c_str(); }
    void ClearText();

    // ---- 外觀設定 ----
    void SetFontHeight(int h);
    void SetFontWeight(int w);
    void SetControlSetFont(const char* fontKey);
    int  GetFontHeight() const { return m_FontHeight; }
    const char* GetFontFace();

    void SetMultiLineSpace(int space);
    int  GetMultiLineSpace() const { return m_LineSpacing; }

    // 預設的多行寬/高（若本身寬高為 0 時使用）
    void SetMultiLineSize(uint16_t width, int height/*保留*/);

    // 計算多行包框之高度（以目前字型與行距推估）
    int  GetCalcedTextBoxHeight(uint16_t width /*0=使用預設寬*/);

    // want space first byte（仍保留與原始行為一致的旗標）
    void SetWantSpaceFirstByte(int v) { m_WantSpaceFirstByte = v; }

    // 是否有字串內容
    BOOL IsStringData() const { return !m_Text.empty(); }

    // 依寬度取得每行字元數（byte），回傳行數
    int  GetCharByteByLine(unsigned char* lineBreakBytes, int maxLines);

    // 取得多行數量（快取版）
    unsigned char GetMultiTextLineCount(uint16_t width /*0=使用預設寬*/);

    // 位置輔助
    void SetTextPosToParentCenter();

    // ---- 繪製 ----
    virtual void Draw() override;

    // 顏色設定
    void SetTextColor(DWORD c) { m_TextColor = c; }
    void SetShadowColor(DWORD c) { m_ShadowColor = c; }
    void SetOutlineColor(DWORD c) { m_OutlineColor = c; }
    void SetAlignment(char a) { m_Alignment = a; }

    // 命中測試（覆蓋成以文字尺寸為準）
    BOOL PtInCtrl(int x, int y);

    // 與基底同名，加入本類別自清理
    void ClearData();

    // UTF-8 / Wide / EUC-KR 輸入接口
    void SetTextUtf8(const char* u8);
    void SetTextW(const wchar_t* ws);
    void SetTextKR(const char* eucKR);

private:
    void EnsureDefaultFontFace();
    void SyncWideFace();

private:
    std::string m_Text;                 // 文字

    int   m_FontHeight{ 12 };          // 字高
    int   m_FontWeight{ 400 };         // 粗細
    DWORD m_TextColor{ 0xFF600A09 };  // 主要字色（對應 -10483191）
    DWORD m_ShadowColor{ 0 };           // 陰影色（多行：以 +1,+1 偏移繪製）
    DWORD m_OutlineColor{ 0 };           // 外框色（多行：八方向一圈）

    int   m_LineSpacing{ 5 };           // 多行行距
    int   m_WantSpaceFirstByte{ 0 };     // 仍保留給 SetTextBoxA 的 unknownFlag
    char  m_Alignment{ 0 };           // 對齊方式（沿用 MoFFont 的定義）

    uint16_t m_DefaultMultiWidth{ 0 };  // 預設多行寬（寬為 0 時使用）
    uint16_t m_DefaultMultiHeight{ 0 };  // 未使用，為還原對齊

    // 字型名稱（保留 ANSI 與寬字元兩份，便於與 MoFFont 介面相容）
    char    m_FontFaceA[256]{ 0 };
    wchar_t m_FontFaceW[128]{ 0 };

    // 行數快取
    mutable bool         m_LineCountDirty{ true };
    mutable unsigned char m_CachedLineCount{ 1 };
};