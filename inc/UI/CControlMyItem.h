#pragma once
#include <cstdint>
#include "UI/CControlBoxBase.h"  // 假設存在；與專案其它控制一致
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

/**
 * @brief 列表/箱型項目：含一張圖與三段文字。
 *
 * 佈局（相對本控制的左上角）：
 * - 圖片：   (1, 2)
 * - TextA：  (40, 2)  多行區域 70x12
 * - TextB：  (150, 2) 多行區域 75x12
 * - TextC：  (204, 2) 多行區域 110x12
 */
class CControlMyItem : public CControlBoxBase
{
public:
    CControlMyItem();
    ~CControlMyItem();

    // 建立子控制（對應反編譯 CreateChildren）
    void CreateChildren();

    // 顯示子控制並重置內部狀態（對應反編譯 ShowChildren）
    void ShowChildren();

private:
    // 成員配置對齊反編譯之偏移（說明性命名）
    CControlImage m_Icon;   // 位移約 +312
    CControlText  m_TextA;  // 位移約 +504
    CControlText  m_TextB;  // 位移約 +936
    CControlText  m_TextC;  // 位移約 +1368

    // 對應 *((_DWORD*)this + 16) 的狀態欄位（用途未知，依反編譯在 ShowChildren() 清 0）
    int m_State16{ 0 };
};
