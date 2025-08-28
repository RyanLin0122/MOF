#include "Effect/CEffect_Skill_HealPray.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00533880
CEffect_Skill_HealPray::CEffect_Skill_HealPray()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
    // 特效資源是在 SetEffect 中動態載入的
}

// 對應反組譯碼: 0x005338C0
CEffect_Skill_HealPray::~CEffect_Skill_HealPray()
{
}

// 對應反組譯碼: 0x005338D0
void CEffect_Skill_HealPray::SetEffect(ClientCharacter* pCaster, unsigned char skillLevel)
{
    if (!pCaster) return;

    const char* szFileName = nullptr;
    int effectID = 100; // 基礎 ID

    // 根據技能等級選擇不同的特效資源
    if (skillLevel == 1) {
        szFileName = "Effect/efn_HealPray-TopLv2.ea";
    }
    else if (skillLevel == 2) {
        szFileName = "Effect/efn_HealPray-TopLv3.ea";
    }
    else { // 預設為等級 0
        szFileName = "Effect/efn_HealPray.ea";
    }

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 綁定擁有者
    m_pOwnerCharacter = pCaster;
}

// 對應反組譯碼: 0x00533940
bool CEffect_Skill_HealPray::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00533950
void CEffect_Skill_HealPray::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 核心邏輯：持續追蹤擁有者位置 ---
    // 將世界座標轉換為螢幕座標
    float screenX = static_cast<float>(m_pOwnerCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pOwnerCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x005339E0
void CEffect_Skill_HealPray::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}