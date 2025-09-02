#pragma once
#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlButton.h"
#include <cstdint>

/// Page up/down 控制，顯示 "cur/max" 文字與左右翻頁按鈕。
class CControlPageScroll : public CControlBase
{
public:
    CControlPageScroll();
    virtual ~CControlPageScroll();

    // 由建構子呼叫
    void CreateChildren();
    void Init();

    // 子控制項事件（符合原函式簽章與語義）
    // a2: 事件代碼（0=按下, 2=移動, 3=放開），a3: 來源子控件
    virtual void ChildKeyInputProcess(int a2, CControlBase* a3, int a4, int a5, int a6, int a7);

    // 自動翻頁（a2!=0 向右；a2==0 向左；到端點時依反編譯碼做回繞/重設）
    void AutoKeyInput(int a2);

    // 設定頁域：每頁數量、總數量、是否將目前頁設回最小頁
    void SetPageRange(uint16_t itemsPerPage, uint16_t totalItems, int resetToMin);

    // 調整目前頁（±1）
    void ChangeCurPage(char delta);

    // 是否有變更（讀出後會清 0；對齊 IDA 的 IsChangedCurPage）
    int IsChangedCurPage();

    // 顯示 "cur/max"
    void PrintTextPage();

    // 直接設定目前頁（不超過界限的情況下）
    void SetCurPage(char page);

    // 讀目前頁
    uint8_t GetCurPage() const { return m_curPage; }

    // 目前是否為最大頁
    bool IsCurrentMaxPage() const { return m_curPage == m_maxPage; }

protected:
    // 依當前頁決定左右鍵 Active/NoneActive
    void DecideButtonActive();

private:
    // 介面元素
    CControlText   m_text;     // +128
    CControlButton m_btnPrev;  // +560
    CControlButton m_btnNext;  // +1284

    // 內部狀態（對齊反編譯：+120 = min、+121 = cur、+122 = max、+31 = changed flag）
    uint8_t m_minPage = 1;    // 預設 1
    uint8_t m_curPage = 1;    // 預設 1
    uint8_t m_maxPage = 1;    // 預設 1
    int     m_changed = 0;    // 是否自上次查詢後變更頁面

    // 反編譯碼中在 CreateChildren 內有幾個 this 上的旗標寫入（例如 +68、+155、+336），
    // 這裡不特別外露行為（多半是 UI 框架內部用途），必要時可補齊為成員或轉為呼叫點行為。
};

// 全域輸入消耗旗標（對齊反編譯 dword_AFD34C）
extern int g_MouseCaptureDirty;
