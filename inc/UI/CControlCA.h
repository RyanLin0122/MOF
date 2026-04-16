#pragma once

#include "UI/CControlBase.h"
#include "Character/CCA.h"
#include "Character/CCAillust.h"

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

// CControlCAillust：插圖角色動畫控制（包裝 CCAillust 於 CControlBase 子樹）
// 反編譯對照：
//   this+0:   CControlBase (120 bytes)
//   this+120: CCAillust 實體 (112 bytes)
// 總大小 = 232 bytes
class CControlCAillust : public CControlBase
{
public:
    CControlCAillust();
    virtual ~CControlCAillust();

    virtual void PrepareDrawing() override;
    virtual void Draw() override;

    void SetCAData(int a2, unsigned char a3, unsigned char a4, unsigned short a5);
    void SetChar(char a2, unsigned short a3, unsigned short a4, unsigned short a5, unsigned int a6, unsigned short* a7, unsigned short* a8);
    void ConversionFromDotToIllust(CCA* a2, char a3, unsigned short a4, int a5, int a6, unsigned int a7);
    void SetShadow();
    void SetHairColor(unsigned int a2);
    CCAillust* GetCAIllust();

    // +120: CCAillust 嵌入物件
    CCAillust m_CAillust;
};

// CControlCAClone：克隆角色動畫控制（使用全域 g_clCAClone）
// 反編譯對照：
//   this+0:   CControlBase (120 bytes)
//   不含嵌入 CCAClone，使用全域 g_clCAClone
class CControlCAClone : public CControlBase
{
public:
    virtual ~CControlCAClone();

    virtual void PrepareDrawing() override;
    virtual void Draw() override;
};
