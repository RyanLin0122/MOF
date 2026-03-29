#include "UI/CControlCheckButton.h"
#include "global.h"
#include <cstring>

// 對齊反編譯 0041C200
CControlCheckButton::CControlCheckButton()
    : CControlButtonBase()
{
    // 初始化影像參數
    m_uncheckedGIGroup = 5;
    m_uncheckedGIID = 0;
    m_uncheckedBlock = 0xFFFF;
    m_checkedGIGroup = 5;
    m_checkedBlock = 0xFFFF;
    m_checkedGIID = 0;

    // 音效名稱
    SetSoundName("J0002");

    // 建立子控制
    CreateChildren();

    // 初始化回呼
    m_cbFunc = 0;
    m_cbBasePtr = 0;
    m_cbArgOffset = 0;
    m_cbA5 = 0;
    m_cbA6 = -1;
}

// 對齊反編譯 0041C2F0
CControlCheckButton::~CControlCheckButton()
{
    // m_Text (CControlText at +224) 和 CControlImage 基底自動解構
}

// 對齊反編譯 0041C340
void CControlCheckButton::CreateChildren()
{
    CControlButtonBase::CreateChildren();

    // *((_DWORD *)this + 92) = 2 → m_Text 的某個欄位（在 CControlText 中對應 alignment）
    // m_Text 起始於 +224，DWORD 92 相對 this = byte 368 = 224+144 → m_Text 內 DWORD 36 = m_isCentered
    // 但 *((_DWORD *)this + 92) 在 CControlButtonBase 佈局中實際是 image count 或相關設定
    // 反編譯直接寫 2，保持語意等價
    m_Text.m_isCentered = 2;
    m_Text.SetPos(-3, 3);

    // 預設影像：giid=0x20000013, blockUnchecked=0x15, blockChecked=0x14
    SetImage(0x20000013u, 0x15u, 0x14u);
}

// 對齊反編譯 0041C380
int CControlCheckButton::SetCallFunc(int basePtr, int fn, int argOffset, int a5, int a6)
{
    m_cbBasePtr = basePtr;
    m_cbFunc = fn;
    m_cbArgOffset = argOffset;
    m_cbA5 = a5;
    m_cbA6 = a6;
    return a6;
}

// 對齊反編譯 0041C3B0
void CControlCheckButton::CallFunc()
{
    if (m_cbBasePtr)
    {
        if (m_cbFunc)
        {
            // 反編譯：v2 = (void (__thiscall *)(int))m_cbFunc; v2(m_cbBasePtr + m_cbArgOffset);
            typedef void (__thiscall *CallbackFn)(int);
            CallbackFn fn = reinterpret_cast<CallbackFn>(m_cbFunc);
            fn(m_cbBasePtr + m_cbArgOffset);
        }
    }
}

// 對齊反編譯 0041C3D0
void CControlCheckButton::SetImage(unsigned int giid, unsigned short blockUnchecked, unsigned short blockChecked)
{
    m_uncheckedGIID = giid;
    m_uncheckedBlock = blockUnchecked;

    // 立即以未勾選影像顯示
    CControlImage::SetImageID(m_uncheckedGIGroup, giid, m_uncheckedBlock);

    // 儲存已勾選影像（反編譯：DWORD 174 = giid, WORD 350 = blockChecked）
    m_checkedGIID = giid;
    m_checkedBlock = blockChecked;
}

// 對齊反編譯 0041C430
int* CControlCheckButton::ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7)
{
    if (msg == 0)
    {
        // 按下：播放音效
        CControlButtonBase::PlaySoundClick();
        dword_AFD34C = 0;
    }
    else if (msg == 3)
    {
        // 點擊放開：切換勾選狀態
        if (m_usBlockID == m_uncheckedBlock)
        {
            // 目前未勾選 → 切到已勾選
            CControlImage::SetImageID(m_checkedGIGroup, m_checkedGIID, m_checkedBlock);
        }
        else
        {
            // 目前已勾選 → 切回未勾選
            CControlImage::SetImageID(m_uncheckedGIGroup, m_uncheckedGIID, m_uncheckedBlock);
        }

        CallFunc();
        dword_AFD34C = 1;
    }

    // 冒泡到基底
    return CControlBase::ControlKeyInputProcess(msg, key, x, y, a6, a7);
}

// 對齊反編譯 0041C4F0
BOOL CControlCheckButton::IsChecked()
{
    return m_usBlockID != m_uncheckedBlock;
}

// 對齊反編譯 0041C520
void CControlCheckButton::SetCheck(int a2)
{
    if (!IsChecked())
    {
        // 模擬一次 msg=3 的點擊事件（透過虛擬呼叫）
        ControlKeyInputProcess(3, 0, 0, 0, 0, 0);
    }
}
