#pragma once
#include "UI/CControlBase.h"
#include "UI/CControlImage.h"

/**
 * @class CEditBoxBackgroundImage
 * @brief 編輯框的九宮格背景圖控制項。
 *
 * 這個類別使用九個 CControlImage 物件來構成一個可伸縮的背景。
 * 它會自動調整中間部分的大小以適應不同的尺寸，同時保持邊角的原始比例。
 */
class CEditBoxBackgroundImage : public CControlBase
{
public:
    // 建構函式與解構函式
    CEditBoxBackgroundImage();
    virtual ~CEditBoxBackgroundImage();

    // 建立控制項的多載版本
    void Create(CControlBase* pParent);
    void Create(int x, int y, int width, int height, CControlBase* pParent);

    // 設定控制項的位置與尺寸
    void SetPos(int x, int y);
    void SetSize(unsigned short width, unsigned short height);

private:
    // 建立並初始化九個圖片子控制項
    void CreateChildren();

private:
    // 九宮格圖片陣列，順序為從左到右，從上到下
    // 0: 左上, 1: 中上, 2: 右上
    // 3: 左中, 4: 中中, 5: 右中
    // 6: 左下, 7: 中下, 8: 右下
    CControlImage m_Images[9];
};

