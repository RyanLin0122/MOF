#pragma once

#include <cstdint>
#include <windows.h>
#include <mmsystem.h>

#include "UI/CControlBase.h"
#include "UI/CControlText.h"
#include "UI/CControlCA.h"
#include "UI/CControlButton.h"
#include "UI/CControlImage.h"

// 角色選擇盒（角色選擇畫面使用）
// 反編譯對照：
//   this+0:     CControlBase (120 bytes)
//   this+120:   CControlText  m_TextName    (角色名稱)
//   this+552:   CControlText  m_TextClass   (角色職業)
//   this+984:   CControlText  m_TextLevel   (角色等級)
//   this+1416:  CControlCA    m_CA          (角色動畫，412 bytes)
//   this+1828:  CControlButton m_Button     (選擇按鈕，724 bytes)
//   this+2552:  CControlImage m_PasyImage   (選擇動畫閃爍圖)
//   this+2744:  int m_bSelected             (DWORD 686，選取狀態)
//   this+2748:  byte m_AnimFrame            (動畫幀索引)
//   this+2752:  DWORD m_lastAnimTick        (DWORD 688，上次動畫更新時間)
class CControlCharBox : public CControlBase
{
public:
    CControlCharBox();
    virtual ~CControlCharBox();

    // 建立子控制（由建構子呼叫）
    void CreateChildren();

    // 狀態切換
    void NoExistChar();          // 無角色時設定文字並隱藏 CA/Image
    void ExistChar();            // 有角色時顯示所有控制
    void SelectChar();           // 選取角色
    void SelectCancelChar();     // 取消選取

    // 查詢
    int IsSelect();

    // 動畫繪製
    void PasyImageDraw();
    virtual void PrepareDrawing() override;

    // 子控制公開存取
    CControlText   m_TextName;   // +120
    CControlText   m_TextClass;  // +552
    CControlText   m_TextLevel;  // +984
    CControlCA     m_CA;         // +1416
    CControlButton m_Button;     // +1828
    CControlImage  m_PasyImage;  // +2552

private:
    int   m_bSelected{ 0 };         // DWORD 686 (byte 2744)
    unsigned char m_AnimFrame{ 0 }; // byte at +2748
    DWORD m_lastAnimTick{ 0 };      // DWORD 688 (byte 2752)
};
