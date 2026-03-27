#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlTab.h"

/**
 * 對齊反編譯：CControlTabMgr
 * - 最多 20 個 CControlTab + 20 個 CControlText（分隔符 "|"）
 * - 提供新增 Tab、設定焦點索引、佈局文字 Tab、設定顏色與透明度等
 */
class CControlTabMgr : public CControlBase
{
public:
    CControlTabMgr();
    virtual ~CControlTabMgr();

    void Create(CControlBase* pParent, char textTabStyle, int enableShadeMode);
    virtual void InitLogIn();
    void CreateChildren();
    void Init();

    void SetTextTabPosition(int wrapWidth);

    // AddTab 三種重載
    void AddTab(int x, int y, unsigned int giid,
                uint16_t blockFocused, uint16_t blockUnfocused,
                uint16_t blockHover, uint16_t blockNoneActive, int textId);
    void AddTab(int x, int y, unsigned int giid,
                uint16_t blockFocused, uint16_t blockUnfocused, int textId);
    void AddTab(int x, int y, char type, int textId);

    void SetAllAlpha(int a);
    void SetTabCount(uint8_t count);
    void SetTabTextColor(uint32_t colFocused, uint32_t colUnfocused,
                         uint32_t colHover, uint32_t colNoneActive);

    void ChildKeyInputProcess(int msg, int a3, int x, int y, int a6, int a7);

    virtual void Show() override;
    virtual void Draw() override;

    void    SetFocusTabIndex(uint8_t idx);
    uint8_t GetFocusTabIndex();

    void ActiveTab(uint8_t idx);
    int  IsActiveTab(uint8_t idx);
    void NoneActiveTab(uint8_t idx);

    CControlTab* GetTab(uint8_t idx) { return (idx < 20) ? &m_Tabs[idx] : nullptr; }

private:
    CControlTab  m_Tabs[20];       // offset +128, each 0x2DC = 732 bytes
    CControlText m_Seps[20];       // offset +14768, each 0x1B0 = 432 bytes

    uint8_t m_focusIndex{ 0 };     // byte at this+120
    uint8_t m_tabCount{ 0 };       // byte at this+121
};
