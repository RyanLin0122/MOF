#include "Effect/CEffect_Skill_Freezing.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// =======================================================================
// CEffect_Skill_Freezing (上層特效)
// =======================================================================

// 對應反組譯碼: 0x00533240
CEffect_Skill_Freezing::CEffect_Skill_Freezing()
    : m_pOwnerCharacter(nullptr),
    m_nUnknown_a3(0),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    CEAManager::GetInstance()->GetEAData(97, "Effect/Efn-Pvp-Ice-Top.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 動畫本身不循環，由 FrameProcess 控制其持續性
}

// 對應反組譯碼: 0x005332E0
CEffect_Skill_Freezing::~CEffect_Skill_Freezing()
{
}

// 對應反組譯碼: 0x005332F0
void CEffect_Skill_Freezing::SetEffect(ClientCharacter* pTarget, int a3)
{
    if (!pTarget) return;

    m_pOwnerCharacter = pTarget;
    m_nUnknown_a3 = a3;
    m_fInitialPosX = static_cast<float>(pTarget->GetPosX());
    m_fInitialPosY = static_cast<float>(pTarget->GetPosY());
}

// 對應反組譯碼: 0x00533320
bool CEffect_Skill_Freezing::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- 核心生命週期判斷 ---
    // 原始碼: return (~*(_BYTE *)(*((_DWORD *)this + 33) + 9700) & 4) != 0;
    // 檢查角色的特殊狀態旗標中，代表「冰凍」的位元是否被清除
    if (m_pOwnerCharacter->IsFreezing()) { // 假設 IsFreezing() 實現了該位元檢查
        return false; // 如果仍在冰凍，特效繼續
    }

    return true; // 否則，特效結束
}

// 對應反組譯碼: 0x00533350
void CEffect_Skill_Freezing::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x005333B0
void CEffect_Skill_Freezing::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}


// =======================================================================
// CEffect_Skill_Freezing_Sub (底層特效)
// =======================================================================

// 對應反組譯碼: 0x005333D0
CEffect_Skill_Freezing_Sub::CEffect_Skill_Freezing_Sub()
    : m_pOwnerCharacter(nullptr),
    m_nUnknown_a3(0),
    m_fInitialPosX(0.0f),
    m_fInitialPosY(0.0f)
{
    CEAManager::GetInstance()->GetEAData(98, "Effect/Efn-Pvp-Ice-Bottom.ea", &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false);
}

// 對應反組譯碼: 0x00533470
CEffect_Skill_Freezing_Sub::~CEffect_Skill_Freezing_Sub()
{
}

// 對應反組譯碼: 0x00533480
void CEffect_Skill_Freezing_Sub::SetEffect(ClientCharacter* pTarget, int a3)
{
    if (!pTarget) return;

    m_pOwnerCharacter = pTarget;
    m_nUnknown_a3 = a3;
    m_fInitialPosX = static_cast<float>(pTarget->GetPosX());
    m_fInitialPosY = static_cast<float>(pTarget->GetPosY());
}

// 對應反組譯碼: 0x005334B0
bool CEffect_Skill_Freezing_Sub::FrameProcess(float fElapsedTime)
{
    m_ccaEffect.FrameProcess(fElapsedTime);

    if (!m_pOwnerCharacter) {
        return true;
    }

    // --- 核心生命週期判斷 (與上層特效相同) ---
    if (m_pOwnerCharacter->IsFreezing()) {
        return false;
    }

    return true;
}

// 對應反組譯碼: 0x005334E0
void CEffect_Skill_Freezing_Sub::Process()
{
    if (!m_pOwnerCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    float screenX = m_fInitialPosX - static_cast<float>(g_Game_System_Info.ScreenX);
    float screenY = m_fInitialPosY - static_cast<float>(g_Game_System_Info.ScreenY);

    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x00533540
void CEffect_Skill_Freezing_Sub::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}