#include "Effect/CEffect_Skill_Type_Sustain.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h" // 假設用於獲取全域變數 g_Game_System_Info

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;

// 對應反組譯碼: (在 CEffectManager::AddEffect 中被 new)
CEffect_Skill_Type_Sustain::CEffect_Skill_Type_Sustain()
    : m_pOwnerCharacter(nullptr), m_sSustainSkillID(0)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: (在 CEffectManager::FrameProcess 中被 delete)
CEffect_Skill_Type_Sustain::~CEffect_Skill_Type_Sustain()
{
}

// 對應反組譯碼: (在 CEffectManager::AddEffect 中被呼叫)
void CEffect_Skill_Type_Sustain::SetEffect(ClientCharacter* pCaster, unsigned short effectKindID, char* szFileName, unsigned short sustainSkillID)
{
    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    // 持續性特效通常是循環播放的
    m_ccaEffect.Play(0, true);

    // 綁定擁有者和技能 ID
    m_pOwnerCharacter = pCaster;
    m_sSustainSkillID = sustainSkillID;
}

// 對應反組譯碼: CEffect_Skill_Type_Sustain::FrameProcess
bool CEffect_Skill_Type_Sustain::FrameProcess(float fElapsedTime)
{
    // 首先，讓內部的 CCAEffect 更新其動畫影格
    m_ccaEffect.FrameProcess(fElapsedTime);

    // 檢查擁有者是否存在以及技能狀態是否還在
    if (m_pOwnerCharacter && m_pOwnerCharacter->GetSustainSkillState(m_sSustainSkillID)) {
        // 如果技能狀態還在，特效就繼續存在
        return false;
    }

    // 如果擁有者不存在或技能狀態已消失，則特效結束
    return true;
}

// 對應反組譯碼: (繼承自 CEffect_Skill_Type_Once 的邏輯)
void CEffect_Skill_Type_Sustain::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 檢查角色是否處於不可見或特殊狀態 ---
    // 原始碼: if ( v2[175] || v2[2882] )
    if (m_pOwnerCharacter->IsHide() || m_pOwnerCharacter->IsTransparent()) { // 假設的函式
        m_bIsVisible = FALSE;
        return;
    }

    // 從擁有者獲取當前位置
    float ownerX = static_cast<float>(m_pOwnerCharacter->GetPosX());
    float ownerY = static_cast<float>(m_pOwnerCharacter->GetPosY());

    m_fCurrentPosX = ownerX;
    m_fCurrentPosY = ownerY;

    // 進行裁剪判斷
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 將世界座標轉換為螢幕座標後傳遞給 CCAEffect
        m_ccaEffect.SetPosition(screenX, m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenY));
        m_ccaEffect.SetFlipX(m_pOwnerCharacter->GetActionSide() != 0); // 與施法者方向同步
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: (繼承自 CEffect_Skill_Type_Once 的邏輯)
void CEffect_Skill_Type_Sustain::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}