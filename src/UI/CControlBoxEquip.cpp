#include "UI/CControlBoxEquip.h"

//--------------------------------------------------
// ctor / dtor
//--------------------------------------------------
CControlBoxEquip::CControlBoxEquip()
    : CControlBoxBase()
    , m_Highlight()
    , m_SealImg6()
    , m_SealImg7()
{
    // vftable 由編譯器處理
    CreateChildren();  // 對齊反編譯：建完立刻建子件
    CControlBoxBase::Init();

    // 對齊反編譯：三個欄位清 0（語意未知）
    m_ReservedA = 0;
    m_ReservedB = 0;
    m_ReservedC = 0;
}

CControlBoxEquip::~CControlBoxEquip()
{
    // 子物件自動解構；順序與反編譯一致即可
}

//--------------------------------------------------
// 建立子件（對齊 00418560）
//--------------------------------------------------
void CControlBoxEquip::CreateChildren()
{
    // 基底會建立底圖（+120）
    CControlBoxBase::CreateChildren();

    // +312 高亮框
    m_Highlight.Create(this);
    m_Highlight.SetSize(32, 32);                            // *((WORD*)this + 172/173) = 32
    m_Highlight.SetColor(1.0f, 0.0f, 0.0f, 0.3921569f);     // 預設紅色半透明

    // +520 疊加圖（block 6）
    m_SealImg6.Create(this);
    m_SealImg6.SetPos(-3, -3);
    m_SealImg6.SetImage(570425419u, 6);

    // +712 疊加圖（block 7）
    m_SealImg7.Create(this);
    m_SealImg7.SetPos(-3, -3);
    m_SealImg7.SetImage(570425419u, 7);
}

//--------------------------------------------------
// 設定封印狀態（對齊 004185F0 邏輯）
//--------------------------------------------------
int CControlBoxEquip::SetSealStatus(int status)
{
    m_SealStatus = status;

    if (status == 1) {
        // 黃色
        m_Highlight.SetColor(1.0f, 1.0f, 0.0f, 0.3921569f);
        m_Highlight.Show();
        m_SealImg6.Show();
        m_SealImg7.Hide();
        m_DrawState = 1;
    }
    else if (status == 2) {
        // 綠色
        m_Highlight.SetColor(0.35294119f, 0.8705883f, 0.0f, 0.3921569f);
        m_Highlight.Show();
        m_SealImg6.Hide();
        m_SealImg7.Show();
        m_DrawState = 2;
    }
    else {
        // 全隱藏
        m_Highlight.Hide();
        m_SealImg6.Hide();
        m_SealImg7.Hide();
        m_DrawState = 0;
    }

    return status;
}

//--------------------------------------------------
// 顯示/隱藏子件（對齊 004186F0 / 00418740）
//--------------------------------------------------
void CControlBoxEquip::ShowChildren()
{
    CControlBoxBase::ShowChildren();
    m_Highlight.Hide();
    m_SealImg6.Hide();
    m_SealImg7.Hide();
    m_DrawState = 0; // *((DWORD*)this + 226) = 0
}

void CControlBoxEquip::HideChildren()
{
    CControlBoxBase::HideChildren();
    m_Highlight.Hide();
    m_SealImg6.Hide();
    m_SealImg7.Hide();
    m_DrawState = 0; // 同 ShowChildren
}

//--------------------------------------------------
// 供渲染管線：當狀態為 1..2 時，確保相關子件被推繪
//（反編譯會在狀態 1/2 時對 +120/+520/+712 呼叫 PD/Draw）
//--------------------------------------------------
void CControlBoxEquip::PrepareDrawing()
{
    CControlBoxBase::PrepareDrawing();

    if (m_DrawState > 0 && m_DrawState <= 2) {
        // 基底的 +120 會由 CControlBoxBase 處理；補齊兩張疊加圖的 PD
        m_SealImg6.PrepareDrawing();
        m_SealImg7.PrepareDrawing();
    }
}

void CControlBoxEquip::Draw()
{
    CControlBoxBase::Draw();

    if (m_DrawState > 0 && m_DrawState <= 2) {
        // 同上，補齊兩張疊加圖的 Draw
        m_SealImg6.Draw();
        m_SealImg7.Draw();
    }
}
