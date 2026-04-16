#include "UI/CControlCA.h"
#include "Character/CCAClone.h"
#include "global.h"

// =============================================================================
// CControlCA
// =============================================================================

// 對齊反編譯 0041F200 (0041F1D0)
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
    // *((float *)this + 62) = (float)CControlBase::GetAbsX(this);  -> m_CA.m_fPosX
    // *((float *)this + 63) = (float)CControlBase::GetAbsY(this);  -> m_CA.m_fPosY
    m_CA.m_fPosX = static_cast<float>(GetAbsX());
    m_CA.m_fPosY = static_cast<float>(GetAbsY());
    m_CA.Process();
}

// 對齊反編譯 0041F2F0
void CControlCA::Draw()
{
    // *((float *)this + 62) = (float)CControlBase::GetAbsX(this);  -> m_CA.m_fPosX
    // *((float *)this + 63) = (float)CControlBase::GetAbsY(this);  -> m_CA.m_fPosY
    m_CA.m_fPosX = static_cast<float>(GetAbsX());
    m_CA.m_fPosY = static_cast<float>(GetAbsY());
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
    // *((_BYTE *)this + 260) = a2 == 1;
    // this+260 = CControlBase(120) + CCA offset 140 = m_bMirrored
    m_CA.m_bMirrored = (a2 == 1) ? 1 : 0;
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

// =============================================================================
// CControlCAillust
// =============================================================================

// 對齊反編譯 0041F4E0
CControlCAillust::CControlCAillust()
    : CControlBase()
    , m_CAillust()
{
    m_CAillust.LoadCA("MoFData/Character/Illust_Character.ca");
}

// 對齊反編譯 0041F560
CControlCAillust::~CControlCAillust()
{
    // m_CAillust 和 CControlBase 自動解構
}

// 對齊反編譯 0041F5B0
void CControlCAillust::SetCAData(int a2, unsigned char a3, unsigned char a4, unsigned short a5)
{
    switch (a2)
    {
    case 1:
        m_CAillust.SetHairDefine(a5, a3, a4);
        break;
    case 2:
        m_CAillust.SetFaceDefine(a5, a3, a4);
        break;
    case 3:
    case 4:
    case 5:
        m_CAillust.SetItemID(a5, a3, a4, 1, 0, 0);
        break;
    default:
        return;
    }
}

// 對齊反編譯 0041F630
void CControlCAillust::SetChar(char a2, unsigned short a3, unsigned short a4, unsigned short a5, unsigned int a6, unsigned short* a7, unsigned short* a8)
{
    unsigned char sex = ExGetIllustCharSexCode(a2);
    unsigned char age = ExGetIllustCharAgeCode(a3);
    m_CAillust.InitItem(sex, age, a4, a5, a6);

    if (a8)
    {
        for (int i = 0; i < 11; ++i)
        {
            if (a8[i])
                m_CAillust.SetItemID(a8[i], sex, age, 1, 0, 0);
        }
    }
}

// 對齊反編譯 0041F6C0
void CControlCAillust::ConversionFromDotToIllust(CCA* a2, char a3, unsigned short a4, int a5, int a6, unsigned int a7)
{
    unsigned char sex = ExGetIllustCharSexCode(a3);
    unsigned char age = ExGetIllustCharAgeCode(a4);
    m_CAillust.InitItem(sex, age, a5, a6, a7);
    m_CAillust.SetItemtoDot(a2, sex, age, a5, a6);
}

// 對齊反編譯 0041F730 — 原始二進位中為空函式 (僅 retn)
void CControlCAillust::SetShadow()
{
}

// 對齊反編譯 0041F740
void CControlCAillust::SetHairColor(unsigned int a2)
{
    m_CAillust.SetHairColor(a2);
}

// 對齊反編譯 0041F760
void CControlCAillust::PrepareDrawing()
{
    // *((float *)this + 52) = (float)CControlBase::GetAbsX(this);  -> m_CAillust.m_fPosX
    // *((float *)this + 53) = (float)CControlBase::GetAbsY(this);  -> m_CAillust.m_fPosY
    m_CAillust.m_fPosX = static_cast<float>(GetAbsX());
    m_CAillust.m_fPosY = static_cast<float>(GetAbsY());
    m_CAillust.Process();
}

// 對齊反編譯 0041F7A0
void CControlCAillust::Draw()
{
    m_CAillust.Draw(0);
}

// 對齊反編譯 0041F7B0
CCAillust* CControlCAillust::GetCAIllust()
{
    return &m_CAillust;
}

// =============================================================================
// CControlCAClone
// =============================================================================

// 對齊反編譯 (inline from CUIEquip — no separate constructor in binary)
// CControlCAClone 只有 CControlBase 基底，由宿主 (CUIEquip) 設定 vftable

// 對齊反編譯 (body not in decompilation — CControlBase::~CControlBase only)
CControlCAClone::~CControlCAClone()
{
    // CControlBase 自動解構
}

// 對齊反編譯 0041F7E0
void CControlCAClone::PrepareDrawing()
{
    // flt_6DD758 = (float)CControlBase::GetAbsX(this);  -> g_clCAClone.m_fPosX
    // flt_6DD75C = (float)CControlBase::GetAbsY(this);  -> g_clCAClone.m_fPosY
    g_clCAClone.m_fPosX = static_cast<float>(GetAbsX());
    g_clCAClone.m_fPosY = static_cast<float>(GetAbsY());
    g_clCAClone.Process();
}

// 對齊反編譯 0041F820
void CControlCAClone::Draw()
{
    g_clCAClone.Draw();
}
