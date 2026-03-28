#pragma once
#include <cstdint>
#include "UI/CControlButtonBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

/**
 * 對齊反編譯：CControlTab
 * 繼承自 CControlButtonBase
 * 包含四組圖態 (focused/unfocused/hover/noneActive)，四組文字色
 * 事件碼：0=Click, 3=MouseDown, 4=恢復常態, 7=Hover
 */
class CControlTab : public CControlButtonBase
{
public:
    CControlTab();
    virtual ~CControlTab();

    void Init();
    void CreateChildren();

    // 圖像設定
    void SetImage(unsigned int giid, uint16_t blockFocused, uint16_t blockUnfocused,
                  uint16_t blockHover, uint16_t blockNoneActive);

    // 文字色
    void SetTextColor(uint32_t colFocused, uint32_t colUnfocused,
                      uint32_t colHover, uint32_t colNoneActive);

    // 文字 Tab 樣式
    void SetTabTextType();
    uint8_t GetTabType();

    // 透明度
    void SetTabAlpha(int a);

    // 事件
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;

    // 狀態
    void SetFocus(int focus);
    void Active();
    void NoneActive();

    // 文字
    void SetText(char* text);
    void SetText(int stringId);

private:
    // 對齊反編譯佈局順序（byte 660 起）：
    // m_bTextTabType 必須在 m_bFocused 之前，兩者都在四組圖之前
    uint8_t  m_bTextTabType{ 0 };           // BYTE at this+660
    // 3 bytes padding (auto)
    int      m_bFocused{ 1 };               // this[166] = byte 664

    // 四狀態圖：每組 {group, id, block}
    // 對齊反編譯 this[167..178]
    uint32_t m_imgFocusedGroup{ 5 };       // this[167]
    uint32_t m_imgFocusedId{ 0 };          // this[168]
    uint16_t m_imgFocusedBlock{ 0xFFFF };  // WORD[338] = byte 676

    uint32_t m_imgUnfocusedGroup{ 5 };     // this[170]
    uint32_t m_imgUnfocusedId{ 0 };        // this[171]
    uint16_t m_imgUnfocusedBlock{ 0xFFFF }; // WORD[344] = byte 688

    uint32_t m_imgHoverGroup{ 5 };         // this[173]
    uint32_t m_imgHoverId{ 0 };            // this[174]
    uint16_t m_imgHoverBlock{ 0xFFFF };    // WORD[350] = byte 700

    uint32_t m_imgNoneActiveGroup{ 5 };    // this[176]
    uint32_t m_imgNoneActiveId{ 0 };       // this[177]
    uint16_t m_imgNoneActiveBlock{ 0xFFFF }; // WORD[356] = byte 712

    // 四狀態文字色：this[179..182]
    // 對齊反編譯：ground truth 建構子中未初始化這四個欄位
    uint32_t m_colFocused;      // this[179]
    uint32_t m_colUnfocused;    // this[180]
    uint32_t m_colHover;        // this[181]
    uint32_t m_colNoneActive;   // this[182]
};
