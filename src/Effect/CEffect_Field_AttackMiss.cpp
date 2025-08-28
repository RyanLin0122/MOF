#include "Effect/CEffect_Field_AttackMiss.h"
#include "Image/GameImage.h"
#include "Image/cltImageManager.h"
#include "Image/CDeviceManager.h"
#include "global.h"

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x005341C0
CEffect_Field_AttackMiss::CEffect_Field_AttackMiss()
{
    // CEffectBase 建構函式已自動呼叫
    m_pMissImage = nullptr;
    m_ucState = 0;
    m_fAlpha = 240.0f;      // 1132396544 (0x43700000)
    m_fColorValue = 100.0f; // 1120403456 (0x42C80000)
    m_fInitialPosX = 0.0f;
    m_fCurrentPosY = 0.0f;

    // 原始碼: *((_DWORD *)this + 42) = 1015580809; (0x3C888889 -> ~1/60)
    m_FrameSkip.m_fTimePerFrame = 1.0f / 60.0f;
}

// 對應反組譯碼: 0x00534250
CEffect_Field_AttackMiss::~CEffect_Field_AttackMiss()
{
}

// 對應反組譯碼: 0x00534270
void CEffect_Field_AttackMiss::SetEffect(float x, float y)
{
    // 設定特效的初始世界座標
    m_fInitialPosX = x;
    m_fCurrentPosY = y;
}

// 對應反組譯碼: 0x00534290
bool CEffect_Field_AttackMiss::FrameProcess(float fElapsedTime)
{
    int frameCount = 0;
    if (!m_FrameSkip.Update(fElapsedTime, frameCount)) {
        return false;
    }

    if (m_ucState == 0) {
        // --- 階段 0: 淡入與停留 ---
        m_fAlpha -= static_cast<float>(frameCount * 2);
        if (m_fAlpha < 220.0f) {
            m_fAlpha = 220.0f;
            m_ucState = 1; // 切換到下一階段
        }
    }
    else if (m_ucState == 1) {
        // --- 階段 1: 淡出與上漂 ---
        m_fAlpha -= static_cast<float>(frameCount * 5);
        m_fCurrentPosY -= static_cast<float>(frameCount); // Y 座標減少，產生上漂效果
    }

    // 當透明度降至 0 以下時，特效生命週期結束
    return m_fAlpha < 0.0f;
}

// 對應反組譯碼: 0x00534380
void CEffect_Field_AttackMiss::Process()
{
    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 從物件池獲取 "MISS" 圖片
        // 資源 ID: 0xB0005A4u (共享數字/文字圖檔), 影格 ID: 62
        m_pMissImage = cltImageManager::GetInstance()->GetGameImage(7, 0xB0005A4u, 0, 1);

        if (m_pMissImage) {
            float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

            m_pMissImage->SetPosition(screenX, screenY);
            m_pMissImage->SetBlockID(62); // "MISS" 字樣在圖檔中的索引

            // 將計算出的 Alpha 和 Color 應用到 GameImage
            float clampedAlpha = (m_fAlpha > 255.0f) ? 255.0f : m_fAlpha;
            m_pMissImage->SetAlpha(static_cast<unsigned int>(clampedAlpha));
            m_pMissImage->SetColor(static_cast<unsigned int>(m_fColorValue));

            // 更新頂點緩衝區
            m_pMissImage->Process();
        }
    }
}

// 對應反組譯碼: 0x00534460
void CEffect_Field_AttackMiss::Draw()
{
    if (m_bIsVisible && m_pMissImage && m_pMissImage->IsInUse())
    {
        // 設定標準的 Alpha 混合模式
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        CDeviceManager::GetInstance()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        m_pMissImage->Draw();
    }
}