#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlNumberBox.h"
#include "UI/CControlImage.h"

// 現金商城倉庫物品格（含數量框與一個 Icon）
class CControlBoxCashShopStrgItem : public CControlBoxBase
{
public:
    CControlBoxCashShopStrgItem();
    ~CControlBoxCashShopStrgItem();

    // 建立子控制項（覆寫自類型家族慣例）
    void CreateChildren();

    // 覆寫：隱藏子控制項
    void HideChildren() override;

    // 設定 / 取得 現金道具 ID
    void        SetCashItemID(uint64_t id, bool showIcon);
    uint64_t    GetCashItemID() const;

private:
    // 成員佈局對應：
    // [ +320 ] 子物件：數字框（其內含 AlphaBox 與 Text；與 IDA 位移相符）
    // [ +1080] 子物件：Icon 圖像
    uint64_t        m_cashItemID = 0;     // 對應 *((_QWORD*)this + 39)（= 8*39 = +312）
    CControlNumberBox m_countBox;         // +320
    CControlImage     m_icon;             // +1080
};
