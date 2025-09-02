#pragma once
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#else
typedef int BOOL;
#endif

#include "UI/CControlBase.h"
#include "UI/CControlImage.h"

/**
 * @brief 盒型基底控制：內建一張背景圖，並提供顯示/隱藏時的共同行為。
 *
 * 反編譯對照：
 * - ctor/dtor：基底 + 內建 CControlImage
 * - Init()：(*this+13)=1, (*this+16)=0, ToolTip.Init()
 * - CreateChildren()：背景圖 Create(this)，SetShadeMode(0)
 * - ShowChildren()：基底後 (*this+16)=1
 * - HideChildren()：多處狀態復位後，基底 HideChildren()，再呼叫 vtbl+84
 * - IsInItem()：回傳 (*this+42)!=0
 * - SetAlpha(a)：委派給背景圖
 */
class CControlBoxBase : public CControlBase
{
public:
    CControlBoxBase();
    virtual ~CControlBoxBase();

    // 初始化（對應反編譯 Init）
    void Init();

    // 子控制建立/顯示/隱藏
    virtual void CreateChildren();
    virtual void ShowChildren();
    virtual void HideChildren();

    // 查詢是否位於「項目內」（對應 (*this+42)!=0）
    BOOL IsInItem();

    // 設定整體透明度（委派背景圖）
    void SetAlpha(int a);

    // 提供子類安全取得背景圖層控制項
    CControlImage* GetBackground() { return &m_Background; }  // m_bg 為 +120 的那顆 CControlImage
protected:
    // 供 HideChildren() 結尾回呼（對應 vtbl+84）
    virtual void OnHidden() {}

protected:
    // this+120：背景圖
    CControlImage m_Background;

    // this+16：反編譯中反覆被設/清的狀態欄位（派生類也會用到）
    int m_State16{ 0 };

    // this+42：IsInItem() 所依據的旗標（派生類可按需求維護）
    int m_InItemFlag{ 0 };

    // 其餘在 HideChildren() 會被重置的欄位（對應 +76,+77,+69,+124 …）
    // 不強制對外暴露語意，僅以欄位名對應維護
    uint16_t m_Word76{ 0 };
    uint16_t m_Word77{ 0 };
    uint32_t m_Dword69{ 0 };
    uint16_t m_Word124{ 0xFFFF };
};
