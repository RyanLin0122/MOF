#ifndef CCONTROLBUTTONBASE_H
#define CCONTROLBUTTONBASE_H

#include "CControlImage.h"
#include "CControlText.h"

// 基底按鈕：以圖片為背景 + 文字子控制，提供按下位移效果
class CControlButtonBase : public CControlImage
{
public:
    CControlButtonBase();
    virtual ~CControlButtonBase();

    // ---- 建立 ----
    // 僅掛載到父控制
    virtual void Create(CControlBase* pParent) override;
    // 設定絕對座標 + 掛父
    virtual void Create(int x, int y, CControlBase* pParent) override;
    // 設定尺寸 + 絕對座標 + 掛父
    virtual void Create(int x, int y, uint16_t w, uint16_t h, CControlBase* pParent) override;

    // ---- 文字 ----
    // 若你的 CControlText 僅支援字串，可忽略整數版本
    void SetText(const char* utf8Text);
    void SetText(int stringId);

    // 取得內建文字控制（可自訂對齊、字型、陰影等）
    CControlText* GetTextCtrl() { return &m_Text; }

    // ---- 按下位移效果 ----
    void EnablePressShift(bool enable) { m_bEnablePressShift = enable; }
    void SetPressShift(int dx, int dy) { m_pressShiftX = dx; m_pressShiftY = dy; }
    bool IsPressed() const { return m_bPressed; }

    // 由事件流程呼叫（或手動呼叫）：
    void ButtonPosDown(); // 進入按下狀態：子物件位移 + 播放音效（可覆寫）
    void ButtonPosUp();   // 離開按下狀態：子物件位移復原

    // 可覆寫：點擊音效（預設不做事，視專案接上音效管理）
    virtual void PlaySoundClick();

protected:
    // 也可視需求覆寫 OnPrepareDrawing/OnDraw，但基底 CControlImage 已處理繪製
    // virtual void OnPrepareDrawing() override {}
    // virtual void OnDraw() override {}

    // 建立子控制（文字）
    void CreateChildren();

protected:
    CControlText m_Text;          // 內建一個文字子控制
    int  m_pressShiftX{ 1 };        // 按下位移 X（預設 1px）
    int  m_pressShiftY{ 1 };        // 按下位移 Y（預設 1px）
    bool m_bEnablePressShift{ true };
    bool m_bPressed{ false };
};

#endif // CCONTROLBUTTONBASE_H