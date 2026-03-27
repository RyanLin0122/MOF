#pragma once
#include <cstdint>
#include <algorithm>
#include "CControlBase.h"
#include "CControlImage.h"

// 三段式計量條控制（左端/中段/右端）
// 反編譯對照：
//   this+120: CControlImage m_Left
//   this+312: CControlImage m_Mid
//   this+504: CControlImage m_Right
//   QWORD[87]: m_Min
//   QWORD[88]: m_Cur
//   QWORD[89]: m_Max
//   float[87]: m_MidScale (注意: 與 QWORD 不衝突，不同偏移)
class CControlCount : public CControlBase
{
public:
    CControlCount();
    virtual ~CControlCount();

    // 反編譯：Create(this, x, y, totalWidth, group, idL, idM, idR, parent)
    void Create(int x, int y, int totalWidth,
        unsigned int group, int idLeft, int idMid, int idRight,
        CControlBase* pParent);

    // 設定三張圖片
    void SetImage(unsigned int group, int idLeft, int idMid, int idRight);

    // 設定計量範圍（反編譯：a2=cur, a3=max, a4=min）
    void SetCountRange(int64_t cur, int64_t max, int64_t min);

    // 依目前值重新計算顯示
    void SetupCurCount();

private:
    CControlImage m_Left;
    CControlImage m_Mid;
    CControlImage m_Right;

    int64_t m_Min{ 0 };
    int64_t m_Cur{ 0 };
    int64_t m_Max{ 0 };

    float m_MidScale{ 0.0f };
};
