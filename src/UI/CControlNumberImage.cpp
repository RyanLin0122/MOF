#include <cstdio>
#include <cstring>
#include "UI/CControlNumberImage.h"
#include "global.h"

// ============================================================================
// 建構 — 對齊 00421C50
// ============================================================================
CControlNumberImage::CControlNumberImage()
    : CControlBase()
{
    // m_Digits[20] 由 C++ 自動建構 (對應 eh vector constructor iterator)
    // 將每張 digit Create 到本控制之下
    for (int i = 0; i < 20; ++i)
    {
        m_Digits[i].Create(this);
    }
    m_StyleIndex = 2;
}

// ============================================================================
// 解構 — 對齊 00421D00
// ============================================================================
CControlNumberImage::~CControlNumberImage()
{
    // m_Digits[20] 由 C++ 自動解構 (對應 eh vector destructor iterator)
}

// ============================================================================
// Create — 對齊 00421D70
// ============================================================================
void CControlNumberImage::Create(int x, int y, int styleIndex, CControlBase* parent)
{
    m_StyleIndex = styleIndex;
    CControlBase::Create(x, y, parent);
    m_Scale = 1.0f;
}

// ============================================================================
// SetScale — 對齊 00422150
// ============================================================================
void CControlNumberImage::SetScale(float s)
{
    m_Scale = s;
}

// ============================================================================
// SetNumber — 對齊 00421DA0
// ============================================================================
void CControlNumberImage::SetNumber(long long value)
{
    char Buffer[256];
    std::memset(Buffer, 0, sizeof(Buffer));
    m_usWidth = 0;

    std::sprintf(Buffer, "%I64d", value);  // 對齊反編譯：固定使用 MSVC 格式

    int strLen = static_cast<int>(std::strlen(Buffer));
    if (strLen > 20) return;

    int digitIdx = 0;
    if (strLen > 0)
    {
        CControlImage* pDigit = &m_Digits[0];
        for (int i = 0; i < strLen; ++i)
        {
            int mapIdx = 13 * m_StyleIndex + static_cast<unsigned char>(Buffer[i]);
            unsigned short block = static_cast<unsigned short>(kGlyphIndexByStyleChar[2 * mapIdx]);

            // SetImage 通過虛表 (vtable offset 84 對應 SetImage)
            pDigit->SetImage(
                static_cast<unsigned int>(kAtlasIdByStyle[13 * m_StyleIndex]),
                block);

            // 從第二張起，依前一張的位置 + 寬度 + advance 設定絕對位置
            if (i)
            {
                CControlImage* pPrev = pDigit - 1;
                uint16_t prevW = pPrev->GetWidth();
                int prevAbsX = pPrev->GetAbsX();
                pDigit->SetAbsX(kAdvanceByStyle[13 * m_StyleIndex] + prevAbsX + prevW);
            }

            // 設定縮放
            pDigit->SetScale(m_Scale, m_Scale);

            // 累加總寬：width * scale + advance + 目前 totalW
            long long v9 = static_cast<long long>(
                static_cast<double>(pDigit->GetWidth()) * static_cast<double>(m_Scale)
                + static_cast<double>(kAdvanceByStyle[13 * m_StyleIndex])
                + static_cast<double>(m_usWidth));
            m_usWidth = static_cast<uint16_t>(v9);

            ++digitIdx;
            ++pDigit;
        }
    }

    // 高度 = 第一張 digit 的高度 * scale
    m_usHeight = static_cast<uint16_t>(
        static_cast<long long>(
            static_cast<double>(m_Digits[0].GetHeight()) * static_cast<double>(m_Scale)));

    // 清除未使用的 digit
    if (strLen < 20)
    {
        for (int i = strLen; i < 20; ++i)
        {
            m_Digits[i].SetImage(static_cast<unsigned int>(-1), 0xFFFF);
        }
    }
}

// ============================================================================
// SetNumberFloat — 對齊 00421F60
// ============================================================================
void CControlNumberImage::SetNumberFloat(float value, int decimals, int forceFixed)
{
    int digitIdx = 0;
    char Buffer[256];
    std::memset(Buffer, 0, sizeof(Buffer));
    m_usWidth = 0;

    std::sprintf(Buffer, "%f", value);

    int strLen = static_cast<int>(std::strlen(Buffer));
    int dotIdx = 0;

    // 找到小數點位置
    int searchIdx = 0;
    while (searchIdx < strLen)
    {
        if (Buffer[searchIdx] == '.')
        {
            dotIdx = searchIdx;
            break;
        }
        ++searchIdx;
        if (searchIdx >= strLen)
            goto LABEL_4;
    }

    if (forceFixed)
        goto LABEL_16;

    // 檢查小數部分是否全為 '0'
    {
        int checkIdx = 0;
        if (decimals <= 0)
            goto LABEL_4;
        while (Buffer[searchIdx + 1 + checkIdx] == '0')
        {
            if (++checkIdx >= decimals)
                goto LABEL_4;
        }
    }

LABEL_16:
    {
        int cutLen = searchIdx + decimals + 1;
        if (cutLen > 20) return;

        CControlImage* pDigit = &m_Digits[0];
        while (digitIdx < cutLen)
        {
            int styleBase = 13 * m_StyleIndex;

            if (digitIdx != dotIdx)
            {
                // 一般數字字元
                int mapIdx = styleBase + static_cast<unsigned char>(Buffer[digitIdx]);
                unsigned short block = static_cast<unsigned short>(kGlyphIndexByStyleChar[2 * mapIdx]);
                pDigit->SetImage(
                    static_cast<unsigned int>(kAtlasIdByStyle[styleBase]),
                    block);
            }
            else
            {
                // 小數點
                int dotBlock = kDotGlyphIndexByStyle[styleBase];
                if (dotBlock != -1)
                {
                    pDigit->SetImage(
                        static_cast<unsigned int>(kAtlasIdByStyle[styleBase]),
                        static_cast<unsigned short>(dotBlock));
                }
                else
                {
                    // 無小數點圖塊：對齊反編譯，仍推進 digitIdx 和 pDigit
                    ++digitIdx;
                    ++pDigit;
                    continue;
                }
            }

            // 從第二張起定位
            if (digitIdx)
            {
                CControlImage* pPrev = pDigit - 1;
                uint16_t prevW = pPrev->GetWidth();
                int prevAbsX = pPrev->GetAbsX();
                pDigit->SetAbsX(kAdvanceByStyle[13 * m_StyleIndex] + prevAbsX + prevW);
            }

            // 累加寬度（不乘 scale，與 SetNumber 不同）
            m_usWidth += static_cast<uint16_t>(
                kAdvanceByStyle[13 * m_StyleIndex] + pDigit->GetWidth());

            ++digitIdx;
            ++pDigit;
        }

        // 高度 = 第一張 digit 的高度（不乘 scale，與 SetNumber 不同）
        m_usHeight = m_Digits[0].GetHeight();

        // 清除未使用的 digit
        if (cutLen < 20)
        {
            for (int i = cutLen; i < 20; ++i)
            {
                m_Digits[i].SetImage(static_cast<unsigned int>(-1), 0xFFFF);
            }
        }
        return;
    }

LABEL_4:
    {
        // 無小數部分，只顯示整數
        int cutLen = dotIdx;
        if (cutLen > 20) return;

        CControlImage* pDigit = &m_Digits[0];
        while (digitIdx < cutLen)
        {
            int styleBase = 13 * m_StyleIndex;

            if (digitIdx != dotIdx)
            {
                int mapIdx = styleBase + static_cast<unsigned char>(Buffer[digitIdx]);
                unsigned short block = static_cast<unsigned short>(kGlyphIndexByStyleChar[2 * mapIdx]);
                pDigit->SetImage(
                    static_cast<unsigned int>(kAtlasIdByStyle[styleBase]),
                    block);
            }
            else
            {
                int dotBlock = kDotGlyphIndexByStyle[styleBase];
                if (dotBlock != -1)
                {
                    pDigit->SetImage(
                        static_cast<unsigned int>(kAtlasIdByStyle[styleBase]),
                        static_cast<unsigned short>(dotBlock));
                }
                else
                {
                    // 對齊反編譯：仍推進 digitIdx 和 pDigit
                    ++digitIdx;
                    ++pDigit;
                    continue;
                }
            }

            if (digitIdx)
            {
                CControlImage* pPrev = pDigit - 1;
                uint16_t prevW = pPrev->GetWidth();
                int prevAbsX = pPrev->GetAbsX();
                pDigit->SetAbsX(kAdvanceByStyle[13 * m_StyleIndex] + prevAbsX + prevW);
            }

            m_usWidth += static_cast<uint16_t>(
                kAdvanceByStyle[13 * m_StyleIndex] + pDigit->GetWidth());

            ++digitIdx;
            ++pDigit;
        }

        m_usHeight = m_Digits[0].GetHeight();

        if (cutLen < 20)
        {
            for (int i = cutLen; i < 20; ++i)
            {
                m_Digits[i].SetImage(static_cast<unsigned int>(-1), 0xFFFF);
            }
        }
    }
}
