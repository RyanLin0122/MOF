#pragma once
#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlEditBox.h"
#include "UI/CControlButton.h"

/**
 * 角色創建用的單行控制元件
 *
 * 依 arrayIndex（由 Create 的 a5 參數決定）分三種模式：
 *   0      : 名稱輸入（可編輯 EditBox，隱藏按鈕）
 *   1..6   : 外觀選擇（唯讀 EditBox + 上下按鈕）
 *   7+     : 能力值顯示（唯讀 EditBox + 上下按鈕 + ToolTip）
 */
class CControlBoxCreateChar : public CControlBase
{
public:
    CControlBoxCreateChar();
    virtual ~CControlBoxCreateChar();

    // 建立控制項。a5=type, a6=labelTextId
    void Create(int x, int y, CControlBase* pParent, uint8_t type, int labelTextId);

    // 覆寫 Show：若 type==0，隱藏按鈕並設 EditBox 焦點
    virtual void Show() override;

    // 設定外觀資料（type 1~6 用）
    // a2: 0=重設, 1=循環上, 2=循環下
    // a3: 性別旗標（非零=男）
    void SetAppearData(uint8_t direction, uint8_t sex);

    // 設定能力值（type 7+ 用）
    void SetAbilityData(int value);

    // 取得 EditBox 中的文字
    char* GetEditBoxText();

    // 取得選定的物品種類（SetAppearData 在 case 4~6 時設定）
    uint16_t GetItemKind() const { return m_itemKind; }

    // 取得目前選擇索引值
    int GetValue() const { return m_value; }

    // 子控制項存取
    CControlText&    Label()   { return m_label; }
    CControlEditBox& EditBox() { return m_editBox; }
    CControlButton&  BtnUp()   { return m_btnUp; }
    CControlButton&  BtnDown() { return m_btnDown; }

    // 靜態外觀裝備代碼表（type 4~6 各最多 MAX_APPEAR_SLOT 個選項）
    static constexpr int MAX_APPEAR_SLOT = 4;
    static const char* s_maleAppearKindCode[3][MAX_APPEAR_SLOT];
    static const char* s_femaleAppearKindCode[3][MAX_APPEAR_SLOT];

    // 每種 type 的最大選項數量（index 0~6）
    static const uint16_t s_maxAppearCount[7];

private:
    void CreateChildren();

    int       m_value{ 0 };        // 目前選擇索引（反編譯 *(DWORD*)(this+30) = byte 120）
    uint16_t  m_itemKind{ 0 };     // 外觀裝備 kind（反編譯 *(WORD*)(this+62) = byte 124）

    CControlText    m_label;       // +128 標籤文字
    CControlEditBox m_editBox;     // +560 編輯框
    CControlButton  m_btnUp;       // +4912 向上按鈕
    CControlButton  m_btnDown;     // +5636 向下按鈕
    CControlBase    m_extra;       // +6360 GT 中構造/析構但未見方法引用的額外控制項
};
