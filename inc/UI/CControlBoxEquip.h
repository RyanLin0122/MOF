#pragma once
#include "CControlBoxBase.h"
#include "CControlAlphaBox.h"
#include "CControlImage.h"

/**
 * 裝備格（帶封印狀態顯示）
 * - 基底底圖：來自 CControlBoxBase（+120）
 * - 高亮框：+312（CControlAlphaBox）
 * - 疊加圖：+520（block 6）、+712（block 7）
 *
 * SetSealStatus:
 *   0 = 關閉（全部隱藏）
 *   1 = 黃色高亮、顯示 block6、隱藏 block7
 *   2 = 綠色高亮、隱示 block6、顯示 block7
 */
class CControlBoxEquip : public CControlBoxBase
{
public:
    CControlBoxEquip();
    virtual ~CControlBoxEquip();

    // 依反編譯：建子件
    void CreateChildren();

    // 設定封印狀態（對齊 004185F0）
    int SetSealStatus(int status);

    // 對齊 004186F0 / 00418740
    virtual void ShowChildren() override;
    virtual void HideChildren() override;

    // 對齊 00418790 / 004187D0
    virtual void PrepareDrawing() override;
    virtual void Draw() override;

private:
    // 子件（位移對齊反編譯）
    CControlAlphaBox m_Highlight; // +312
    CControlImage    m_SealImg6;  // +520（imageId=570425419, block=6）
    CControlImage    m_SealImg7;  // +712（imageId=570425419, block=7）

    // 狀態欄位
    int m_SealStatus{ 0 };   // 對應 this+904 的外部寫入；這裡以語意欄位呈現
    int m_DrawState{ 0 };    // 對應 *((DWORD*)this + 226)，控制 PD/Draw 的額外推繪

    // 反編譯見到被清為 0 的三個欄位（+92/+144/+192），語意不明，保留以對齊行為
    int m_ReservedA{ 0 };
    int m_ReservedB{ 0 };
    int m_ReservedC{ 0 };
};
