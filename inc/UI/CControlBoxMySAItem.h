#pragma once

#include "UI/CControlBoxBase.h"
#include "UI/CControlNumberBox.h"
#include "UI/CControlText.h"

/**
 * 代售物品格：顯示代售中的物品資訊（名稱、數量、價格、備註）。
 *
 * 記憶體佈局：
 *   +0..+120    CControlBoxBase
 *   +312        int16_t  m_nSlotIndex  (由 SetBoxData 設定)
 *   +314        (pad)
 *   +316        CControlNumberBox m_NumberBox
 *   +1076       CControlText      m_NameText     (物品名稱, pos 42,2)
 *   +1508       CControlText      m_QtyText      (數量/其他)
 *   +1940       CControlText      m_PriceText    (價格文字, pos 42,19)
 */
class CControlBoxMySAItem : public CControlBoxBase
{
public:
    CControlBoxMySAItem();
    virtual ~CControlBoxMySAItem();

    void CreateChildren();
    void Init();
    void ShowChildren() override;

    // a2=slotIndex, a3=unused?, a4=itemKind(lo16)+qty(hi16), Value=price, Value_4=unused?
    void SetBoxData(int16_t a2, int a3, int a4, unsigned int Value, int Value_4);

private:
    int16_t  m_nSlotIndex{ 0 };    // +312
    int16_t  m_nPad312{ 0 };       // +314
    CControlNumberBox m_NumberBox;  // +316
    CControlText      m_NameText;   // +1076
    CControlText      m_QtyText;    // +1508
    CControlText      m_PriceText;  // +1940
};
