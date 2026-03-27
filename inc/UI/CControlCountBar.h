#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlButton.h"
#include "UI/CControlImage.h"

// 可拖曳的三段式數值條，含左右箭頭按鈕與可點選/拖曳的軌道區域
// 反編譯對照：
//   this+120:  CControlButton m_btnLeft
//   this+844:  CControlButton m_btnRight
//   this+1568: CControlImage  m_imgLeft
//   this+1760: CControlImage  m_imgMid
//   this+1952: CControlImage  m_imgRight
//   this+2144: CControlBase   m_track
//   this[566]: m_cur
//   this[567]: m_max
//   this[568]: m_min
//   BYTE[2276]: m_dragging
//   float[449]: m_imgMid.m_fScaleX（不是獨立欄位）
//   WORD[1088]: m_layerW
//   WORD[1089]: m_layerH
class CControlCountBar : public CControlBase
{
public:
    CControlCountBar();
    virtual ~CControlCountBar();

    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;
    virtual void ChildKeyInputProcess(int msg, CControlBase* child, int a4, int a5, int a6, int a7);

    void SetArrowPos(int xLeft, int yTop, int xRight, int unused);
    void SetCountLayer(int x, int y, int16_t width, int height);
    void SetCountInfo(int maxValue, int curValue, int minValue);
    void ArrowPosLeft(int step);
    void ArrowPosRight(int step);

    // 公開存取子控制
    CControlButton& BtnLeft() { return m_btnLeft; }
    CControlButton& BtnRight() { return m_btnRight; }
    CControlImage& ImgLeft() { return m_imgLeft; }
    CControlImage& ImgMid() { return m_imgMid; }
    CControlImage& ImgRight() { return m_imgRight; }
    int GetCur() const { return m_cur; }

private:
    void ControlRebuild();
    void CalcCurCount(int mouseAbsX);

private:
    CControlButton m_btnLeft;
    CControlButton m_btnRight;
    CControlImage  m_imgLeft;
    CControlImage  m_imgMid;
    CControlImage  m_imgRight;
    CControlBase   m_track;

    int m_cur{ 0 };
    int m_max{ 0 };
    int m_min{ 0 };
    unsigned char m_dragging{ 0 };  // 反編譯：BYTE[2276]
    // 對齊反編譯：float[449] 實際上是 m_imgMid.m_fScaleX，不是獨立欄位
};
