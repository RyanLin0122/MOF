#include "UI/CControlBoxSkill.h"

CControlBoxSkill::CControlBoxSkill()
    : CControlBoxBase()
    , m_icon()
    , m_title()
    , m_subIcon()
    , m_numberImg()
    , m_overlay()
    , m_frame()
{
    // vftable 由編譯器處理
    CreateChildren();
    CControlBoxBase::Init();
}

CControlBoxSkill::~CControlBoxSkill()
{
    // RAII：子物件會自動解構；順序與反編譯一致但不必手動寫
}

void CControlBoxSkill::CreateChildren()
{
    // 背景（父類內部 +120）
    CControlBoxBase::CreateChildren();
    if (auto* bg = GetBackground())
        bg->SetPos(1, 2);

    // +312：技能圖示  (資源 0x2200000B, frame 0x17)
    m_icon.Create(this);
    m_icon.SetPos(10, 10);
    m_icon.SetImage(0x2200000Bu, 0x17u);

    // +504：標題文字
    m_title.Create(this);
    m_title.SetPos(43, 6);
    m_title.SetTextColor(-7590900); // *((_DWORD*)this + 163) = -7590900

    // +936：小圖（資源 570425345, frame 71），位置 (43,23)
    m_subIcon.Create(this);
    m_subIcon.SetImage(570425345, 71);
    m_subIcon.SetPos(43, 23);

    // +1128：數字圖片 (x=60, y=23, kind=0, parent=this)
    m_numberImg.Create(60, 23, /*kind*/0, this);

    // +5100：半透明覆蓋，位置(1,2)、大小 32×32、顏色(0.588..., 0, 0, 0.3137)
    m_overlay.Create(this);
    m_overlay.SetPos(1, 2);
    m_overlay.SetSize(32, 32);                 // 若你的 API 叫 SetWH，就改成 SetWH(32,32)
    m_overlay.SetColor(0.58823532f, 0.0f, 0.0f, 0.3137255f);

    // +5308：前景框 (資源 536870931, frame 24)，位置(2,3)
    m_frame.Create(this);
    m_frame.SetImage(536870931, 24);
    m_frame.SetPos(2, 3);
}
