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
    CreateChildren();
    CControlBoxBase::Init();
    // 反編譯：*(this+92)=0, *(this+144)=0, *(this+192)=0
    // 這三個寫入對應子物件內部的初始化，C++ 建構子已處理。
    m_SealStatus = 0;
}

CControlBoxEquip::~CControlBoxEquip()
{
    // 子物件自動解構
}

//--------------------------------------------------
// 建立子件（對齊反編譯 00418560）
//--------------------------------------------------
void CControlBoxEquip::CreateChildren()
{
    CControlBoxBase::CreateChildren();

    // +312 高亮框
    m_Highlight.Create(this);
    m_Highlight.SetSize(32, 32);
    m_Highlight.SetColor(1.0f, 0.0f, 0.0f, 0.3921569f);

    // +520 疊加圖（block 6）— 使用 5 參數 Create
    m_SealImg6.Create(-3, -3, 570425419u, 6, this);

    // +712 疊加圖（block 7）
    m_SealImg7.Create(-3, -3, 570425419u, 7, this);
}

//--------------------------------------------------
// 設定封印狀態（對齊 004185F0）
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
    }
    else if (status == 2) {
        // 綠色
        m_Highlight.SetColor(0.35294119f, 0.8705883f, 0.0f, 0.3921569f);
        m_Highlight.Show();
        m_SealImg6.Hide();
        m_SealImg7.Show();
    }
    else {
        // 全隱藏
        m_Highlight.Hide();
        m_SealImg6.Hide();
        m_SealImg7.Hide();
    }
    return 0;
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
    m_SealStatus = 0;
}

void CControlBoxEquip::HideChildren()
{
    CControlBoxBase::HideChildren();
    m_Highlight.Hide();
    m_SealImg6.Hide();
    m_SealImg7.Hide();
    m_SealStatus = 0;
}

//--------------------------------------------------
// 渲染（對齊 00418790 / 004187D0）
// 當 m_SealStatus 為 1 或 2 時，額外繪製底圖與兩張疊加圖
//--------------------------------------------------
void CControlBoxEquip::PrepareDrawing()
{
    CControlBoxBase::PrepareDrawing();

    if (m_SealStatus > 0 && m_SealStatus <= 2) {
        GetBackground()->PrepareDrawing();
        m_SealImg6.PrepareDrawing();
        m_SealImg7.PrepareDrawing();
    }
}

void CControlBoxEquip::Draw()
{
    CControlBoxBase::Draw();

    if (m_SealStatus > 0 && m_SealStatus <= 2) {
        GetBackground()->Draw();
        m_SealImg6.Draw();
        m_SealImg7.Draw();
    }
}
