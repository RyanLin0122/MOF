#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlNumberBox.h"

/**
 * SA (拍賣) 收取道具格
 * 繼承 CControlBoxBase，顯示一個道具圖示 + 數量
 *
 * 成員位移（對照反編譯）：
 *   +312 m_itemIndex   (DWORD) — 存放外部 index
 *   +316 m_itemKind    (WORD)  — 道具 Kind
 *   +320 m_countBox    (CControlNumberBox)
 */
class CControlBoxSAReceiveItem : public CControlBoxBase
{
public:
    CControlBoxSAReceiveItem();
    virtual ~CControlBoxSAReceiveItem();

    void CreateChildren();    // 00419AB0
    void Init();              // 00419AD0

    // 設定道具資料 (00419AF0)
    void SetBoxData(unsigned int itemIdx, unsigned short itemKind, unsigned short quantity);

    // 取得 item index (00419B50)
    unsigned int GetItemIndex() const;

private:
    unsigned int   m_itemIndex{ 0 };   // +312 (DWORD)
    unsigned short m_itemKind{ 0 };    // +316 (WORD)
    unsigned short _pad318{ 0 };

    CControlNumberBox m_countBox;      // +320
};
