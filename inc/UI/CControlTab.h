// ================================
// CControlTab.h
// ================================
#pragma once
#include <cstdint>
#include "UI/CControlButtonBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

/**
 * 邏輯還原自 IDA Pro 反編譯：CControlTab
 * - 提供「焦點/非焦點/滑入/未啟用」四種圖組態（(group,id,block) 三元組）
 * - 提供四種文字色（焦點/非焦點/滑入/未啟用）
 * - 事件碼對齊：0=Click、3=MouseDown、4=恢復常態、7=Hover
 */
class CControlTab : public CControlButtonBase
{
public:
    CControlTab();
    virtual ~CControlTab();

    // 初始化（對齊反編譯：把焦點旗標預設為 1）
    void Init();

    // 文字 Tab 樣式（對齊：SetTabTextType）
    void SetTabTextType();
    uint8_t GetTabType() const { return m_bTextTabType ? 1u : 0u; }

    // 設定圖像：使用同一 giid，分別指定四種 block（a5/a6=0xFFFF 表示無此狀態）
    void SetImage(unsigned int giid,
        uint16_t blockFocused,
        uint16_t blockUnfocused,
        uint16_t blockHover,
        uint16_t blockNoneActive);

    // 文字色（焦點/非焦點/滑入/未啟用）
    void SetTextColor(uint32_t colFocused,
        uint32_t colUnfocused,
        uint32_t colHover,
        uint32_t colNoneActive);

    // 透明度（對齊：SetTabAlpha，且會關閉 ShadeMode）
    void SetTabAlpha(int a);

    // 狀態切換
    void SetFocus(bool focus);
    void Active();
    void NoneActive();

    // 事件流程（維持與基底相同簽名與回傳語義）
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;

    // 字串/字串ID 載入後，若是 TextTab 會以字寬高回填自身尺寸
    void SetText(const char* text);  // 覆寫同名（以使用 m_Text）
    void SetText(int stringId);      // 覆寫整數版本（資源字串）

    // Tab 索引（供管理器設定/讀取）
    void SetIndex(int idx) { m_index = idx; }
    int  GetIndex() const { return m_index; }

private:
    struct StateImg { uint32_t group{ 5 }; uint32_t id{ 0 }; uint16_t block{ 0xFFFF }; };

    void ApplyStateImage(const StateImg& st);
    void ApplyTextColor(uint32_t c);

private:
    // 四狀態圖
    StateImg m_imgFocused{};     // 反編譯索引 167..169
    StateImg m_imgUnfocused{};   // 170..172
    StateImg m_imgHover{};       // 173..175
    StateImg m_imgNoneActive{};  // 176..178

    // 文字色
    uint32_t m_colFocused{ 0xFFFFFFFF };
    uint32_t m_colUnfocused{ 0xFFFFFFFF };
    uint32_t m_colHover{ 0xFFFFFFFF };
    uint32_t m_colNoneActive{ 0xFFFFFFFF };
    uint32_t m_curTextColor{ 0xFFFFFFFF }; // 對齊 this[93]

    // 樣式與狀態
    bool m_bTextTabType{ false }; // 對齊 *((BYTE*)this+660)
    bool m_bFocused{ true };      // 對齊 *((DWORD*)this+166)

    int  m_index{ 0 };            // 供管理器透過事件帶回來
};