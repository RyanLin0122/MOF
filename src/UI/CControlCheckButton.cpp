#include "UI/CControlCheckButton.h"
#include <cstring>

int g_uiInputConsumed = 0; // 若專案已有全域旗標，請移除此定義，保留 extern 即可

CControlCheckButton::CControlCheckButton()
{
    // 依反編譯初始化：
    // 未勾：group=5（用 CControlImage 內部預設 group），giid=0，block=-1
    // 已勾：group=5，giid=0，block=-1
    m_giidUnchecked = 0;
    m_giidChecked = 0;
    m_blockUnchecked = 0xFFFF;
    m_blockChecked = 0xFFFF;

    // 預設的回呼暫存
    m_cbBasePtr = 0;
    m_cbFunc = nullptr;
    m_cbArgOffset = 0;
    m_cbA5 = 0;
    m_cbA6 = 0;

    // 反編譯：strcpy(this+208,"J0002") 是音效 key；
    // 我們用 CControlButtonBase 的 PlaySoundClick() 鉤子，不強制存 key，
    // 若你有音效管理，可在此記下 key 再 PlaySoundClick() 取用。
}

CControlCheckButton::~CControlCheckButton()
{
    // 基底會負責 m_Text 與影像的釋放鏈結
}

void CControlCheckButton::CreateChildren()
{
    // 先讓基底建立文字子控制
    CControlButtonBase::CreateChildren(); // 內含 m_Text.Create(this) 等預設處理  :contentReference[oaicite:2]{index=2}

    // 反編譯：*((_DWORD*)this + 92) = 2; 這裡用可讀 API 設定對齊
    if (auto* txt = GetTextCtrl())
    {
        txt->SetAlignment(2);        // 你現有 CControlText 的 API
        txt->SetPos(-3, 3);          // 反編譯的 SetPos((char*)this+224,-3,3)
    }

    // 預設影像（反編譯：SetImage(this, 0x20000013, 0x15, 0x14)）
    SetImage(0x20000013u, 0x15u, 0x14u);
}

void CControlCheckButton::SetImage(unsigned int giid, unsigned short blockUnchecked, unsigned short blockChecked)
{
    // 存兩組：同一 giid，不同 block
    m_giidUnchecked = giid;
    m_blockUnchecked = blockUnchecked;

    m_giidChecked = giid;
    m_blockChecked = blockChecked;

    // 立刻以「未勾」顯示（反編譯在 SetImage 內有立即 SetImageID）
    // 用 CControlImage 的簡化版 SetImageID(giid, block)（沿用內部 m_nGIGroup） :contentReference[oaicite:3]{index=3}
    CControlImage::SetImageID(m_giidUnchecked, m_blockUnchecked);
}

bool CControlCheckButton::IsChecked() const
{
    // 反編譯：return (unsigned __int16)this[32] != *((_WORD*)this + 344);
    // 等價：目前 block != 未勾 block => 已勾
    return m_usBlockID != m_blockUnchecked;
}

void CControlCheckButton::SetCheck(bool checked, bool triggerCallback)
{
    if (checked == IsChecked()) return;

    // 切換到目標狀態
    if (checked)
        CControlImage::SetImageID(m_giidChecked, m_blockChecked);
    else
        CControlImage::SetImageID(m_giidUnchecked, m_blockUnchecked);

    if (triggerCallback)
        CallFunc();
}

void CControlCheckButton::SetCallFunc(int basePtr, void (*fn)(int), int argOffset, int a5, int a6)
{
    // 反編譯：this[164]=a2; this[166]=a3; this[167]=a4; this[168]=a5; this[169]=a6
    m_cbBasePtr = basePtr;
    m_cbFunc = fn;
    m_cbArgOffset = argOffset;
    m_cbA5 = a5;
    m_cbA6 = a6;
}

void CControlCheckButton::CallFunc()
{
    // 反編譯： v1=this[164]; v2=this[166]; if(v1 && v2) v2(v1 + this[167]);
    if (m_cbBasePtr && m_cbFunc)
        m_cbFunc(m_cbBasePtr + m_cbArgOffset);
}

int* CControlCheckButton::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    // 反編譯：
    // msg==0（按下）：播放按鈕音效；g_flag=0
    // msg==3（點擊/放開）：依目前 block 切換到另一組（checked <-> unchecked），呼叫回呼；g_flag=1

    if (msg == 0)
    {
        CControlButtonBase::PlaySoundClick();
        g_uiInputConsumed = 0; // dword_AFD34C = 0
    }
    else if (msg == 3)
    {
        const bool nowChecked = IsChecked();
        if (!nowChecked) {
            // 目前未勾 -> 切到已勾
            CControlImage::SetImageID(m_giidChecked, m_blockChecked);
        }
        else {
            // 目前已勾 -> 切回未勾
            CControlImage::SetImageID(m_giidUnchecked, m_blockUnchecked);
        }

        CallFunc();
        g_uiInputConsumed = 1; // dword_AFD34C = 1
    }

    // 事件冒泡：維持你基底的既有流程與回傳語義（回傳父物件指標或其處理後的結果） :contentReference[oaicite:4]{index=4}
    return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}
