#include "UI/CControlChatPrint.h"
#include "global.h"
#include "Logic/cltChattingMgr.h"

// ================================================================
//  建構 / 解構
// ================================================================
CControlChatPrint::CControlChatPrint()
{
}

CControlChatPrint::~CControlChatPrint()
{
    // CControlText 與 CControlBase 的解構子由 C++ 自動呼叫
    // 對齊反編譯：先 ~CControlText(+560)，再 ~CControlText(+128)，再 ~CControlBase
}

// ================================================================
//  Create — 初始化基底及兩個內嵌 CControlText
// ================================================================
void CControlChatPrint::Create(CControlBase* pParent)
{
    char Buffer[256];

    CControlBase::Create(pParent);

    // 建立遊標文字控制（反編譯 offset +128）
    m_TextCursor.Create(this);
    sprintf(Buffer, "%c", 95);                     // '_'
    m_TextCursor.SetText(Buffer);
    m_TextCursor.SetTextColor(0xFFFFFFFF);          // 對齊 *((_DWORD*)this + 69) = -1

    // 建立名稱文字控制（反編譯 offset +560）
    m_TextName.Create(this);
    m_TextName.SetTextColor(0xFFFFFFFF);            // 對齊 *((_DWORD*)this + 177) = -1
}

// ================================================================
//  SetInputChat — 設定聊天輸入顯示（名稱 + 遊標位置）
// ================================================================
void CControlChatPrint::SetInputChat(int a2, int a3, char* a4, int a5, char* lpString)
{
    // ---- 名稱區塊（m_TextName，反編譯 offset +560）----
    if (a4)
    {
        m_TextName.SetPos(0, 0);
        m_TextName.SetText(a4);
        m_TextName.Show();
    }
    else
    {
        m_TextName.Hide();
    }

    // ---- 遊標區塊（m_TextCursor，反編譯 offset +128）----
    if (a5)
    {
        m_TextCursor.Show();

        // 取得字型名稱（textId 3264）
        const char* fontFace = g_DCTTextManager.GetText(3264);

        // 計算輸入文字的像素寬度
        int width = 0, height = 0;
        g_MoFFont.GetTextLength(&width, &height, 12, fontFace, lpString, 400);

        // 把遊標放在輸入文字後面
        m_TextCursor.SetPos(width, 3);
    }
    else
    {
        m_TextCursor.Hide();
    }

    // ---- 設定聊天區塊位置 ----
    // 對齊 ground truth：呼叫兩次 GetAbsPos，一次取 Y、一次取 X
    int v11X, absY;
    GetAbsPos(v11X, absY);

    int absX, v12Y;
    GetAbsPos(absX, v12Y);

    g_clChattingMgr.SetChatBlock(absX, absY);
}

// ================================================================
//  SetAbsPos — 委派至基底
// ================================================================
void CControlChatPrint::SetAbsPos(int a2, int a3)
{
    CControlBase::SetAbsPos(a2, a3);
}

// ================================================================
//  Draw — 繪製基底後再繪製聊天遮罩
// ================================================================
void CControlChatPrint::Draw()
{
    CControlBase::Draw();
    g_clChattingMgr.DrawBlockBox();
}
