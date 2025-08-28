#include "Effect/CEffect_UI_HP_MP_Number.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>
#include <cmath>

// 對應反組譯碼: 0x005329C0
CEffect_UI_HP_MP_Number::CEffect_UI_HP_MP_Number()
{
    m_pNumberImages = nullptr;
    m_cDigitCount = 0;
    m_ucState = 0;
    m_fAlpha = 255.0f;        // 1132396544 (0x437F0000)
    m_fScale = 80.0f;         // 1117782016 (0x42A00000)
    memset(m_ucDigitFrames, 0, sizeof(m_ucDigitFrames));
    m_fInitialPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fTotalWidth = 0.0f;

    // 1015580809 -> 1.0f / 60.0f
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
}

// 對應反組譯碼: 0x00532A60
CEffect_UI_HP_MP_Number::~CEffect_UI_HP_MP_Number()
{
    delete[] m_pNumberImages;
}

// 對應反組譯碼: 0x00532AA0
void CEffect_UI_HP_MP_Number::SetEffect(int value, float x, float y, int type)
{
    char first_digit_frame, plus_frame, minus_frame;

    if (type == 1) { // MP (藍色數字)
        first_digit_frame = 48;
        plus_frame = 58;
        minus_frame = 59;
    }
    else { // 預設為 HP (綠色數字)
        first_digit_frame = 36;
        plus_frame = 46;
        minus_frame = 47;
    }

    int absValue = abs(value);

    if (absValue == 0) {
        m_ucDigitFrames[m_cDigitCount++] = first_digit_frame;
    }
    else {
        int tempValue = absValue;
        while (tempValue > 0) {
            m_ucDigitFrames[m_cDigitCount++] = (tempValue % 10) + first_digit_frame;
            tempValue /= 10;
        }
    }
    m_ucDigitFrames[m_cDigitCount++] = (value >= 0) ? plus_frame : minus_frame;

    m_pNumberImages = new (std::nothrow) GameImage * [m_cDigitCount];

    m_fInitialPosX = x;
    m_fCurrentPosY = y;

    // 預先計算總寬度以進行居中
    GameImage* pTempImage = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
    if (pTempImage) {
        for (int i = 0; i < m_cDigitCount; ++i) {
            RECT rect;
            pTempImage->SetBlockID(m_ucDigitFrames[i]);
            pTempImage->GetBlockRect(&rect);
            m_fTotalWidth += static_cast<float>(rect.right - rect.left);
        }
        cltImageManager::GetInstance()->ReleaseGameImage(pTempImage);
    }
    m_fTotalWidth *= 0.5f; // 取半寬
}

// 對應反組譯碼: 0x00532C20
bool CEffect_UI_HP_MP_Number::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    if (m_ucState == 0) { // 階段 0: 淡入與停留
        m_fAlpha -= fFrameCount * 4.0f;
        if (m_fAlpha < 220.0f) {
            m_fAlpha = 220.0f;
            m_ucState = 1;
        }
        m_fCurrentPosY -= fFrameCount * 0.8f;
    }
    else if (m_ucState == 1) { // 階段 1: 淡出與上漂
        m_fAlpha -= fFrameCount * 5.0f;
        m_fCurrentPosY -= fFrameCount * 1.5f;
    }

    return m_fAlpha < 0.0f;
}

// 對應反組譯碼: 0x00532D30
void CEffect_UI_HP_MP_Number::Process()
{
    // 此特效為 UI 層級，不考慮攝影機座標
    m_bIsVisible = TRUE;

    float currentX = m_fInitialPosX - m_fTotalWidth;

    // 從後往前處理數字 (從個位數開始)
    for (int i = 0; i < m_cDigitCount; ++i) {
        m_pNumberImages[i] = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
        if (m_pNumberImages[i]) {
            unsigned char frameID = m_ucDigitFrames[i];
            m_pNumberImages[i]->SetBlockID(frameID);
            m_pNumberImages[i]->SetPosition(currentX, m_fCurrentPosY);

            float clampedAlpha = (m_fAlpha > 254.0f) ? 254.0f : m_fAlpha;
            m_pNumberImages[i]->SetAlpha(static_cast<unsigned int>(clampedAlpha));

            RECT rect;
            m_pNumberImages[i]->GetBlockRect(&rect);
            currentX += static_cast<float>(rect.right - rect.left - 6); // 原始碼有 -6 的間距調整
        }
    }
}

// 對應反組譯碼: 0x00532E60
void CEffect_UI_HP_MP_Number::Draw()
{
    if (!m_pNumberImages) return;

    CDeviceManager::GetInstance()->ResetRenderState();

    // 從後往前繪製 (先畫的在下層，即個位數)
    for (int i = 0; i < m_cDigitCount; ++i) {
        if (m_pNumberImages[i] && m_pNumberImages[i]->IsInUse()) {
            m_pNumberImages[i]->Draw();
        }
    }
}