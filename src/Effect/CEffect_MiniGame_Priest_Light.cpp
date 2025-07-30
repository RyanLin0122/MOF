#include "Effect/CEffect_MiniGame_Priest_Light.h"
#include "Effect/CEAManager.h"
#include "global.h"

// 對應反組譯碼: 0x00538E60
CEffect_MiniGame_Priest_Light::CEffect_MiniGame_Priest_Light()
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 116, 檔案名稱: "Effect/efn_MiniGame_Class_Sword.ea"
    // 注意：此處載入的是劍士的特效資源
    CEAManager::GetInstance()->GetEAData(116, "Effect/efn_MiniGame_Class_Sword.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x00538EF0
CEffect_MiniGame_Priest_Light::~CEffect_MiniGame_Priest_Light()
{
}

// 對應反組譯碼: 0x00538F00
void CEffect_MiniGame_Priest_Light::SetEffect(float x, float y)
{
    // 設定特效的初始世界座標
    // 原始碼: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// 對應反組譯碼: 0x00538F20
bool CEffect_MiniGame_Priest_Light::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00538F30
void CEffect_MiniGame_Priest_Light::Process()
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

// 對應反組譯碼: 0x00538F50
void CEffect_MiniGame_Priest_Light::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}