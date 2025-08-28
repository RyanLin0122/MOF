#include "Effect/CEffect_MiniGame_Priest_Heal.h"
#include "Effect/CEAManager.h"
#include "global.h"

// 對應反組譯碼: 0x00538D60
CEffect_MiniGame_Priest_Heal::CEffect_MiniGame_Priest_Heal()
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 120, 檔案名稱: "Effect/Minigame_Priest_Heal.ea"
    CEAManager::GetInstance()->GetEAData(120, "Effect/Minigame_Priest_Heal.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x00538DF0
CEffect_MiniGame_Priest_Heal::~CEffect_MiniGame_Priest_Heal()
{
}

// 對應反組譯碼: 0x00538E00
void CEffect_MiniGame_Priest_Heal::SetEffect(float x, float y)
{
    // 設定特效的初始世界座標
    // 原始碼: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// 對應反組譯碼: 0x00538E20
bool CEffect_MiniGame_Priest_Heal::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "Minigame_Priest_Heal.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00538E30
void CEffect_MiniGame_Priest_Heal::Process()
{
    // --- 核心邏輯：直接將世界座標作為螢幕座標 ---
    // 原始碼: *((float *)this + 12) = *((float *)this + 2);
    //         *((float *)this + 13) = *((float *)this + 3);
    float screenX = m_fCurrentPosX;
    float screenY = m_fCurrentPosY;

    // 更新內部 CCAEffect 的狀態
    m_ccaEffect.SetPosition(screenX, screenY);
    m_ccaEffect.Process();

    // 因為是 UI 特效，總是在可視範圍內，不需要裁剪
    m_bIsVisible = TRUE;
}

// 對應反組譯碼: 0x00538E50
void CEffect_MiniGame_Priest_Heal::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}