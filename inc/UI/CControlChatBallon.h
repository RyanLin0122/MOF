#pragma once

#include <cstdint>
#include "UI/CControlBase.h"
#include "UI/CControlImage.h"
#include "UI/CControlText.h"
#include "global.h"

// 九宮格氣球圖 block 表：每種樣式 11 個 uint16_t（對齊反編譯 7106256 = 0x6C6AD0）
extern const uint16_t g_ChatBallonStyleTable[];

class CControlChatBallon : public CControlBase
{
public:
    CControlChatBallon();                      // 004256D0
    virtual ~CControlChatBallon();             // 00425770

    void Create(CControlBase* parent, uint8_t styleIndex,
                unsigned int wrapWidth, uint16_t fontHeight);    // 004257E0
    void ResizeChatBallon(unsigned int wrapWidth, uint16_t fontHeight); // 00425870
    void SetString(char* text, int x, int y,
                   uint16_t width, uint16_t height, Direction dir); // 00425930

    void SetImageChange(unsigned int resId, const uint16_t* frames11,
                        unsigned int wrapWidth, uint16_t fontHeight); // 00425E40
    void ResetImage(uint8_t styleIndex, unsigned int wrapWidth,
                    uint16_t fontHeight);                             // 00425E90

    int  IsSameImage(unsigned int resId, uint16_t frame);            // 00425EF0

    void Show() override;                                            // 00425F60
    void SetPos(int x, int y);                                       // 00425F80
    void SetArrowPos(int x, int y);                                  // 00425FC0
    void Draw() override;                                            // 004260B0

    // 反編譯佈局：
    //   +120:  int m_nDirection (DWORD 30，預設 3=DirDown)
    //   +124:  CControlImage m_piece[11] (11 × 192 bytes)
    //   +2236: CControlText  m_text
    int           m_nDirection{ 3 };   // +120 (DirDown)
    CControlImage m_piece[11];         // +124
    CControlText  m_text;              // +2236
    unsigned int  m_wrapWidth{ 0 };    // 反編譯 *((_DWORD *)this + 596)，ResizeChatBallon 存入
};
