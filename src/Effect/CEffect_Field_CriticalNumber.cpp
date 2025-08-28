#include "Effect/CEffect_Field_CriticalNumber.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"
#include <cstdlib>
#include <cmath>

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x005344C0
CEffect_Field_CriticalNumber::CEffect_Field_CriticalNumber()
{
    m_pNumberImages = nullptr;
    m_pFlashImage = nullptr;
    m_pTextImage = nullptr;

    m_fScale = 120.0f;        // 1128792064
    m_fAlpha = 240.0f;        // 1132068864
    m_cDigitCount = 0;
    m_ucState = 0;

    m_fInitialPosX = 0.0f;
    m_fCurrentPosY = 0.0f;
    m_fTotalWidth = 0.0f;

    m_usTextImageFrame = 0;
    memset(m_ucDigitFrames, 0, sizeof(m_ucDigitFrames));

    // 1015580809 -> 1.0f / 60.0f
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
}

// 對應反組譯碼: 0x00534580
CEffect_Field_CriticalNumber::~CEffect_Field_CriticalNumber()
{
    delete[] m_pNumberImages;
}

// 對應反組譯碼: 0x005345C0
void CEffect_Field_CriticalNumber::SetEffect(int damageValue, float x, float y, int type)
{
    char first_digit_frame, plus_frame, minus_frame;

    // 根據類型決定使用的數字圖樣
    if (type == 0) { // 玩家對怪物 (黃色數字)
        first_digit_frame = 24;
        minus_frame = 35;
        plus_frame = 34;
        m_usTextImageFrame = 60; // "CRITICAL!"
    }
    else { // 怪物對玩家 (紅色數字)
        first_digit_frame = 64;
        minus_frame = 75;
        plus_frame = 74;
        m_usTextImageFrame = 61; // "CRITICAL!"
    }

    int absValue = abs(damageValue);

    // --- 將數字拆解為影格 ID ---
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

    // 加入正負號
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
            m_fTotalWidth += static_cast<float>(rect.right - rect.left);
        }
        cltImageManager::GetInstance()->ReleaseGameImage(pTempImage);
    }
    m_fTotalWidth *= 0.5f; // 取半寬
}

// 對應反組譯碼: 0x00534750
bool CEffect_Field_CriticalNumber::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }

    float fFrameCount = static_cast<float>(frameCount);

    if (m_ucState == 0) { // 階段 0: 放大
        m_fScale += fFrameCount * 40.0f;
        if (m_fScale >= 300.0f) {
            m_fScale = 300.0f;
            m_ucState = 1;
        }
    }
    else if (m_ucState == 1) { // 階段 1: 縮小
        m_fScale -= fFrameCount * 10.0f;
        if (m_fScale <= 80.0f) {
            m_fScale = 80.0f;
            m_ucState = 2;
        }
    }
    else if (m_ucState == 2) { // 階段 2: 淡出上漂
        m_fAlpha -= fFrameCount * 5.0f;
        m_fCurrentPosY -= fFrameCount * 1.5f;
    }

    return m_fAlpha < 0.0f;
}

// 對應反組譯碼: 0x00534940
void CEffect_Field_CriticalNumber::Process()
{
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);
        float currentX = screenX - m_fTotalWidth;

        // 處理數字
        for (int i = 0; i < m_cDigitCount; ++i) {
            m_pNumberImages[i] = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
            if (m_pNumberImages[i]) {
                m_pNumberImages[i]->SetBlockID(m_ucDigitFrames[i]);
                m_pNumberImages[i]->SetPosition(currentX, screenY);
                m_pNumberImages[i]->SetAlpha(static_cast<unsigned int>(m_fAlpha));
                m_pNumberImages[i]->SetScale(static_cast<int>(m_fScale));

                RECT rect;
                m_pNumberImages[i]->GetBlockRect(&rect);
                currentX += static_cast<float>(rect.right - rect.left) * (m_fScale / 100.f);
            }
        }

        // 處理背景閃光
        m_pFlashImage = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
        if (m_pFlashImage) {
            m_pFlashImage->SetBlockID(63); // 閃光影格
            m_pFlashImage->SetPosition(screenX, screenY - 25.0f);
            m_pFlashImage->SetAlpha(static_cast<unsigned int>(m_fAlpha * 0.5f)); // 半透明
            m_pFlashImage->SetScale(static_cast<int>(m_fScale));
        }

        // 處理前景文字
        m_pTextImage = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);
        if (m_pTextImage) {
            m_pTextImage->SetBlockID(m_usTextImageFrame);
            m_pTextImage->SetPosition(screenX, screenY - 25.0f);
            m_pTextImage->SetAlpha(static_cast<unsigned int>(m_fAlpha));
            m_pTextImage->SetScale(static_cast<int>(m_fScale));
        }
    }
}

// 對應反組譯碼: 0x00534C00
void CEffect_Field_CriticalNumber::Draw()
{
    if (!m_bIsVisible) return;

    CDeviceManager::GetInstance()->ResetRenderState();

    if (m_pFlashImage && m_pFlashImage->IsInUse()) m_pFlashImage->Draw();
    if (m_pTextImage && m_pTextImage->IsInUse()) m_pTextImage->Draw();

    if (m_pNumberImages) {
        // 從後往前畫 (個位數先畫)
        for (int i = 0; i < m_cDigitCount; ++i) {
            if (m_pNumberImages[i] && m_pNumberImages[i]->IsInUse()) {
                m_pNumberImages[i]->Draw();
            }
        }
    }
}