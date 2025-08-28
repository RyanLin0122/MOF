#include "Effect/CEffect_Player_WeatherPreView.h"
#include "Effect/CEAManager.h"
#include "global.h"

// 對應反組譯碼: 0x00532790
CEffect_Player_WeatherPreView::CEffect_Player_WeatherPreView()
    : m_fScreenPosX(0.0f), m_fScreenPosY(0.0f)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: 0x005327D0
CEffect_Player_WeatherPreView::~CEffect_Player_WeatherPreView()
{
}

// 對應反組譯碼: 0x005327E0
void CEffect_Player_WeatherPreView::SetEffect(float x, float y, int baseEffectID, char* szFileName)
{
    // 構建檔案路徑
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", szFileName);

    // 根據基礎 ID 計算最終的 Effect Kind ID
    int finalEffectID = baseEffectID + 53;

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(finalEffectID, szFullPath, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 儲存固定的螢幕座標
    m_fScreenPosX = x;
    m_fScreenPosY = y;
}

// 對應反組譯碼: 0x00532860
bool CEffect_Player_WeatherPreView::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // --- 核心生命週期邏輯 ---
    // 原始碼: return 0;
    // 永遠返回 false，表示此特效不會自動結束。
    return false;
}

// 對應反組譯碼: 0x00532880
void CEffect_Player_WeatherPreView::Process()
{
    // --- 核心邏輯：直接將儲存的座標作為螢幕座標 ---
    // 原始碼: *((float *)this + 12) = *((float *)this + 33);
    //         *((float *)this + 13) = *((float *)this + 34);

    // 更新內部 CCAEffect 的狀態，不進行攝影機座標轉換
    m_ccaEffect.SetPosition(m_fScreenPosX, m_fScreenPosY);
    m_ccaEffect.Process();

    // UI 特效總是可見
    m_bIsVisible = TRUE;
}

// 對應反組譯碼: 0x005328A0
void CEffect_Player_WeatherPreView::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}