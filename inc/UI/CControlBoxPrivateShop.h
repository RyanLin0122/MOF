#pragma once

#include "UI/CControlBoxBase.h"
#include "UI/CControlNumberBox.h"
#include "UI/CControlText.h"

struct stSellingItem;

/**
 * 個人商店物品格：顯示一個商品的圖示、名稱、價格文字與數量。
 *
 * 記憶體佈局：
 *   +0..+120    CControlBoxBase (含 CControlImage m_Background)
 *   +312        CControlNumberBox m_NumberBox   (數量顯示)
 *   +1072       CControlText      m_NameText    (物品名稱)
 *   +1504       CControlText      m_PriceText   (價格文字)
 *   +1936       uint16  m_wItemKind
 *   +1938       uint16  m_wInvenSlot
 *   +1940       uint16  m_wQty
 *   +1942       (pad)
 *   +1944       uint32  m_dwPrice
 *   +1948       uint32  m_dwParam
 *   +1952       int     m_bNotOpen
 */
class CControlBoxPrivateShop : public CControlBoxBase
{
public:
    CControlBoxPrivateShop();
    virtual ~CControlBoxPrivateShop();

    void CreateChildren();

    // 設定「尚未開店」的外觀（只顯示指定圖片）
    void SetNotOpenBox(unsigned int giid, unsigned short block);

    // 依物品 kind / 數量 / 價格 更新盒子資訊
    void SetBoxData(unsigned short itemKind, unsigned short qty, unsigned int price);

    // 以 stSellingItem 一次填入所有資料
    void SetItemData(stSellingItem* pItem);

    // 更新數量；若 qty==0 則清空資料。回傳 1=成功，0=已清空
    int SetItemQty(unsigned short qty);

    // 清除所有商品資料（偏移 +1936 起 16 bytes 歸零）
    void ClearItemData();

    // 是否有商品
    BOOL IsData();

private:
    CControlNumberBox m_NumberBox;   // +312
    CControlText      m_NameText;    // +1072
    CControlText      m_PriceText;   // +1504

    // 商品記錄（+1936）
    uint16_t m_wItemKind{ 0 };       // *((_WORD*)this + 968)
    uint16_t m_wInvenSlot{ 0 };      // *((_WORD*)this + 969)
    uint16_t m_wQty{ 0 };            // *((_WORD*)this + 970)
    uint16_t m_wPad{ 0 };
    uint32_t m_dwPrice{ 0 };         // *((_DWORD*)this + 486)
    uint32_t m_dwParam{ 0 };         // *((_DWORD*)this + 487)
    int      m_bNotOpen{ 0 };        // *((_DWORD*)this + 488)
};
