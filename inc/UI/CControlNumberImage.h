#pragma once
#include <cstdint>
#include <cstddef>
#include "UI/CControlBase.h"
#include "UI/CControlImage.h"

/**
 * 以連續圖塊組合顯示數字/小數的控制。
 * - 內含 20 張子圖（digits），可顯示最多 20 個字元（含小數點）。
 * - 位置採左到右依序排列；每字之間加入 style 依賴的 fixed advance。
 */
class CControlNumberImage : public CControlBase
{
public:
    CControlNumberImage();
    virtual ~CControlNumberImage();

    // 反編譯：Create(this, x, y, styleIndex, parent)
    void Create(int x, int y, int styleIndex, CControlBase* parent);

    // 設定整數數字（對齊 00421DA0）
    void SetNumber(long long value);

    // 設定浮點數字（對齊 00421F60）
    // decimals: 小數位數；forceFixed: 1=強制顯示 decimals 位，0=若小數前 decimals 位全 0 則省略小數
    void SetNumberFloat(float value, int decimals, int forceFixed);

    // 設定縮放（對齊 00422150）
    void SetScale(float s);

    // （可選）查詢本控制計算出的總寬/高
    uint16_t GetTotalWidth() const { return m_TotalW; }
    uint16_t GetTotalHeight() const { return m_TotalH; }

private:
    // 內部：設定第 idx 張 digit 的圖塊與相對座標，並嘗試套用 scale
    void PlaceGlyph(int renderIndex, int texId, int block, int xOffset);

private:
    CControlImage m_Digits[20];     // +128 起 20 張（每張約 0xC0）
    int           m_StyleIndex{ 2 };  // *((DWORD*)this + 31)
    float         m_Scale{ 1.0f };    // *((float*)this + 992)
    uint16_t      m_TotalW{ 0 };      // *((WORD*)this + 16)
    uint16_t      m_TotalH{ 0 };      // *((WORD*)this + 17)
};
