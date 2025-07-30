#include "Effect/CEffect_MiniGame_Class_Sword.h"
#include "Effect/CEAManager.h"
#include "global.h" 

// 對應反組譯碼: 0x00537F80
CEffect_MiniGame_Class_Sword::CEffect_MiniGame_Class_Sword()
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 113, 檔案名稱: "Effect/efn_MiniGame_Class_Sword.ea"
    CEAManager::GetInstance()->GetEAData(113, "Effect/efn_MiniGame_Class_Sword.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x00538010
CEffect_MiniGame_Class_Sword::~CEffect_MiniGame_Class_Sword()
{
}

// 對應反組譯碼: 0x00538020
void CEffect_MiniGame_Class_Sword::SetEffect(float x, float y)
{
    // 設定特效的初始世界座標
    // 原始碼: *((float *)this + 2) = a2; *((float *)this + 3) = a3;
    m_fCurrentPosX = x;
    m_fCurrentPosY = y;
}

// 對應反組譯碼: 0x00538040
bool CEffect_MiniGame_Class_Sword::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "efn_MiniGame_Class_Sword.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00538050
void CEffect_MiniGame_Class_Sword::Process()
{
    // --- 核心邏輯：直接將世界座標作為螢幕座標 ---
    // 這與其他特效不同，它不減去攝影機座標 (g_Game_System_Info.ScreenX/Y)
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

// 對應反組譯碼: 0x00538070
void CEffect_MiniGame_Class_Sword::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}