#pragma once

#include "UI/CControlBase.h"
#include "UI/CControlText.h"

/**
 * 彈出選單按鈕項：內含一行文字，具備按下/抬起偏移與滑鼠顏色回饋。
 *
 * 繼承 CControlBase（非 CControlBoxBase）。
 *
 * 記憶體佈局（CControlBase 之後）：
 *   +120~+132   float[4] m_fColor1_RGBA   Normal 色 (case 7 / SetTextColorMouseInput 的 a6-a9)
 *   +136~+148   float[4] m_fColor2_RGBA   Active/MouseOver 色 (Active() / case 4 使用)
 *   +152        int      m_nBtnDown       按鈕偏移旗標
 *   +156        CControlText m_Text       文字控制項
 *   +304        m_Text.m_TextColor          合成後 ARGB 文字色（CControlText 內部偏移 148）
 */
class CControlBoxPopMenu : public CControlBase
{
public:
    CControlBoxPopMenu();
    virtual ~CControlBoxPopMenu();

    void CreateChildren();
    void Init();

    void Create(int x, int y, uint16_t w, uint16_t h, CControlBase* pParent);

    // 設定兩組顏色：
    //   color2(a2-a5) → +136（mouseOver / Active）
    //   color1(a6-a9) → +120（normal / case 7）
    // 同時以 color2 計算並寫入 m_Text 的文字色
    int SetTextColorMouseInput(float color2R, float color2G, float color2B, float color2A,
                               float color1R, float color1G, float color1B, float color1A);

    void NoneActive();
    void Active();
    void Show();

    int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7);

    void SetText(int stringId);

    void ButtonPosDown();
    void ButtonPosUp();

private:
    static int FloatToByte(float v);
    static DWORD PackARGB(float r, float g, float b, float a);

private:
    // +120：Color1 (normal / case 7 使用)
    float m_fColor1R{ 0.0f };
    float m_fColor1G{ 0.0f };
    float m_fColor1B{ 0.0f };
    float m_fColor1A{ 1.0f };

    // +136：Color2 (Active / mouseOver / case 4 使用)
    //       ctor 設定初始 Active 色；SetTextColorMouseInput 可覆蓋為 mouseOver 色
    float m_fColor2R;
    float m_fColor2G;
    float m_fColor2B;
    float m_fColor2A;

    // +152：按鈕偏移狀態
    int m_nBtnDown{ 0 };

    // +156：文字控制項
    CControlText m_Text;

    // 注意：文字 ARGB 色存於 m_Text.m_TextColor（CControlText 內部偏移 148，
    //       對應 CControlBoxPopMenu 全域偏移 304）。
    //       不需要額外成員變數，直接透過 m_Text.SetTextColor() 設定。
};
