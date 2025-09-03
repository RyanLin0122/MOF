#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlNumberBox.h"
#include "UI/CControlText.h"
#include "UI/CControlAlphaBox.h"

class CControlBoxShopItem : public CControlBoxBase
{
public:
    CControlBoxShopItem();
    virtual ~CControlBoxShopItem();

    // lifecycle
    void CreateChildren();       // 00419D50
    void ShowChildren();         // 0041A020
    void HideChildren();         // 0041A080
    void PrepareDrawing();       // 00419EA0
    void Draw();                 // 00419EE0

    // behaviors
    void SetBoxItemCountSetNumber(char flagShow, int count); // 00419E20
    void NoTrade(bool yes);                                  // 00419E60
    bool IsNoTrade() const;                                  // 00419E90
    int  SetSealStatus(int status);                          // 00419F20

    // optional：存取文字
    CControlText& GetNameText() { return m_nameText; }
    CControlText& GetDescText() { return m_descText; }
    CControlImage& GetIconNoTrade() { return m_noTradeIcon; }

private:
    // 版面元素（對應各欄位位移）
    CControlImage     m_noTradeIcon;  // +312  (資源 0x20000013, frame 0x18)
    CControlImage     m_frameL;       // +504  (資源 570425419, frame 6)
    CControlImage     m_frameR;       // +696  (資源 570425419, frame 7)
    CControlNumberBox m_countBox;     // +888  (掛在背景上)
    CControlText      m_nameText;     // +1648 (位置 44,2)
    CControlText      m_descText;     // +2080 (位置 44,16, 顏色 -65536)
    CControlAlphaBox  m_sealOverlay;  // +2512 (32x32，顏色於 SetSealStatus 設定)

    // 狀態
    bool m_noTrade = false;   // 對應 IsNoTrade()
    int  m_fxState = 0;       // 反編譯的 *((_DWORD*)this + 680)，用來決定是否額外繪邊框/背景
};
