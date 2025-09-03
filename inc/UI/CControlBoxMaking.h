#pragma once

#include "UI/CControlBoxBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

// 簡單資訊框：左側一個小圖(偏移+312)，右側一行文字(偏移+504)
class CControlBoxMaking : public CControlBoxBase
{
public:
    CControlBoxMaking();
    virtual ~CControlBoxMaking();

    // 生成子物件（依照反編譯碼：先建 image(+312)，再建 text(+504) 並定位）
    void CreateChildren();

    // 顯示時覆寫：呼叫 CControlBase::ShowChildren() 後把某旗標清 0
    // （與反編譯碼保持一致：它不是叫 CControlBoxBase::ShowChildren，而是調 CControlBase 版本）
    void ShowChildren() override;

private:
    // 版面與成員配置需與反編譯位移一致
    // 父類 CControlBoxBase 內含一個 m_bg(CControlImage) 於 +120
    // 這裡新增：
    CControlImage m_icon; // +312
    CControlText  m_text; // +504
};
