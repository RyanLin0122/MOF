#ifndef CCONTROLBASE_H
#define CCONTROLBASE_H

#include "UI/stToolTipData.h"

// Windows POINT 結構的簡易定義，以避免引入 <windows.h>
struct stPoint {
    long x;
    long y;
};

/**
 * @struct stRect
 * @brief 自訂的矩形結構，用來取代 RECT。
 */
struct stRect {
    long left;
    long top;
    long right;
    long bottom;
};

/**
 * @class CControlBase
 * @brief 所有 UI 控制項的基底類別。
 *
 * 定義了 UI 元件的通用屬性與行為，例如位置、大小、父子階層關係、
 * 繪製、輸入處理以及工具提示等。
 */
class CControlBase
{
public:
    // 建構函式與虛擬解構函式
    CControlBase();
    virtual ~CControlBase();

    // --- 虛擬函式 (可被子類別覆寫) ---
    virtual void Create(CControlBase* pParent);
    virtual void Create(int nPosX, int nPosY, CControlBase* pParent);
    virtual void Create(int nPosX, int nPosY, unsigned short usWidth, unsigned short usHeight, CControlBase* pParent);

    // 處理鍵盤與滑鼠輸入事件
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7);

    // 準備繪製，通常用於計算座標或更新狀態
    virtual void PrepareDrawing();

    // 執行繪製操作
    virtual void Draw();

    // 初始化登入相關資料
    virtual void InitLogIn();

    // 清除控制項資料
    virtual void ClearData();

    // 顯示與隱藏控制項
    virtual void Show();
    virtual void Hide();

    // 顯示/隱藏所有子控制項
    void ShowChildren();
    void HideChildren();

    // --- 位置與大小相關 ---
    void SetPos(int nPosX, int nPosY);
    void SetPos(stPoint pos); // 多載版本
    void SetX(int nPosX);
    void SetY(int nPosY);
    int GetX() const;
    int GetY() const;
    unsigned short GetWidth() const;
    unsigned short GetHeight() const;

    // 設定與獲取相對於最上層父物件的絕對位置
    void SetAbsPos(int nAbsX, int nAbsY);
    void SetAbsPos(stPoint pos); // 多載版本
    void SetAbsX(int nAbsX);
    void SetAbsY(int nAbsY);
    int GetAbsX();
    int GetAbsY();
    float* GetAbsPos(float* pPoint);

    // 將控制項的中心點設置到指定的絕對座標
    void SetCenterPos(int nAbsCenterX, int nAbsCenterY);
    // 將控制項置於父物件或螢幕的中央
    void SetCenterPos();
    void SetCenterXToParent();
    void SetCenterYToParent();
    void SetWindowCenterAbsPos();

    // --- 狀態管理 ---
    void Active();
    void NoneActive();
    bool IsActive() const;
    bool IsVisible() const { return m_bIsVisible; }

    // --- 工具提示 (ToolTip) ---
    void SetToolTipData(short usData, int type, int unk, int data, char uiType, short slotIndex, void* extraData);
    void SetToolTipDataString(char* text, int data);
    void SetToolTipDataDesc(unsigned short descriptionID);

    // --- 其他 ---
    bool PtInCtrl(stPoint pt); // 判斷座標點是否在控制項範圍內
    void SetArrayIndex(int nIndex);

protected:
    // --- 虛擬函式 (供子類別實作的事件) ---
    virtual void OnShow() {}
    virtual void OnHide() {}

    // --- 子控制項管理 ---
    void AddChild(CControlBase* pChild);
    void DelLastChild();
    CControlBase* GetChild(int nIndex);
    CControlBase* FindClickedChild(stPoint  pt); // 尋找被點擊的子控制項

    void MoveWindow(int nDeltaX, int nDeltaY);
    void SetChildPosMove(int nDeltaX, int nDeltaY);

protected:
    // --- 成員變數 ---
    CControlBase* m_pParent;      // 指向父控制項
    CControlBase* m_pPrev;        // 指向前一個兄弟控制項
    CControlBase* m_pNext;        // 指向後一個兄弟控制項
    CControlBase* m_pChildHead;   // 指向子控制項鏈結串列的頭部
    CControlBase* m_pChildTail;   // 指向子控制項鏈結串列的尾部

    int m_nPosX;                  // 相對於父控制項的 X 座標
    int m_nPosY;                  // 相對於父控制項的 Y 座標
    unsigned short m_usWidth;     // 寬度
    unsigned short m_usHeight;    // 高度

    float m_fScaleX;              // X 軸縮放比例
    float m_fScaleY;              // Y 軸縮放比例

    bool m_bIsActive;             // 是否啟用 (可與玩家互動)
    bool m_bIsVisible;            // 是否可見
    bool m_bAbsorbClick;          // 是否吸收點擊事件 (不傳遞給子控制項)
    bool m_bIsEnable;             // 是否啟用 (接收事件)
    bool m_bPassKeyInputToParent; // 是否將輸入事件傳遞給父控制項
    bool m_bHasToolTip;           // 是否有工具提示
    int  m_nArrayIndex;           // 在容器中的索引

    stToolTipData m_ToolTipData;  // 工具提示資料

    bool m_bIsCenterOrigin;       // 座標原點是否在中心
    int m_nControlID;             // 控制項的類型 ID
};

#endif // CCONTROLBASE_H