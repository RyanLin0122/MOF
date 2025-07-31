#include "Effect/CEffect_Field_DamageNumber.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>
#include <cmath>

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x00534CA0
CEffect_Field_DamageNumber::CEffect_Field_DamageNumber()
{
    m_pNumberImages = nullptr;
    m_cDigitCount = 0;
    m_ucState = 0;
    m_fAlpha = 240.0f;        // 1132396544 (0x43700000)
    m_fScale = 80.0f;         // 1117782016 (0x42A00000)
    memset(m_ucDigitFrames, 0, sizeof(m_ucDigitFrames));
    m_fInitialPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fTotalWidth = 0.0f;

    // 1015580809 -> 1.0f / 60.0f
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
}

// 對應反組譯碼: 0x00534D40
CEffect_Field_DamageNumber::~CEffect_Field_DamageNumber()
{
    delete[] m_pNumberImages;
}

// 對應反組譯碼: 0x00534D80
void CEffect_Field_DamageNumber::SetEffect(int damageValue, float x, float y, int type)
{
    char first_digit_frame, plus_frame, minus_frame;

    if (type == 0) { // 怪物對玩家 (紅色數字)
        first_digit_frame = 48;
        minus_frame = 59;
        plus_frame = 58;
    }
    else { // 玩家對怪物 (白色數字)
        first_digit_frame = 0;
        minus_frame = 11;
        plus_frame = 10;
    }

    int absValue = abs(damageValue);

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
    m_ucDigitFrames[m_cDigitCount++] = (damageValue >= 0) ? plus_frame : minus_frame;

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
            m_fTotalWidth += static_cast<float>(rect.right - rect.left - 5); // 原始碼有 -5 的間距調整
        }
        cltImageManager::GetInstance()->ReleaseGameImage(pTempImage);
    }
    m_fTotalWidth *= 0.5f; // 取半寬
}

// 對應反組譯碼: 0x00534F00
bool CEffect_Field_DamageNumber::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }
    float fFrameCount = static_cast<float>(frameCount);

    switch (m_ucState)
    {
    case 0: // 階段 0: 放大
        m_fScale += fFrameCount * 45.0f;
        if (m_fScale >= 210.0f) {
            m_fScale = 210.0f;
            m_ucState = 1;
        }
        m_fCurrentPosY -= fFrameCount * 2.0f;
        break;

    case 1: // 階段 1: 縮小
        m_fScale -= fFrameCount * 10.0f;
        if (m_fScale <= 80.0f) {
            m_fScale = 80.0f;
            m_ucState = 2;
        }
        break;

    case 2: // 階段 2: 淡出上漂
        m_fAlpha -= fFrameCount * 5.0f;
        m_fCurrentPosY -= fFrameCount * 1.5f;
        break;
    }

    return m_fAlpha < 0.0f;
}

// 對應反組譯碼: 0x00535040
void CEffect_Field_DamageNumber::Process()
{
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);
        float currentX = screenX - m_fTotalWidth;

        // 從後往前處理數字 (從個位數開始)
        for (int i = 0; i < m_cDigitCount; ++i) {
            m_pNumberImages[i] = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
            if (m_pNumberImages[i]) {
                unsigned char frameID = m_ucDigitFrames[i];
                m_pNumberImages[i]->SetBlockID(frameID);
                m_pNumberImages[i]->SetPosition(currentX, screenY);
                m_pNumberImages[i]->SetAlpha(static_cast<unsigned int>(m_fAlpha));
                m_pNumberImages[i]->SetScale(static_cast<int>(m_fScale));

                RECT rect;
                m_pNumberImages[i]->GetBlockRect(&rect);
                // 根據每個數字的寬度來調整下一個數字的位置
                currentX += static_cast<float>(rect.right - rect.left - 5) * (m_fScale / 100.f);
            }
        }
    }
}

// 對應反組譯碼: 0x005351A0
void CEffect_Field_DamageNumber::Draw()
{
    if (!m_bIsVisible || !m_pNumberImages) return;

    CDeviceManager::GetInstance()->ResetRenderState();

    // 從後往前繪製 (先畫的在下層)
    for (int i = m_cDigitCount - 1; i >= 0; --i) {
        if (m_pNumberImages[i] && m_pNumberImages[i]->IsInUse()) {
            m_pNumberImages[i]->Draw();
        }
    }
}