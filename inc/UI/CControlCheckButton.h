#pragma once
#include "UI/CControlButtonBase.h"

// 反編譯位址：0041C200 ~ 0041C520
class CControlCheckButton final : public CControlButtonBase
{
public:
    CControlCheckButton();
    ~CControlCheckButton() override;

    // 建立子物件（覆寫：設定文字位置，並給預設影像）
    void CreateChildren() /*override*/;

    // 設定「同一張圖」的兩個影格（未勾/已勾）
    // giid        : 資源 GIID（例如 0x20000013）
    // blockUnchecked : 未勾選時的 block（例：0x15）
    // blockChecked   : 勾選時的 block（例：0x14）
    void SetImage(unsigned int giid, unsigned short blockUnchecked, unsigned short blockChecked);

    // 事件處理：攔截按下/放開以切換影像與回呼
    int* ControlKeyInputProcess(int msg, int key, int x, int y, int a6, int a7) override;

    // 目前是否為「已勾選」狀態
    bool IsChecked() const;

    // 設定成勾選/不勾選（預設：設為勾選）。triggerCallback=true 時會走和點擊一致的回呼路徑
    void SetCheck(bool checked = true, bool triggerCallback = true);

    // 指定回呼：模擬反編譯 SetCallFunc(a2,a3,a4,a5,a6)
    // basePtr + argOffset 會當作唯一參數傳入 fn(int)
    void SetCallFunc(int basePtr, void (*fn)(int), int argOffset, int a5 = 0, int a6 = 0);

private:
    void CallFunc();

private:
    // 兩組影像參數（同圖不同 block）
    unsigned int   m_giidUnchecked{ 0 };
    unsigned int   m_giidChecked{ 0 };
    unsigned short m_blockUnchecked{ 0xFFFF };
    unsigned short m_blockChecked{ 0xFFFF };

    // 反編譯的 +164, +166~+169
    int  m_cbBasePtr{ 0 };                 // a2
    void (*m_cbFunc)(int) { nullptr };      // a3
    int  m_cbArgOffset{ 0 };               // a4
    int  m_cbA5{ 0 };                      // a5（保留）
    int  m_cbA6{ 0 };                      // a6（保留）
};

//（可選）若你專案有「輸入被消耗」旗標，這裡宣告一下；沒有就移除此 extern
extern int g_uiInputConsumed;
