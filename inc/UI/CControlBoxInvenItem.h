#pragma once
#include "UI/CControlBoxBase.h"
#include "UI/CControlNumberBox.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CControlImage.h"

// 0: 無 / 1: 黃框(與主框) / 2: 綠框(與主框)
enum class SealStatus : int
{
    None = 0,
    Yellow = 1,
    Green = 2,
};

class CControlBoxInvenItem : public CControlBoxBase
{
public:
    CControlBoxInvenItem();
    ~CControlBoxInvenItem() override;

    // 子控制建立/顯示/隱藏
    void CreateChildren();
    void ShowChildren(int showItemImage);   // a2
    void HideChildren();

    // 覆寫：讓外框在需要時也走 Prepare/Draw
    void PrepareDrawing() override;
    void Draw() override;

    // 封印狀態：0=關閉(全部隱藏)，1=黃框，2=綠框
    int  SetSealStatus(int status);

    // 透明度
    void SetAlpha(int a);

    // 數量顯示：a2==1 或 a3<=0 則隱藏，否則顯示數字盒並設值
    void SetBoxItemCountSetNumber(char showCount, int count);

private:
    // +312 起的複合子物件（對齊反編譯碼的結構佈局）
    CControlNumberBox m_countBox;     // offset +312（其內含 AlphaBox(+120) 與 Text(+328)）
    CControlAlphaBox  m_sealBox;      // offset +1072
    CControlImage     m_itemImage;    // offset +1280 主物品圖
    CControlImage     m_frameYellow;  // offset +1472 黃色外框
    CControlImage     m_frameGreen;   // offset +1664 綠色外框

    // *(this+1856)；Prepare/Draw 判斷用（1/2 才渲染外框）
    int               m_sealStatus;   // == 0/1/2
};
