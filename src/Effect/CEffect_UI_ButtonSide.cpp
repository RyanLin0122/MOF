#include "Effect/CEffect_UI_ButtonSide.h"
#include "Effect/CEAManager.h"
#include "global.h"

// 對應反組譯碼: 0x005328B0
CEffect_UI_ButtonSide::CEffect_UI_ButtonSide()
    : m_bIsFinished(false)
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 70, 檔案名稱: "Effect/efn_BtnSide.ea"
    CEAManager::GetInstance()->GetEAData(70, "Effect/efn_BtnSide.ea", &m_ccaEffect);

    // 設定影格時間並循環播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, true); // Play(..., 0, 1) -> 循環播放
}

// 對應反組譯碼: 0x00532950
CEffect_UI_ButtonSide::~CEffect_UI_ButtonSide()
{
}

// 對應反組譯碼: 0x00532960
void CEffect_UI_ButtonSide::SetEffect(float x, float y)
{
    // --- 核心邏輯：直接設定 CCAEffect 的螢幕座標 ---
    // 原始碼: *((float *)this + 12) = a2;
    //         *((float *)this + 13) = a3;
    m_ccaEffect.SetPosition(x, y);
}

// 對應反組譯碼: 0x00532980
bool CEffect_UI_ButtonSide::FrameProcess(float fElapsedTime)
{
    // 更新內部動畫的影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 返回生命週期旗標，由外部邏輯決定何時結束
    return m_bIsFinished;
}

// 對應反組譯碼: 0x005329A0
void CEffect_UI_ButtonSide::Process()
{
    // 直接將處理任務委託給內部的 CCAEffect
    // 因為座標已在 SetEffect 中設定，無需再次更新
    m_ccaEffect.Process();

    // UI 特效總是可見
    m_bIsVisible = TRUE;
}

// 對應反組譯碼: 0x005329B0
void CEffect_UI_ButtonSide::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}