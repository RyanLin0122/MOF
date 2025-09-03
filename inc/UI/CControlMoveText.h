#pragma once
#include "UI/CControlText.h"

// 反編譯結構 (節點) 對應：大小 0x30
// +0  : prev
// +4  : next
// +8  : xStart
// +12 : yStart
// +16 : xEnd
// +20 : yEnd
// +24 : counter/remaining (用於輪播等待；建構時設 0，SetMoveText/ReturnValue 設 3)
// +28 : timerLeft（Poll 時會遞減；ReturnValue 流程會用到）
// +32 : text（主要顯示字串）
// +36 : altText（第二行/陰影字串，依 style 顯示，可為 nullptr）
// +40 : style(低位元組)：0=無、1/3=白(-1)色、2=特殊(-250)色
// +44 : retFlag（SetMoveTextReturnValue 會設為 1）
//
// 注意：上面是「實際在位移中使用」的欄位 —— 我有把名稱取成語意化。
struct MTSInfo {
    MTSInfo* prev{ nullptr };
    MTSInfo* next{ nullptr };
    int      xStart{ 0 };
    int      yStart{ 0 };
    int      xEnd{ 0 };
    int      yEnd{ 0 };
    int      counter{ 0 };       // 初始化 0
    int      timerLeft{ 0 };     // 初始化 0（在 loop 模式裡會被遞減）
    char* text{ nullptr };    // 主要顯示字串（malloc / free）
    char* altText{ nullptr }; // 陰影/描邊要畫的第二份（可為 nullptr）
    int      style{ 0 };         // 低位元組用作 style（1/3 白、2 特色）
    int      retFlag{ 0 };       // SetMoveTextReturnValue 會設 1
};

class CControlMoveText : public CControlText
{
public:
    CControlMoveText();
    ~CControlMoveText() override;

    // 新增一筆可移動文字（等同反編譯 SetMoveText）
    // lpString: 主字串
    // altStr:   陰影/第二行要畫的文字（a10!=0 時才會用到），可為 nullptr
    // x0,y0 → x1,y1: 位移起迄
    // useCut:   是否要以 [min(x0,x1), max(x0,x1)] 作為文字裁切區間
    // doLoop:   是否在結束/等待後跳到下一筆（或循環）
    // shadowStyle(a10): 0=無、1/3=白(-1)、2=特殊(-250)
    void SetMoveText(
        const char* lpString,
        const char* altStr,
        int x0, int y0, int x1, int y1,
        bool useCut,
        bool doLoop,
        unsigned char shadowStyle);

    // 插入一筆在「目前節點之後」並回傳該節點（等同 SetMoveTextReturnValue）
    // 注意：這版沒有 altStr，並且 retFlag 會被設為 1
    MTSInfo* SetMoveTextReturnValue(
        const char* lpString,
        int x0, int y0, int x1, int y1,
        bool useCut,
        bool doLoop,
        unsigned char shadowStyle);

    // 調整「目前節點」的位移座標（a6=useCut）
    // 回傳 1 表示成功（有目前節點），0 表示沒有任何節點
    int SetCurrentMovePos(int x0, int y0, int x1, int y1, bool useCut);

    // 一次性調整所有節點座標（等同 SetAllPos，且強制 useCut=true）
    void SetAllPos(int x0, int y0, int x1, int y1);

    // 設定裁切區（畫字時只畫 [cutL, cutR] 之間；-1, -1 表示取消）
    void SetTextCutPos(int cutL, int cutR);

    // 每幀更新（等同 Poll）
    void Poll();

    // 覆寫 Draw：依據目前節點畫主文字與可選 alt（陰影/描邊）一份
    void Draw() override;

private:
    // 佇列操作（等同 Add / InsertCurrent / DeleteAll / DeleteHeadText / DeleteText）
    MTSInfo* Add();
    MTSInfo* InsertCurrent();
    void     DeleteAll();
    int      DeleteHeadText(); // 移除頭節點；成功回 1、空佇列回 0
    int      DeleteText();     // 移除「目前節點」；若空佇列回 0，否則回 1

    // 位移一步：根據方向把 (x,y) 以步長 step 移向 endX，回傳是否「已越過 endX」
    bool ProcessMoveText(int& x, int& y, int step);

    // 計算一段字串的像素寬度（供 cut 與「反向起點」修正使用）
    int  MeasureTextWidth(const char* text) const;

private:
    // 反編譯 this+108, +109, +110, +111, +112, +113
    MTSInfo* m_head{ nullptr };     // +108
    MTSInfo* m_tail{ nullptr };     // +109
    MTSInfo* m_cur{ nullptr };      // +110
    int      m_loop{ 0 };           // +111（是否循環/接續）
    int      m_cutL{ -1 };          // +112
    int      m_cutR{ -1 };          // +113
};
