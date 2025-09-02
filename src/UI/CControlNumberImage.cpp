#include <cstdio>
#include <cstring>
#include <algorithm>
#include "Ui/CControlNumberImage.h"
#include "global.h"

CControlNumberImage::CControlNumberImage()
    : CControlBase()
{
    // 建立 20 張子圖並附掛到本控制
    for (int i = 0; i < 20; ++i) {
        m_Digits[i].Create(this);
    }
    // 反編譯：預設 styleIndex = 2
    m_StyleIndex = 2;
}

CControlNumberImage::~CControlNumberImage()
{
    // 成員自動解構
}

void CControlNumberImage::Create(int x, int y, int styleIndex, CControlBase* parent)
{
    m_StyleIndex = styleIndex;
    CControlBase::Create(x, y, parent);
    m_Scale = 1.0f;  // *((float*)this + 992) = 1065353216
}

void CControlNumberImage::SetScale(float s)
{
    m_Scale = s;
}

void CControlNumberImage::PlaceGlyph(int renderIndex, int texId, int block, int xOffset)
{
    if (renderIndex < 0 || renderIndex >= 20) return;
    CControlImage& img = m_Digits[renderIndex];
    img.SetImage(static_cast<unsigned int>(texId), static_cast<unsigned short>(block));
    img.SetPos(xOffset, 0);
    // 若 CControlImage 提供縮放 API，套用與反編譯一致的等比例縮放
    // （反編譯直接寫入子圖之 m_fScaleX/Y）
#if defined(HAS_CCONTROLIMAGE_SET_SCALE) || 1
    img.SetScale(m_Scale, m_Scale);
#endif
}

void CControlNumberImage::SetNumber(long long value)
{
    // 反編譯：_sprintf(Buffer, "%I64d", a2)
#if defined(_WIN32) || defined(_WIN64)
    constexpr const char* kFmt = "%I64d";
#else
    constexpr const char* kFmt = "%lld";
#endif
    char buf[256]{};
    std::snprintf(buf, sizeof(buf), kFmt, value);

    m_TotalW = 0;
    m_TotalH = 0;

    const int base = 13 * m_StyleIndex;
    const int advance = kAdvanceByStyle[base];
    const int texId = kAtlasIdByStyle[base];

    const int len = static_cast<int>(std::min<size_t>(std::strlen(buf), 20));
    int x = 0;
    int out = 0;

    for (int i = 0; i < len; ++i) {
        unsigned char ch = static_cast<unsigned char>(buf[i]);
        // 反編譯：LOWORD(v7) = word_6C6B6C[2 * (13*style + ch)]
        const int mapIdx = base + static_cast<int>(ch);
        const int block = static_cast<unsigned short>(kGlyphIndexByStyleChar[2 * mapIdx]);

        PlaceGlyph(out, texId, block, x);

        // 寬度累計（以本張圖原始寬 * scale + advance）
        const int w = m_Digits[out].GetWidth();
        x += advance + static_cast<int>(w * m_Scale);
        m_TotalW = static_cast<uint16_t>(x);

        ++out;
        if (out >= 20) break;
    }

    // 設定高度（反編譯：height = firstDigitHeight * scale）
    if (out > 0) {
        const int h0 = m_Digits[0].GetHeight();
        m_TotalH = static_cast<uint16_t>(h0 * m_Scale);
    }
    else {
        m_TotalH = 0;
    }

    // 清餘下未使用格（反編譯：(-1, 0xFFFF)）
    for (int i = out; i < 20; ++i) {
        m_Digits[i].SetImage(static_cast<unsigned int>(-1), 0xFFFF);
    }
}

void CControlNumberImage::SetNumberFloat(float value, int decimals, int forceFixed)
{
    // 反編譯使用 `_sprintf(Buffer, `string', a2)`；此處採常見格式 "%f"
    // 後續會裁切到所需小數位
    char buf[256]{};
    std::snprintf(buf, sizeof(buf), "%f", value);

    // 找到 '.' 位置
    const size_t n = std::strlen(buf);
    int dotIdx = -1;
    for (size_t i = 0; i < n; ++i) {
        if (buf[i] == '.') { dotIdx = static_cast<int>(i); break; }
    }

    int cutLen = static_cast<int>(n); // 預設全列
    if (dotIdx >= 0) {
        if (forceFixed) {
            // 強制固定 decimals 位
            cutLen = min(dotIdx + 1 + max(decimals, 0), static_cast<int>(n));
        }
        else {
            // 若小數點後前 decimals 位皆為 '0'，省略小數
            bool allZero = true;
            for (int k = 0; k < decimals && dotIdx + 1 + k < static_cast<int>(n); ++k) {
                if (buf[dotIdx + 1 + k] != '0') { allZero = false; break; }
            }
            if (allZero) {
                cutLen = dotIdx; // 直接砍到小數點前（不包含 '.')
            }
            else {
                cutLen = min(dotIdx + 1 + max(decimals, 0), static_cast<int>(n));
            }
        }
    }
    cutLen = max(0, min(cutLen, 20)); // 最多 20

    m_TotalW = 0;
    m_TotalH = 0;

    const int base = 13 * m_StyleIndex;
    const int advance = kAdvanceByStyle[base];
    const int texId = kAtlasIdByStyle[base];
    const int dotBlk = kDotGlyphIndexByStyle[base]; // -1 代表沒有小數點圖

    int x = 0;
    int out = 0;

    for (int i = 0; i < cutLen; ++i) {
        unsigned char ch = static_cast<unsigned char>(buf[i]);

        if (dotIdx >= 0 && i == dotIdx) {
            // 小數點
            if (dotBlk != -1) {
                PlaceGlyph(out, texId, dotBlk, x);
                const int w = m_Digits[out].GetWidth();
                x += advance + static_cast<int>(w * m_Scale);
                m_TotalW = static_cast<uint16_t>(x);
                ++out;
                if (out >= 20) break;
            }
            continue; // 若無 dot 圖塊，跳過
        }

        // 其他字元走一般映射
        const int mapIdx = base + static_cast<int>(ch);
        const int block = static_cast<unsigned short>(kGlyphIndexByStyleChar[2 * mapIdx]);

        PlaceGlyph(out, texId, block, x);

        const int w = m_Digits[out].GetWidth();
        x += advance + static_cast<int>(w * m_Scale);
        m_TotalW = static_cast<uint16_t>(x);

        ++out;
        if (out >= 20) break;
    }

    // 反編譯：高度 = 第一張 digit 的 GetHeight()（注意：**未乘上 scale**）
    if (out > 0) {
        m_TotalH = static_cast<uint16_t>(m_Digits[0].GetHeight());
    }
    else {
        m_TotalH = 0;
    }

    // 清餘下未使用格
    for (int i = out; i < 20; ++i) {
        m_Digits[i].SetImage(static_cast<unsigned int>(-1), 0xFFFF);
    }
}
