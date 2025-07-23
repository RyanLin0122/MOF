#include "Effect/CEffect_Skill_Type_Directed.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h" // 假設用於獲取全域變數 g_Game_System_Info

// 假設的全域變數
extern GameSystemInfo g_Game_System_Info;


// 對應反組譯碼: (在 CEffectManager::AddEffect 中被 new)
CEffect_Skill_Type_Directed::CEffect_Skill_Type_Directed()
    : m_pCasterCharacter(nullptr), m_pTargetCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: (在 CEffectManager::FrameProcess 中被 delete)
CEffect_Skill_Type_Directed::~CEffect_Skill_Type_Directed()
{
}

// 對應反組譯碼: (在 CEffectManager::AddEffect 中被呼叫)
void CEffect_Skill_Type_Directed::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName, unsigned char ucDirectionFlag)
{
    // 獲取並設定特效數據
    CEAManager::GetInstance()->GetEAData(effectKindID, szFileName, &m_ccaEffect);
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 綁定施法者和目標
    m_pCasterCharacter = pCaster;
    m_pTargetCharacter = pTarget;

    // --- 翻轉邏輯 ---
    // 原始碼: if ( a6 >= 2u ) { if ( *((_DWORD *)a2 + 143) == 1 ) *((_BYTE *)this + 56) = 1; }
    // else { *((_BYTE *)this + 56) = a6 != 0; }
    if (ucDirectionFlag >= 2) {
        // 如果旗標 >= 2，則翻轉與否取決於施法者的朝向
        if (pCaster && pCaster->GetActionSide() == 1) { // 假設 GetActionSide() 獲取朝向
            m_bIsFlip = true;
        }
    }
    else {
        // 否則，直接根據旗標決定是否翻轉
        m_bIsFlip = (ucDirectionFlag != 0);
    }
}

// 對應反組譯碼: (在 CEffectManager::FrameProcess 中被呼叫)
bool CEffect_Skill_Type_Directed::FrameProcess(float fElapsedTime)
{
    // 與 CEffect_Skill_Type_Once 相同，生命週期完全由內部動畫決定
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: (在 CEffectManager::Process 中被呼叫)
void CEffect_Skill_Type_Directed::Process()
{
    // 檢查目標是否存在
    if (!m_pTargetCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // 從目標獲取當前位置
    float targetX = static_cast<float>(m_pTargetCharacter->GetPosX());
    float targetY = static_cast<float>(m_pTargetCharacter->GetPosY());

    // 設定特效自身的繪製座標
    m_fCurrentPosX = targetX;
    m_fCurrentPosY = targetY;

    // 進行裁剪判斷
    float screenX = m_fCurrentPosX - static_cast<float>(g_Game_System_Info.ScreenWidth);
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 將世界座標轉換為螢幕座標後傳遞給 CCAEffect
        m_ccaEffect.SetPosition(screenX, m_fCurrentPosY - static_cast<float>(g_Game_System_Info.ScreenHeight));
        m_ccaEffect.SetFlipX(m_bIsFlip); // 將翻轉狀態傳遞給 CCAEffect
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: (在 CEffectManager::Draw 中被呼叫)
void CEffect_Skill_Type_Directed::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}