#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlAlphaBox.h"
#include "UI/CControlText.h"

/**
 * 數字顯示盒：
 * - +120: CControlAlphaBox (底框，13,-2, 寬19, 高15，黑色，alpha≈0.502)
 * - +328: CControlText     (字型 "NumberBox"，相對於底框座標 9,2)
 */
class CControlNumberBox : public CControlBase
{
public:
    CControlNumberBox();
    virtual ~CControlNumberBox();

    void SetNumber(int value);     // 對齊 0041F190
    int  GetNumber();              // 對齊 0041F1B0

private:
    // 子件（位移對齊）
    CControlAlphaBox m_Frame;  // +120
    CControlText     m_Text;   // +328

    // 反編譯中被設定的兩個欄位（+118、+119）；語義未出現於他處，保留以對齊行為
    int m_FlagA{ 0 };            // *((DWORD*)this + 118) = 1
    int m_FlagB{ -1 };           // *((DWORD*)this + 119) = -1
};
