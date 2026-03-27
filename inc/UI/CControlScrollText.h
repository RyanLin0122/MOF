#pragma once
#include <cstdint>
#include <cstring>
#include "UI/CControlScrollBar.h"
#include "UI/CTextBoxScroll.h"

/**
 * 對齊反編譯：CControlScrollText
 * 帶捲動條的文字框，繼承 CControlScrollBar，內含 CTextBoxScroll
 */
class CControlScrollText : public CControlScrollBar
{
public:
    CControlScrollText();
    virtual ~CControlScrollText();

    // 建立：指定文字框位置、大小、捲動條位置、大小、顏色、父控制、字型等
    void Create(int x, int y, unsigned short w, unsigned short h,
                int textOffsetX, int textOffsetY,
                unsigned short textW, unsigned short textH,
                unsigned int textColor, CControlBase* pParent,
                int fontHeight, int fontWeight,
                char* fontFace, int shadowFlag, char* initText);

    virtual void Show() override;
    virtual void Draw() override;

    void SetText(char* text, int startLine);
    void RefreshText(int startLine);

    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;
    void ChildKeyInputProcess(int msg, int a3, int x, int y, int a6, int a7);

    void ScrollTextSetLine();
    void SetScrollPosBottom();
    void SetShadowColor(unsigned int color);
    void SetTextY(int y);
    void SetTextSize(unsigned short w, unsigned short h);
    int  GetTextY();

private:
    // 字型名稱（對齊反編譯 (char*)this + 3068）
    char m_szFontFace[56]{ 0 };

    // 文字緩衝區指標（對齊 this[766]）
    char* m_pText{ nullptr };

    // 文字顏色（對齊 this[777]）
    unsigned int m_TextColor{ 0xFF000000 };

    // 陰影（對齊 this[778..780]）
    unsigned int m_ShadowColor2{ 0xFF000000 };
    int m_bHasShadow{ 0 };     // this[779]
    int m_nShadowFlag{ 0 };    // this[780]

    // 文字偏移（對齊 this[804..805]）
    int m_nTextOffsetX{ 0 };
    int m_nTextOffsetY{ 0 };

    // 文字大小（對齊 word this[1612..1613]）
    unsigned short m_usTextW{ 0 };
    unsigned short m_usTextH{ 0 };

    // 字型參數（對齊 this[775..776]）
    int m_nFontHeight{ 11 };
    int m_nFontWeight{ 400 };

    // CTextBoxScroll（對齊 offset +3124）
    CTextBoxScroll m_TextBox;
};
