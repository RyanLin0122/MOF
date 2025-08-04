#include "Effect/CEffect_UI_SkillSet.h"
#include "Effect/CEAManager.h"
#include "global.h"

// 對應反組譯碼: 0x00532ED0
CEffect_UI_SkillSet::CEffect_UI_SkillSet()
{
    // CEffectBase 的建構函式會被自動呼叫

    // 向 CEAManager 請求特效數據
    // 特效 ID: 68, 檔案名稱: "Effect/efi_skillset.ea"
    CEAManager::GetInstance()->GetEAData(68, "Effect/efi_skillset.ea", &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環
}

// 對應反組譯碼: 0x00532F60
CEffect_UI_SkillSet::~CEffect_UI_SkillSet()
{
}

// 對應反組譯碼: 0x00532F70
void CEffect_UI_SkillSet::SetEffect(float x, float y)
{
    // --- 核心邏輯：直接設定 CCAEffect 的螢幕座標 ---
    // 原始碼: *((float *)this + 12) = a2;
    //         *((float *)this + 13) = a3;
    m_ccaEffect.SetPosition(x, y);
}

// 對應反組譯碼: 0x00532F90
bool CEffect_UI_SkillSet::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    // 當 "efi_skillset.ea" 動畫播放完畢時，此函式會回傳 true。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00532FA0
void CEffect_UI_SkillSet::Process()
{
    // 直接將處理任務委託給內部的 CCAEffect
    // 因為座標已在 SetEffect 中設定，無需再次更新
    m_ccaEffect.Process();

    // UI 特效總是可見
    m_bIsVisible = TRUE;
}

// 對應反組譯碼: 0x00532FB0
void CEffect_UI_SkillSet::Draw()
{
    // 直接將繪製任務委託給內部的 CCAEffect 物件
    m_ccaEffect.Draw();
}