#pragma once
#include <cstdint>
#include "stToolTipData.h"

// 為避免與 Windows 的 windef.h 衝突，改用較冷門的型別名稱
struct stPoint { int x{ 0 }; int y{ 0 }; };
class CControlBase
{
public:
    CControlBase();
    virtual ~CControlBase();

    // ---- 建立流程 ----
    virtual void Create(CControlBase* pParent);
    virtual void Create(int x, int y, CControlBase* pParent);
    virtual void Create(int x, int y, uint16_t w, uint16_t h, CControlBase* pParent);

    // ---- 繪製流程 ----
    virtual void PrepareDrawing();
    virtual void Draw();

    // ---- 顯示/隱藏 ----
    virtual void Show();
    virtual void Hide();
    void ShowChildren();
    void HideChildren();

    // ---- 事件傳遞 ----
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7);

    // ---- 子物件鏈結 ----
    void AddChild(CControlBase* pChild);
    void DeleteChild();
    void DelLastChild();
    CControlBase* GetChild(int index);
    CControlBase* GetFirstChild() const { return m_pFirstChild; }
    CControlBase* GetLastChild()  const { return m_pLastChild; }
    CControlBase* GetParent()     const { return m_pParent; }

    // ---- 位置/尺寸 ----
    void SetPos(int x, int y);
    void SetPos(stPoint pt);
    void SetX(int x);
    void SetY(int y);
    int  GetX() const { return m_x; }
    int  GetY() const { return m_y; }

    void SetAbsPos(int absX, int absY);
    void SetAbsPos(stPoint pt);
    void SetAbsX(int absX);
    void SetAbsY(int absY);

    void GetAbsPos(int& outAbsX, int& outAbsY) const;
    int  GetAbsX() const;
    int  GetAbsY() const;

    void     SetSize(uint16_t w, uint16_t h) { m_usWidth = w; m_usHeight = h; }
    uint16_t GetWidth()  const { return m_usWidth; }
    uint16_t GetHeight() const { return m_usHeight; }
    void SetWidth(uint16_t w) { m_usWidth = w; }
    void SetHeight(uint16_t h) { m_usHeight = h; }

    void MoveWindow(int dx, int dy);
    void SetChildPosMove(int dx, int dy);

    // ---- 置中 ----
    void SetCenterPos(int centerAbsX, int centerAbsY);
    void SetCenterPos();
    void SetCenterXToParent();
    void SetCenterYToParent();
    void SetWindowCenterAbsPos();

    // ---- 命中測試（右/下為開區間）----
    bool PtInCtrl(stPoint pt) const;

    // ---- 旗標/屬性 ----
    void SetVisible(bool v) { m_bIsVisible = v; }
    bool IsVisible() const { return m_bIsVisible; }

    void SetPassKeyInputToParent(bool b) { m_bPassKeyInputToParent = b; }
    bool GetPassKeyInputToParent() const { return m_bPassKeyInputToParent; }

    void SetCenterOrigin(bool b) { m_bCenterOrigin = b; }
    bool GetCenterOrigin() const { return m_bCenterOrigin; }

    void SetScale(float sx, float sy) { m_fScaleX = sx; m_fScaleY = sy; }
    float GetScaleX() const { return m_fScaleX; }
    float GetScaleY() const { return m_fScaleY; }

    // 由滑鼠座標找被點擊的子控件（找不到則回傳自己）
    CControlBase* FindClickedChild(stPoint pt);

    // 尋找子樹中的 ScrollBar 控件（classId == 100），找不到回傳 nullptr
    CControlBase* FindScrollBarCtrlChild(int a2, int a3);

    // Active 狀態
    void Active();
    void NoneActive();
    int  IsActive() const;

    // ---- ToolTip ----
    void InitLogIn();
    void ClearData();
    void EnableToolTip(bool enable) { m_bToolTipEnabled = enable; }
    bool IsToolTipEnabled() const { return m_bToolTipEnabled; }
    // ToolTip：Kind/Type 版本（對應 SetKindType）
    void SetToolTipData(int16_t a2, int a3, int a4, int a5, char a6, int16_t a7, int a8);

    // ToolTip：字串版本
    void SetToolTipDataString(char* a2, int a3);

    // ToolTip：描述版本（kind=17, descId=a2）
    void SetToolTipDataDesc(uint16_t a2);

protected:
    virtual void OnPrepareDrawing() {}
    virtual void OnDraw() {}

protected:
    // 鏈結/階層
    CControlBase* m_pParent{ nullptr };
    CControlBase* m_pPrev{ nullptr };
    CControlBase* m_pNext{ nullptr };
    CControlBase* m_pFirstChild{ nullptr };
    CControlBase* m_pLastChild{ nullptr };

    // 幾何
    int      m_x{ 0 };
    int      m_y{ 0 };
    uint16_t m_usWidth{ 0 };
    uint16_t m_usHeight{ 0 };
    float    m_fScaleX{ 1.0f };
    float    m_fScaleY{ 1.0f };

    // 旗標
    bool m_bIsVisible{ true };
    bool m_bPassKeyInputToParent{ false };
    bool m_bCenterOrigin{ false };
    bool m_bToolTipEnabled{ false };

    // ToolTip
    stToolTipData m_ToolTip;

    bool m_bActive{ false };   // 對應 *((DWORD*)this+11)
    int  m_nClassId{ 0 };      // 用於 FindScrollBarCtrlChild 比對（100=scrollbar）
};