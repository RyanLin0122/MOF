#pragma once
#include <cstdint>
#include "UI/CControlButtonBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"

// 具四態(常態/滑入/按下/停用)外觀的按鈕控制
// 反編譯對照：
//   this[164..166]: Normal 圖 (group, id, block at WORD+332)
//   this[167..169]: Hover 圖  (group, id, block at WORD+338)
//   this[170..172]: Pressed 圖 (group, id, block at WORD+344)
//   this[173..175]: Disabled 圖 (group, id, block at WORD+350)
//   this[177]: colNormal (-1 = 0xFFFFFFFF)
//   this[178]: colHover (-1376512 = 0xFFEB5400)
//   this[179]: colPressed (-1376512 = 0xFFEB5400)
//   this[180]: colDisabled (-3566989 = 0xFFC98E33)
//   this[93]:  curTextColor
//   this[92]:  unknown field (set to 1 in Create)
//   BYTE at 704: unknown flag (set to 1 in Create)
//   this[49]:  mouseOver/hover flag (set in ControlKeyInputProcess)
class CControlButton : public CControlButtonBase
{
public:
    CControlButton();
    virtual ~CControlButton();

    // 建立：僅掛到父控制
    void Create(CControlBase* pParent);
    // 建立：指定座標 + 父控制
    void Create(int x, int y, CControlBase* pParent);

    // 顯示（反編譯：套用 normal 圖與 normal 文字色）
    void Show();

    // 狀態切換
    void NoneActive();
    void Active();

    // ---- 設定圖片（對齊反編譯多載） ----
    // 同一 imageId，不同 block 分別對應四態
    void SetImage(unsigned int imageIdAll,
        uint16_t blockNormal,
        uint16_t blockHover,
        uint16_t blockPressed,
        uint16_t blockDisabled);

    // 僅設定「常態」的 imageId + block
    void SetImage(unsigned int imageIdNormal, uint16_t blockNormal);

    // 為常態/滑入/按下各自指定 imageId + block
    void SetImage(unsigned int imageIdNormal, uint16_t blockNormal,
        unsigned int imageIdHover, uint16_t blockHover,
        unsigned int imageIdPressed, uint16_t blockPressed);

    // 再加上停用（disabled）
    void SetImage(unsigned int imageIdNormal, uint16_t blockNormal,
        unsigned int imageIdHover, uint16_t blockHover,
        unsigned int imageIdPressed, uint16_t blockPressed,
        unsigned int imageIdDisabled, uint16_t blockDisabled);

    // 設定四態的文字顏色
    void SetButtonTextColor(uint32_t colNormal,
        uint32_t colHover,
        uint32_t colPressed,
        uint32_t colDisabled);

    // 事件處理
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;

    // 對齊反編譯：取得 pressed 態圖像資訊（供 CControlScrollBar::ProcessMoveThumb 使用）
    void GetPressedImageState(unsigned int& group, unsigned int& id, uint16_t& block) const {
        group = m_imgPressed.group;
        id = m_imgPressed.id;
        block = m_imgPressed.block;
    }

private:
    struct StateImg {
        uint32_t group{ 5 };     // 反編譯預設 5
        uint32_t id{ 0 };
        uint16_t block{ 0xFFFF }; // 0xFFFF 表示無效/不切換
    };

    // 四態資料 (this[164..175])
    StateImg m_imgNormal{};    // this[164,165,WORD+332]
    StateImg m_imgHover{};     // this[167,168,WORD+338]
    StateImg m_imgPressed{};   // this[170,171,WORD+344]
    StateImg m_imgDisabled{};  // this[173,174,WORD+350]

    // 四態文字色 (this[177..180])
    uint32_t m_colNormal{ 0xFFFFFFFF };   // this[177] = -1
    uint32_t m_colHover{ 0xFFEB5400 };    // this[178] = -1376512
    uint32_t m_colPressed{ 0xFFEB5400 };  // this[179] = -1376512
    uint32_t m_colDisabled{ 0xFFC98E33 }; // this[180] = -3566989

    // this[93] 對應 m_Text.m_TextColor（直接透過 m_Text.SetTextColor 操作，不再額外存副本）
};
