#include "Effect/CEffect_Skill_Heal.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00533710
CEffect_Skill_Heal::CEffect_Skill_Heal()
    : m_pOwnerCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
    // 特效資源是在 SetEffect 中動態載入的
}

// 對應反組譯碼: 0x00533750
CEffect_Skill_Heal::~CEffect_Skill_Heal()
{
}

// 對應反組譯碼: 0x00533760
void CEffect_Skill_Heal::SetEffect(ClientCharacter* pTarget, unsigned char skillLevel)
{
    if (!pTarget) return;

    const char* szFileName = nullptr;
    int effectID = 0;

    // 根據技能等級選擇不同的特效資源
    if (skillLevel == 1) {
        effectID = 75;
        szFileName = "Effect/efn_healLv2.ea";
    }
    else if (skillLevel == 2) {
        effectID = 76;
        szFileName = "Effect/efn_healLv3.ea";
    }
    else { // 預設為等級 0
        effectID = 74;
        szFileName = "Effect/efn_heal.ea";
    }

    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectID, (char*)szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 綁定擁有者
    m_pOwnerCharacter = pTarget;
}

// 對應反組譯碼: 0x005337D0
bool CEffect_Skill_Heal::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x005337E0
void CEffect_Skill_Heal::Process()
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

// 對應反組譯碼: 0x00533860
void CEffect_Skill_Heal::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}