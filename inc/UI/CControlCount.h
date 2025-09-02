#pragma once
#include <cstdint>
#include <algorithm>
#include "CControlBase.h"
#include "CControlImage.h"

/**
 * @brief 三段式計量條控制（左端/中段/右端），以目前值在 [min, max] 範圍內的比率，決定中段填滿量。
 *
 * 反編譯重點對照：
 * - Create(x, y, totalWidth, grp, idL, idM, idR, parent)
 * - SetImage(grp, idL, idM, idR)：三張圖水平相鄰排列
 * - SetCountRange(cur, max, min)：注意參數順序依反編譯；內部會夾制 cur ∈ [min, max]
 * - SetupCurCount()：依 cur/max 計算需填長度，決定是否顯示三圖，並以 midWidth * scale 推進右端圖的 AbsX
 */
class CControlCount : public CControlBase
{
public:
    CControlCount() = default;
    virtual ~CControlCount() = default;

    // 反編譯签名：Create(this, x, y, totalWidth, group, idL, idM, idR, parent)
    void Create(int x, int y, int totalWidth,
        unsigned int group, int idLeft, int idMid, int idRight,
        CControlBase* pParent);

    // 設定三張圖片與初始水平位置
    void SetImage(unsigned int group, int idLeft, int idMid, int idRight);

    // 設定計量範圍（依反編譯：a2=cur、a3=max、a4=min）
    void SetCountRange(int64_t cur, int64_t max, int64_t min);

    // 依目前值重新計算顯示（反編譯：SetupCurCount）
    void SetupCurCount();

private:
    // 三段圖片
    CControlImage m_Left;   // +120
    CControlImage m_Mid;    // +312
    CControlImage m_Right;  // +504

    // 範圍/目前值（對應 QWORD 索引 87,88,89）
    int64_t m_Min{ 0 };       // +174/+175（QWORD idx 87）
    int64_t m_Cur{ 0 };       // +176/+177（QWORD idx 88）
    int64_t m_Max{ 0 };       // +178/+179（QWORD idx 89）

    // 中段縮放係數（對應 *((float*)this + 87)）
    float   m_MidScale{ 0.0f };
};
