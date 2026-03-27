#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlImageTriple.h"
#include "UI/CControlButton.h"

/**
 * 對齊反編譯：CControlScrollBar
 * 捲動條控制項，含：
 * - CControlImageTriple (背景三段式)
 * - CControlButton x3 (上箭頭、下箭頭、滑塊)
 * - 捲動範圍 / 頁面大小 / 步進 / 回呼
 */
class CControlScrollBar : public CControlBase
{
public:
    CControlScrollBar();
    virtual ~CControlScrollBar();

    void Create(int x, int y, unsigned short w, unsigned short h, CControlBase* pParent, char a7);

    void SetHeight(unsigned short h);
    void SetArrowUpImage(unsigned int giid, int a3, int a4, int a5, int a6);
    void SetThumbImage(unsigned int giid, int a3, int a4, int a5, int a6);
    void SetArrowDownImage(unsigned int giid, int a3, int a4, int a5, int a6);

    void SetCallFunc(int obj, int funcPtr, int param1, int param2, int param3);
    void CallScrollFunc();
    void HideBackground();

    void SetScrollRange(int min, int max, int pageSize, int lineStep, int clearFlag);
    void Scroll(int mode, int param, int updateThumb);
    void ProcessMoveThumb(int absY);
    void SetThumbPosition(int scrollPos);

    int  GetScrollPos();
    int  GetScrollMax();
    BOOL IsInViewLegion(unsigned short idx);
    void ClearScroll();

    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;
    void ChildKeyInputProcess(int msg, int a3, int x, int y, int a6, int a7);

    virtual void Show();

private:
    // 對齊反編譯 this[30..47] 等偏移
    int m_scrollMin{ 0 };         // this[30]
    int m_scrollMax{ 1 };         // this[31]
    int m_scrollPos{ 0 };         // this[32]
    int m_pageSize{ 1 };          // this[33]
    int m_lineStep{ 1 };          // this[34]

    // 滑軌邊界
    int m_thumbMinY{ 0 };         // this[36] (上箭頭高度)
    int m_thumbMaxY{ 0 };         // this[37] (下箭頭 Y - thumb H)
    unsigned short m_trackLen{ 0 }; // word this[71] (可用滑動長度)

    // 縮放比
    double m_pixelsPerStep{ 0 };  // double this[19]
    double m_stepsPerPixel{ 0 };  // double this[20]

    // 回呼
    int m_callbackObj{ 0 };       // this[42]
    int m_callbackFunc{ 0 };      // this[44]
    int m_callbackOffset{ 0 };    // this[45]
    int m_callbackParam1{ 0 };    // this[46]
    int m_callbackParam2{ -1 };   // this[47]

    // 子控制項
    CControlImageTriple m_Background;  // offset +192
    CControlButton m_ArrowUp;          // offset +892
    CControlButton m_ArrowDown;        // offset +1616
    CControlButton m_Thumb;            // offset +2340

    // 啟用旗標（對齊 this[597]）
    int m_bEnabled{ 0 };
};
