#include "Effect/CEffect_Skill_Type_Directed_Target.h"
#include "Effect/CEAManager.h"
#include "Character/ClientCharacter.h"
#include "global.h"

// 對應反組譯碼: 0x00539550
CEffect_Skill_Type_Directed_Target::CEffect_Skill_Type_Directed_Target()
    : m_pCasterCharacter(nullptr), m_pTargetCharacter(nullptr)
{
    // CEffectBase 的建構函式會被自動呼叫
}

// 對應反組譯碼: 0x00539590
CEffect_Skill_Type_Directed_Target::~CEffect_Skill_Type_Directed_Target()
{
}

// 對應反組譯碼: 0x005395A0
void CEffect_Skill_Type_Directed_Target::SetEffect(ClientCharacter* pCaster, ClientCharacter* pTarget, unsigned short effectKindID, char* szFileName)
{
    if (!pCaster || !pTarget) return;

    // 構建檔案路徑並載入特效數據
    char szFullPath[256];
    sprintf_s(szFullPath, sizeof(szFullPath), "Effect/%s", szFileName);
    CEAManager::GetInstance()->GetEAData(effectKindID, szFullPath, &m_ccaEffect);

    // 設定影格時間並播放動畫
    m_ccaEffect.SetFrameTime();
    m_ccaEffect.Play(0, false); // 播放第一個動畫序列，不循環

    // 綁定施法者和目標
    m_pCasterCharacter = pCaster;
    m_pTargetCharacter = pTarget;

    // 根據施法者的朝向設定翻轉
    // 原始碼: if ( *((_DWORD *)a2 + 143) == 1 ) *((_BYTE *)this + 56) = 1;
    if (pCaster->GetActionSide() == 1) {
        m_bIsFlip = true;
    }
}

// 對應反組譯碼: 0x00539630
bool CEffect_Skill_Type_Directed_Target::FrameProcess(float fElapsedTime)
{
    // 將生命週期管理的任務完全委託給內部的 CCAEffect 物件。
    return m_ccaEffect.FrameProcess(fElapsedTime);
}

// 對應反組譯碼: 0x00539640
void CEffect_Skill_Type_Directed_Target::Process()
{
    if (!m_pCasterCharacter || !m_pTargetCharacter) {
        m_bIsVisible = FALSE;
        return;
    }

    // --- 核心邏輯：持續追蹤目標位置 ---
    // 將世界座標轉換為螢幕座標
    float screenX = static_cast<float>(m_pTargetCharacter->GetPosX() - g_Game_System_Info.ScreenX);
    float screenY = static_cast<float>(m_pTargetCharacter->GetPosY() - g_Game_System_Info.ScreenY);

    // 進行裁剪判斷
    m_bIsVisible = IsCliping(screenX, 0.0f);

    if (m_bIsVisible) {
        // 更新內部 CCAEffect 的狀態
        m_ccaEffect.SetPosition(screenX, screenY);
        m_ccaEffect.SetFlipX(m_bIsFlip);
        m_ccaEffect.Process();
    }
}

// 對應反組譯碼: 0x005396D0
void CEffect_Skill_Type_Directed_Target::Draw()
{
    if (m_bIsVisible) {
        m_ccaEffect.Draw();
    }
}