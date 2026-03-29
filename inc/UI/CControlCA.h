#pragma once

#include "UI/CControlBase.h"

// 前置宣告（CCA 完整定義在 Character/ 或其他模組中）
struct CEffectBase;
struct LAYERINFO;

// CCA：角色動畫核心（size = 292 bytes in binary）
// 只宣告 CControlCA 所需的介面，實際定義留給 CCA 自己的模組
class CCA
{
public:
    CCA();
    ~CCA();

    void LoadCA(const char* path, CEffectBase** a2 = nullptr, CEffectBase** a3 = nullptr);
    void Play(int motion, bool loop);
    void Process();
    void Draw();
    void InitItem(unsigned char sex, unsigned short a3, unsigned short a4, unsigned int a5);
    void ResetItem(unsigned char sex, unsigned short a3, unsigned short a4, unsigned char a5);
    void SetItemID(unsigned short itemId, unsigned char sex, int a4, int a5, int a6, unsigned char a7);
    void BegineEmoticon(int a2);
    void EndEmoticon(unsigned short a2, unsigned char a3);

private:
    // 佔位：二進制中 CCA 大小為 292 bytes
    char m_data[292];
};

// 外部函式（定義在 Character 模組）
unsigned char ExGetIllustCharSexCode(char charKind);

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
