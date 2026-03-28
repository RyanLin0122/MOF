#pragma once

#include "CControlImage.h"
#include "CControlText.h"

// 基底按鈕：以圖片為背景 + 文字子控制，提供按下位移效果
// 反編譯對照：
//   this+0:     CControlImage 基底
//   this+208:   char[16] 音效名稱 (預設 "J0003")
//   this+224:   CControlText 內嵌子控制
//   this[48]:   m_bChildMoveByClick (DWORD, 預設 0)
//   this[49]:   m_bMouseOver (DWORD, 預設 0)
//   this[50]:   m_bPressed (DWORD, 預設 0)
//   this[51]:   m_nShiftAmount (DWORD, 預設 2)
class CControlButtonBase : public CControlImage
{
public:
    CControlButtonBase();
    virtual ~CControlButtonBase();

    // ---- 文字 ----
    void SetText(const char* a2);
    void SetText(int stringId);

    // 取得內建文字控制
    CControlText* GetTextCtrl() { return &m_Text; }

    // ---- 按下位移效果 ----
    // 反編譯：SetChildMoveByClick(a2=enable, a3=shiftAmount)
    void SetChildMoveByClick(int a2, int a3);

    // 反編譯：IsMouseOver() => return this[49]
    int IsMouseOver();

    virtual void ButtonPosDown();
    virtual void ButtonPosUp();

    // 音效（呼叫 g_GameSoundManager.PlaySoundA）
    virtual void PlaySoundClick();

    // 建立子控制（文字）
    void CreateChildren();

    // 初始化（呼叫 virtual Show，設定音效名稱）
    void Init();

protected:
    // 對齊反編譯佈局：ints 在前，音效名稱在中，文字控制在後
    // 反編譯：this[48]=enable, this[49]=mouseOver, this[50]=pressed, this[51]=shiftAmount
    int  m_bChildMoveByClick{ 0 };     // +192 (DWORD idx 48)
    int  m_bMouseOver{ 0 };            // +196 (DWORD idx 49)
    int  m_bPressed{ 0 };             // +200 (DWORD idx 50)
    int  m_nShiftAmount{ 2 };          // +204 (DWORD idx 51, 預設 2)

    // 音效名稱（offset +208, 反編譯：strcpy((char*)this + 208, "J0003")）
    char m_szSoundName[16]{};          // +208

    // 內建文字子控制（offset +224）
    CControlText m_Text;               // +224
};
