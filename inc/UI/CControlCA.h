#pragma once

#include "UI/CControlBase.h"
#include "Character/CCA.h"

// CControlCA：角色動畫控制（包裝 CCA 於 CControlBase 子樹）
// 反編譯對照：
//   this+0:   CControlBase (120 bytes)
//   this+120: CCA 實體 (292 bytes)
// 總大小 = 412 bytes
class CControlCA : public CControlBase
{
public:
    CControlCA();
    virtual ~CControlCA();

    virtual void PrepareDrawing() override;
    virtual void Draw() override;

    void SetCharMotion(unsigned char motion);
    void TakeOffAllCloth(unsigned char sex, unsigned short a3, unsigned short a4, unsigned int a5);
    void SetCloth(unsigned char sex, int a3, unsigned int a4, unsigned short* equipKind1, unsigned short* equipKind2);
    void SetChar(char charKind, int a3, unsigned int a4, unsigned int a5, unsigned short* equipKind1, unsigned short* equipKind2);
    void SetReverse(unsigned char a2);
    CCA* GetCA();
    void BegineEmoticon(int a2);
    void EndEmoticon(unsigned short a2, unsigned char a3);

    // +120: CCA 嵌入物件
    CCA m_CA;
};
