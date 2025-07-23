#include "Effect/CEffect_Battle_GunSpark.h"
#include "Effect/CEAManager.h"
#include "global.h" // 假設用於獲取全域變數 g_Game_System_Info

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: 0x0052E720
CEffect_Battle_GunSpark::CEffect_Battle_GunSpark()
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 5, 檔案名稱: "MoFData/Effect/efn_GunSpark.ea"
    CEAManager::GetInstance()->GetEAData(5, "MoFData/Effect/efn_GunSpark.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x0052E7B0
CEffect_Battle_GunSpark::~CEffect_Battle_GunSpark()
{
}

// 對應反組譯碼: 0x0052E7C0
void CEffect_Battle_GunSpark::SetEffect(float x, float y, bool bFlip)
{
    // 設定特效的初始世界座標
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;

    // 設定特效是否需要翻轉
    m_bIsFlip = bFlip;
}

// 對應反組譯碼: 0x0052E7E0
bool CEffect_Battle_GunSpark::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "efn_GunSpark.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x0052E7F0
void CEffect_Battle_GunSpark::Process()
{
    // 將世界座標轉換為螢幕座標
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    float screenY = m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x0052E830
void CEffect_Battle_GunSpark::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}