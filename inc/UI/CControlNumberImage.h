#pragma once
#include <cstdint>
#include <cstddef>
#include "UI/CControlBase.h"
#include "UI/CControlImage.h"

// 全域查找表 (定義於 global.h)
extern short kGlyphIndexByStyleChar[];
extern int   kDotGlyphIndexByStyle[];
extern int   kAdvanceByStyle[];
extern int   kAtlasIdByStyle[];

/**
 * 以連續圖塊組合顯示數字/小數的控制。
 * - 內含 20 張子圖 (CControlImage)，可顯示最多 20 個字元。
 * - 對齊反編譯 00421C50 ~ 00422150
 */
class CControlNumberImage : public CControlBase
{
public:
    CControlNumberImage();
    virtual ~CControlNumberImage();

    // 反編譯 00421D70
    void Create(int x, int y, int styleIndex, CControlBase* parent);

    // 反編譯 00421DA0
    void SetNumber(long long value);

    // 反編譯 00421F60
    void SetNumberFloat(float value, int decimals, int forceFixed);

    // 反編譯 00422150
    void SetScale(float s);

private:
    // 對齊反編譯佈局：CControlBase 結尾在 byte 120 (DWORD[30])。
    // Ground truth 中 m_StyleIndex 位於 DWORD[31] (byte 124)，m_Digits 從 byte 128 開始。
    // 因此 DWORD[30] (byte 120) 為未使用的 4-byte 空間。
    int           m_pad30{ 0 };       // *((DWORD*)this + 30) — 填充，對齊 m_Digits 至 byte 128
    int           m_StyleIndex{ 2 };  // *((DWORD*)this + 31) = byte 124
    CControlImage m_Digits[20];       // +128 起 20 張 (每張 0xC0 = 192 bytes)
    float         m_Scale{ 1.0f };    // *((float*)this + 992)
};
