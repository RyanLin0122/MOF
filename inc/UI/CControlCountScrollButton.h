#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlButton.h"

/**
 * @brief 可拖曳的數值捲動按鈕（沿著一段「滑軌層」水平移動）
 *
 * 反編譯對照：
 * - 成員：+120 = CControlButton（拖曳鈕）、+844 = CControlBase（滑軌層）
 * - SetButtonPos(x,y,w,h)：設定本控制位置與尺寸，並把按鈕相對放在 (0,0)
 * - SetCountLayer(..., a4, a5)：把滑軌層相對放在 (0,0)，紀錄兩個 16 位參數（僅保存，不影響邏輯）
 * - SetCountInfo(a2,a3,a4)：Max=a2, Cur=a3, Min=a4；然後 ControlRebuild()
 * - ControlRebuild()：按 Cur/Max 佈局按鈕絕對 X
 * - CalcCurCount(mouseX)：由滑鼠 X 反推 Cur（含邊界夾制與 -1 調整）
 * - ChildKeyInputProcess()：只對 m_Button 之 0/2/3 事件（Down/Move/Up）處理拖曳
 */
class CControlCountScrollButton : public CControlBase
{
public:
    CControlCountScrollButton();
    virtual ~CControlCountScrollButton();

    // 事件轉發（僅關心來自 m_Button 的 0/2/3）
    void ChildKeyInputProcess(int msg, CControlBase* child, int x, int y, int a6, int a7);

    // 佈局/資料設定
    void SetButtonPos(int x, int y, int16_t w, int16_t h);
    void SetCountLayer(int /*x*/, int /*y*/, int16_t p1, int16_t p2); // 依反編譯：x,y 未用，僅保存 p1/p2
    void SetCountInfo(int a2_max, int a3_cur, int a4_min);

    // 由外部變更資料後重建位置
    void ControlRebuild();

    // 將滑鼠絕對 X 轉換為 Cur，並重建
    void CalcCurCount(int mouseAbsX);

    // 子件公開存取（如需綁圖、調色等）
    CControlButton& Button() { return m_Button; }
    CControlBase& Layer() { return m_Layer; }

private:
    CControlButton m_Button;   // +120：拖曳鈕
    CControlBase   m_Layer;    // +844：滑軌層（計算寬度與絕對起點）

    // 計數資訊（對應 this+[241..244]）
    int m_Cur{ 0 };              // +241：目前值
    int m_Min{ 0 };              // +242：最小值（僅保存，重建時未直接使用）
    int m_Max{ 0 };              // +243：最大值
    int m_Dragging{ 0 };         // +244：是否正在拖曳（0/1）

    // 額外保存的兩個 16-bit 參數（SetCountLayer 設定，僅保存）
    int16_t m_Param1{ 0 };       // +438
    int16_t m_Param2{ 0 };       // +439
};
