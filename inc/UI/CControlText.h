#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include "UI/CControlBase.h"

// ===== 對齊你提供的基準：外部系統與函式 =====
struct stFontInfo;       // 只前向宣告，實際由字型系統提供
class  MoFFont;
class  DCTTextManager;

// ===== 無 windef.h：自訂 Rect 與工具，語義等同 Win32 PtInRect（右/下開區間）=====
struct RectI__ { int left, top, right, bottom; };
inline void SetRect__(RectI__* r, int L, int T, int R, int B) { r->left = L; r->top = T; r->right = R; r->bottom = B; }
inline bool PtInRectOpen__(const RectI__* r, int x, int y) {
    return (x >= r->left) && (x < r->right) && (y >= r->top) && (y < r->bottom);
}

// ====== CControlText ====================================================
class CControlText : public CControlBase
{
public:
    CControlText();
    virtual ~CControlText();

    // 建立
    virtual void Create(CControlBase* pParent) override;
    virtual void Create(int x, int y, CControlBase* pParent) override;
    virtual void Create(int x, int y, uint16_t w, uint16_t h, CControlBase* pParent) override;

    // 反編譯對應 API
    void ClearData();                    // 覆寫，清除本物件的文字資料
    void SetText(const char* s);         // SetText(char*)
    void SetText(int stringId);          // SetText(int)
    void SetParsedText(int stringId);    // SetParsedText(int)
    void ClearText();                    // 清成空字串
    void SetTextItoa(int value);         // 整數轉字串（十進位）
    void SetTextMoney(unsigned int value);
    void SetTextMoney(int fmtId, unsigned int value);
    void SetParsedTextMoney(int fmtId, unsigned int value);

    void SetFontHeight(int h);           // [34]
    void SetFontWeight(int w);           // [35]
    void SetControlSetFont(const char* fontName); // 由 MoFFont::GetFontInfo 設定高度/粗細/字型名

    void SetMultiLineSpace(int px);      // [40]
    void SetMultiLineSize(uint16_t w, int h/*存入 WORD*/); // [16],[17]

    // 繪製 / 命中
    virtual void Draw() override;
    bool PtInCtrl(stPoint pt);

    // 量測與資訊
    void GetTextLength(int& outW, int& outH); // 回傳像素寬高（走 MoFFont::GetTextLength）
    int  GetCalcedTextBoxHeight(uint16_t width /*0 表示取 m_multiWidth*/);

    int  GetFontHeight() const { return m_fontHeight; } // 對應 0x4255C0
    const char* GetFontFace() const { return m_fontFace.c_str(); } // 0x4255D0

    int  GetCharByteByLine(uint8_t* outBytesPerLine, int maxLines);
    int  GetMultiLineSpace() const { return m_lineSpace; } // 0x425620
    void SetWantSpaceFirstByte(int v) { m_wantSpaceFirstByte = v; } // 0x425630
    bool IsStringData() const { return m_isStringData; } // 0x425640

    uint8_t GetMultiTextLineCount(uint16_t width /*0=取 m_multiWidth*/); // 0x425650

    void SetTextPosToParentCenter(); // 0x4255A0

    // 顏色：主色 / 外框(描邊) / 陰影（對應 [37][38][39]）
    void SetMainColor(uint32_t argb) { m_colorMain = argb; }
    void SetEdgeColor(uint32_t argb) { m_colorEdge = argb; }
    void SetShadowColor(uint32_t argb) { m_colorShadow = argb; }

    // 其他 flag
    void SetDrawFlag(uint8_t f) { m_drawFlag = f; } // 對應 *((_BYTE*)this + 144)

private:
    // 任何會影響版面/行數的變更，都要把 m_needRecalcLineCount = 1
    void MarkLayoutDirty();

private:
    // 對齊基準成員語意（不逐位對齊，但行為一致）
    std::string m_text;                // [31..33] 反編譯中的 std::string
    bool        m_isStringData{ false }; // [32] 是否有有效字串（反編譯在多處用來 gate）

    uint8_t     m_cachedLineCount{ 1 };  // [168] 緩存行數
    int         m_needRecalcLineCount{ 1 }; // [43] 是否需重算行數

    int         m_fontHeight{ 12 };      // [34]
    int         m_fontWeight{ 400 };     // [35]
    std::string m_fontFace;            // [176..] 長度 0x100，這裡用 std::string

    uint32_t    m_colorMain{ 0xFF5F5F59 }; // [37] -10483191 (對應 0xFF5F5F59)
    uint32_t    m_colorEdge{ 0 };        // [38]
    uint32_t    m_colorShadow{ 0 };      // [39]

    int         m_lineSpace{ 5 };        // [40]
    int         m_wantSpaceFirstByte{ 0 }; // [41]

    uint16_t    m_multiWidth{ 0 };       // [16] 多行計算寬（0 表示未設）
    uint16_t    m_multiCellH{ 0 };       // [17] 反編譯存成 WORD，這裡按 WORD 存

    uint8_t     m_drawFlag{ 0 };         // [144] 傳給 MoFFont 的旗標
    bool        m_centeredYMinus5{ false }; // [36] 在置中後是否做 y-5 的調整（記錄狀態，方便除錯）
};