#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlImage.h"

/**
 * @class CControlImageTriple
 * @brief 三段式可伸縮圖片控制：左右（或上下）固定，中段沿主軸縮放填滿。
 *
 * 反編譯重建重點：
 * - SetImage(group, idL, idM, idR, horizontal)
 * - SetSize(totalPrimary)：橫向=總寬，縱向=總高
 * - CreateChildren()：建立三個子 CControlImage
 */
class CControlImageTriple : public CControlBase
{
public:
    CControlImageTriple() = default;
    virtual ~CControlImageTriple() = default;

    // 對齊反編譯：Create(this, parent); 之後呼叫一個虛函式（索引+80）= CreateChildren()
    void Create(CControlBase* pParent);

    // 對齊反編譯：建立三個子圖片
    void CreateChildren();

    // 對齊反編譯：設定三張圖與方向（a6：1=水平三段、0=垂直三段）
    void SetImage(unsigned int group, uint16_t idLeftOrTop, uint16_t idMid, uint16_t idRightOrBottom, uint8_t horizontal);

    // 對齊反編譯：清空（group=0、id=0xFFFF）
    void Clear();

    // 對齊反編譯：設定主軸總尺寸（水平=總寬；垂直=總高），中段做單軸縮放
    void SetSize(uint16_t totalPrimary);

private:
    // 子圖：m_Left/m_Mid/m_Right（水平）或 m_Top/m_Mid/m_Bottom（垂直）
    CControlImage m_LeftOrTop;
    CControlImage m_Mid;
    CControlImage m_RightOrBottom;

    // 方向旗標：1=水平（X 軸拼接）、0=垂直（Y 軸拼接）
    uint8_t m_bHorizontal{ 1 };
};
