#pragma once
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
typedef int BOOL;
#endif

#include "UI/CControlBoxBase.h"
#include "UI/CControlButton.h"
#include "UI/CControlText.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CControlImage.h"
#include "UI/CControlNumberImage.h"

struct stCashShopItemList;

/**
 * @brief 現金商城商品格控制
 *
 * 反編譯對照成員偏移（原始二進位，僅供參考）：
 *   +312  stCashShopItemList*  m_pItemList
 *   +316  int                  m_canEquip   (init=1)
 *   +320  int                  m_isWishList (init=0)
 *   +324  CControlButton       m_BtnBuy
 *   +1048 CControlButton       m_BtnGift
 *   +1772 CControlButton       m_BtnWish
 *   +2496 CControlButton       m_BtnPackageDetail
 *   +3220 CControlText         m_TextName
 *   +3652 CControlText         m_TextPrice
 *   +4084 CControlAlphaBox     m_AlphaBox
 *   +4292 CControlImage        m_ImgQuantityBg
 *   +4484 CControlNumberImage  m_NumberImage
 *   +8456 CControlImage        m_ImgFrame
 */
class CControlBoxCashShop : public CControlBoxBase
{
public:
    CControlBoxCashShop();
    virtual ~CControlBoxCashShop();

    // 初始化（CControlBoxBase::Init 後將 m_bIsVisible 設為 false）
    void Init();

    // 子控制建立
    void CreateChildren();

    // 顯示子控制（覆寫基底）
    virtual void ShowChildren() override;

    // 設定商品資料
    // a2=imageId, a3=blockId, a4=名稱字串, Value=價格, Value_4=折扣價,
    // a7=數量, a8=框架blockId, a9=商品類型
    void SetData(unsigned int a2, uint16_t a3, char* a4,
                 int Value, int Value_4,
                 uint16_t a7, uint16_t a8, uint8_t a9);

    // 商品清單指標
    void SetCashShopItemList(stCashShopItemList* a2);
    stCashShopItemList* GetCashShopItemList() const;

    // 可裝備狀態
    void SetCanEquip(int a2);
    int  CanEquip() const;

    // 願望清單狀態
    void SetWishList(int a2);
    int  IsWishList() const;

    // 取得按鈕
    CControlButton* GetButtonBuy();
    CControlButton* GetButtonGift();
    CControlButton* GetButtonWish();
    CControlButton* GetButtonPackageDetail();

private:
    // 資料成員
    stCashShopItemList* m_pItemList{ nullptr };   // 反編譯 *((_DWORD *)this + 78)
    int                 m_canEquip{ 1 };           // 反編譯 *((_DWORD *)this + 79)
    int                 m_isWishList{ 0 };         // 反編譯 *((_DWORD *)this + 80)

    // 子控制（成員宣告順序對齊建構/析構順序）
    CControlButton      m_BtnBuy;              // +324
    CControlButton      m_BtnGift;             // +1048
    CControlButton      m_BtnWish;             // +1772
    CControlButton      m_BtnPackageDetail;    // +2496
    CControlText        m_TextName;            // +3220
    CControlText        m_TextPrice;           // +3652
    CControlAlphaBox    m_AlphaBox;            // +4084
    CControlImage       m_ImgQuantityBg;       // +4292
    CControlNumberImage m_NumberImage;         // +4484
    CControlImage       m_ImgFrame;            // +8456
};
