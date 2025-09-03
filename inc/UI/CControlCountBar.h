#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlButton.h"
#include "UI/CControlImage.h"
#include "global.h"

/**
 * @brief 可拖曳的三段式數值條，含左右箭頭按鈕與可點選/拖曳的軌道區域。
 *
 * 反編譯對照重點：
 * - 兩顆按鈕：this+120（左）、this+844（右）
 * - 三張圖：  this+1568（左帽）、+1760（中段）、+1952（右帽）
 * - 軌道區：  this+2144（用來偵測點選/拖曳）
 * - 目前/最大/最小：this+[566]/[567]/[568]
 * - 拖曳中旗標：this+2276 (BYTE)
 * - 中段縮放係數：this+[449] (float)
 */
class CControlCountBar : public CControlBase
{
public:
    CControlCountBar();
    virtual ~CControlCountBar();

    // 事件處理：與既有系統一致 (msg,key,x,y,a6,a7)
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;
    virtual void ChildKeyInputProcess(int msg, CControlBase* child, int a4, int a5, int a6, int a7);

    // 設定左右箭頭的總區域（決定本控制項的寬高與左右箭頭位置）
    // 反編譯：SetArrowPos(this, a2, a3, a4, a5)
    void SetArrowPos(int xLeft, int yTop, int xRight, int unused);

    // 設定數值條的擺放層與區域（也會同步三張圖位置連續排列）
    // 反編譯：SetCountLayer(this, a2, a3, a4/*W*/, a5/*H*/)
    void SetCountLayer(int x, int y, int16_t width, int height);

    // 設定數值資訊；反編譯：a2=max, a3=cur, a4=min
    void SetCountInfo(int maxValue, int curValue, int minValue);

    // 由左右箭頭驅動的遞減/遞增（會夾在 [min,max] 內）
    void ArrowPosLeft(int step);
    void ArrowPosRight(int step);

private:
    // 依目前數值重建顯示（對應 ControlRebuild）
    void ControlRebuild();

    // 依滑鼠絕對座標 X 計算目前值（對應 CalcCurCount）
    void CalcCurCount(int mouseAbsX);

private:
    // 子控制
    CControlButton m_btnLeft;    // +120
    CControlButton m_btnRight;   // +844
    CControlImage  m_imgLeft;    // +1568
    CControlImage  m_imgMid;     // +1760
    CControlImage  m_imgRight;   // +1952
    CControlBase   m_track;      // +2144（用於命中/拖曳的區域）

    // 內部狀態／參數
    int m_unused406{ 0 }, m_unused454{ 0 }, m_unused502{ 0 }, m_unused550{ 0 };
    int m_cur{ 0 };    // +566
    int m_max{ 0 };    // +567
    int m_min{ 0 };    // +568
    bool m_dragging{ false }; // +2276

    // 中段縮放係數（讓右帽位置 = mid.AbsX + mid.Width * m_scale）
    float m_scale{ 0.0f }; // +449

    // 保留：SetCountLayer() 內寫入的兩個 16bit 參數（對照 +1088/+1089）
    uint16_t m_layerW{ 0 };
    uint16_t m_layerH{ 0 };
};
