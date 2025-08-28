#include "Effect/CEffect_Battle_Pierce.h"
#include "Effect/CEAManager.h"
#include "global.h" // 假設用於獲取全域變數 g_Game_System_Info

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x0052F300
CEffect_Battle_Pierce::CEffect_Battle_Pierce()
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 2, 檔案名稱: "Effect/efn_pierce.ea"
    CEAManager::GetInstance()->GetEAData(2, "Effect/efn_pierce.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x0052F390
CEffect_Battle_Pierce::~CEffect_Battle_Pierce()
{
}

// 對應反組譯碼: 0x0052F3A0
void CEffect_Battle_Pierce::SetEffect(float x, float y, bool bFlip, unsigned char a5)
{
    // 設定特效的初始世界座標
    // 原始碼: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;

    // 設定特效是否需要翻轉
    // 原始碼: *((_BYTE *)this + 56) = a4;
    m_bIsFlip = bFlip;
}

// 對應反組譯碼: 0x0052F3C0
bool CEffect_Battle_Pierce::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "efn_pierce.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x0052F3D0
void CEffect_Battle_Pierce::Process()
{
    // 將世界座標轉換為螢幕座標
    // 原始碼: v3 = *((float *)this + 2) - (double)dword_A73088;
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    // 進行裁剪判斷
    // 原始碼: v2 = CEffectBase::IsCliping(this, v3, 0.0); *((_DWORD *)this + 8) = v2;
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        // 原始碼: *((float *)this + 12) = ...; *((float *)this + 13) = ...;
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x0052F410
void CEffect_Battle_Pierce::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}