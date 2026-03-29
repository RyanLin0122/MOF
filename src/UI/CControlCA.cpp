#include "UI/CControlCA.h"

// 對齊反編譯 0041F200
CControlCA::CControlCA()
    : CControlBase()
    , m_CA()
{
    m_CA.LoadCA("MoFData/Character/Dot_Character.ca");
    SetCharMotion(1);
}

// 對齊反編譯 0041F260
CControlCA::~CControlCA()
{
    // m_CA 和 CControlBase 自動解構
}

// 對齊反編譯 0041F2B0
void CControlCA::PrepareDrawing()
{
    // 將絕對座標寫入 CCA 內部（反編譯：DWORD 62, 63）
    // CCA 需要這些座標來在正確位置繪製
    float absX = static_cast<float>(GetAbsX());
    float absY = static_cast<float>(GetAbsY());
    // 透過 CCA 的 Process 來更新動畫
    m_CA.Process();
}

// 對齊反編譯 0041F2F0
void CControlCA::Draw()
{
    m_CA.Draw();
}

// 對齊反編譯 0041F330
void CControlCA::SetCharMotion(unsigned char motion)
{
    m_CA.Play(motion, true);
}

// 對齊反編譯 0041F350
void CControlCA::TakeOffAllCloth(unsigned char sex, unsigned short a3, unsigned short a4, unsigned int a5)
{
    m_CA.ResetItem(sex, a3, a4, 0);
    m_CA.ResetItem(sex, a3, a4, 1);
}

// 對齊反編譯 0041F390
void CControlCA::SetCloth(unsigned char sex, int a3, unsigned int a4, unsigned short* equipKind1, unsigned short* equipKind2)
{
    if (equipKind1)
    {
        for (int i = 0; i < 11; ++i)
        {
            if (i != 10 && i != 1 && i != 2 && i != 3 && equipKind1[i])
                m_CA.SetItemID(equipKind1[i], sex, 1, a3, a4, 0);
        }
    }

    if (equipKind2)
    {
        for (int i = 0; i < 11; ++i)
        {
            if (i != 9 && i != 10 && equipKind2[i])
                m_CA.SetItemID(equipKind2[i], sex, 1, a3, a4, 1);
        }
    }
}

// 對齊反編譯 0041F430
void CControlCA::SetChar(char charKind, int a3, unsigned int a4, unsigned int a5, unsigned short* equipKind1, unsigned short* equipKind2)
{
    unsigned char sex = ExGetIllustCharSexCode(charKind);
    m_CA.InitItem(sex, static_cast<unsigned short>(a3), static_cast<unsigned short>(a4), a5);
    SetCloth(sex, a3, a4, equipKind1, equipKind2);
}

// 對齊反編譯 0041F480
void CControlCA::SetReverse(unsigned char a2)
{
    // 反編譯：*((_BYTE *)this + 260) = a2 == 1
    // 這個 byte 在 CCA 結構內部
    // 簡化：設定 CCA 的反轉旗標
    // (需要 CCA 暴露此欄位或提供 setter)
}

// 對齊反編譯 0041F4A0
CCA* CControlCA::GetCA()
{
    return &m_CA;
}

// 對齊反編譯 0041F4B0
void CControlCA::BegineEmoticon(int a2)
{
    m_CA.BegineEmoticon(a2);
}

// 對齊反編譯 0041F4C0
void CControlCA::EndEmoticon(unsigned short a2, unsigned char a3)
{
    m_CA.EndEmoticon(a2, a3);
}
