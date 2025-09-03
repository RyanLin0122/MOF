#pragma once
#include "UI/CControlBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"
#include "global.h"

class CControlChatBallon : public CControlBase
{
public:
    CControlChatBallon();                      // 004256D0
    virtual ~CControlChatBallon();             // 00425770

    // 與反編譯對齊的 API
    void Create(CControlBase* parent, uint8_t styleIndex, unsigned int wrapWidth, uint16_t fontHeight); // 004257E0
    void ResizeChatBallon(unsigned int wrapWidth, uint16_t fontHeight);                                 // 00425870
    void SetString(char* text, int x, int y,
        uint16_t width, uint16_t height, Direction dir);                                     // 00425930

    // 重設/換圖
    void SetImageChange(unsigned int resId, const uint16_t* frames11,
        unsigned int wrapWidth, uint16_t fontHeight);                                    // 00425E40
    void ResetImage(uint8_t styleIndex, unsigned int wrapWidth, uint16_t fontHeight);                    // 00425E90

    // 查詢
    int  IsSameImage(unsigned int resId, uint16_t frame) const;                                          // 00425EF0

    // 顯示/定位
    void Show() override;                                                                                // 00425F60
    void SetPos(int x, int y);                                                                  // 00425F80
    void SetArrowPos(int x, int y);                                                                      // 00425FC0
    void Draw() override;                                                                                // 004260B0

private:
    // 11 片元件：九宮格 9 片 + 箭頭(2 片)
    // 依位移對照：
    // [0] TL, [1] TM, [2] TR
    // [3] ML, [4] MM, [5] MC
    // [6] BL, [7] BM, [8] BR
    // [9] Arrow 主體, [10] Arrow 底座/補塊（僅某些方向需要）
    CControlImage m_piece[11];      // +124 開始，每個 0xC0 = 192 bytes（反編譯）
    CControlText  m_text;           // +2236

    // 內部狀態
    Direction     m_dir = DirDown;  // *((_DWORD*)this + 30) = 3（預設朝下）
    bool          m_multilineFixed = false;  // *((_DWORD*)this + 595)
    unsigned int  m_wrapWidthMax = 0;      // *((_DWORD*)this + 596)
    bool FirstCreated = true;
    // 方便計算的快取尺寸（文字框）
    uint16_t      m_textW = 0;      // *((_WORD*)this + 16) 對應寬/或整體寬
    uint16_t      m_textH = 0;      // *((_WORD*)this + 17) 對應高/或整體高

    // 幫助函式：依九宮格與箭頭方向排版
    void layoutPieces(uint16_t contentW, uint16_t contentH);
    void placeArrow(uint16_t totalW, uint16_t totalH);
};
