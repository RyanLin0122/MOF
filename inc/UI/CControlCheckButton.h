#pragma once

#include "UI/CControlButtonBase.h"

// 勾選按鈕
// 反編譯對照 (0041C200 ~ 0041C520)：
// CControlButtonBase 基底（CControlImage + 音效 + CControlText）
// 之後依序：
//   DWORD 164: m_cbBasePtr（回呼物件指標）
//   DWORD 166: m_cbFunc（回呼函式指標）
//   DWORD 167: m_cbArgOffset
//   DWORD 168: m_cbA5
//   DWORD 169: m_cbA6（預設 -1）
//   DWORD 170: m_uncheckedGIGroup（預設 5）
//   DWORD 171: m_uncheckedGIID
//   WORD  344: m_uncheckedBlock（0xFFFF）
//   DWORD 173: m_checkedGIGroup（預設 5）
//   DWORD 174: m_checkedGIID
//   WORD  350: m_checkedBlock（0xFFFF）
class CControlCheckButton : public CControlButtonBase
{
public:
    CControlCheckButton();
    virtual ~CControlCheckButton();

    void CreateChildren();

    // 設定影像（同一 GIID，不同 block 分別對應未勾/已勾）
    void SetImage(unsigned int giid, unsigned short blockUnchecked, unsigned short blockChecked);

    // 事件處理
    virtual int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;

    // 查詢是否已勾選
    BOOL IsChecked();

    // 設定為已勾選（反編譯：若未勾選則模擬一次 msg=3 的點擊）
    void SetCheck(int a2);

    // 設定回呼函式
    int SetCallFunc(int basePtr, int fn, int argOffset, int a5, int a6);

private:
    void CallFunc();

private:
    // 回呼相關
    int  m_cbBasePtr{ 0 };      // DWORD 164
    int  m_pad165{ 0 };         // DWORD 165（未使用/對齊）
    int  m_cbFunc{ 0 };         // DWORD 166（函式指標，以 int 存）
    int  m_cbArgOffset{ 0 };    // DWORD 167
    int  m_cbA5{ 0 };           // DWORD 168
    int  m_cbA6{ -1 };          // DWORD 169（預設 -1）

    // 未勾選狀態影像
    unsigned int   m_uncheckedGIGroup{ 5 };   // DWORD 170
    unsigned int   m_uncheckedGIID{ 0 };      // DWORD 171
    unsigned short m_uncheckedBlock{ 0xFFFF }; // WORD 344
    unsigned short m_pad172{ 0 };

    // 已勾選狀態影像
    unsigned int   m_checkedGIGroup{ 5 };     // DWORD 173
    unsigned int   m_checkedGIID{ 0 };        // DWORD 174
    unsigned short m_checkedBlock{ 0xFFFF };   // WORD 350
    unsigned short m_pad175{ 0 };
};
