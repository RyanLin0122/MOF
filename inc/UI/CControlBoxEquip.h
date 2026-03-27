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
 *   2 = 綠色高亮、隱藏 block6、顯示 block7
 */
class CControlBoxEquip : public CControlBoxBase
{
public:
    CControlBoxEquip();
    virtual ~CControlBoxEquip();

    void CreateChildren();

    // 設定封印狀態（對齊 004185F0）
    void SetSealStatus(int status);

    // 對齊 004186F0 / 00418740
    virtual void ShowChildren() override;
    virtual void HideChildren() override;

    // 對齊 00418790 / 004187D0
    virtual void PrepareDrawing() override;
    virtual void Draw() override;

private:
    CControlAlphaBox m_Highlight; // +312
    CControlImage    m_SealImg6;  // +520（imageId=570425419, block=6）
    CControlImage    m_SealImg7;  // +712（imageId=570425419, block=7）

    // 封印狀態：同時作為 PrepareDrawing/Draw 的判斷值
    // 反編譯中 SetSealStatus 寫入 *(this+904)，PrepareDrawing/Draw 讀取 *(DWORD*)(this+226) 即同一欄位
    int m_SealStatus{ 0 };
};
