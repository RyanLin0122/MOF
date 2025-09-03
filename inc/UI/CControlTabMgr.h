#pragma once
#include <cstdint>
#include <algorithm>
#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlTab.h"

/**
 * 邏輯還原自 IDA Pro 反編譯：CControlTabMgr
 * - 最多 20 個 Tab 與 20 個分隔符（文字「|」）
 * - 供新增 Tab、設定焦點索引、佈局文字 Tab、設定顏色與透明度等
 */
class CControlTabMgr : public CControlBase
{
public:
    CControlTabMgr();
    virtual ~CControlTabMgr();

    // 建立：若 textTabStyle==true，會幫全部 Tab 切為文字 Tab 並可選擇是否關閉 ShadeMode
    void Create(CControlBase* pParent, bool textTabStyle = false, bool enableShadeMode = true);

    // 初始化登入（對齊反編譯：預設焦點索引=0，若為文字 Tab 會觸發一次 hover 流程）
    virtual void InitLogIn();

    // 產生子物件（20 個 Tab + 20 個分隔符）
    void CreateChildren();

    // 內部狀態清設
    void Init();

    // 僅針對文字 Tab：以版寬（a2）自動換行配置「文字 + 分隔符」
    void SetTextTabPosition(int wrapWidth /*可為 0 表示不換行*/);

    // 新增 Tab（完整四狀態）
    void AddTab(int x, int y,
        unsigned int giid,
        uint16_t blockFocused,
        uint16_t blockUnfocused,
        uint16_t blockHover,
        uint16_t blockNoneActive,
        int textId /*0=不設定*/);

    // 新增 Tab（僅 focused/unfocused）
    void AddTab(int x, int y,
        unsigned int giid,
        uint16_t blockFocused,
        uint16_t blockUnfocused,
        int textId /*0=不設定*/);

    // 新增 Tab（用類型快速指定圖組）: a4=0/1/2 對應不同預設資源
    void AddTab(int x, int y, char type, int textId /*0=不設定*/);

    // 大量設定
    void SetAllAlpha(int a);
    void SetTabCount(uint8_t count);
    void SetTabTextColor(uint32_t colFocused,
        uint32_t colUnfocused,
        uint32_t colHover,
        uint32_t colNoneActive);

    // 事件來自子 Tab（對齊反編譯命名）
    void ChildKeyInputProcess(int msg, int a3, int x, int y, int a6, int a7);

    // 顯示/繪製
    virtual void Show() override;
    virtual void Draw() override;

    // 焦點索引
    void        SetFocusTabIndex(uint8_t idx);
    uint8_t     GetFocusTabIndex() const { return m_focusIndex; }

    // 控制某索引 Tab 的啟用/停用（對齊反編譯 Active/NoneActive）
    void ActiveTab(uint8_t idx);
    int  IsActiveTab(uint8_t idx);
    void NoneActiveTab(uint8_t idx);

    // 取得底下的 Tab 指標
    CControlTab* GetTab(uint8_t idx) { return (idx < 20) ? &m_Tabs[idx] : nullptr; }

private:
    void UpdateBoundingSizeByTab(uint8_t idx);
    void UpdateSeparatorsVisibility();

private:
    CControlTab  m_Tabs[20];
    CControlText m_Seps[20];

    uint8_t m_tabCount{ 0 };
    uint8_t m_focusIndex{ 0 };
};