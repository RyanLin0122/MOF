#pragma once
#include "UI/CControlText.h"

// 反編譯結構 (節點) 對應：大小 0x30 (48 bytes)
// +0  : prev
// +4  : next
// +8  : xStart
// +12 : yStart
// +16 : xEnd
// +20 : yEnd
// +24 : counter (Poll 時會遞減)
// +28 : timerLeft
// +32 : text (malloc 分配的字串指標)
// +36 : altText (第二行/陰影字串)
// +40 : style (低位元組)：0=無、1/3=白(-1)色、2=特殊(-250)色
// +44 : retFlag (SetMoveTextReturnValue 會設為 1)
struct MTSInfo {
    MTSInfo* prev{ nullptr };    // +0
    MTSInfo* next{ nullptr };    // +4
    int      xStart{ 0 };       // +8
    int      yStart{ 0 };       // +12
    int      xEnd{ 0 };         // +16
    int      yEnd{ 0 };         // +20
    int      counter{ 0 };      // +24
    int      timerLeft{ 0 };    // +28
    char*    text{ nullptr };    // +32
    char*    altText{ nullptr }; // +36
    int      style{ 0 };        // +40
    int      retFlag{ 0 };      // +44
};

class CControlMoveText : public CControlText
{
public:
    CControlMoveText();
    ~CControlMoveText() override;

    // 對齊反編譯 004211E0
    void SetMoveText(
        const char* lpString,
        char* altStr,
        int x0, int y0, int x1, int y1,
        int useCut,
        int doLoop,
        char shadowStyle);

    // 對齊反編譯 00421AD0
    MTSInfo* SetMoveTextReturnValue(
        const char* lpString,
        int x0, int y0, int x1, int y1,
        int useCut,
        int doLoop,
        char shadowStyle);

    // 對齊反編譯 00421390
    int SetCurrentMovePos(int x0, int y0, int x1, int y1, int useCut);

    // 對齊反編譯 00421480
    void SetAllPos(int x0, int y0, int x1, int y1);

    // 對齊反編譯 00421460
    void SetTextCutPos(int cutL, int cutR);

    // 對齊反編譯 00421540
    void Poll();

    // 對齊反編譯 00421680
    void Draw() override;

private:
    // 佇列操作
    MTSInfo* Add();
    MTSInfo* InsertCurrent();
    void     DeleteAll();
    int      DeleteHeadText();
    int      DeleteText();

    // 位移處理
    int ProcessMoveText(int* pX, int* pY, int step);

private:
    MTSInfo* m_head{ nullptr };     // +108
    MTSInfo* m_tail{ nullptr };     // +109
    MTSInfo* m_cur{ nullptr };      // +110
    int      m_loop{ 0 };           // +111
    int      m_cutL{ -1 };          // +112
    int      m_cutR{ -1 };          // +113
};
