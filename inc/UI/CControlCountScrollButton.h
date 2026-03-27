#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlButton.h"

// 可拖曳的數值捲動按鈕（沿著一段「滑軌層」水平移動）
// 反編譯對照：
//   this+120: CControlButton m_Button（拖曳鈕）
//   this+844: CControlBase   m_Layer（滑軌層）
//   this[241]: m_Cur
//   this[242]: m_Min
//   this[243]: m_Max
//   this[244]: m_Dragging
//   WORD[438]: m_Param1
//   WORD[439]: m_Param2
class CControlCountScrollButton : public CControlBase
{
public:
    CControlCountScrollButton();
    virtual ~CControlCountScrollButton();

    // 事件轉發（來自 m_Button 的 0/2/3）
    void ChildKeyInputProcess(int msg, CControlBase* child, int x, int y, int a6, int a7);

    // 佈局/資料設定
    void SetButtonPos(int x, int y, int16_t w, int16_t h);
    void SetCountLayer(int x, int y, int16_t p1, int16_t p2);
    void SetCountInfo(int a2_max, int a3_cur, int a4_min);

    // 由外部變更資料後重建位置
    void ControlRebuild();

    // 將滑鼠絕對 X 轉換為 Cur，並重建
    void CalcCurCount(int mouseAbsX);

    // 子件公開存取
    CControlButton& Button() { return m_Button; }
    CControlBase& Layer() { return m_Layer; }
    int GetCur() const { return m_Cur; }

private:
    CControlButton m_Button;   // +120
    CControlBase   m_Layer;    // +844

    int m_Cur{ 0 };            // +241 (DWORD)
    int m_Min{ 0 };            // +242
    int m_Max{ 0 };            // +243
    int m_Dragging{ 0 };       // +244
};
